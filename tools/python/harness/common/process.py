"""Own external process trees across normal exit, timeout, and owner death."""

from __future__ import annotations

import json
import os
import selectors
import signal
import subprocess
import sys
import threading
import time
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import IO, Any


def _kill_group(pid: int, sig: signal.Signals = signal.SIGKILL) -> None:
    try:
        os.killpg(pid, sig)
    except ProcessLookupError:
        pass


def _supervise(owner_fd: int, argv: Sequence[str]) -> int:
    """Own one child group; owner EOF and direct-child exit clean the tree."""

    child = subprocess.Popen(argv, start_new_session=True)
    owner_gone = threading.Event()

    def watch_owner() -> None:
        try:
            while os.read(owner_fd, 1):
                pass
        finally:
            os.close(owner_fd)
            owner_gone.set()
            _kill_group(child.pid)

    watcher = threading.Thread(target=watch_owner, daemon=True)
    watcher.start()
    result = child.wait()
    _kill_group(child.pid, signal.SIGTERM)
    try:
        os.killpg(child.pid, 0)
    except ProcessLookupError:
        pass
    else:
        # Descendants retaining inherited descriptors get one short graceful
        # interval before the complete group is forcibly removed.
        owner_gone.wait(0.1)
        _kill_group(child.pid)
    return result


class OwnedProcess:
    """Popen-compatible handle whose owner pipe governs a supervised tree."""

    def __init__(self, process: subprocess.Popen[Any], owner_fd: int | None) -> None:
        self._process = process
        self._owner_fd = owner_fd

    def __getattr__(self, name: str) -> Any:
        return getattr(self._process, name)

    @property
    def returncode(self) -> int | None:
        return self._process.returncode

    def _release_owner(self) -> None:
        if self._owner_fd is not None:
            os.close(self._owner_fd)
            self._owner_fd = None

    def terminate_tree(self, timeout: float = 5) -> None:
        """Close ownership, then reap the supervisor after complete-tree cleanup."""

        self._release_owner()
        try:
            self._process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            if sys.platform.startswith("linux"):
                self._process.kill()
            else:
                _kill_group(self._process.pid)
            self._process.wait(timeout=timeout)

    def communicate(self, *args: Any, **kwargs: Any) -> tuple[Any, Any]:
        try:
            return self._process.communicate(*args, **kwargs)
        finally:
            if self._process.poll() is not None:
                self._release_owner()

    def wait(self, *args: Any, **kwargs: Any) -> int:
        result = self._process.wait(*args, **kwargs)
        self._release_owner()
        return result


def owned_popen(
    argv: Sequence[str | Path],
    *,
    cwd: str | Path | None = None,
    env: Mapping[str, str] | None = None,
    stdin: int | IO[Any] | None = None,
    stdout: int | IO[Any] | None = None,
    stderr: int | IO[Any] | None = None,
    text: bool = False,
    bufsize: int = -1,
) -> OwnedProcess:
    """Start one external tree with a dedicated group and owner-death pipe."""

    command = [os.fspath(item) for item in argv]
    if sys.platform.startswith("linux"):
        owner_read, owner_write = os.pipe()
        launcher = [
            sys.executable,
            "-m",
            "harness.common.process",
            str(owner_read),
            json.dumps(command),
        ]
        try:
            process = subprocess.Popen(
                launcher,
                cwd=cwd,
                env=env,
                stdin=stdin,
                stdout=stdout,
                stderr=stderr,
                text=text,
                bufsize=bufsize,
                pass_fds=(owner_read,),
                start_new_session=True,
            )
        except BaseException:
            os.close(owner_write)
            raise
        finally:
            os.close(owner_read)
        return OwnedProcess(process, owner_write)
    process = subprocess.Popen(
        command,
        cwd=cwd,
        env=env,
        stdin=stdin,
        stdout=stdout,
        stderr=stderr,
        text=text,
        bufsize=bufsize,
        start_new_session=True,
    )
    return OwnedProcess(process, None)


def run_bounded(
    root: Path,
    argv: Sequence[str | Path],
    *,
    timeout: float,
    output_limit: int,
    errors: str = "replace",
) -> dict[str, Any]:
    """Bound captured output and runtime while owning the entire child group."""
    if timeout <= 0 or output_limit < 1:
        raise ValueError("command timeout and output limit must be positive")
    process = owned_popen(
        argv, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    streams = {"stdout": bytearray(), "stderr": bytearray()}
    failure = None
    deadline = time.monotonic() + timeout
    try:
        with selectors.DefaultSelector() as poll:
            for name in streams:
                poll.register(getattr(process, name), selectors.EVENT_READ, name)
            while poll.get_map():
                if time.monotonic() >= deadline:
                    failure = "timeout"
                    break
                for key, _ in poll.select(
                    min(0.2, max(0, deadline - time.monotonic()))
                ):
                    chunk = os.read(key.fd, 65536)
                    if not chunk:
                        poll.unregister(key.fileobj)
                    else:
                        remaining = output_limit - sum(map(len, streams.values()))
                        streams[key.data].extend(chunk[:remaining])
                        if len(chunk) > remaining:
                            failure = "output limit"
                            break
                if failure:
                    break
        if failure:
            process.terminate_tree(timeout=2)
        else:
            process.wait(timeout=max(0.01, deadline - time.monotonic()))
    except subprocess.TimeoutExpired:
        failure = "timeout"
        process.terminate_tree(timeout=2)
    except BaseException:
        process.terminate_tree(timeout=2)
        raise
    finally:
        process.stdout.close()
        process.stderr.close()
    return {
        "argv": argv,
        "exit_code": process.returncode,
        "failure": failure,
        **{
            name: bytes(value).decode("utf-8", errors="replace" if failure else errors)
            for name, value in streams.items()
        },
    }


def run_command(
    argv: Sequence[str | Path],
    *,
    cwd: str | Path,
    text: bool = True,
    capture_output: bool = True,
) -> subprocess.CompletedProcess[str]:
    """Run one native transaction gate within 120 seconds and 2 MiB of output."""
    if text is not True or capture_output is not True:
        raise ValueError("transaction commands require captured text output")
    result = run_bounded(
        Path(cwd), argv, timeout=120, output_limit=2 * 1024 * 1024, errors="strict"
    )
    failure = result["failure"]
    return subprocess.CompletedProcess(
        argv,
        (124 if failure == "timeout" else 125) if failure else result["exit_code"],
        result["stdout"].replace("\r\n", "\n").replace("\r", "\n"),
        result["stderr"].replace("\r\n", "\n").replace("\r", "\n")
        + (f"\ncommand aborted: {failure}\n" if failure else ""),
    )


def run_analyzer(
    argv: Sequence[str], *, timeout: int
) -> subprocess.CompletedProcess[str]:
    """Run an analyzer in an owned process tree and reap it on interruption."""

    process = owned_popen(
        argv,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    try:
        stdout, stderr = process.communicate(timeout=timeout)
    except BaseException:
        process.terminate_tree()
        raise
    return subprocess.CompletedProcess(argv, process.returncode, stdout, stderr)


def _main() -> int:
    return _supervise(int(sys.argv[1]), json.loads(sys.argv[2]))


if __name__ == "__main__":
    raise SystemExit(_main())
