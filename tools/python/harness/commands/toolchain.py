from __future__ import annotations

import argparse
from pathlib import Path

from ..io import repo_layout
from ..toolchain.setup_disc import DEFAULT_BOF3_ARCHIVE_URL, import_bof3_disc
from ..toolchain.setup_psyq import (
    DEFAULT_PSYQ_VERSION,
    default_psyq_archive_url,
    find_psyq_source,
    import_psyq_sdk,
    stage_psyq_sdk,
)
from ._common import run_main


def run_psyq_find(args: argparse.Namespace) -> int:
    source = find_psyq_source(
        source_root=args.source_root,
        archive=args.archive,
        version=args.version,
    )
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
        version=args.version,
        force=args.force,
    )
    print(f"staged: {dest}")
    return 0


def run_psyq_import(args: argparse.Namespace) -> int:
    dest = import_psyq_sdk(
        dest=args.dest,
        archive=args.archive,
        archive_url=args.archive_url,
        private_assets_root=args.private_root,
        version=args.version,
        force=args.force,
    )
    print(f"staged: {dest}")
    return 0


def run_disc_import(args: argparse.Namespace) -> int:
    result = import_bof3_disc(
        dest=args.dest,
        archive=args.archive,
        archive_url=args.archive_url or DEFAULT_BOF3_ARCHIVE_URL,
        private_assets_root=args.private_root,
        force=args.force,
    )
    print(f"archive: {result.archive_path}")
    print(f"extracted: {result.extracted_root}")
    print(f"cue: {result.cue_path}")
    print(f"staged: {', '.join(str(path) for path in result.staged_paths)}")
    return 0


def configure_psyq_find_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--version", help="PsyQ version; defaults to 4.7")
    parser.add_argument("--source-root", type=Path)
    parser.add_argument("--archive", type=Path)
    parser.set_defaults(handler=run_psyq_find)


def configure_psyq_setup_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--version", help="PsyQ version; defaults to 4.7")
    parser.add_argument("--source-root", type=Path)
    parser.add_argument("--archive", type=Path)
    parser.add_argument(
        "--dest",
        type=Path,
        help="repo-consumable PsyQ destination; defaults to toolchains/psyq/<version>",
    )
    parser.add_argument("--force", action="store_true")
    parser.set_defaults(handler=run_psyq_setup)


def configure_psyq_import_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    default_url = default_psyq_archive_url(DEFAULT_PSYQ_VERSION)
    url_help = (
        f"source archive URL; defaults to the public Arthus {DEFAULT_PSYQ_VERSION} archive"
        if default_url
        else "source archive URL"
    )
    parser.add_argument("--version", help="PsyQ version; defaults to 4.7")
    parser.add_argument("--archive", type=Path)
    parser.add_argument("--archive-url", help=url_help)
    parser.add_argument(
        "--dest",
        type=Path,
        help="repo-consumable PsyQ destination; defaults to toolchains/psyq/<version>",
    )
    parser.add_argument(
        "--private-root",
        type=Path,
        default=layout.private_assets_dir,
        help="optional private download and processing workspace; not a runtime SDK path",
    )
    parser.add_argument("--force", action="store_true")
    parser.set_defaults(handler=run_psyq_import)


def configure_disc_import_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--archive", type=Path)
    parser.add_argument("--archive-url")
    parser.add_argument(
        "--dest",
        type=Path,
        default=layout.disc_dir,
        help="repo-consumable BOF3 disc destination; defaults to inputs/disc",
    )
    parser.add_argument(
        "--private-root",
        type=Path,
        default=layout.private_assets_dir,
        help="optional private download and processing workspace; not a runtime disc path",
    )
    parser.add_argument("--force", action="store_true")
    parser.set_defaults(handler=run_disc_import)


def configure_root_parser(parser: argparse.ArgumentParser) -> None:
    subparsers = parser.add_subparsers(required=True)

    psyq = subparsers.add_parser("psyq")
    psyq_subparsers = psyq.add_subparsers(required=True)

    psyq_find = psyq_subparsers.add_parser("find")
    configure_psyq_find_parser(psyq_find)

    psyq_setup = psyq_subparsers.add_parser("setup")
    configure_psyq_setup_parser(psyq_setup)

    psyq_import = psyq_subparsers.add_parser("import")
    configure_psyq_import_parser(psyq_import)

    disc = subparsers.add_parser("disc")
    disc_subparsers = disc.add_subparsers(required=True)

    disc_import = disc_subparsers.add_parser("import")
    configure_disc_import_parser(disc_import)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="toolchain")
    configure_root_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
