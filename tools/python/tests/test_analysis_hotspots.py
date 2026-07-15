"""Regression tests for the hotspot ranking behavior."""

from __future__ import annotations

import json
from pathlib import Path

import pytest

from harness.analysis.hotspots import build_hotspots, hotspot_analysis
from harness.analysis.graph import (
    GRAPH_SCHEMA,
    AnalysisGraph,
    GraphCall,
    GraphFunction,
    GraphUnresolvedCall,
)


def _make_graph(
    functions: list[GraphFunction],
    calls: list[GraphCall] | None = None,
    unresolved: list[GraphUnresolvedCall] | None = None,
    duplicates: list[list[str]] | None = None,
) -> AnalysisGraph:
    return AnalysisGraph(
        schema=GRAPH_SCHEMA,
        engine={"name": "rizin", "version": "test"},
        targets_analyzed=["emi/etc/game/00"],
        targets_skipped=[],
        functions=functions,
        calls=calls or [],
        unresolved_calls=unresolved or [],
        duplicate_groups=duplicates or [],
        snapshot_paths={},
    )


def _func(
    address: int,
    *,
    target: str = "emi/etc/game/00",
    size: int = 0x40,
    reviewed: bool = False,
    lifted: bool = False,
) -> GraphFunction:
    return GraphFunction(
        id=f"{target}@{address:08x}",
        target=target,
        address=address,
        analyzer_size=size,
        is_reviewed=reviewed,
        is_lifted=lifted,
        exact_sha256="aaa",
        analyzer_name=f"func_{address:08x}",
        source_name=None,
        semantic_name=None,
        source=None,
    )


def _call(caller: int, callee: int) -> GraphCall:
    return GraphCall(
        caller=f"emi/etc/game/00@{caller:08x}",
        callee=f"emi/etc/game/00@{callee:08x}",
        callsite=caller + 0x10,
    )


def _unresolved(caller: int, target_addr: int) -> GraphUnresolvedCall:
    return GraphUnresolvedCall(
        caller=f"emi/etc/game/00@{caller:08x}",
        target_address=target_addr,
        callsite=caller + 0x10,
        kind="unknown",
    )


def test_hotspots_leaves_isolates_out_degree_zero() -> None:
    graph = _make_graph(
        functions=[_func(0x801D0C00), _func(0x801D0C80)],
        calls=[_call(0x801D0C00, 0x801D0C80)],
    )
    report = build_hotspots(graph)
    assert report["leaves"]
    assert report["leaves"][0]["name"] == "func_801d0c80"
    assert report["leaves"][0]["out_degree"] == 0
    assert report["leaves"][0]["callers"] == 1


def test_hotspots_roots_isolates_in_degree_zero() -> None:
    graph = _make_graph(
        functions=[_func(0x801D0C00), _func(0x801D0C80)],
        calls=[_call(0x801D0C00, 0x801D0C80)],
    )
    report = build_hotspots(graph)
    assert report["roots"]
    assert report["roots"][0]["name"] == "func_801d0c00"
    assert report["roots"][0]["callers"] == 0


def test_hotspots_hot_sorted_by_callers() -> None:
    graph = _make_graph(
        functions=[_func(0x801D0C00), _func(0x801D0C80), _func(0x801D0D00)],
        calls=[_call(0x801D0C00, 0x801D0D00), _call(0x801D0C80, 0x801D0D00)],
    )
    report = build_hotspots(graph)
    assert report["hot"]
    assert report["hot"][0]["name"] == "func_801d0d00"
    assert report["hot"][0]["callers"] == 2


def test_hotspots_discovery_from_unresolved_calls() -> None:
    graph = _make_graph(
        functions=[_func(0x801D0C00, reviewed=True)],
        unresolved=[_unresolved(0x801D0C00, 0x800A0000)],
    )
    report = build_hotspots(graph)
    assert report["discovery"]
    assert report["discovery"][0]["unknown_callees"] == 1


def test_hotspots_unknown_targets_from_unresolved() -> None:
    graph = _make_graph(
        functions=[_func(0x801D0C00)],
        unresolved=[_unresolved(0x801D0C00, 0x800A0000)],
    )
    report = build_hotspots(graph)
    assert report["unknown_targets"]
    assert report["unknown_targets"][0]["address"] == "0x800a0000"


def test_hotspots_rejects_negative_top(tmp_path: Path) -> None:
    graph_path = tmp_path / "out" / "analysis" / "graph.json"
    graph_path.parent.mkdir(parents=True)
    graph_path.write_text(
        json.dumps(_make_graph([]).to_dict(), indent=2),
        encoding="utf-8",
    )
    with pytest.raises(ValueError, match="--top"):
        hotspot_analysis(tmp_path, kind="leaves", top=-1)


def test_hotspots_rejects_min_size_above_max_size(tmp_path: Path) -> None:
    graph_path = tmp_path / "out" / "analysis" / "graph.json"
    graph_path.parent.mkdir(parents=True)
    graph_path.write_text(
        json.dumps(_make_graph([]).to_dict(), indent=2),
        encoding="utf-8",
    )
    with pytest.raises(ValueError, match="--min-size"):
        hotspot_analysis(tmp_path, kind="leaves", min_size=0x100, max_size=0x80)


def test_hotspots_status_reviewed_filters(tmp_path: Path) -> None:
    graph = _make_graph(
        functions=[
            _func(0x801D0C00, reviewed=True),
            _func(0x801D0C80, reviewed=False),
        ],
    )
    graph_path = tmp_path / "out" / "analysis" / "graph.json"
    graph_path.parent.mkdir(parents=True)
    graph_path.write_text(
        json.dumps(graph.to_dict(), indent=2),
        encoding="utf-8",
    )
    result = hotspot_analysis(tmp_path, kind="roots", status="reviewed")
    assert result["selection"]
    assert all(row["is_reviewed"] for row in result["selection"])


def test_hotspots_address_sort_returns_ascending(tmp_path: Path) -> None:
    graph = _make_graph(
        functions=[_func(0x801D0C80), _func(0x801D0C00)],
    )
    graph_path = tmp_path / "out" / "analysis" / "graph.json"
    graph_path.parent.mkdir(parents=True)
    graph_path.write_text(
        json.dumps(graph.to_dict(), indent=2),
        encoding="utf-8",
    )
    result = hotspot_analysis(tmp_path, kind="roots", top=10, sort="address")
    assert result["selection"]
    addrs = [row["address"] for row in result["selection"]]
    assert addrs == sorted(addrs)


def test_hotspots_exact_duplicates_from_graph() -> None:
    graph = _make_graph(
        functions=[
            _func(0x801D0C00),
            _func(0x801D0C80),
        ],
        duplicates=[["emi/etc/game/00@801d0c00", "emi/etc/game/00@801d0c80"]],
    )
    report = build_hotspots(graph)
    assert report["exact_duplicates"]
    assert len(report["exact_duplicates"][0]["functions"]) == 2


def test_hotspots_file_missing_raises(tmp_path: Path) -> None:
    with pytest.raises(FileNotFoundError, match="graph missing"):
        hotspot_analysis(tmp_path, kind="roots")
