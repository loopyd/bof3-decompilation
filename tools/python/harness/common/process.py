"""Own external process trees across normal exit, interruption, and owner death."""

from __future__ import annotations

import json
import os
import selectors
import subprocess
import sys
import tempfile
import time
from collections.abc import Callable, Mapping, Sequence
from pathlib import Path
from typing import IO, Any

from harness.common.children import adopt_children, open_child_exit, reap_children
from harness.common.digests import file_record


class ProcessCleanupError(RuntimeError):
    """Descendant termination is unconfirmed; mutation recovery must stop.

    ``observed`` retains the bounded execution details one supervised run actually
    produced before cleanup could not be confirmed: the launched argv, the
    retained stdout/stderr, the launched process id and the elapsed seconds.  It
    is ``None`` when no bounded launch was observed, and it never carries an exit
    code or a completed record for unconfirmed cleanup.
    """

    observed: dict[str, Any] | None = None
    capture_failures: tuple[tuple[str, str, str, str], ...] = ()


def _close_inherited(fds: Sequence[int]) -> None:
    for descriptor in fds:
        try:
            os.close(descriptor)
        except OSError:
            pass


def _supervise(
    owner_fd: int,
    argv: Sequence[str],
    pass_fds: tuple[int, ...] = (),
    *,
    env: Mapping[str, str],
) -> int:
    adopt_children()
    with selectors.DefaultSelector() as poll:
        poll.register(owner_fd, selectors.EVENT_READ)
        try:
            child = subprocess.Popen(
                argv, env=env, start_new_session=True, pass_fds=tuple(pass_fds)
            )
        except BaseException:
            _close_inherited(pass_fds)
            raise
        _close_inherited(pass_fds)
        child_exit = None
        try:
            child_exit = open_child_exit(child.pid)
            if child_exit is not None:
                poll.register(child_exit, selectors.EVENT_READ)
            while child.poll() is None:
                events = poll.select(0.05)
                if any(key.fd == owner_fd for key, _ in events) and not os.read(
                    owner_fd, 1
                ):
                    break
        finally:
            try:
                os.close(owner_fd)
            finally:
                try:
                    reap_children(child)
                finally:
                    if child_exit is not None:
                        os.close(child_exit)
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
            self._complete_exit()
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

    def _complete_exit(self) -> None:
        self._release_owner()
        self._confirm_cleanup()

    def _release_owner(self) -> None:
        if self._owner_fd is not None:
            os.close(self._owner_fd)
            self._owner_fd = None

    def terminate_tree(self, timeout: float = 5) -> None:
        """Close ownership, then reap the supervisor after complete-tree cleanup."""
        self._release_owner()
        self._wait_cleanup(timeout)

    def _wait_cleanup(self, timeout: float) -> None:
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
                self._complete_exit()

    def wait(self, *args: Any, **kwargs: Any) -> int:
        result = self._process.wait(*args, **kwargs)
        self._complete_exit()
        return result


def validate_pass_fds(pass_fds: Sequence[int]) -> tuple[int, ...]:
    """Validate caller-owned descriptors that a supervised child must inherit.

    Only explicit descriptors at or above 3 are accepted, so stdio and the
    supervisor's own control descriptors can never be forwarded by accident.
    Each must be a unique, currently open integer descriptor; validation runs
    before any internal pipe is opened, so an internal descriptor can never be
    named by a caller.  The caller retains ownership of the returned
    descriptors; only the supervisor's inherited copies are closed.
    """

    validated: list[int] = []
    seen: set[int] = set()
    for descriptor in pass_fds:
        if type(descriptor) is not int or descriptor < 3:
            raise ValueError("pass_fds must be integer descriptors at or above 3")
        if descriptor in seen:
            raise ValueError("pass_fds descriptors must be unique")
        seen.add(descriptor)
        try:
            os.fstat(descriptor)
        except OSError as error:
            raise ValueError(
                f"pass_fds descriptor is not open: {descriptor}"
            ) from error
        validated.append(descriptor)
    return tuple(validated)


#: Reviewed startup-isolating supervisor flag: ``-S`` disables ``site`` and
#: therefore user-site and ``sitecustomize``/``usercustomize`` processing for the
#: owning supervisor process.
ISOLATED_SUPERVISOR_FLAGS = ("-S",)
_SUPERVISOR_ENTRY = (
    "-c",
    "import sys\n"
    "trusted_root = sys.argv.pop(1)\n"
    "sys.path.insert(0, trusted_root)\n"
    "import runpy\n"
    "runpy.run_module('harness.common.process', run_name='__main__', alter_sys=True)\n",
    str(Path(__file__).resolve().parents[2]),
)


def resolve_supervisor() -> tuple[str, ...]:
    """Describe the Python launcher actually used by owned process execution."""
    return (sys.executable, *_SUPERVISOR_ENTRY)


def resolve_isolated_supervisor() -> tuple[str, ...]:
    """The same launcher with Python startup customization disabled."""
    return (sys.executable, *ISOLATED_SUPERVISOR_FLAGS, *_SUPERVISOR_ENTRY)


def supervisor_variants() -> frozenset[tuple[str, ...]]:
    """Every reviewed supervisor launch policy owned process execution accepts."""
    return frozenset({resolve_supervisor(), resolve_isolated_supervisor()})


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
    supervisor: tuple[str, ...] | None = None,
    pass_fds: Sequence[int] = (),
) -> OwnedProcess:
    """Start one Linux-owned tree with subreaper and cleanup acknowledgement.

    ``pass_fds`` names caller-owned descriptors that only the supervised child
    must inherit; they are validated before any internal pipe is opened.
    """

    if not sys.platform.startswith("linux"):
        raise RuntimeError("native process ownership requires Linux subreaper support")
    extra_fds = validate_pass_fds(pass_fds)
    if supervisor is not None and tuple(supervisor) not in supervisor_variants():
        raise ValueError(
            "owned process supervisor differs from its reviewed launch policies"
        )
    target_environment = dict(os.environ) if env is None else dict(env)
    if any(
        not isinstance(key, str) or not isinstance(value, str)
        for key, value in target_environment.items()
    ):
        raise TypeError("process environment keys and values must be strings")
    environment_bytes = json.dumps(target_environment).encode("utf-8")
    command = json.dumps([os.fspath(item) for item in argv])
    selected = resolve_supervisor() if supervisor is None else tuple(supervisor)
    context = tempfile.TemporaryFile(mode="w+b")
    try:
        context.write(environment_bytes)
        context.flush()
        context.seek(0)
        owner_read, owner_write = os.pipe()
        try:
            completion_read, completion_write = os.pipe()
        except BaseException:
            os.close(owner_read)
            os.close(owner_write)
            raise
        launcher = [
            *selected,
            str(owner_read),
            str(completion_write),
            command,
            json.dumps(list(extra_fds)),
            str(context.fileno()),
        ]
        forwarded = (owner_read, completion_write, context.fileno(), *extra_fds)
        if len(set(forwarded)) != len(forwarded):
            os.close(owner_read)
            os.close(owner_write)
            os.close(completion_read)
            os.close(completion_write)
            raise ValueError("pass_fds overlaps an internal control descriptor")
        try:
            os.set_blocking(completion_read, False)
            process = subprocess.Popen(
                launcher,
                cwd=cwd,
                env=env,
                stdin=stdin,
                stdout=stdout,
                stderr=stderr,
                text=text,
                bufsize=bufsize,
                pass_fds=forwarded,
                start_new_session=True,
            )
        except BaseException:
            os.close(owner_write)
            os.close(completion_read)
            raise
        finally:
            os.close(owner_read)
            os.close(completion_write)
        owned = OwnedProcess(process, owner_write, completion_read)
    except BaseException:
        context.close()
        raise
    try:
        context.close()
    except BaseException:
        owned.terminate_tree()
        raise
    return owned


def run_bounded(
    root: Path,
    argv: Sequence[str | Path],
    *,
    output_limit: int,
    errors: str = "replace",
    input_data: bytes | None = None,
    on_output: Callable[[str, bytes], None] | None = None,
    on_spawn: Callable[[OwnedProcess], None] | None = None,
    env: Mapping[str, str] | None = None,
    supervisor: tuple[str, ...] | None = None,
    pass_fds: Sequence[int] = (),
) -> dict[str, Any]:
    """Run owned work with bounded output.

    Optional input bytes are written concurrently with output reads. Trusted
    callbacks observe the spawned supervisor and retained output chunks; their
    exceptions require cleanup and retain identity unless cleanup is uncertain.
    ``pass_fds`` forwards caller-owned descriptors to the child only.
    ``raw_exit`` is the owned supervisor's process return code after cleanup;
    it is ``None`` when no supervisor was launched.
    """
    if type(output_limit) is not int or output_limit < 1:
        raise ValueError("command output limit must be positive")
    if (
        (input_data is not None and type(input_data) is not bytes)
        or (on_output is not None and not callable(on_output))
        or (on_spawn is not None and not callable(on_spawn))
    ):
        raise ValueError("invalid command input bytes or stream callbacks")
    started = time.monotonic()
    process = owned_popen(
        argv,
        cwd=root,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        pass_fds=pass_fds,
        **({"supervisor": supervisor} if supervisor is not None else {}),
        **({"env": env} if env is not None else {}),
        **({"stdin": subprocess.PIPE} if input_data is not None else {}),
    )
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

    def retain_observed(error: ProcessCleanupError) -> None:
        if error.observed is None:
            error.observed = {
                "argv": [str(item) for item in argv],
                "stdout": bytes(streams["stdout"]).decode("utf-8", "replace"),
                "stderr": bytes(streams["stderr"]).decode("utf-8", "replace"),
                "failure": failure,
                "pid": process.pid,
                "seconds": round(time.monotonic() - started, 3),
            }

    def terminate_owned() -> None:
        try:
            process.terminate_tree(timeout=2)
        except ProcessCleanupError as error:
            retain_observed(error)
            raise

    def wait_owned() -> None:
        try:
            process.wait()
        except ProcessCleanupError as error:
            retain_observed(error)
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
                for key, _ in poll.select(0.2):
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
        if failure:
            terminate_owned()
        else:
            wait_owned()
    except ProcessCleanupError:
        if callback_failed:
            terminate_owned()
        raise
    except BaseException:
        terminate_owned()
        raise
    finally:
        if input_data is not None:
            process.stdin.close()
        process.stdout.close()
        process.stderr.close()
    return {
        "argv": argv,
        "exit_code": process.returncode,
        "raw_exit": process.returncode,
        "failure": failure,
        **{
            name: bytes(value).decode("utf-8", errors="replace" if failure else errors)
            for name, value in streams.items()
        },
    }


def capture(
    cwd: Path,
    argv: Sequence[str | Path],
    *,
    environment: Mapping[str, str],
    output_limit: int,
    name: str,
    input_data: bytes | None = None,
) -> dict:
    paths = {channel: cwd / f"{name}.{channel}.bin" for channel in ("stdout", "stderr")}
    failure = None
    result = {}
    record = {}
    streams = {}
    errors = []
    for channel, path in paths.items():
        try:
            streams[channel] = path.open("xb")
        except Exception as error:
            errors.append(("open", str(path), error))
            break
    if len(streams) == 2:

        def retain(channel, chunk):
            if streams[channel].write(chunk) != len(chunk):
                raise ValueError("raw capture write failed")

        try:
            result = run_bounded(
                cwd,
                argv,
                output_limit=output_limit,
                env=environment,
                supervisor=resolve_isolated_supervisor(),
                input_data=input_data,
                on_output=retain,
            )
        except Exception as error:
            failure = error
    for channel, stream in streams.items():
        try:
            stream.close()
        except Exception as error:
            errors.append(("close", str(paths[channel]), error))
    identities = {}
    for channel in streams:
        try:
            identities[channel] = file_record(paths[channel])
        except Exception as error:
            errors.append(("hash", str(paths[channel]), error))
    if len(identities) == 2:
        retained_failure = failure or (errors[0][2] if errors else None)
        record = {
            "argv": argv,
            "cwd": str(cwd),
            "environment": environment,
            "exit_code": result.get("exit_code"),
            "raw_exit": result.get("raw_exit"),
            "raw_exit_owner": "common.process owned supervisor",
            "failure": str(retained_failure)
            if retained_failure
            else result.get("failure"),
            "owner_observation": getattr(failure, "observed", None),
            "capture_complete": failure is None
            and not errors
            and result.get("failure") is None,
            "streams": identities,
        }
        record_path = cwd / f"{name}.json"
        phase = "serialize"
        stream = None
        try:
            data = (
                json.dumps(record, sort_keys=True, indent=2, allow_nan=False).encode()
                + b"\n"
            )
            phase = "open"
            stream = record_path.open("xb")
            phase = "write"
            if stream.write(data) != len(data):
                raise ValueError("file write was incomplete")
        except Exception as error:
            errors.append((phase, str(record_path), error))
        if stream is not None:
            try:
                stream.close()
            except Exception as error:
                errors.append(("close", str(record_path), error))
            try:
                record_path.chmod(0o600)
            except Exception as error:
                errors.append(("chmod", str(record_path), error))
    primary = (
        failure
        if isinstance(failure, ProcessCleanupError)
        else errors[0][2]
        if errors
        else None
    )
    if primary is not None:
        facts = tuple(
            (phase, path, type(error).__name__, str(error))
            for phase, path, error in errors
        )
        setattr(
            primary,
            "capture_failures",
            (*getattr(primary, "capture_failures", ()), *facts),
        )
        raise primary
    return record


def run_command(
    argv: Sequence[str | Path],
    *,
    cwd: str | Path,
    text: bool = True,
    capture_output: bool = True,
) -> subprocess.CompletedProcess[str]:
    """Run one owned native transaction gate with up to 2 MiB of output."""
    if text is not True or capture_output is not True:
        raise ValueError("transaction commands require captured text output")
    result = run_bounded(
        Path(cwd),
        argv,
        output_limit=2 * 1024 * 1024,
        errors="strict",
    )
    failure = result["failure"]
    return subprocess.CompletedProcess(
        argv,
        125 if failure else result["exit_code"],
        result["stdout"].replace("\r\n", "\n").replace("\r", "\n"),
        result["stderr"].replace("\r\n", "\n").replace("\r", "\n")
        + (f"\ncommand aborted: {failure}\n" if failure else ""),
    )


def run_analyzer(argv: Sequence[str]) -> subprocess.CompletedProcess[str]:
    """Run an analyzer in an owned process tree and reap it on interruption."""

    process = owned_popen(
        argv,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    try:
        stdout, stderr = process.communicate()
    except BaseException:
        process.terminate_tree()
        raise
    return subprocess.CompletedProcess(argv, process.returncode, stdout, stderr)


def _main() -> int:
    completion = int(sys.argv[2])
    try:
        forwarded = tuple(json.loads(sys.argv[4]))
        with os.fdopen(int(sys.argv[5]), "rb") as context:
            environment = json.load(context)
        result = _supervise(
            int(sys.argv[1]),
            json.loads(sys.argv[3]),
            forwarded,
            env=environment,
        )
        os.write(completion, b"complete\n")
        return result
    finally:
        os.close(completion)


if __name__ == "__main__":
    return_code = _main()
    for stream in (sys.stdout, sys.stderr):
        if stream is not None:
            stream.flush()
    os._exit(return_code)
