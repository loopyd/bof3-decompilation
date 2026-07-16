"""Stage the PsyQ 4.7 build-header baseline."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

from ..io import repo_layout
from ..toolchain.setup_psyq import DEFAULT_PSYQ_VERSION, import_psyq_sdk
from ._common import run_main


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    layout = repo_layout(root)
    destination = args.dest or layout.psyq_root
    staged = import_psyq_sdk(
        dest=destination,
        archive=args.archive,
        archive_url=args.archive_url,
        private_assets_root=args.private_root or layout.private_assets_dir,
        version=args.version,
        force=args.force,
    )
    print(staged)
    return 0


def build_parser() -> argparse.ArgumentParser:
    layout = repo_layout()
    parser = argparse.ArgumentParser(prog="psyq-import")
    parser.add_argument("--root", type=Path, default=layout.root)
    parser.add_argument("--version", default=DEFAULT_PSYQ_VERSION)
    parser.add_argument("--archive", type=Path)
    parser.add_argument("--archive-url")
    parser.add_argument("--dest", type=Path)
    parser.add_argument("--private-root", type=Path)
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--example", action="store_true")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    argv = sys.argv[1:] if argv is None else argv
    if argv == ["--example"]:
        print("bin/psyq-import --archive inputs/external/psyq-4.7.zip")
        return 0
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
