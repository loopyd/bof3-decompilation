"""Run serial, independent Rizin captures without a command-input transport."""

from __future__ import annotations

import subprocess
import threading
from pathlib import Path
from typing import Any

from harness.analysis.engine import EngineIdentity, find_engine
from harness.analysis.project import prepare_target, rizin_argv
from harness.common.process import OwnedProcess, ProcessCleanupError, owned_popen
from harness.domain.manifests import load_target_manifests
from harness.naming import native as evidence_native


class RizinSession:
    """Queue serial one-shot captures with separate output pipes and stdin EOF."""

    def __init__(
        self,
        root: Path,
        target: str,
        *,
        executable: str | Path | None = None,
    ) -> None:
        self.root = root
        self.target = target
        self.executable = executable
        self.process: OwnedProcess | None = None
        self.commands_executed = 0
        self.commands_killed = 0
        self.pending: list[tuple[str, str]] = []
        self.results: list[evidence_native.SemanticResult] = []
        self._lock = threading.RLock()

    @property
    def process_count(self) -> int:
        """Return the active owned process count, at most one."""
        return int(self.process is not None)

    def queue(self, command: str, selector: str) -> None:
        """Queue an independently reproducible command in execution order."""
        with self._lock:
            self.pending.append((command, selector))

    def run_queued(self) -> list[evidence_native.SemanticResult]:
        """Consume queued commands once, retaining each completed result."""
        with self._lock:
            if self.process is not None:
                raise ProcessCleanupError(
                    "previous analyzer cleanup is unconfirmed; close before retry"
                )
            results = []
            while self.pending:
                command, selector = self.pending.pop(0)
                result = self._run_one(command, selector)
                results.append(result)
                self.results.append(result)
            return results

    def _run_one(self, command: str, selector: str) -> evidence_native.SemanticResult:
        manifests = load_target_manifests(self.root)
        if self.target not in manifests:
            raise ValueError(f"unknown target: {self.target}")
        spec = prepare_target(self.root, self.target, manifest=manifests[self.target])
        engine = (
            find_engine("rizin", root=self.root)
            if self.executable is None
            else _StubEngine(Path(self.executable))
        )
        argv = rizin_argv(spec, engine, quiet=True, commands=(command, "q!"))
        argv[1:1] = [
            "-e",
            "scr.prompt=false",
            "-e",
            "scr.color=0",
            "-e",
            "scr.interactive=false",
        ]
        self.process = owned_popen(
            argv,
            cwd=self.root,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self.commands_executed += 1
        process = self.process
        try:
            stdout, stderr = process.communicate()
            status = process.returncode
            assert status is not None
        except BaseException:
            self.commands_killed += 1
            self.kill()
            raise
        self.process = None
        stdout = stdout.decode(errors="replace")
        stderr = stderr.decode(errors="replace")
        failed = any(line.lstrip().startswith("ERROR:") for line in stderr.splitlines())
        return evidence_native.SemanticResult(
            command=command,
            selector=selector,
            exit=status if status else int(failed),
            output=evidence_native._bounded_text((stdout + stderr).strip()),
            raw=stdout,
            stderr=stderr,
        )

    def kill(self) -> None:
        """Terminate and reap the owned tree; retain uncertainty for recovery."""
        with self._lock:
            if self.process is not None:
                self.process.terminate_tree()
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
