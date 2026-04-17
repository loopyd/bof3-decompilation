from __future__ import annotations

import argparse
from pathlib import Path

from ..paths import repo_layout
from ..toolchain.aspsx import ALL_ASPSX_PSYQ_VERSIONS, download_aspsx_binaries
from ..toolchain.setup_psyq import find_psyq_source, stage_psyq_sdk
from ._common import run_main


def run_psyq_find(args: argparse.Namespace) -> int:
    source = find_psyq_source(source_root=args.source_root, archive=args.archive)
    if source is None:
        print("not found")
        return 1
    print(f"{source.kind}: {source.path}")
    return 0


def run_psyq_setup(args: argparse.Namespace) -> int:
    dest = stage_psyq_sdk(
        dest=args.dest,
        source_root=args.source_root,
        archive=args.archive,
        force=args.force,
    )
    print(f"staged: {dest}")
    return 0


def run_aspsx_download(args: argparse.Namespace) -> int:
    result = download_aspsx_binaries(
        repo_layout(),
        versions=ALL_ASPSX_PSYQ_VERSIONS if args.all_versions else None,
        force=args.force,
    )
    print(f"downloaded: {result.root}")
    print(f"versions: {', '.join(result.versions)}")
    return 0


def configure_psyq_find_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--source-root", type=Path)
    parser.add_argument("--archive", type=Path)
    parser.set_defaults(handler=run_psyq_find)


def configure_psyq_setup_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--source-root", type=Path)
    parser.add_argument("--archive", type=Path)
    parser.add_argument("--dest", type=Path, required=True)
    parser.add_argument("--force", action="store_true")
    parser.set_defaults(handler=run_psyq_setup)


def configure_aspsx_download_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--all-versions", action="store_true")
    parser.add_argument("--force", action="store_true")
    parser.set_defaults(handler=run_aspsx_download)


def configure_root_parser(parser: argparse.ArgumentParser) -> None:
    subparsers = parser.add_subparsers(required=True)

    psyq = subparsers.add_parser("psyq")
    psyq_subparsers = psyq.add_subparsers(required=True)

    psyq_find = psyq_subparsers.add_parser("find")
    configure_psyq_find_parser(psyq_find)

    psyq_setup = psyq_subparsers.add_parser("setup")
    configure_psyq_setup_parser(psyq_setup)

    aspsx = subparsers.add_parser("aspsx")
    aspsx_subparsers = aspsx.add_subparsers(required=True)

    aspsx_download = aspsx_subparsers.add_parser("download")
    configure_aspsx_download_parser(aspsx_download)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="toolchain")
    configure_root_parser(parser)
    return parser


def add_legacy_parser(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    parser = subparsers.add_parser("toolchain")
    configure_root_parser(parser)


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
