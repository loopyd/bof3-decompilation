"""List current naming facts, check their consistency or rename one spelling."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import add_example_argument, add_root_argument, run_main
from .inspection import KINDS, check_report, list_report
from .rename import apply_report, plan_report


def run(args: argparse.Namespace) -> int:
    if args.action == "rename":
        result = (
            apply_report(args.root, args.target, args.old, args.new)
            if args.apply
            else plan_report(args.root, args.target, args.old, args.new)
        )
    elif args.action == "check":
        result = check_report(args.root, args.target, tuple(args.selectors))
    else:
        result = list_report(args.root, args.target, kind=args.kind, limit=args.limit)
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0 if result["passed"] else 1
    if args.action == "rename":
        print(
            f"naming rename: {json.dumps(result['scope'], sort_keys=True)} "
            f"applied={result['applied']} passed={result['passed']}"
        )
        for row in result["edits"]:
            print(json.dumps(row, sort_keys=True))
        for row in result["renames"]:
            print(json.dumps(row, sort_keys=True))
        for row in result["proof"]:
            print(json.dumps(row, sort_keys=True))
        for row in result["refusals"]:
            print(json.dumps(row, sort_keys=True))
        return 0 if result["passed"] else 1
    print(f"naming {result['action']}: {json.dumps(result['scope'], sort_keys=True)}")
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
    for name in ("list", "check", "rename"):
        action = actions.add_parser(name)
        add_root_argument(action)
        action.add_argument("target", metavar="TARGET")
        action.add_argument("--json", action="store_true")
        if name == "list":
            action.add_argument("--kind", choices=KINDS, default="debt")
            action.add_argument("--limit", type=int)
        elif name == "check":
            action.add_argument("selectors", metavar="SELECTOR", nargs="*")
        else:
            action.add_argument("old", metavar="OLD")
            action.add_argument("new", metavar="NEW")
            action.add_argument("--apply", action="store_true")
        action.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
