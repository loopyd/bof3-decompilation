"""Hotspot rankings from the canonical analysis graph.

Pure graph consumer.  Never launches an analyzer.
"""

from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from .graph import (
    GRAPH_SCHEMA,
    AnalysisGraph,
    GraphFunction,
    read_graph,
)


HOTSPOT_SCHEMA = "bof3.analysis-hotspots/v2"


def _function_id(target: str, address: int) -> str:
    return f"{target}@{address:08x}"


@dataclass(frozen=True)
class _Hotspot:
    address: int
    target: str
    name: str
    size: int
    in_degree: int
    out_degree: int
    is_reviewed: bool
    is_lifted: bool
    semantic_name: str | None = None
    source: str | None = None

    def to_row(self) -> dict[str, Any]:
        row: dict[str, Any] = {
            "address": f"0x{self.address:08x}",
            "target": self.target,
            "name": self.name,
            "size": self.size,
            "callers": self.in_degree,
            "in_degree": self.in_degree,
            "out_degree": self.out_degree,
            "is_reviewed": self.is_reviewed,
            "is_lifted": self.is_lifted,
        }
        if self.semantic_name is not None:
            row["semantic_name"] = self.semantic_name
        if self.source is not None:
            row["source"] = self.source
        return row


def build_hotspots(graph: AnalysisGraph) -> dict[str, Any]:
    """Derive all hotspot rankings from the canonical graph."""

    # Build adjacency from internal calls.
    callers: dict[str, set[str]] = defaultdict(set)
    callees: dict[str, set[str]] = defaultdict(set)
    for call in graph.calls:
        if call.caller != call.callee:
            callers[call.callee].add(call.caller)
            callees[call.caller].add(call.callee)

    # Build function lookup.
    func_map: dict[str, GraphFunction] = {}
    for f in graph.functions:
        func_map[f.id] = f

    # Build unresolved-call aggregation.
    unresolved_by_target: dict[tuple[str, int], dict[str, Any]] = {}
    for uc in graph.unresolved_calls:
        # Group by caller target + target_address.
        caller_target = uc.caller.split("@")[0]
        key = (caller_target, uc.target_address)
        bucket = unresolved_by_target.setdefault(
            key,
            {
                "target": caller_target,
                "address": f"0x{uc.target_address:08x}",
                "callers": 0,
                "sample_callers": [],
                "kind": uc.kind,
                "symbol": uc.symbol,
            },
        )
        bucket["callers"] += 1
        if uc.caller not in bucket["sample_callers"]:
            bucket["sample_callers"].append(uc.caller)
    unknown_targets = sorted(
        unresolved_by_target.values(), key=lambda row: -row["callers"]
    )

    # Build discovery: reviewed functions with unresolved callees.
    unresolved_callers: dict[str, int] = defaultdict(int)
    for uc in graph.unresolved_calls:
        unresolved_callers[uc.caller] += 1
    discovery: list[dict[str, Any]] = []
    for func_id, count in unresolved_callers.items():
        func = func_map.get(func_id)
        if func is None or not func.is_reviewed:
            continue
        total_callees = len(callees.get(func_id, set())) + count
        discovery.append(
            {
                "address": f"0x{func.address:08x}",
                "target": func.target,
                "name": func.analyzer_name,
                "is_reviewed": func.is_reviewed,
                "is_lifted": func.is_lifted,
                "total_callees": total_callees,
                "unknown_callees": count,
                "known_callees": total_callees - count,
                "callers": len(callers.get(func_id, set())),
            }
        )
    discovery.sort(key=lambda row: (-row["unknown_callees"], -row["callers"]))

    # Build per-function rankings.
    hot: list[_Hotspot] = []
    leaves: list[_Hotspot] = []
    roots: list[_Hotspot] = []
    shallow: list[_Hotspot] = []

    for func in graph.functions:
        in_deg = len(callers.get(func.id, set()))
        out_deg = len(callees.get(func.id, set()))
        name = func.semantic_name or func.analyzer_name
        hotspot = _Hotspot(
            address=func.address,
            target=func.target,
            name=name,
            size=func.analyzer_size,
            in_degree=in_deg,
            out_degree=out_deg,
            is_reviewed=func.is_reviewed,
            is_lifted=func.is_lifted,
            semantic_name=func.semantic_name,
            source=func.source,
        )
        if in_deg > 0:
            hot.append(hotspot)
        if out_deg == 0:
            leaves.append(hotspot)
        if in_deg == 0:
            roots.append(hotspot)
        if func.is_reviewed and 0 < out_deg <= 3 and in_deg <= 5:
            shallow.append(hotspot)

    hot.sort(key=lambda h: (-h.in_degree, h.address))
    leaves.sort(key=lambda h: (-h.in_degree, h.size, h.address))
    roots.sort(key=lambda h: h.address)
    shallow.sort(key=lambda h: (h.out_degree, -h.in_degree, h.address))

    return {
        "schema": HOTSPOT_SCHEMA,
        "graph_schema": GRAPH_SCHEMA,
        "targets_analyzed": graph.targets_analyzed,
        "targets_skipped": graph.targets_skipped,
        "total_functions": len(graph.functions),
        "total_call_edges": len(graph.calls),
        "hot": [h.to_row() for h in hot[:50]],
        "leaves": [h.to_row() for h in leaves[:40]],
        "roots": [h.to_row() for h in roots[:30]],
        "shallow": [h.to_row() for h in shallow[:30]],
        "unknown_targets": unknown_targets[:30],
        "discovery": discovery[:30],
        "exact_duplicates": [
            {"size": 0, "functions": group}
            for group in graph.duplicate_groups[:20]
        ],
    }


def hotspot_analysis(
    root: Path,
    *,
    kind: str | None = None,
    top: int = 40,
    min_callers: int = 0,
    max_out: int | None = None,
    min_size: int | None = None,
    max_size: int | None = None,
    status: str = "all",
    sort: str | None = None,
) -> dict[str, Any]:
    """Run hotspot analysis from the canonical graph.

    Does not invoke the analyzer.  Reads the graph from
    ``out/analysis/graph.json`` and derives rankings.
    """

    graph_path = root / "out" / "analysis" / "graph.json"
    if not graph_path.is_file():
        raise FileNotFoundError(
            "analysis graph missing; run: bin/harness analysis graph"
        )
    if top < 0:
        raise ValueError("--top must be non-negative")
    graph = read_graph(graph_path)
    report = build_hotspots(graph)

    if kind is not None:
        _validate_filters(
            kind=kind,
            min_callers=min_callers,
            max_out=max_out,
            min_size=min_size,
            max_size=max_size,
            status=status,
            sort=sort,
        )
        base = report.get(kind, [])
        items = [
            row
            for row in base
            if _filter_row(
                row,
                kind=kind,
                min_callers=min_callers,
                max_out=max_out,
                min_size=min_size,
                max_size=max_size,
                status=status,
            )
        ]
        sort_key = sort or _default_sort(kind)
        reverse = sort_key != "address"
        items.sort(key=lambda row: _sort_key(row, sort_key), reverse=reverse)
        report["selection_kind"] = kind
        report["selection_params"] = {
            "kind": kind,
            "top": top,
            "min_callers": min_callers,
            "max_out": max_out,
            "min_size": min_size,
            "max_size": max_size,
            "status": status,
            "sort": sort_key,
        }
        report["selection"] = items[:top]

    return report


_SORTABLE_KINDS = {
    "hot",
    "leaves",
    "roots",
    "shallow",
    "unknown_targets",
    "discovery",
    "exact_duplicates",
}

_SUPPORTED_FILTER_KINDS = {"hot", "leaves", "roots", "shallow", "discovery"}
_SUPPORTED_DEGREE_KINDS = {"leaves", "roots", "shallow", "discovery"}
_SUPPORTED_SIZE_KINDS = {"hot", "leaves", "roots", "shallow"}
_STATUS_KINDS = {"hot", "leaves", "roots", "shallow", "discovery"}
_SORT_CHOICES = ("callers", "size", "address", "out_degree")


def _validate_filters(
    *,
    kind: str,
    min_callers: int,
    max_out: int | None,
    min_size: int | None,
    max_size: int | None,
    status: str,
    sort: str | None,
) -> None:
    if kind not in _SORTABLE_KINDS:
        raise ValueError(f"unknown hotspot kind: {kind}")
    if min_callers < 0:
        raise ValueError("--min-callers must be non-negative")
    if max_out is not None and max_out < 0:
        raise ValueError("--max-out must be non-negative")
    if min_size is not None and min_size < 0:
        raise ValueError("--min-size must be non-negative")
    if max_size is not None and max_size < 0:
        raise ValueError("--max-size must be non-negative")
    if min_size is not None and max_size is not None and min_size > max_size:
        raise ValueError("--min-size cannot exceed --max-size")
    if status not in {"all", "reviewed", "lifted", "unreviewed", "unlifted"}:
        raise ValueError("--status must be all, reviewed, lifted, unreviewed, or unlifted")
    if kind not in _SUPPORTED_FILTER_KINDS and (
        min_callers > 0 or max_out is not None or status != "all"
    ):
        raise ValueError(
            f"filters are not supported for kind {kind!r}; choose a function kind"
        )
    if sort is not None and sort not in _SORT_CHOICES:
        raise ValueError(f"unknown sort key: {sort}")


def _filter_row(
    row: dict[str, Any],
    *,
    kind: str,
    min_callers: int,
    max_out: int | None,
    min_size: int | None,
    max_size: int | None,
    status: str,
) -> bool:
    if kind in _SUPPORTED_DEGREE_KINDS and max_out is not None:
        if int(row.get("out_degree", 0)) > max_out:
            return False
    if kind in _SUPPORTED_FILTER_KINDS and min_callers > 0:
        if int(row.get("callers", row.get("in_degree", 0))) < min_callers:
            return False
    if kind in _SUPPORTED_SIZE_KINDS:
        if min_size is not None and int(row.get("size", 0)) < min_size:
            return False
        if max_size is not None and int(row.get("size", 0)) > max_size:
            return False
    if kind in _STATUS_KINDS and status != "all":
        is_reviewed = bool(row.get("is_reviewed"))
        is_lifted = bool(row.get("is_lifted"))
        if status == "reviewed" and not is_reviewed:
            return False
        if status == "lifted" and not is_lifted:
            return False
        if status == "unreviewed" and is_reviewed:
            return False
        if status == "unlifted" and is_lifted:
            return False
    return True


def _sort_key(row: dict[str, Any], key: str) -> int:
    if key == "callers":
        return int(row.get("callers", row.get("in_degree", 0)))
    if key == "size":
        return int(row.get("size", 0))
    if key == "address":
        return int(str(row.get("address", "0x0")), 16)
    if key == "out_degree":
        return int(row.get("out_degree", 0))
    return 0


def _default_sort(kind: str) -> str:
    if kind in {"roots"}:
        return "address"
    if kind in {"exact_duplicates"}:
        return "size"
    return "callers"


__all__ = [
    "HOTSPOT_SCHEMA",
    "build_hotspots",
    "hotspot_analysis",
]
