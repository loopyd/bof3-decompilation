"""Validate every tracked lift has required metadata and a linked diff."""

from __future__ import annotations

import argparse
from pathlib import Path

from ..decomp_status import build_report, write_report
from ._common import add_root_argument, run_main


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    report = build_report(root)
    if args.out is not None:
        output = args.out if args.out.is_absolute() else root / args.out
        write_report(output, report)
    for target in report["targets"]:
        for function in target["functions"]:
            if function["status"] == "invalid":
                print(f"{function['source']}: {function['reason']}")
    lifts = report["lifts"]
    print(
        f"lifts: exact={lifts['exact']} partial={lifts['partial']} "
        f"invalid={lifts['invalid']}"
    )
    return 2 if lifts["invalid"] else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="validate-sources")
    add_root_argument(parser)
    parser.add_argument("-o", "--out", type=Path, help="write a JSON audit report")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
