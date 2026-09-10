"""Bound native grouped-object inspection to one caller-owned work cutoff."""

from __future__ import annotations

from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path
import time

from harness.common.deadlines import DeadlineExpired, resolve_deadline
from harness.common.process import run_bounded


@dataclass(frozen=True)
class NativeExecution:
    root: Path
    deadline: float
    output_limit: int = 2 * 1024 * 1024

    def __post_init__(self) -> None:
        cutoff = resolve_deadline(self.deadline)
        if cutoff is None:
            raise ValueError("native grouped work requires an original deadline")
        if type(self.output_limit) is not int or self.output_limit < 1:
            raise ValueError("native output limit must be positive")
        object.__setattr__(self, "deadline", cutoff)

    def check_deadline(self) -> None:
        if time.monotonic() >= self.deadline:
            raise DeadlineExpired("native grouped work deadline expired")

    def run_bytes(self, argv: Sequence[str | Path]) -> bytes:
        self.check_deadline()
        output = bytearray()

        def capture(stream: str, chunk: bytes) -> None:
            if stream == "stdout":
                output.extend(chunk)

        result = run_bounded(
            self.root,
            argv,
            timeout=120,
            deadline=self.deadline,
            output_limit=self.output_limit,
            on_output=capture,
        )
        if result["failure"] or result["exit_code"]:
            raise RuntimeError(
                f"native grouped command failed: {result['failure'] or result['exit_code']}; "
                f"{result['stderr']}"
            )
        self.check_deadline()
        return bytes(output)

    def run_text(self, argv: Sequence[str | Path]) -> str:
        return self.run_bytes(argv).decode("utf-8", errors="strict")
