"""Query the generated cross-target reverse index."""

from __future__ import annotations

import argparse
from collections import defaultdict
import json
from pathlib import Path
import sys
from typing import Any

from ..domain import normalize_target_id, parse_function_id
from ..io import repo_layout
from ..output import add_detail_argument, resolve_detail
from ..reverse_index import connect, rows
from ._common import run_main


def _root(args: argparse.Namespace) -> Path:
    return args.root.resolve()


_RANK_FIELDS = {
    "minimal": {
        "duplicates": (
            "representative",
            "members",
            "unlifted_members",
            "estimated_saved_instructions",
        ),
        "default": (
            "id",
            "instruction_count",
            "cyclomatic_complexity",
            "unique_callers",
            "duplicate_leverage",
            "leaf_status",
            "lifted",
        ),
    },
    "normal": {
        "duplicates": (
            "representative",
            "size",
            "members",
            "unlifted_members",
            "targets",
            "estimated_saved_instructions",
            "functions",
        ),
        "default": (
            "id",
            "size",
            "instruction_count",
            "basic_blocks",
            "cyclomatic_complexity",
            "loops",
            "unique_callers",
            "unique_callees",
            "unresolved_calls",
            "duplicate_leverage",
            "leaf_status",
            "lifted",
            "metric_missing",
        ),
    },
}


def _project_rows(
    payload: list[dict[str, Any]], *, command: str, detail: str
) -> list[dict[str, Any]]:
    if detail == "full":
        return payload
    fields = _RANK_FIELDS[detail].get(command, _RANK_FIELDS[detail]["default"])
    return [{key: row[key] for key in fields if key in row} for row in payload]


def _print(
    payload: list[dict[str, object]], as_json: bool, *, labeled: bool = False
) -> None:
    if as_json:
        print(json.dumps(payload, indent=2, sort_keys=True))
        return
    for row in payload:
        if labeled:
            print(" ".join(f"{key}={value}" for key, value in row.items()))
        else:
            print("\t".join(str(value) for value in row.values()))


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


def _priority_rows(connection, args: argparse.Namespace) -> list[dict[str, Any]]:
    def cost(row: dict[str, Any], name: str) -> int:
        value = row[name]
        return value if value is not None else sys.maxsize

    payload = _function_metrics(connection, args.target)
    _enrich_graph(connection, payload)
    if getattr(args, "function", None):
        payload = [row for row in payload if row["id"] == args.function]
    if args.command != "metrics":
        payload = [row for row in payload if row["reviewed"] and row["size"] >= 8]
        if not args.include_trivial:
            payload = [row for row in payload if row["trivial_kind"] is None]
    if args.unlifted:
        payload = [row for row in payload if not row["lifted"]]
    if args.command == "leafs":
        payload = [row for row in payload if row["leaf_status"] != "non_leaf"]
        payload.sort(
            key=lambda row: (
                row["leaf_status"] != "analyzer_no_edge",
                row["instruction_count"],
                row["id"],
            )
        )
    elif args.command == "quick-wins":
        payload.sort(
            key=lambda row: (
                row["leaf_status"] != "analyzer_no_edge",
                row["metric_missing"],
                row["unresolved_calls"],
                cost(row, "cyclomatic_complexity"),
                cost(row, "loops"),
                row["instruction_count"],
                -row["duplicate_leverage"],
                -row["unique_callers"],
                row["id"],
            )
        )
    elif args.command == "hotspots":
        payload.sort(
            key=lambda row: (
                -row["unique_callers"],
                -row["caller_callsites"],
                -row["duplicate_leverage"],
                row["metric_missing"],
                row["unresolved_calls"],
                cost(row, "cyclomatic_complexity"),
                row["instruction_count"],
                row["id"],
            )
        )
    elif args.command == "pareto":
        payload = [row for row in payload if not row["metric_missing"]]
        payload = [
            row
            for row in payload
            if not any(_dominates(other, row) for other in payload if other is not row)
        ]
        payload.sort(
            key=lambda row: (
                -row["unique_callers"],
                -row["duplicate_leverage"],
                row["instruction_count"],
                row["id"],
            )
        )
    if args.command != "metrics":
        unique: list[dict[str, Any]] = []
        seen: set[tuple[str, int]] = set()
        for row in payload:
            identity = (row["exact_sha256"], row["size"])
            if identity in seen:
                continue
            seen.add(identity)
            unique.append(row)
        payload = unique
    return payload[: args.limit] if args.limit else payload


def run_query(args: argparse.Namespace) -> int:
    connection = connect(_root(args))
    try:
        if getattr(args, "target", None):
            args.target = normalize_target_id(args.target).value
            if (
                connection.execute(
                    "SELECT 1 FROM targets WHERE id = ?", (args.target,)
                ).fetchone()
                is None
            ):
                raise ValueError(f"unknown target: {args.target}")
        if getattr(args, "function", None):
            args.function = str(parse_function_id(args.function))
        limit = args.limit
        sql_limit = -1 if limit == 0 else limit
        if args.command == "symbols":
            pattern = f"%{args.pattern}%" if args.pattern else "%"
            payload = rows(
                connection,
                "SELECT target_id, printf('0x%08X', address) AS address, name, kind FROM symbols WHERE name LIKE ? ORDER BY target_id, address, name LIMIT ?",
                (pattern, sql_limit),
            )
        elif args.command == "xrefs":
            function = parse_function_id(args.function)
            payload = rows(
                connection,
                "SELECT target_id, printf('0x%08X', source) AS source, printf('0x%08X', destination) AS destination, kind FROM xrefs WHERE target_id = ? AND destination = ? ORDER BY source LIMIT ?",
                (function.target.value, function.address, sql_limit),
            )
        elif args.command == "duplicates":
            metrics = _function_metrics(connection, None)
            grouped: dict[tuple[str, int], list[dict[str, Any]]] = defaultdict(list)
            for row in metrics:
                if row["duplicate_members"] > 1:
                    grouped[(row["exact_sha256"], row["size"])].append(row)
            payload = []
            for (digest, size), members in sorted(grouped.items()):
                members.sort(key=lambda row: row["id"])
                if args.target and not any(
                    row["target"] == args.target for row in members
                ):
                    continue
                if args.function and args.function not in {
                    row["id"] for row in members
                }:
                    continue
                unlifted = [row for row in members if not row["lifted"]]
                if args.unlifted and not unlifted:
                    continue
                representative = min(
                    members,
                    key=lambda row: (
                        -row["reviewed"],
                        -row["lifted"],
                        -row["unique_callers"],
                        row["id"],
                    ),
                )
                payload.append(
                    {
                        "hash": digest,
                        "size": size,
                        "members": len(members),
                        "unlifted_members": len(unlifted),
                        "targets": len({row["target"] for row in members}),
                        "representative": representative["id"],
                        "estimated_saved_instructions": representative[
                            "instruction_count"
                        ]
                        * max(
                            0, len(unlifted) - (0 if representative["lifted"] else 1)
                        ),
                        "functions": [row["id"] for row in members],
                        "score_version": "reverse-priority/v1",
                    }
                )
            payload.sort(
                key=lambda row: (
                    -row["estimated_saved_instructions"],
                    -row["unlifted_members"],
                    -row["members"],
                    row["hash"],
                )
            )
            payload = payload[:limit] if limit else payload
        elif args.command in {"metrics", "hotspots", "leafs", "quick-wins", "pareto"}:
            payload = _priority_rows(connection, args)
        elif args.command == "calls":
            payload = rows(
                connection,
                "SELECT caller, callee, printf('0x%08X', callsite) AS callsite FROM calls WHERE caller = ? OR callee = ? ORDER BY caller, callsite LIMIT ?",
                (args.function, args.function, sql_limit),
            )
        elif args.command == "variables":
            pattern = f"%{args.pattern}%" if args.pattern else "%"
            payload = rows(
                connection,
                "SELECT target_id, printf('0x%08X', address) AS address, name FROM symbols WHERE kind = 'data' AND name LIKE ? ORDER BY target_id, address LIMIT ?",
                (pattern, sql_limit),
            )
        else:  # status
            payload = rows(
                connection,
                "SELECT t.id, t.engine, COUNT(DISTINCT f.id) AS functions, COUNT(DISTINCT s.address) AS symbols FROM targets t LEFT JOIN functions f ON f.target_id = t.id LEFT JOIN symbols s ON s.target_id = t.id GROUP BY t.id ORDER BY t.id",
            )
        detail = "full"
        ranked = args.command in {
            "metrics",
            "hotspots",
            "leafs",
            "quick-wins",
            "pareto",
            "duplicates",
        }
        if ranked:
            detail = resolve_detail(requested=args.detail, json_output=args.json)
            payload = _project_rows(payload, command=args.command, detail=detail)
        _print(payload, args.json, labeled=ranked and detail != "full")
    finally:
        connection.close()
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="rev-query")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.add_argument("--json", action="store_true")

    def nonnegative(value: str) -> int:
        parsed = int(value)
        if parsed < 0:
            raise argparse.ArgumentTypeError("must be nonnegative")
        return parsed

    parser.add_argument(
        "--limit", type=nonnegative, default=20, help="maximum rows; 0 means all"
    )
    sub = parser.add_subparsers(dest="command", required=True)
    symbols = sub.add_parser("symbols", help="find canonical target-local symbols")
    symbols.add_argument("pattern", nargs="?")
    xrefs = sub.add_parser(
        "xrefs", help="find target-local indexed references to an address"
    )
    xrefs.add_argument("function", metavar="TARGET@ADDRESS")
    calls = sub.add_parser("calls", help="show calls to or from TARGET@ADDRESS")
    calls.add_argument("function")
    variables = sub.add_parser("variables", help="list mapped data symbols")
    variables.add_argument("pattern", nargs="?")
    ranked = (
        ("metrics", "show raw and derived function metrics"),
        ("quick-wins", "rank low-effort, high-leverage candidates"),
        ("hotspots", "rank high-impact functions"),
        ("leafs", "show SCC-aware leaf candidates"),
        ("pareto", "show nondominated effort/value candidates"),
        ("duplicates", "show exact duplicate groups"),
    )
    for name, help_text in ranked:
        command = sub.add_parser(name, help=help_text)
        command.add_argument("--json", action="store_true", default=argparse.SUPPRESS)
        command.add_argument("--limit", type=nonnegative, default=argparse.SUPPRESS)
        add_detail_argument(command)
        command.add_argument("--target")
        command.add_argument("--unlifted", action="store_true")
        if name != "duplicates":
            command.add_argument(
                "--include-trivial",
                action="store_true",
                help="include classified return-only stubs in rankings",
            )
        if name in {"metrics", "duplicates"}:
            command.add_argument("function", nargs="?")
    sub.add_parser("status", help="show index coverage")
    for command in sub.choices.values():
        command.set_defaults(handler=run_query)
    return parser


def main(argv: list[str] | None = None) -> int:
    arguments = sys.argv[1:] if argv is None else argv
    if arguments == ["--example"]:
        print("bin/rev-query symbols func_")
        return 0
    return run_main(build_parser, arguments)


if __name__ == "__main__":
    raise SystemExit(main())
