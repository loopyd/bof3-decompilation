"""Call-graph metrics, SCCs, enrichment, and Pareto dominance for rev-query."""

from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path
from typing import Any

from ..reverse_index import rows


def _root(args: argparse.Namespace) -> Path:
    return args.root.resolve()


def _function_metrics(connection, target: str | None) -> list[dict[str, Any]]:
    where = "WHERE f.target_id = ?" if target else ""
    params = (target,) if target else ()
    payload = rows(
        connection,
        f"""
        WITH incoming AS (
            SELECT callee AS id, COUNT(*) AS callsites,
                   COUNT(DISTINCT caller) AS unique_callers
            FROM calls GROUP BY callee
        ), outgoing AS (
            SELECT caller AS id, COUNT(*) AS callsites,
                   COUNT(DISTINCT callee) AS unique_callees
            FROM calls GROUP BY caller
        ), unresolved AS (
            SELECT caller AS id, COUNT(*) AS calls
            FROM unresolved_calls GROUP BY caller
        ), duplicates AS (
            SELECT dm.function_id AS id, dg.members,
                   SUM(CASE WHEN f2.lifted = 0 THEN 1 ELSE 0 END) AS unlifted_members,
                   COUNT(DISTINCT f2.target_id) AS targets
            FROM duplicate_members dm
            JOIN duplicate_groups dg ON dg.hash = dm.hash
            JOIN duplicate_members dm2 ON dm2.hash = dm.hash
            JOIN functions f2 ON f2.id = dm2.function_id
            GROUP BY dm.function_id, dg.members
        )
        SELECT f.id, f.target_id AS target, printf('0x%08X', f.address) AS address,
               f.size, f.instruction_count, f.basic_blocks, f.cfg_edges,
               f.cyclomatic_complexity, f.loops, f.stack_frame,
               f.local_count, f.argument_count,
               f.trivial_kind,
               COALESCE(i.callsites, 0) AS caller_callsites,
               COALESCE(i.unique_callers, 0) AS unique_callers,
               COALESCE(o.callsites, 0) AS callee_callsites,
               COALESCE(o.unique_callees, 0) AS unique_callees,
               COALESCE(u.calls, 0) AS unresolved_calls,
               CAST(f.reviewed AS INTEGER) AS reviewed,
               CAST(f.lifted AS INTEGER) AS lifted, f.exact_sha256,
               COALESCE(d.members, 1) AS duplicate_members,
               COALESCE(d.unlifted_members, CASE WHEN f.lifted = 0 THEN 1 ELSE 0 END)
                   AS unlifted_duplicate_members,
               COALESCE(d.targets, 1) AS duplicate_targets
        FROM functions f
        LEFT JOIN incoming i ON i.id = f.id
        LEFT JOIN outgoing o ON o.id = f.id
        LEFT JOIN unresolved u ON u.id = f.id
        LEFT JOIN duplicates d ON d.id = f.id
        {where}
        ORDER BY f.id
        """,
        params,
    )
    for row in payload:
        row["reviewed"] = bool(row["reviewed"])
        row["lifted"] = bool(row["lifted"])
    return payload


def _sccs(nodes: list[str], edges: dict[str, set[str]]) -> list[list[str]]:
    """Return deterministic SCCs without depending on Python recursion depth."""
    reverse: dict[str, set[str]] = defaultdict(set)
    for caller, callees in edges.items():
        for callee in callees:
            reverse[callee].add(caller)
    visited: set[str] = set()
    order: list[str] = []
    for root in sorted(nodes):
        if root in visited:
            continue
        visited.add(root)
        stack = [(root, False)]
        while stack:
            node, finished = stack.pop()
            if finished:
                order.append(node)
                continue
            stack.append((node, True))
            for callee in sorted(edges.get(node, ()), reverse=True):
                if callee not in visited:
                    visited.add(callee)
                    stack.append((callee, False))
    assigned: set[str] = set()
    result: list[list[str]] = []
    for root in reversed(order):
        if root in assigned:
            continue
        assigned.add(root)
        component: list[str] = []
        stack = [root]
        while stack:
            node = stack.pop()
            component.append(node)
            for caller in sorted(reverse.get(node, ()), reverse=True):
                if caller not in assigned:
                    assigned.add(caller)
                    stack.append(caller)
        result.append(sorted(component))
    return sorted(result, key=lambda component: component[0])


def _enrich_graph(connection, metrics: list[dict[str, Any]]) -> None:
    ids = {row["id"] for row in metrics}
    edges: dict[str, set[str]] = defaultdict(set)
    for caller, callee in connection.execute(
        "SELECT caller, callee FROM calls ORDER BY caller, callee"
    ):
        if caller in ids and callee in ids:
            edges[caller].add(callee)
    components = _sccs(sorted(ids), edges)
    component_for = {
        member: component_index
        for component_index, component in enumerate(components)
        for member in component
    }
    rows_by_id = {row["id"]: row for row in metrics}
    for component_index, component in enumerate(components):
        outgoing = {
            component_for[callee]
            for member in component
            for callee in edges.get(member, ())
            if component_for[callee] != component_index
        }
        unresolved = sum(rows_by_id[member]["unresolved_calls"] for member in component)
        status = (
            "non_leaf"
            if outgoing
            else ("unresolved_edge" if unresolved else "analyzer_no_edge")
        )
        for member in component:
            row = rows_by_id[member]
            metric_missing = sum(
                row[name] is None
                for name in (
                    "basic_blocks",
                    "cfg_edges",
                    "cyclomatic_complexity",
                    "loops",
                    "stack_frame",
                    "local_count",
                    "argument_count",
                )
            )
            row.update(
                scc_id=component[0],
                scc_members=len(component),
                scc_outgoing=len(outgoing),
                leaf_status=status,
                duplicate_leverage=max(
                    0, row["unlifted_duplicate_members"] - (0 if row["lifted"] else 1)
                ),
                metric_missing=metric_missing,
                confidence_band=("partial" if metric_missing else "analyzer_only"),
                score_version="reverse-priority/v1",
            )


def _dominates(left: dict[str, Any], right: dict[str, Any]) -> bool:
    # Every Pareto dimension is emitted in the result; there is no hidden score.
    if left["metric_missing"] or right["metric_missing"]:
        return False
    better = (
        left["unique_callers"] >= right["unique_callers"]
        and left["duplicate_leverage"] >= right["duplicate_leverage"]
        and left["instruction_count"] <= right["instruction_count"]
        and left["cyclomatic_complexity"] <= right["cyclomatic_complexity"]
        and left["unresolved_calls"] <= right["unresolved_calls"]
    )
    strict = any(
        (left[key] > right[key] if maximize else left[key] < right[key])
        for key, maximize in (
            ("unique_callers", True),
            ("duplicate_leverage", True),
            ("instruction_count", False),
            ("cyclomatic_complexity", False),
            ("unresolved_calls", False),
        )
    )
    return better and strict
