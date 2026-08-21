"""Query the generated cross-target reverse index."""

from __future__ import annotations

import argparse
import json
from typing import Any

from ..domain import (
    load_target_manifests,
    normalize_target_id,
    parse_function_id,
)
from ..domain.naming_debt import collect_naming_debt
from ..output import resolve_detail
from ..analysis.index import connect
from ..analysis.mission import mission_brief
from ..analysis.naming_readiness import transaction_scope
from ..analysis.priority import RANK_FIELDS, priority_rows
from ..analysis.rev_queries import (
    analyzer_candidates_payload,
    calls_payload,
    describe_payload,
    duplicates_payload,
    known_target,
    macro_opportunities_payload,
    macro_uses_payload,
    macros_payload,
    near_duplicates_payload,
    owners_payload,
    status_payload,
    symbols_payload,
    type_candidates_payload,
    type_usages_payload,
    types_payload,
    variables_payload,
    xrefs_payload,
)

from ._common import resolved_root, run_main
from ._rev_query_parsers import build_parser


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
    if args.command == "inventory":
        root = resolved_root(args)
        target = normalize_target_id(args.target).value
        manifests = load_target_manifests(root)
        if target not in manifests:
            raise ValueError(f"unknown target: {target}")
        debt = collect_naming_debt(root, manifests)
        payload: list[dict[str, Any]] = [
            {"kind": kind, "name": row.split(":", 1)[1]}
            for kind, entries in (
                ("function", debt.raw_functions),
                ("data", debt.raw_data),
            )
            for row in sorted(entries)
            if row.startswith(f"{target}:")
        ]
        _print(payload, args.json)
        return 0
    connection = connect(resolved_root(args))
    try:
        if getattr(args, "target", None):
            args.target = normalize_target_id(args.target).value
            if not known_target(connection, args.target):
                raise ValueError(f"unknown target: {args.target}")
        limit = args.limit
        if args.command == "describe":
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
        elif args.command == "transaction-scope":
            scope = transaction_scope(resolved_root(args), args.target, args.symbol)
            _print([scope], args.json, labeled=True)
            return 0
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
        elif args.command == "types":
            payload = types_payload(
                connection,
                target=getattr(args, "target", None),
                pattern=getattr(args, "pattern", None),
                untyped=getattr(args, "untyped", False),
                limit=limit,
                detail=resolve_detail(
                    requested=getattr(args, "detail", None), json_output=args.json
                ),
            )
        elif args.command == "type-uses":
            payload = type_usages_payload(
                connection,
                target=getattr(args, "target", None),
                pattern=getattr(args, "pattern", None),
                limit=limit,
            )
        elif args.command == "macros":
            payload = macros_payload(
                connection,
                target=getattr(args, "target", None),
                pattern=getattr(args, "pattern", None),
                classification=getattr(args, "classification", None),
                limit=limit,
            )
        elif args.command == "macro-uses":
            payload = macro_uses_payload(
                connection,
                target=getattr(args, "target", None),
                pattern=getattr(args, "pattern", None),
                limit=limit,
            )
        elif args.command == "macro-opportunities":
            payload = macro_opportunities_payload(
                connection,
                resolved_root(args),
                target=getattr(args, "target", None),
                kind=getattr(args, "kind", None),
                limit=limit,
            )
        elif args.command == "near-duplicates":
            payload = near_duplicates_payload(
                connection,
                resolved_root(args),
                target=getattr(args, "target", None),
                limit=limit,
            )
        elif args.command == "type-candidates":
            payload = type_candidates_payload(
                connection,
                target=getattr(args, "target", None),
                status=getattr(args, "status", None),
                kind=getattr(args, "kind", None),
                limit=limit,
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
        _print(payload, args.json, labeled=ranked and detail != "full")
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


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
