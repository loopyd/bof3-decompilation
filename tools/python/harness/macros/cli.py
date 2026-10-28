"""List current macros facts or check their mechanical consistency."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import add_example_argument, add_root_argument, run_main
from .inspection import KINDS, check_report, list_report
import sqlite3
from typing import Any
from .queries import macros_payload, macro_uses_payload


def add_query_commands(sub: argparse._SubParsersAction) -> None:
    macros = sub.add_parser("macros", help="list indexed existing macros and templates")
    macros.add_argument("pattern", nargs="?")
    macros.add_argument("--target")
    macros.add_argument("--classification")
    macros.set_defaults(query_handler=_query_definitions)
    uses = sub.add_parser("macro-uses", help="list indexed macro expansion sites")
    uses.add_argument("pattern", nargs="?")
    uses.add_argument("--target")
    uses.set_defaults(query_handler=_query_uses)


def _query_definitions(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return macros_payload(
        connection,
        target=args.target,
        pattern=args.pattern,
        classification=args.classification,
        limit=args.limit,
    )


def _query_uses(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return macro_uses_payload(
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
            f"macros {result['action']}: {json.dumps(result['scope'], sort_keys=True)}"
        )
        for row in result["records"]:
            print(json.dumps(row, sort_keys=True))
        for declaration in result.get("definitions", ()):
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
    parser = argparse.ArgumentParser(prog="bin/harness macros")
    add_example_argument(parser, "bin/harness macros list exe/logo --limit 10")
    actions = parser.add_subparsers(dest="action", required=True)
    for name in ("list", "check"):
        action = actions.add_parser(name)
        add_root_argument(action)
        action.add_argument("target", metavar="TARGET")
        action.add_argument("--json", action="store_true")
        if name == "list":
            action.add_argument("--kind", choices=KINDS, default="definitions")
            action.add_argument("--limit", type=int)
        else:
            action.add_argument("selectors", metavar="SELECTOR", nargs="*")
        action.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
