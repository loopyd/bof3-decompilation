"""One serial, reusable Rizin session plus bounded native byte operations.

The P0 runner reuses a single Rizin process for every semantic
(original-byte) command of one run: the target binary is opened once in
read-only mode (``-N``, no project write-back) with its verified replay
recipe, commands are queued in order and executed serially, and every
command returns bounded output plus an honest exit status.  Sentinel
markers around each command delimit its output in the single interleaved
stdout stream.  A hung command is terminated and reaped within its
deadline; the session never outlives the run.
"""

from __future__ import annotations

import os
import secrets
import select
import subprocess
import threading
import time
from pathlib import Path
from typing import Any

from harness.analysis.engine import EngineIdentity, find_engine
from harness.analysis.project import prepare_target, rizin_argv
from harness.common.process import OwnedProcess, owned_popen
from harness.common.deadlines import resolve_deadline
from harness.domain.manifests import load_target_manifests
from harness.naming import native as evidence_native

DEFAULT_COMMAND_TIMEOUT = 120


def _complete_lines(text: str) -> list[str]:
    """Return LF/CRLF-terminated lines, retaining an incomplete final fragment."""

    return [line for line in text.splitlines(keepends=True) if line.endswith("\n")]


class RizinSession:
    """One reusable Rizin target session; every command stays serial."""

    def __init__(
        self,
        root: Path,
        target: str,
        *,
        command_timeout: int = DEFAULT_COMMAND_TIMEOUT,
        executable: str | Path | None = None,
        work_deadline: float | None = None,
    ) -> None:
        self.root = root
        self.target = target
        self.command_timeout = command_timeout
        self.executable = executable
        self.work_deadline = resolve_deadline(work_deadline)
        self.process: OwnedProcess | None = None
        self.commands_executed = 0
        self.commands_killed = 0
        self.pending: list[tuple[str, str]] = []
        self.results: list[evidence_native.SemanticResult] = []
        self._lock = threading.RLock()
        self._stderr = bytearray()
        self._stderr_lock = threading.Lock()
        self._stderr_changed = threading.Condition(self._stderr_lock)
        self._stderr_thread: threading.Thread | None = None

    @property
    def process_count(self) -> int:
        """Live Rizin processes spawned by this session (at most one)."""

        return 1 if self.process is not None else 0

    def open(self) -> None:
        """Start the single Rizin process for the run; idempotent."""

        with self._lock:
            if self.process is not None:
                return
            manifests = load_target_manifests(self.root)
            if self.target not in manifests:
                raise ValueError(f"unknown target: {self.target}")
            spec = prepare_target(
                self.root, self.target, manifest=manifests[self.target]
            )
            engine = (
                find_engine("rizin", root=self.root)
                if self.executable is None
                else _StubEngine(Path(self.executable))
            )
            argv = rizin_argv(spec, engine)
            argv[1:1] = ["-e", "scr.prompt=false", "-e", "scr.color=0"]
            self.process = owned_popen(
                argv,
                cwd=self.root,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                **(
                    {"deadline": self.work_deadline}
                    if self.work_deadline is not None
                    else {}
                ),
            )
            assert self.process.stderr is not None
            self._stderr_thread = threading.Thread(
                target=self._drain_stderr,
                args=(self.process.stderr,),
                daemon=True,
            )
            self._stderr_thread.start()

    def _drain_stderr(self, stream: Any) -> None:
        """Drain stderr concurrently so a noisy command cannot deadlock stdout."""

        descriptor = stream.fileno()
        while chunk := os.read(descriptor, 4096):
            with self._stderr_changed:
                self._stderr.extend(chunk)
                self._stderr_changed.notify_all()

    def _stderr_size(self) -> int:
        with self._stderr_lock:
            return len(self._stderr)

    def _stderr_since(self, offset: int) -> str:
        with self._stderr_lock:
            return bytes(self._stderr[offset:]).decode(errors="replace")

    def queue(self, command: str, selector: str) -> None:
        """Record one semantic command; execution happens in serial order."""

        self.pending.append((command, selector))

    def run_queued(
        self, timeout: float | None = None
    ) -> list[evidence_native.SemanticResult]:
        """Run every queued command on the single session, in order."""

        with self._lock:
            if self.process is None:
                self.open()
            process = self.process
            assert process is not None
            results: list[evidence_native.SemanticResult] = []
            for command, selector in self.pending:
                end = time.monotonic() + min(
                    self.command_timeout,
                    self.command_timeout if timeout is None else timeout,
                )
                # The stdin loop enables interactivity after startup; recheck each
                # capture because commands in a reused session can change it.
                for setup in ("e scr.interactive=false", "e scr.interactive"):
                    handshake = self._run_one(
                        process, setup, selector, max(0, end - time.monotonic())
                    )
                    if (
                        handshake.exit != 0
                        or handshake.killed
                        or (
                            setup == "e scr.interactive"
                            and handshake.raw.strip() != "false"
                        )
                    ):
                        raise ValueError(
                            "Rizin refused verified noninteractive capture"
                        )
                result = self._run_one(
                    process, command, selector, max(0, end - time.monotonic())
                )
                results.append(result)
                self.results.append(result)
            self.pending.clear()
            return results

    def _run_one(
        self,
        process: OwnedProcess,
        command: str,
        selector: str,
        timeout: float | None = None,
    ) -> evidence_native.SemanticResult:
        with self._lock:
            if process.poll() is not None:
                self.commands_killed += 1
                return evidence_native.SemanticResult(
                    command=command,
                    selector=selector,
                    exit=process.returncode or 1,
                    output="rizin session closed; command not executed",
                    killed=True,
                )
            nonce = secrets.token_hex(32)
            marker = f"BOF3_FRAME_{nonce}_START"
            end_marker = f"BOF3_FRAME_{nonce}_END"
            stderr_marker = f"BOF3_FRAME_{nonce}_STDERR_END"
            self.commands_executed += 1
            started = time.monotonic()
            stderr_start = self._stderr_size()
            try:
                if (
                    self.work_deadline is not None
                    and time.monotonic() >= self.work_deadline
                ):
                    return self.expire_command(command, selector)
                # The deliberately invalid final command makes Rizin write a
                # unique marker to stderr.  Waiting until the drain thread has
                # acknowledged that marker establishes a cross-pipe command
                # boundary without merging legitimate stdout and diagnostics.
                process.stdin.write(
                    f"echo {marker}\n{command}\necho {end_marker}\n"
                    f"?e {stderr_marker}\n".encode()
                )
                process.stdin.flush()
            except (BrokenPipeError, OSError):
                self.kill()
                self.commands_killed += 1
                return evidence_native.SemanticResult(
                    command=command,
                    selector=selector,
                    exit=1,
                    output="rizin session closed; command not executed",
                    killed=True,
                )
            raw = b""
            limit = (
                self.command_timeout
                if timeout is None
                else min(self.command_timeout, timeout)
            )
            if self.work_deadline is not None:
                limit = min(limit, max(0, self.work_deadline - started))
            while True:
                if time.monotonic() - started > limit:
                    return self.expire_command(
                        command, selector, raw=raw.decode(errors="replace")
                    )
                ready, _, _ = select.select(
                    [process.stdout] if process.stdout else [], [], [], 0.05
                )
                chunk = process.stdout.read1(4096) if ready and process.stdout else b""
                if chunk:
                    raw += chunk
                    stdout_lines = _complete_lines(raw.decode(errors="replace"))
                    start_line = next(
                        (
                            index
                            for index, line in enumerate(stdout_lines)
                            if line.rstrip("\r\n") == marker
                        ),
                        None,
                    )
                    end_line = next(
                        (
                            index
                            for index, line in enumerate(stdout_lines)
                            if start_line is not None
                            and index > start_line
                            and line.rstrip("\r\n") == end_marker
                        ),
                        None,
                    )
                    if start_line is not None and end_line is not None:
                        output = "".join(stdout_lines[start_line + 1 : end_line])
                        boundary_line = (
                            "ERROR: core: Error while parsing command: "
                            f"`?e {stderr_marker}`"
                        )
                        with self._stderr_changed:
                            while True:
                                interval = bytes(self._stderr[stderr_start:]).decode(
                                    errors="replace"
                                )
                                stderr_lines = _complete_lines(interval)
                                boundary_index = next(
                                    (
                                        index
                                        for index, line in enumerate(stderr_lines)
                                        if line.rstrip("\r\n") == boundary_line
                                    ),
                                    None,
                                )
                                if boundary_index is not None:
                                    break
                                remaining = limit - (time.monotonic() - started)
                                if remaining <= 0:
                                    self.kill()
                                    self.commands_killed += 1
                                    return evidence_native.SemanticResult(
                                        command=command,
                                        selector=selector,
                                        exit=124,
                                        output=(
                                            "deadline exceeded waiting for rizin "
                                            "stderr boundary"
                                        ),
                                        killed=True,
                                    )
                                self._stderr_changed.wait(min(remaining, 0.05))
                        diagnostic_lines = [
                            line.rstrip("\r\n")
                            for line in stderr_lines[:boundary_index]
                        ]
                        stderr = "\n".join(diagnostic_lines)
                        failed = any(
                            line.lstrip().startswith("ERROR:")
                            for line in diagnostic_lines
                        )
                        forensic = output + stderr
                        if time.monotonic() - started >= limit:
                            return self.expire_command(
                                command, selector, raw=output, stderr=stderr
                            )
                        return evidence_native.SemanticResult(
                            command=command,
                            selector=selector,
                            exit=1 if failed else 0,
                            output=evidence_native._bounded_text(forensic.strip()),
                            raw=output,
                            stderr=stderr,
                        )
                if process.poll() is not None and not chunk:
                    self.commands_killed += 1
                    return evidence_native.SemanticResult(
                        command=command,
                        selector=selector,
                        exit=process.returncode or 1,
                        output="rizin session exited before command completed",
                        killed=True,
                    )

    def expire_command(
        self, command: str, selector: str, *, raw: str = "", stderr: str = ""
    ) -> evidence_native.SemanticResult:
        self.kill()
        self.commands_killed += 1
        return evidence_native.SemanticResult(
            command=command,
            selector=selector,
            exit=124,
            output=f"deadline exceeded: rizin command {command!r} killed",
            killed=True,
            raw=raw,
            stderr=stderr,
        )

    def kill(self) -> None:
        """Terminate and reap the session so nothing outlives the run."""

        with self._lock:
            process = self.process
            if process is not None:
                process.terminate_tree()
            if self._stderr_thread is not None:
                self._stderr_thread.join(timeout=1)
                self._stderr_thread = None
            self.process = None

    def close(self) -> None:
        self.kill()

    def __enter__(self) -> "RizinSession":
        return self

    def __exit__(self, *_: Any) -> None:
        self.close()


class _StubEngine(EngineIdentity):
    """Test engine identity pinned to one executable path."""

    def __init__(self, executable: Path) -> None:
        super().__init__(
            name="rizin",
            executable=executable,
            version="test",
            capabilities={"mips32_little_endian": True, "json": True},
        )


__all__ = ["RizinSession"]
