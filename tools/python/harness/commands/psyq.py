"""Narrow compatibility surface for Psy-Q signature evidence."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

from ..io import repo_layout
from ..psyq.signatures import write_calls, write_index
from ._common import run_main


def _require_all(args: argparse.Namespace) -> None:
    if not args.all:
        raise ValueError("this command scans every manifest; pass --all")


def run_scan(args: argparse.Namespace) -> int:
    _require_all(args)
    payload = write_index(args.root.resolve())
    print(f"targets={len(payload['targets'])} matches={len(payload['matches'])}")
    return 0


def run_calls(args: argparse.Namespace) -> int:
    _require_all(args)
    payload = write_calls(args.root.resolve())
    print(f"calls={len(payload['calls'])}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness psyq")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    sub = parser.add_subparsers(dest="command", required=True)
    for name, handler, help_text in (
        ("scan", run_scan, "scan all manifests against complete Psy-Q object signatures"),
        ("calls", run_calls, "join Rizin call xrefs with generated Psy-Q signatures"),
    ):
        command = sub.add_parser(name, help=help_text)
        command.add_argument("--all", action="store_true", help="operate on every target manifest")
        command.set_defaults(handler=handler)
    return parser


def main(argv: list[str] | None = None) -> int:
    arguments = sys.argv[1:] if argv is None else argv
    if arguments == ["--example"]:
        print("bin/harness psyq scan --all\nbin/harness psyq calls --all")
        return 0
    return run_main(build_parser, arguments)


if __name__ == "__main__":
    raise SystemExit(main())
