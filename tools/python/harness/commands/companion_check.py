"""Evidence gate CLI for a caller's declared EMI companion static calls."""

from __future__ import annotations

import argparse
import json

from ..domain import FUNCTION_ID_HELP
from ..emi.companions import build_companion_report
from ._common import add_root_argument, run_main


def run(args: argparse.Namespace) -> int:
    report = build_companion_report(args.root.resolve(), args.function)
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["ready_to_lift"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/companion-check",
        description="report whether companion evidence makes one lift safe",
    )
    parser.add_argument("function", help=FUNCTION_ID_HELP)
    add_root_argument(parser)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
