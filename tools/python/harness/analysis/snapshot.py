"""Normalized per-target snapshots.

Normalizes raw Rizin output into target-qualified, typed records.
Classification comes from layout, source inventory, and PsyQ
bindings — never from naming heuristics.

Schema: ``bof3.analysis-snapshot/v1``
"""

from __future__ import annotations

import hashlib
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..domain.registry import ResolvedTarget
from ..layout import ReviewedLayout
from ..source_inventory import SourceInventory
from .replay import ReplayInputs
from .rizin import AnalyzerDump


SNAPSHOT_SCHEMA = "bof3.analysis-snapshot/v1"


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
        return row


@dataclass(frozen=True)
class SnapshotCall:
    """A normalized internal call."""

    caller: str
    callee: str
    callsite: int

    def to_row(self) -> dict[str, Any]:
        return {"caller": self.caller, "callee": self.callee, "callsite": self.callsite}


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


def _function_id(target: str, address: int) -> str:
    return f"{target}@{address:08x}"


def _containing(ranges: list[tuple[int, int, str]], address: int) -> str | None:
    for start, end, function_id in ranges:
        if start <= address < end:
            return function_id
    return None


def _exact_callee_start(functions: dict[int, str], address: int) -> str | None:
    """Return the function ID only if ``address`` is an exact function start."""

    return functions.get(address)


def build_snapshot(
    *,
    resolved: ResolvedTarget,
    layout: ReviewedLayout,
    inventory: SourceInventory,
    dump: AnalyzerDump,
    inputs: ReplayInputs,
    root: Path,
) -> TargetSnapshot:
    """Normalize a raw Rizin dump into a typed snapshot.

    Caller resolution uses callsite containment.
    Callee resolution requires an exact function start.
    Recursive calls are retained as internal calls.
    PsyQ lookup uses only the current target's bindings.
    """

    binary = (root / resolved.binary_path).read_bytes()
    reviewed = set(layout.reviewed_function_addresses)
    lifted = set(inventory.lifted_addresses())
    semantic_names = inventory.semantic_names()
    source_files = {
        f.address: f.source_path for f in inventory.functions if f.source_path is not None
    }
    psyq_by_addr = {p.address: p for p in inventory.psyq}

    # Build normalized functions.
    functions: list[SnapshotFunction] = []
    function_starts: dict[int, str] = {}  # address -> function_id
    ranges: list[tuple[int, int, str]] = []  # (start, end, function_id)

    for raw in dump.functions:
        addr = raw.addr
        size = raw.size
        offset = addr - resolved.load_address
        if offset < 0 or offset + size > len(binary) or size <= 0:
            continue
        fid = _function_id(resolved.id.value, addr)
        data = binary[offset : offset + size]
        exact_sha256 = hashlib.sha256(data).hexdigest()
        source_path = source_files.get(addr)

        functions.append(
            SnapshotFunction(
                id=fid,
                address=addr,
                analyzer_size=size,
                analyzer_name=raw.name,
                source_name=inventory.function_by_name(raw.name) and raw.name,
                semantic_name=semantic_names.get(addr),
                is_reviewed=addr in reviewed,
                is_lifted=addr in lifted,
                source=str(source_path.relative_to(root)) if source_path else None,
                exact_sha256=exact_sha256,
            )
        )
        function_starts[addr] = fid
        ranges.append((addr, addr + max(size, 0), fid))

    ranges.sort(key=lambda r: r[0])
    functions.sort(key=lambda f: f.address)

    # Build normalized calls.
    calls: set[SnapshotCall] = set()
    unresolved: list[SnapshotUnresolvedCall] = []

    for xref in dump.xrefs:
        if xref.xref_type != "CALL":
            continue
        from_addr = xref.from_addr
        to_addr = xref.to_addr

        caller = _containing(ranges, from_addr)
        if caller is None:
            continue

        # Exact callee start required.
        callee = _exact_callee_start(function_starts, to_addr)
        if callee is not None:
            calls.add(SnapshotCall(caller=caller, callee=callee, callsite=from_addr))
        else:
            # Check for PsyQ symbol.
            psyq = psyq_by_addr.get(to_addr)
            unresolved.append(
                SnapshotUnresolvedCall(
                    caller=caller,
                    target_address=to_addr,
                    callsite=from_addr,
                    kind="psyq" if psyq is not None else "unknown",
                    symbol=psyq.name if psyq is not None else None,
                )
            )

    return TargetSnapshot(
        schema=SNAPSHOT_SCHEMA,
        target=resolved.id.value,
        engine={"name": "rizin", "version": ""},
        inputs={
            "manifest_sha256": inputs.manifest_sha256,
            "binary_sha256": inputs.binary_sha256,
            "splat_sha256": inputs.splat_sha256,
            "source_inventory_sha256": inputs.source_inventory_sha256,
            "generated_replay_sha256": inputs.generated_replay_sha256,
            "reviewed_replay_sha256": inputs.reviewed_replay_sha256,
        },
        functions=tuple(functions),
        calls=tuple(sorted(calls, key=lambda c: (c.caller, c.callee))),
        unresolved_calls=tuple(
            sorted(unresolved, key=lambda c: (c.caller, c.target_address, c.callsite))
        ),
    )


def write_snapshot(snapshot: TargetSnapshot, path: Path) -> None:
    """Atomically write a snapshot to disk."""

    path.parent.mkdir(parents=True, exist_ok=True)
    content = json.dumps(snapshot.to_dict(), indent=2, sort_keys=True) + "\n"
    import os
    import tempfile

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
        SnapshotUnresolvedCall(**row) for row in raw.get("unresolved_calls", [])
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


__all__ = [
    "SNAPSHOT_SCHEMA",
    "SnapshotCall",
    "SnapshotFunction",
    "SnapshotUnresolvedCall",
    "TargetSnapshot",
    "build_snapshot",
    "read_snapshot",
    "write_snapshot",
]
