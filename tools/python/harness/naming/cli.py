"""List current naming facts or check their mechanical consistency."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import add_example_argument, add_root_argument, run_main
from .inspection import KINDS, check_report, list_report


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
            f"naming {result['action']}: {json.dumps(result['scope'], sort_keys=True)}"
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
    parser = argparse.ArgumentParser(prog="bin/harness naming")
    add_example_argument(parser, "bin/harness naming list exe/logo --limit 10")
    actions = parser.add_subparsers(dest="action", required=True)
    for name in ("list", "check"):
        action = actions.add_parser(name)
        add_root_argument(action)
        action.add_argument("target", metavar="TARGET")
        action.add_argument("--json", action="store_true")
        if name == "list":
            action.add_argument("--kind", choices=KINDS, default="debt")
            action.add_argument("--limit", type=int)
        else:
            action.add_argument("selectors", metavar="SELECTOR", nargs="*")
        action.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
