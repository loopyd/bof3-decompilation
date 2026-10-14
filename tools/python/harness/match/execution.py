"""Run native grouped-object inspection with an explicit output limit."""

from __future__ import annotations

from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from harness.common.process import run_bounded


@dataclass(frozen=True)
class NativeExecution:
    root: Path
    output_limit: int = 2 * 1024 * 1024

    def __post_init__(self) -> None:
        if type(self.output_limit) is not int or self.output_limit < 1:
            raise ValueError("native output limit must be positive")

    def run_bytes(self, argv: Sequence[str | Path]) -> bytes:
        output = bytearray()

        def capture(stream: str, chunk: bytes) -> None:
            if stream == "stdout":
                output.extend(chunk)

        result = run_bounded(
            self.root,
            argv,
            output_limit=self.output_limit,
            on_output=capture,
        )
        if result["failure"] or result["exit_code"]:
            raise RuntimeError(
                f"native grouped command failed: {result['failure'] or result['exit_code']}; "
                f"{result['stderr']}"
            )
        return bytes(output)

    def run_text(self, argv: Sequence[str | Path]) -> str:
        return self.run_bytes(argv).decode("utf-8", errors="strict")
