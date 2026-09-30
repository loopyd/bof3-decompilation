"""Bounded native byte-proof command execution for naming evidence."""

from __future__ import annotations

import subprocess
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from harness.common.commands import command_at
from harness.common.process import owned_popen
from harness.common.deadlines import resolve_deadline

OUTPUT_BUDGET = 8 * 1024


@dataclass
class SemanticResult:
    """The bounded result of one original-byte command."""

    command: str
    selector: str
    exit: int
    output: str
    killed: bool = False
    raw: str = ""
    stderr: str = ""


def _bounded_text(text: str, budget: int = OUTPUT_BUDGET) -> str:
    if len(text) <= budget:
        return text
    marker = " ...[truncated; full evidence in digest-bound file]"
    if len(marker) >= budget:
        return text[:budget]
    return text[: budget - len(marker)] + marker


class NativeByteOps:
    """Bounded ``asm-diff``/``byte-match`` subcommand execution."""

    def __init__(
        self, root: Path, deadline: int, *, work_deadline: float | None = None
    ) -> None:
        self.root = root
        self.deadline = deadline
        self.work_deadline = resolve_deadline(work_deadline)

    def run(
        self, operation: dict[str, Any], timeout: float | None = None
    ) -> SemanticResult:
        kind = operation["kind"]
        argv = command_at(self.root, kind, operation["target"], *operation["args"])
        limit = min(self.deadline, timeout) if timeout is not None else self.deadline
        cutoff = (
            min(time.monotonic() + limit, self.work_deadline)
            if self.work_deadline is not None
            else None
        )
        process = owned_popen(
            argv,
            cwd=self.root,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            **({"deadline": cutoff} if cutoff is not None else {}),
        )
        try:
            stdout, stderr = process.communicate(
                timeout=max(0, cutoff - time.monotonic())
                if cutoff is not None
                else limit
            )
            if cutoff is not None and time.monotonic() >= cutoff:
                raise subprocess.TimeoutExpired(argv, limit)
        except subprocess.TimeoutExpired:
            process.terminate_tree(timeout=2)
            stdout, stderr = "", ""
            return SemanticResult(
                command=" ".join(argv),
                selector=operation["target"],
                exit=124,
                output=f"deadline exceeded: {kind} owned process tree reaped",
                killed=True,
                raw=stdout,
                stderr=stderr,
            )
        except BaseException:
            process.terminate_tree(timeout=2)
            raise
        raw = stdout + stderr
        return SemanticResult(
            command=" ".join(argv),
            selector=operation["target"],
            exit=process.returncode if process.returncode is not None else 1,
            output=_bounded_text(raw),
            raw=stdout,
            stderr=stderr,
        )
