"""List authored function metadata or check it against a fresh native comparison."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import add_root_argument, run_main
from harness.decomp.metadata import report


def run(args: argparse.Namespace) -> int:
    result = report(
        args.root,
        args.target,
        check=args.action == "check",
        selectors=tuple(getattr(args, "selectors", ())),
        limit=getattr(args, "limit", None),
    )
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(f"metadata {result['action']}: {result['scope']['target']}")
        print("scope: " + (", ".join(result["scope"]["selectors"]) or "whole target"))
        print("consumers: " + (", ".join(result["consumers"]) or "none"))
        for row in result["records"]:
            claim = row["claim"] or {}
            measured = row["measurement"]
            print(
                f"{row['selector'] or '?'} {row['source']}:{row['line'] or '?'} "
                f"claim={claim.get('status') or 'unrecorded'} "
                f"match={claim.get('match_percent')} residual={claim.get('residual')} "
                f"measurement={measured['state']} status={measured.get('status')} "
                f"match={measured.get('match_percent')} "
                f"original_size={measured.get('original_size')} "
                f"current_size={measured.get('current_size')}"
            )
            if claim.get("behavior"):
                print("  " + claim["behavior"])
        for finding in result["findings"]:
            print(f"{finding['status']}: {finding['source']}: {finding['message']}")
        print(
            f"records={result['total']} truncated={result['truncated']} "
            f"incomplete={result['incomplete']} passed={result['passed']}"
        )
    return 0 if result["passed"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness source metadata")
    actions = parser.add_subparsers(dest="action", required=True)
    for name in ("list", "check"):
        action = actions.add_parser(name)
        add_root_argument(action)
        action.add_argument("target", metavar="TARGET")
        if name == "list":
            action.add_argument("--limit", type=int)
        else:
            action.add_argument("selectors", metavar="SELECTOR", nargs="*")
        action.add_argument("--json", action="store_true")
        action.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
