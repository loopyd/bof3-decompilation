"""Own external process trees across normal exit, timeout, and owner death."""

from __future__ import annotations

import json
import math
import os
import selectors
import subprocess
import sys
import time
from collections.abc import Callable, Mapping, Sequence
from pathlib import Path
from typing import IO, Any

from harness.common.children import adopt_children, reap_children
from harness.common.deadlines import resolve_deadline, validate_deadline


class ProcessCleanupError(RuntimeError):
    """Descendant termination is unconfirmed; mutation recovery must stop."""


def _supervise(owner_fd: int, argv: Sequence[str], deadline: float | None) -> int:
    """Reap owned descendants after direct exit or loss of the owner pipe."""

    adopt_children()
    validate_deadline(deadline)
    with selectors.DefaultSelector() as poll:
        poll.register(owner_fd, selectors.EVENT_READ)
        if deadline is not None and time.monotonic() >= deadline:
            os.close(owner_fd)
            return 124
        child = subprocess.Popen(argv, start_new_session=True)
        try:
            while child.poll() is None:
                remaining = (
                    deadline - time.monotonic() if deadline is not None else 0.05
                )
                if remaining <= 0:
                    break
                if poll.select(min(0.05, remaining)) and not os.read(owner_fd, 1):
                    break
        finally:
            os.close(owner_fd)
            reap_children(child)
    return child.returncode


class OwnedProcess:
    """Popen-compatible handle whose owner pipe governs a supervised tree."""

    def __init__(
        self,
        process: subprocess.Popen[Any],
        owner_fd: int | None,
        completion_fd: int,
    ) -> None:
        self._process = process
        self._owner_fd = owner_fd
        self._completion_fd = completion_fd
        self._cleanup_confirmed = False

    def __getattr__(self, name: str) -> Any:
        return getattr(self._process, name)

    @property
    def returncode(self) -> int | None:
        if self._process.returncode is not None:
            self._confirm_cleanup()
        return self._process.returncode

    def poll(self) -> int | None:
        result = self._process.poll()
        if result is not None:
            self._release_owner()
            self._confirm_cleanup()
        return result

    def _confirm_cleanup(self) -> None:
        if self._completion_fd is not None:
            descriptor, self._completion_fd = self._completion_fd, None
            try:
                self._cleanup_confirmed = os.read(descriptor, 32) == b"complete\n"
            except BlockingIOError:
                self._cleanup_confirmed = False
            finally:
                os.close(descriptor)
        if not self._cleanup_confirmed:
            raise ProcessCleanupError(
                "process supervisor exited without confirmed descendant cleanup"
            )

    def _release_owner(self) -> None:
        if self._owner_fd is not None:
            os.close(self._owner_fd)
            self._owner_fd = None

    def terminate_tree(self, timeout: float = 5) -> None:
        """Close ownership, then reap the supervisor after complete-tree cleanup."""

        self._release_owner()
        try:
            self._process.wait(timeout=timeout)
        except subprocess.TimeoutExpired as error:
            raise ProcessCleanupError(
                "descendant cleanup remains active; preserve state for parent recovery"
            ) from error
        self._confirm_cleanup()

    def communicate(self, *args: Any, **kwargs: Any) -> tuple[Any, Any]:
        try:
            return self._process.communicate(*args, **kwargs)
        finally:
            if self._process.poll() is not None:
                self._release_owner()
                self._confirm_cleanup()

    def wait(self, *args: Any, **kwargs: Any) -> int:
        result = self._process.wait(*args, **kwargs)
        self._release_owner()
        self._confirm_cleanup()
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
    deadline: float | None = None,
) -> OwnedProcess:
    """Start one Linux-owned tree with subreaper and cleanup acknowledgement."""

    if not sys.platform.startswith("linux"):
        raise RuntimeError("native process ownership requires Linux subreaper support")
    validate_deadline(deadline)
    if deadline is not None and time.monotonic() >= deadline:
        raise subprocess.TimeoutExpired(argv, 0)
    command = json.dumps([os.fspath(item) for item in argv])
    owner_read, owner_write = os.pipe()
    try:
        completion_read, completion_write = os.pipe()
    except BaseException:
        os.close(owner_read)
        os.close(owner_write)
        raise
    launcher = [
        sys.executable,
        "-m",
        "harness.common.process",
        str(owner_read),
        str(completion_write),
        command,
        json.dumps(deadline),
    ]
    try:
        os.set_blocking(completion_read, False)
        if deadline is not None and time.monotonic() >= deadline:
            raise subprocess.TimeoutExpired(argv, 0)
        process = subprocess.Popen(
            launcher,
            cwd=cwd,
            env=env,
            stdin=stdin,
            stdout=stdout,
            stderr=stderr,
            text=text,
            bufsize=bufsize,
            pass_fds=(owner_read, completion_write),
            start_new_session=True,
        )
    except BaseException:
        os.close(owner_write)
        os.close(completion_read)
        raise
    finally:
        os.close(owner_read)
        os.close(completion_write)
    return OwnedProcess(process, owner_write, completion_read)


def run_bounded(
    root: Path,
    argv: Sequence[str | Path],
    *,
    timeout: float,
    output_limit: int,
    errors: str = "replace",
    deadline: float | None = None,
    input_data: bytes | None = None,
    on_output: Callable[[str, bytes], None] | None = None,
    on_spawn: Callable[[OwnedProcess], None] | None = None,
    env: Mapping[str, str] | None = None,
) -> dict[str, Any]:
    """Bound owned work by pre-spawn timeout and an optional monotonic deadline.

    Optional input bytes are written concurrently with output reads. Trusted
    callbacks observe the spawned supervisor and retained output chunks; their
    exceptions require cleanup and retain identity unless cleanup is uncertain.
    """
    if (
        type(timeout) not in {int, float}
        or not math.isfinite(timeout)
        or timeout <= 0
        or type(output_limit) is not int
        or output_limit < 1
    ):
        raise ValueError("command timeout and output limit must be positive")
    if (
        (input_data is not None and type(input_data) is not bytes)
        or (on_output is not None and not callable(on_output))
        or (on_spawn is not None and not callable(on_spawn))
    ):
        raise ValueError("invalid command input bytes or stream callbacks")
    validate_deadline(deadline)
    started = time.monotonic()
    deadline = (
        min(started + timeout, deadline) if deadline is not None else started + timeout
    )
    try:
        process = owned_popen(
            argv,
            cwd=root,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            deadline=deadline,
            **({"env": env} if env is not None else {}),
            **({"stdin": subprocess.PIPE} if input_data is not None else {}),
        )
    except subprocess.TimeoutExpired:
        return {
            "argv": argv,
            "exit_code": 124,
            "failure": "timeout",
            "stdout": "",
            "stderr": "work deadline expired before launch\n",
        }
    streams = {"stdout": bytearray(), "stderr": bytearray()}
    failure = None
    callback_failed = False

    def invoke_callback(callback: Callable[..., None], *arguments: Any) -> None:
        nonlocal callback_failed
        try:
            callback(*arguments)
        except BaseException:
            callback_failed = True
            raise

    try:
        if on_spawn is not None:
            invoke_callback(on_spawn, process)
        with selectors.DefaultSelector() as poll:
            for name in streams:
                poll.register(getattr(process, name), selectors.EVENT_READ, name)
            offset = 0
            if input_data:
                os.set_blocking(process.stdin.fileno(), False)
                poll.register(process.stdin, selectors.EVENT_WRITE, "stdin")
            elif input_data is not None:
                process.stdin.close()
            while poll.get_map():
                if time.monotonic() >= deadline:
                    failure = "timeout"
                    break
                for key, _ in poll.select(
                    min(0.2, max(0, deadline - time.monotonic()))
                ):
                    if key.data == "stdin":
                        try:
                            offset += os.write(
                                key.fd, input_data[offset : offset + 65536]
                            )
                        except BlockingIOError:
                            continue
                        except BrokenPipeError:
                            failure = "input closed"
                            break
                        if offset == len(input_data):
                            poll.unregister(key.fileobj)
                            process.stdin.close()
                        continue
                    chunk = os.read(key.fd, 65536)
                    if not chunk:
                        poll.unregister(key.fileobj)
                    else:
                        remaining = output_limit - sum(map(len, streams.values()))
                        streams[key.data].extend(chunk[:remaining])
                        if on_output is not None and remaining:
                            invoke_callback(on_output, key.data, chunk[:remaining])
                        if len(chunk) > remaining:
                            failure = "output limit"
                            break
                if failure:
                    break
        if failure is None and time.monotonic() >= deadline:
            failure = "timeout"
        if failure:
            process.terminate_tree(timeout=2)
        else:
            process.wait(timeout=max(0.01, deadline - time.monotonic()))
            if time.monotonic() >= deadline:
                failure = "timeout"
    except subprocess.TimeoutExpired:
        if callback_failed:
            process.terminate_tree(timeout=2)
            raise
        failure = "timeout"
        process.terminate_tree(timeout=2)
    except ProcessCleanupError:
        if callback_failed:
            process.terminate_tree(timeout=2)
        raise
    except BaseException:
        process.terminate_tree(timeout=2)
        raise
    finally:
        if input_data is not None:
            process.stdin.close()
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
    deadline: float | None = None,
) -> subprocess.CompletedProcess[str]:
    """Run one native transaction gate within 120 seconds and 2 MiB of output."""
    if text is not True or capture_output is not True:
        raise ValueError("transaction commands require captured text output")
    deadline = resolve_deadline(deadline)
    options = {"deadline": deadline} if deadline is not None else {}
    result = run_bounded(
        Path(cwd),
        argv,
        timeout=120,
        output_limit=2 * 1024 * 1024,
        errors="strict",
        **options,
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
    completion = int(sys.argv[2])
    try:
        result = _supervise(
            int(sys.argv[1]), json.loads(sys.argv[3]), json.loads(sys.argv[4])
        )
        os.write(completion, b"complete\n")
        return result
    finally:
        os.close(completion)


if __name__ == "__main__":
    raise SystemExit(_main())
