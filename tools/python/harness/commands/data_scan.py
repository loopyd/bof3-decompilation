"""Report unlabeled in-image data regions referenced by lifted functions."""

from __future__ import annotations

import argparse
import json

from ..analysis.data import collect_unlabeled_regions
from ._common import add_root_argument, run_main


def run(args: argparse.Namespace) -> int:
    report = collect_unlabeled_regions(
        args.root.resolve(), args.targets, lifted_only=not args.all
    )
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
        return 0
    for target, regions in report.items():
        for region in regions:
            print(
                f"{target} {region['start']}..{region['end']} "
                f"{region['class']} refs={region['refs']} "
                f"type_candidates={len(region['type_candidates'])}"
            )
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="data-scan")
    add_root_argument(parser)
    parser.add_argument("targets", nargs="*")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--all", action="store_true", help="include unlifted functions")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
