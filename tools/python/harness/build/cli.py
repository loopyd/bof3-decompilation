"""Internal build adapters for metadata-owned compiler output."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

from harness.common.cli import add_root_argument, run_main

from .translation import prepare_translation


def run_translation(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    source = args.source.resolve()
    sys.stdout.write(prepare_translation(root, source, sys.stdin.read()))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="build-adapter")
    add_root_argument(parser)
    commands = parser.add_subparsers(dest="command", required=True)
    translation = commands.add_parser("translation")
    translation.add_argument("source", type=Path)
    translation.set_defaults(handler=run_translation)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
