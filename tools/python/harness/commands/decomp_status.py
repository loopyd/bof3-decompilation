"""Report the live matching status of every tracked C lift."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from ..decomp_status import build_report, project_report, render_text, write_report
from ..output import add_detail_argument, resolve_detail
from ._common import add_example_argument, add_root_argument, run_main


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    report = build_report(root, args.targets, use_cache=not args.no_cache)
    if args.out is not None:
        output = args.out if args.out.is_absolute() else root / args.out
        write_report(output, report)
    detail = resolve_detail(requested=args.detail, json_output=args.json)
    if args.json:
        print(json.dumps(project_report(report, detail), indent=2, sort_keys=True))
    else:
        print(render_text(report, detail))
    return 2 if report["lifts"]["invalid"] else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="decomp-status")
    add_root_argument(parser)
    add_example_argument(parser, "bin/decomp-status exe/logo")
    parser.add_argument(
        "--json",
        action="store_true",
        help="print JSON; complete unless --detail projects it",
    )
    add_detail_argument(parser)
    parser.add_argument(
        "--no-cache",
        action="store_true",
        help="recompute every lift instead of reusing disposable audit summaries",
    )
    parser.add_argument("-o", "--out", type=Path, help="write the complete JSON report")
    parser.add_argument(
        "targets", metavar="TARGET", nargs="*", help="target IDs to check"
    )
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
