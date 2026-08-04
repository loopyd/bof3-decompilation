"""Query the generated cross-target reverse index."""

from __future__ import annotations

import argparse
from collections import defaultdict
import json
from typing import Any

from ..domain import (
    FUNCTION_ID_FORMAT,
    FUNCTION_ID_HELP,
    normalize_target_id,
    parse_function_id,
)
from ..output import add_detail_argument, resolve_detail
from ..reverse_index import connect, rows

from ._common import add_example_argument, add_root_argument, run_main

from ._rev_query_graph import _function_metrics, _root
from ._rev_query_mission import run_mission
from ._rev_query_priority import _RANK_FIELDS, _priority_rows

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
            payload = _priority_rows(connection, args, root=_root(args))
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
            detail = (
                "full"
                if getattr(args, "exclusions", False)
                else resolve_detail(requested=args.detail, json_output=args.json)
            )
            payload = _project_rows(payload, command=args.command, detail=detail)
        _print(payload, args.json, labeled=ranked and detail != "full")
    finally:
        connection.close()
    return 0

def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="rev-query")
    add_root_argument(parser)
    add_example_argument(parser, "bin/rev-query symbols func_")
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
    xrefs.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    calls = sub.add_parser("calls", help="show calls to or from a function selector")
    calls.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
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
        if name != "duplicates":
            command.add_argument(
                "--exclusions",
                action="store_true",
                help="show candidate rows rejected by canonical-code checks",
            )
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
    mission = sub.add_parser("mission", help="compose a single-function lifting brief")
    mission.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    mission.add_argument("--json", action="store_true", default=argparse.SUPPRESS)
    mission.set_defaults(handler=run_mission)
    return parser

def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)

if __name__ == "__main__":
    raise SystemExit(main())
