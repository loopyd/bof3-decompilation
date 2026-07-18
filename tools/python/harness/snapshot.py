"""Engine-independent snapshot models and validation.

Migrated from ``harness.analysis.snapshot`` for subprocess-based stateless
analysis.  Contains only portable snapshot data structures, I/O, and
validation helpers.
"""

from __future__ import annotations

import json
import os
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any

SNAPSHOT_SCHEMA = "bof3.analysis-snapshot/v2"


def snapshot_path(root: Path, target_id: str) -> Path:
    """Return the canonical generated snapshot path for a target."""

    return root / "out" / "reverse" / target_id / "snapshot.json"


@dataclass(frozen=True)
class SnapshotFunction:
    """A normalized function record."""

    id: str
    address: int
    analyzer_size: int
    analyzer_name: str
    exact_sha256: str
    source_name: str | None = None
    semantic_name: str | None = None
    is_reviewed: bool = False
    is_lifted: bool = False
    source: str | None = None
    basic_blocks: int | None = None
    cyclomatic_complexity: int | None = None
    edges: int | None = None
    loops: int | None = None
    stack_frame: int | None = None
    local_count: int | None = None
    argument_count: int | None = None

    def to_row(self) -> dict[str, Any]:
        row: dict[str, Any] = {
            "id": self.id,
            "address": self.address,
            "analyzer_size": self.analyzer_size,
            "analyzer_name": self.analyzer_name,
            "is_reviewed": self.is_reviewed,
            "is_lifted": self.is_lifted,
            "exact_sha256": self.exact_sha256,
        }
        if self.source_name is not None:
            row["source_name"] = self.source_name
        if self.semantic_name is not None:
            row["semantic_name"] = self.semantic_name
        if self.source is not None:
            row["source"] = self.source
        row.update(
            {
                "basic_blocks": self.basic_blocks,
                "cyclomatic_complexity": self.cyclomatic_complexity,
                "edges": self.edges,
                "loops": self.loops,
                "stack_frame": self.stack_frame,
                "local_count": self.local_count,
                "argument_count": self.argument_count,
            }
        )
        return row


@dataclass(frozen=True)
class SnapshotCall:
    """A normalized internal call."""

    caller: str
    callee: str
    callsite: int

    def to_row(self) -> dict[str, Any]:
        return {
            "caller": self.caller,
            "callee": self.callee,
            "callsite": self.callsite,
        }


@dataclass(frozen=True)
class SnapshotUnresolvedCall:
    """A call whose target is not a known function start."""

    caller: str
    target_address: int
    callsite: int
    kind: str
    symbol: str | None = None

    def to_row(self) -> dict[str, Any]:
        row: dict[str, Any] = {
            "caller": self.caller,
            "target_address": self.target_address,
            "callsite": self.callsite,
            "kind": self.kind,
        }
        if self.symbol is not None:
            row["symbol"] = self.symbol
        return row


@dataclass(frozen=True)
class TargetSnapshot:
    """A complete normalized snapshot for one target."""

    schema: str
    target: str
    engine: dict[str, str]
    inputs: dict[str, str | None]
    functions: tuple[SnapshotFunction, ...]
    calls: tuple[SnapshotCall, ...]
    unresolved_calls: tuple[SnapshotUnresolvedCall, ...]

    def to_dict(self) -> dict[str, Any]:
        return {
            "schema": self.schema,
            "target": self.target,
            "engine": self.engine,
            "inputs": self.inputs,
            "functions": [f.to_row() for f in self.functions],
            "calls": [c.to_row() for c in self.calls],
            "unresolved_calls": [c.to_row() for c in self.unresolved_calls],
        }


def write_snapshot(snapshot: TargetSnapshot, path: Path) -> None:
    """Atomically write a snapshot to disk."""

    path.parent.mkdir(parents=True, exist_ok=True)
    content = json.dumps(snapshot.to_dict(), indent=2, sort_keys=True) + "\n"
    descriptor, temp_name = tempfile.mkstemp(
        dir=path.parent, prefix=f".{path.name}.", suffix=".tmp"
    )
    temp_path = Path(temp_name)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as f:
            f.write(content)
            f.flush()
            os.fsync(f.fileno())
        temp_path.replace(path)
    except BaseException:
        temp_path.unlink(missing_ok=True)
        raise


def read_snapshot(path: Path) -> TargetSnapshot:
    """Read and validate a snapshot from disk."""

    raw = json.loads(path.read_text(encoding="utf-8"))
    if raw.get("schema") != SNAPSHOT_SCHEMA:
        raise ValueError(
            f"snapshot schema mismatch: expected {SNAPSHOT_SCHEMA!r}, "
            f"got {raw.get('schema')!r}"
        )
    functions = tuple(
        SnapshotFunction(**row) for row in raw.get("functions", [])
    )
    calls = tuple(SnapshotCall(**row) for row in raw.get("calls", []))
    unresolved = tuple(
        SnapshotUnresolvedCall(**row)
        for row in raw.get("unresolved_calls", [])
    )
    return TargetSnapshot(
        schema=raw["schema"],
        target=raw["target"],
        engine=raw["engine"],
        inputs=raw["inputs"],
        functions=functions,
        calls=calls,
        unresolved_calls=unresolved,
    )


# ---------------------------------------------------------------------------
# Snapshot validation
# ---------------------------------------------------------------------------


def validate_snapshot_identity(snapshot: TargetSnapshot) -> list[str]:
    """Validate snapshot identity and structural integrity.

    Returns a list of error strings; an empty list means the snapshot is
    structurally valid.
    """

    errors: list[str] = []
    if snapshot.schema != SNAPSHOT_SCHEMA:
        errors.append(
            f"schema mismatch: expected {SNAPSHOT_SCHEMA!r}, "
            f"got {snapshot.schema!r}"
        )
    if not snapshot.target:
        errors.append("missing target identifier")
    if not snapshot.engine.get("name"):
        errors.append("missing engine name")

    function_ids = {f.id for f in snapshot.functions}
    if len(function_ids) != len(snapshot.functions):
        errors.append("duplicate function IDs")

    for call in snapshot.calls:
        if call.caller not in function_ids:
            errors.append(f"call references unknown caller: {call.caller}")
        if call.callee not in function_ids:
            errors.append(f"call references unknown callee: {call.callee}")

    metric_names = (
        "basic_blocks",
        "cyclomatic_complexity",
        "edges",
        "loops",
        "stack_frame",
        "local_count",
        "argument_count",
    )
    for function in snapshot.functions:
        for name in metric_names:
            value = getattr(function, name)
            if value is not None and (
                isinstance(value, bool) or not isinstance(value, int) or value < 0
            ):
                errors.append(f"invalid {name} for {function.id}: {value!r}")
        if function.basic_blocks == 0:
            errors.append(f"invalid basic_blocks for {function.id}: 0")
        if function.cyclomatic_complexity == 0:
            errors.append(f"invalid cyclomatic_complexity for {function.id}: 0")

    return errors


def validate_snapshot_freshness(
    snapshot: TargetSnapshot,
    *,
    expected_target: str | None = None,
    expected_engine: str | None = None,
) -> list[str]:
    """Validate snapshot freshness against expected parameters.

    Returns a list of error strings; an empty list means the snapshot is
    fresh relative to the supplied expectations.
    """

    errors: list[str] = []
    if expected_target is not None and snapshot.target != expected_target:
        errors.append(
            f"target mismatch: expected {expected_target!r}, "
            f"got {snapshot.target!r}"
        )
    if expected_engine is not None:
        engine_name = snapshot.engine.get("name", "")
        if engine_name != expected_engine:
            errors.append(
                f"engine mismatch: expected {expected_engine!r}, "
                f"got {engine_name!r}"
            )
    if not snapshot.inputs:
        errors.append("missing input hashes")
    return errors


def validate_snapshot_hashes(
    snapshot: TargetSnapshot, hashes: dict[str, str | None]
) -> list[str]:
    """Validate snapshot input hashes against provided hashes.

    Returns a list of error strings; an empty list means all supplied
    hashes match.
    """

    errors: list[str] = []
    for key, expected in hashes.items():
        actual = snapshot.inputs.get(key)
        if expected is not None and actual != expected:
            errors.append(
                f"hash mismatch for {key}: expected {expected!r}, got {actual!r}"
            )
    return errors


__all__ = [
    "SNAPSHOT_SCHEMA",
    "SnapshotCall",
    "SnapshotFunction",
    "SnapshotUnresolvedCall",
    "TargetSnapshot",
    "read_snapshot",
    "snapshot_path",
    "write_snapshot",
    "validate_snapshot_freshness",
    "validate_snapshot_hashes",
    "validate_snapshot_identity",
]
