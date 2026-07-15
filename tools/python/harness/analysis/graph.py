"""Cross-target analysis graph.

Consumes normalized snapshots to produce one deterministic graph.
Never launches an analyzer.

Schema: ``bof3.analysis-graph/v2``
"""

from __future__ import annotations

import json
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from .snapshot import (
    TargetSnapshot,
)


GRAPH_SCHEMA = "bof3.analysis-graph/v2"


@dataclass(frozen=True)
class GraphFunction:
    """A target-qualified function in the global graph."""

    id: str
    target: str
    address: int
    analyzer_size: int
    is_reviewed: bool
    is_lifted: bool
    exact_sha256: str
    analyzer_name: str
    source_name: str | None
    semantic_name: str | None
    source: str | None

    def to_row(self) -> dict[str, Any]:
        row: dict[str, Any] = {
            "id": self.id,
            "target": self.target,
            "address": self.address,
            "analyzer_size": self.analyzer_size,
            "is_reviewed": self.is_reviewed,
            "is_lifted": self.is_lifted,
            "exact_sha256": self.exact_sha256,
            "analyzer_name": self.analyzer_name,
        }
        if self.source_name is not None:
            row["source_name"] = self.source_name
        if self.semantic_name is not None:
            row["semantic_name"] = self.semantic_name
        if self.source is not None:
            row["source"] = self.source
        return row


@dataclass(frozen=True)
class GraphCall:
    """An internal call in the global graph."""

    caller: str
    callee: str
    callsite: int

    def to_row(self) -> dict[str, Any]:
        return {"caller": self.caller, "callee": self.callee, "callsite": self.callsite}


@dataclass(frozen=True)
class GraphUnresolvedCall:
    """An unresolved call in the global graph."""

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
class AnalysisGraph:
    """The complete cross-target analysis graph."""

    schema: str
    engine: dict[str, str]
    targets_analyzed: list[str]
    targets_skipped: list[dict[str, str]]
    functions: list[GraphFunction]
    calls: list[GraphCall]
    unresolved_calls: list[GraphUnresolvedCall]
    duplicate_groups: list[list[str]]
    snapshot_paths: dict[str, str]

    def to_dict(self) -> dict[str, Any]:
        return {
            "schema": self.schema,
            "engine": self.engine,
            "targets_analyzed": self.targets_analyzed,
            "targets_skipped": self.targets_skipped,
            "functions": [f.to_row() for f in self.functions],
            "calls": [c.to_row() for c in self.calls],
            "unresolved_calls": [c.to_row() for c in self.unresolved_calls],
            "duplicate_groups": self.duplicate_groups,
            "snapshot_paths": self.snapshot_paths,
        }


def build_graph(
    snapshots: dict[str, TargetSnapshot],
    *,
    engine_name: str = "rizin",
    engine_version: str = "",
    skipped: list[dict[str, str]] | None = None,
) -> AnalysisGraph:
    """Merge normalized snapshots into one deterministic graph.

    Each function is keyed by ``target@address``.
    Duplicate groups are keyed by ``(size, exact_sha256)``.
    """

    functions: list[GraphFunction] = []
    calls: list[GraphCall] = []
    unresolved: list[GraphUnresolvedCall] = []
    exact: dict[tuple[int, str], list[str]] = defaultdict(list)
    snapshot_paths: dict[str, str] = {}

    for target_id in sorted(snapshots):
        snapshot = snapshots[target_id]
        for func in snapshot.functions:
            gf = GraphFunction(
                id=func.id,
                target=snapshot.target,
                address=func.address,
                analyzer_size=func.analyzer_size,
                is_reviewed=func.is_reviewed,
                is_lifted=func.is_lifted,
                exact_sha256=func.exact_sha256,
                analyzer_name=func.analyzer_name,
                source_name=func.source_name,
                semantic_name=func.semantic_name,
                source=func.source,
            )
            functions.append(gf)
            exact[(func.analyzer_size, func.exact_sha256)].append(func.id)
        for call in snapshot.calls:
            calls.append(
                GraphCall(caller=call.caller, callee=call.callee, callsite=call.callsite)
            )
        for uc in snapshot.unresolved_calls:
            unresolved.append(
                GraphUnresolvedCall(
                    caller=uc.caller,
                    target_address=uc.target_address,
                    callsite=uc.callsite,
                    kind=uc.kind,
                    symbol=uc.symbol,
                )
            )

    duplicate_groups = sorted(
        [sorted(group) for group in exact.values() if len(group) > 1],
        key=lambda g: g[0],
    )

    return AnalysisGraph(
        schema=GRAPH_SCHEMA,
        engine={"name": engine_name, "version": engine_version},
        targets_analyzed=sorted(snapshots.keys()),
        targets_skipped=skipped or [],
        functions=sorted(functions, key=lambda f: (f.target, f.address)),
        calls=sorted(calls, key=lambda c: (c.caller, c.callee)),
        unresolved_calls=sorted(
            unresolved, key=lambda c: (c.caller, c.target_address, c.callsite)
        ),
        duplicate_groups=duplicate_groups,
        snapshot_paths=snapshot_paths,
    )


def write_graph(graph: AnalysisGraph, path: Path) -> None:
    """Atomically write the graph to disk."""

    path.parent.mkdir(parents=True, exist_ok=True)
    content = json.dumps(graph.to_dict(), indent=2, sort_keys=True) + "\n"
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


def read_graph(path: Path) -> AnalysisGraph:
    """Read and validate a graph from disk."""

    raw = json.loads(path.read_text(encoding="utf-8"))
    if raw.get("schema") != GRAPH_SCHEMA:
        raise ValueError(
            f"graph schema mismatch: expected {GRAPH_SCHEMA!r}, got {raw.get('schema')!r}"
        )
    functions = [
        GraphFunction(
            id=row["id"],
            target=row["target"],
            address=row["address"],
            analyzer_size=row["analyzer_size"],
            is_reviewed=row["is_reviewed"],
            is_lifted=row["is_lifted"],
            exact_sha256=row["exact_sha256"],
            analyzer_name=row["analyzer_name"],
            source_name=row.get("source_name"),
            semantic_name=row.get("semantic_name"),
            source=row.get("source"),
        )
        for row in raw.get("functions", [])
    ]
    calls = [GraphCall(**row) for row in raw.get("calls", [])]
    unresolved = [GraphUnresolvedCall(**row) for row in raw.get("unresolved_calls", [])]
    return AnalysisGraph(
        schema=raw["schema"],
        engine=raw["engine"],
        targets_analyzed=raw.get("targets_analyzed", []),
        targets_skipped=raw.get("targets_skipped", []),
        functions=functions,
        calls=calls,
        unresolved_calls=unresolved,
        duplicate_groups=raw.get("duplicate_groups", []),
        snapshot_paths=raw.get("snapshot_paths", {}),
    )


__all__ = [
    "GRAPH_SCHEMA",
    "AnalysisGraph",
    "GraphCall",
    "GraphFunction",
    "GraphUnresolvedCall",
    "build_graph",
    "read_graph",
    "write_graph",
]
