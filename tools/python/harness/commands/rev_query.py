"""Query the generated cross-target reverse index."""

from __future__ import annotations

import argparse
import json
from typing import Any

from harness.analysis.index import connect, connect_status
from harness.analysis.mission import mission_brief
from harness.analysis.priority import RANK_FIELDS, priority_rows
from harness.analysis.rev_queries import (
    analyzer_candidates_payload,
    calls_payload,
    describe_payload,
    duplicates_payload,
    known_target,
    owners_payload,
    status_payload,
    symbols_payload,
    variables_payload,
    xrefs_payload,
)
from harness.common.cli import (
    add_example_argument,
    add_root_argument,
    resolved_root,
    run_main,
)
from harness.domain.ids import (
    FUNCTION_ID_FORMAT,
    FUNCTION_ID_HELP,
    normalize_target_id,
    parse_function_id,
)
from harness.domain.manifests import load_target_manifests
from harness.macros.cli import add_query_commands as add_macro_queries
from harness.naming.cli import add_query_commands as add_naming_queries
from harness.output import add_detail_argument, resolve_detail
from harness.types.cli import add_query_commands as add_type_queries


def _project_rows(
    payload: list[dict[str, Any]], *, command: str, detail: str
) -> list[dict[str, Any]]:
    if detail == "full":
        return payload
    fields = RANK_FIELDS[detail].get(command, RANK_FIELDS[detail]["default"])
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
    if args.command == "status":
        connection = connect_status(resolved_root(args))
        try:
            payload = status_payload(connection)
            _print(payload, args.json)
        finally:
            connection.close()
        return 0
    if not getattr(args, "query_requires_index", True):
        payload = args.query_handler(args, None)
        _print(payload, args.json, labeled=getattr(args, "query_labeled", False))
        return 0
    connection = connect(resolved_root(args))
    try:
        if getattr(args, "target", None):
            args.target = normalize_target_id(args.target).value
            if not known_target(connection, args.target):
                raise ValueError(f"unknown target: {args.target}")
        limit = args.limit
        if getattr(args, "query_handler", None) is not None:
            payload = args.query_handler(args, connection)
        elif args.command == "describe":
            function = parse_function_id(args.function)
            root = resolved_root(args)
            manifests = load_target_manifests(root)
            payload = describe_payload(
                connection,
                function,
                root=root,
                manifests=manifests,
                limit=limit,
            )
        elif args.command == "symbols":
            payload = symbols_payload(
                connection, getattr(args, "pattern", None), limit=limit
            )
        elif args.command == "xrefs":
            function = parse_function_id(args.function)
            payload = xrefs_payload(connection, function, limit=limit)
        elif args.command == "owners":
            function = parse_function_id(args.function)
            payload = owners_payload(connection, function, limit=limit)
        elif args.command == "duplicates":
            payload = duplicates_payload(
                connection,
                target=args.target,
                function=getattr(args, "function", None),
                unlifted=getattr(args, "unlifted", False),
                include_trivial=getattr(args, "include_trivial", False),
            )
            payload = payload[:limit] if limit else payload
        elif args.command == "analyzer-candidates":
            payload = analyzer_candidates_payload(
                connection,
                target=args.target,
                function=getattr(args, "function", None),
                unlifted=getattr(args, "unlifted", False),
            )
            payload = payload[:limit] if limit else payload
        elif args.command in {"metrics", "hotspots", "leafs", "quick-wins", "pareto"}:
            payload = priority_rows(
                connection,
                target=getattr(args, "target", None),
                command=args.command,
                limit=args.limit,
                exclusions=getattr(args, "exclusions", False),
                include_trivial=getattr(args, "include_trivial", False),
                unlifted=getattr(args, "unlifted", False),
                function=getattr(args, "function", None),
                root=resolved_root(args),
            )
        elif args.command == "calls":
            function_id = str(parse_function_id(args.function))
            payload = calls_payload(connection, function_id, limit=limit)
        elif args.command == "variables":
            payload = variables_payload(
                connection, getattr(args, "pattern", None), limit=limit
            )
        else:  # status
            payload = status_payload(connection)
        detail = "full"
        ranked = args.command in {
            "metrics",
            "hotspots",
            "leafs",
            "quick-wins",
            "pareto",
            "duplicates",
            "analyzer-candidates",
        }
        if ranked:
            detail = (
                "full"
                if getattr(args, "exclusions", False)
                else resolve_detail(requested=args.detail, json_output=args.json)
            )
            payload = _project_rows(payload, command=args.command, detail=detail)
        _print(
            payload,
            args.json,
            labeled=getattr(args, "query_labeled", False)
            or ranked
            and detail != "full",
        )
    finally:
        connection.close()
    return 0


def _print_mission(brief: dict[str, Any]) -> None:
    metrics = brief["metrics"]
    risk = brief["risk"]
    print(f"mission {brief['function']} (space={brief['psyq_space']})")
    print(
        f"  source: {brief['source']} "
        f"(exists={brief['source_exists']}, lifted={brief['lifted']})"
    )
    print(f"  splat asm: {brief['splat_asm']} (exists={brief['splat_asm_exists']})")
    print(
        f"  insn={metrics['instruction_count']} cc={metrics['cyclomatic_complexity']} "
        f"loops={metrics['loops']} bb={metrics['basic_blocks']} "
        f"callers={metrics['unique_callers']} callees={metrics['unique_callees']} "
        f"leaf={metrics['leaf_status']} dup_leverage={metrics['duplicate_leverage']}"
    )
    print(
        f"  risk: unresolved_calls={risk['unresolved_calls']} "
        f"metric_missing={risk['metric_missing']} confidence={risk['confidence_band']}"
    )
    if brief["sdk_callees"]:
        names = ", ".join(f"{c['name']}@{c['address']}" for c in brief["sdk_callees"])
        print(f"  SDK callees: {names}")
    if brief["sdk_unresolved"]:
        names = ", ".join(
            f"{c['name']}@{c['address']}" for c in brief["sdk_unresolved"]
        )
        print(f"  SDK unresolved: {names}")
    if brief["callers"]:
        print(
            f"  callers ({len(brief['callers'])}): "
            + ", ".join(str(c["caller"]) for c in brief["callers"][:12])
        )
    if brief["callees"]:
        print(
            f"  callees ({len(brief['callees'])}): "
            + ", ".join(str(c["callee"]) for c in brief["callees"][:12])
        )
    if brief["duplicate_group"]:
        print(
            f"  duplicate group ({len(brief['duplicate_group'])}): "
            + ", ".join(brief["duplicate_group"])
        )


def run_mission(args: argparse.Namespace) -> int:
    """Compose and print one function's mission brief."""
    brief = mission_brief(resolved_root(args), args.function)
    if args.json:
        print(json.dumps(brief, indent=2, sort_keys=True))
    else:
        _print_mission(brief)
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
    describe = sub.add_parser(
        "describe",
        help="describe target-qualified payload, Splat, symbol, and references",
    )
    describe.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    symbols = sub.add_parser("symbols", help="find canonical target-local symbols")
    symbols.add_argument("pattern", nargs="?")
    xrefs = sub.add_parser(
        "xrefs", help="find target-local indexed references to an address"
    )
    xrefs.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    add_naming_queries(sub)
    owners = sub.add_parser(
        "owners",
        help="find other indexed images with function bytes covering an address",
    )
    owners.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    calls = sub.add_parser("calls", help="show calls to or from a function selector")
    calls.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    variables = sub.add_parser("variables", help="list mapped data symbols")
    variables.add_argument("pattern", nargs="?")
    add_type_queries(sub)
    add_macro_queries(sub)
    ranked = (
        ("metrics", "show raw and derived function metrics"),
        ("quick-wins", "rank low-effort, high-leverage candidates"),
        ("hotspots", "rank high-impact functions"),
        ("leafs", "show SCC-aware leaf candidates"),
        ("pareto", "show nondominated effort/value candidates"),
        ("duplicates", "show exact duplicate groups"),
        (
            "analyzer-candidates",
            "show unconfirmed analyzer-equality candidate groups",
        ),
    )
    for name, help_text in ranked:
        command = sub.add_parser(name, help=help_text)
        command.add_argument("--json", action="store_true", default=argparse.SUPPRESS)
        command.add_argument("--limit", type=nonnegative, default=argparse.SUPPRESS)
        add_detail_argument(command)
        command.add_argument("--target")
        if name not in {"duplicates", "analyzer-candidates"}:
            command.add_argument(
                "--exclusions",
                action="store_true",
                help="show candidate rows rejected by canonical-code checks",
            )
        command.add_argument("--unlifted", action="store_true")
        command.add_argument(
            "--include-trivial",
            action="store_true",
            help="include classified return-only stubs",
        )
        if name in {"metrics", "duplicates", "analyzer-candidates"}:
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
