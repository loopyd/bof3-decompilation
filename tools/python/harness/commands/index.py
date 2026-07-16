"""Rebuild the generated cross-target reverse index."""

from __future__ import annotations

import argparse
from pathlib import Path

from ..io import repo_layout
from ..reverse_index import rebuild
from ._common import run_main


def run(args: argparse.Namespace) -> int:
    path = rebuild(args.root.resolve())
    print(path.relative_to(args.root.resolve()))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="index")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
