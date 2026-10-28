"""List current types facts or check their mechanical consistency."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import add_example_argument, add_root_argument, run_main
from .inspection import KINDS, check_report, list_report
import sqlite3
from typing import Any
from harness.output import add_detail_argument, resolve_detail
from .queries import types_payload, type_usages_payload


def add_query_commands(sub: argparse._SubParsersAction) -> None:
    types = sub.add_parser("types", help="list target-owned type declarations")
    types.add_argument("pattern", nargs="?")
    types.add_argument("--target")
    types.add_argument("--untyped", action="store_true")
    add_detail_argument(types)
    types.set_defaults(query_handler=_query_declarations)
    uses = sub.add_parser("type-uses", help="list indexed type use sites")
    uses.add_argument("pattern", nargs="?")
    uses.add_argument("--target")
    add_detail_argument(uses)
    uses.set_defaults(query_handler=_query_uses)


def _query_declarations(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return types_payload(
        connection,
        target=args.target,
        pattern=args.pattern,
        untyped=args.untyped,
        limit=args.limit,
        detail=resolve_detail(requested=args.detail, json_output=args.json),
    )


def _query_uses(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return type_usages_payload(
        connection, target=args.target, pattern=args.pattern, limit=args.limit
    )


def run(args: argparse.Namespace) -> int:
    result = (
        check_report(args.root, args.target, tuple(args.selectors))
        if args.action == "check"
        else list_report(args.root, args.target, kind=args.kind, limit=args.limit)
    )
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(
            f"types {result['action']}: {json.dumps(result['scope'], sort_keys=True)}"
        )
        for row in result["records"]:
            print(json.dumps(row, sort_keys=True))
        for declaration in result.get("declarations", ()):
            print(json.dumps(declaration, sort_keys=True))
        if "consumers" in result:
            print("consumers=" + json.dumps(result["consumers"]))
        for finding in result["findings"]:
            print(json.dumps(finding, sort_keys=True))
        print(
            f"records={result['total']} truncated={result['truncated']} "
            f"incomplete={result['incomplete']} passed={result['passed']}"
        )
    return 0 if result["passed"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness types")
    add_example_argument(parser, "bin/harness types list exe/logo --limit 10")
    actions = parser.add_subparsers(dest="action", required=True)
    for name in ("list", "check"):
        action = actions.add_parser(name)
        add_root_argument(action)
        action.add_argument("target", metavar="TARGET")
        action.add_argument("--json", action="store_true")
        if name == "list":
            action.add_argument("--kind", choices=KINDS, default="declarations")
            action.add_argument("--limit", type=int)
        else:
            action.add_argument("selectors", metavar="SELECTOR", nargs="*")
        action.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
