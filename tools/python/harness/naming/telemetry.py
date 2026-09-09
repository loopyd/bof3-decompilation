"""Bounded naming-evidence phase telemetry."""

from __future__ import annotations

import json
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from harness.naming.collection import EvidenceNamespace


def artifact_bytes(
    root: Path, namespace: EvidenceNamespace, entries: dict[str, dict[str, Any]]
) -> tuple[int, int]:
    """Return receipt-file and model-facing output bytes for committed rows."""

    receipts = 0
    stdout = 0
    for entry in entries.values():
        evidence = entry.get("evidence")
        if not isinstance(evidence, str):
            continue
        try:
            path = namespace.resolve_record(evidence)
        except ValueError:
            continue
        if not path.is_file():
            continue
        payload = json.loads(path.read_text(encoding="utf-8"))
        for command in payload.get("commands", []):
            if not isinstance(command, dict):
                continue
            receipt = command.get("receipt")
            output = command.get("output")
            if isinstance(receipt, str):
                try:
                    receipt_path = namespace.resolve_record(receipt)
                except ValueError:
                    continue
                if receipt_path.is_file():
                    receipts += receipt_path.stat().st_size
            if isinstance(output, str):
                stdout += len(output.encode("utf-8"))
    return receipts, stdout


@dataclass
class RunTelemetry:
    """Reconciled counters and monotonic phase durations for one run."""

    started: float = field(default_factory=time.monotonic)
    phase_started: float = field(default_factory=time.monotonic)
    phases: dict[str, float] = field(default_factory=dict)
    rows_planned: int = 0
    rows_completed: int = 0
    rows_skipped: int = 0
    rows_stale: int = 0
    external_processes: int = 0
    index_opens: int = 0
    receipt_bytes: int = 0
    stdout_bytes: int = 0

    def phase(self, name: str) -> None:
        """Close the prior phase and start ``name``."""

        now = time.monotonic()
        self.phases[name] = self.phases.get(name, 0.0) + now - self.phase_started
        self.phase_started = now

    def payload(self, checkpoint_id: str) -> dict[str, Any]:
        """Return one bounded, reconciled summary."""

        now = time.monotonic()
        phases = {key: round(value, 6) for key, value in sorted(self.phases.items())}
        phases["finalize"] = round(now - self.phase_started, 6)
        wall = round(now - self.started, 6)
        accounted = round(sum(phases.values()), 6)
        return {
            "schema": "bof3.naming-evidence-telemetry/v1",
            "wall_seconds": wall,
            "accounted_seconds": accounted,
            "phases": phases,
            "index_opens": self.index_opens,
            "external_processes": self.external_processes,
            "rows": {
                "planned": self.rows_planned,
                "completed": self.rows_completed,
                "skipped": self.rows_skipped,
                "stale": self.rows_stale,
            },
            "receipt_bytes": self.receipt_bytes,
            "stdout_bytes": self.stdout_bytes,
            "checkpoint_id": checkpoint_id,
        }


def write_telemetry(path: Path, payload: dict[str, Any]) -> None:
    """Atomically write a telemetry object no larger than 4 KiB."""

    data = (json.dumps(payload, sort_keys=True, separators=(",", ":")) + "\n").encode()
    if len(data) > 4096:
        raise ValueError("telemetry exceeds 4 KiB")
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(".tmp")
    temporary.write_bytes(data)
    temporary.replace(path)
