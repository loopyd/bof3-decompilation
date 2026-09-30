"""Validate every tracked lift has required metadata and a linked diff.

The native exact/partial comparison belongs to ``bin/harness lift status``. This
command is the cheap metadata/ownership/link gate: it never compiles a lift, so
it stays usable as a fast ``just check`` gate.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from harness.common.cli import add_root_argument, run_main

from ..decomp.status import build_metadata_report, write_report


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    report = build_metadata_report(root)
    if args.out is not None:
        output = args.out if args.out.is_absolute() else root / args.out
        write_report(output, report)
    for target in report["targets"]:
        for function in target["functions"]:
            if function["status"] == "invalid":
                print(f"{function['source']}: {function['reason']}")
    lifts = report["lifts"]
    print(f"lifts: valid={lifts['valid']} invalid={lifts['invalid']}")
    return 2 if lifts["invalid"] else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness source validate")
    add_root_argument(parser)
    parser.add_argument("-o", "--out", type=Path, help="write a JSON audit report")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
