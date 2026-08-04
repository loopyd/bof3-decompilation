"""Rebuild the generated cross-target reverse index."""

from __future__ import annotations

import argparse

from ..reverse_index import rebuild
from ._common import add_root_argument, run_main


def run(args: argparse.Namespace) -> int:
    path = rebuild(args.root.resolve())
    print(path.relative_to(args.root.resolve()))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="index")
    add_root_argument(parser)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
