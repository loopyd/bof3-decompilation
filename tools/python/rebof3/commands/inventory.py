from __future__ import annotations

import argparse
from pathlib import Path

from ..inventory import group_exact_duplicates, scan_inventory
from ..jsonio import read_json, write_json
from ..models import InventorySnapshot
from ..paths import repo_layout
from ._common import run_main


def run_scan(args: argparse.Namespace) -> int:
    snapshot = scan_inventory(
        slus_path=args.slus,
        logo_path=args.logo,
        emi_root=args.emi_root,
    )
    write_json(args.output, snapshot.to_dict())
    print(f"wrote {len(snapshot.programs)} programs to {args.output}")
    return 0


def run_group(args: argparse.Namespace) -> int:
    snapshot = InventorySnapshot.from_dict(read_json(args.input))
    groups = group_exact_duplicates(snapshot)
    write_json(args.output, groups.to_dict())
    print(f"wrote {len(groups.groups)} duplicate groups to {args.output}")
    return 0


def configure_scan_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--slus", type=Path, default=layout.slus_path)
    parser.add_argument("--logo", type=Path, default=layout.logo_path)
    parser.add_argument("--emi-root", type=Path, default=layout.emi_root)
    parser.add_argument("--output", type=Path, default=layout.inventory_path)
    parser.set_defaults(handler=run_scan)


def configure_group_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--input", type=Path, default=layout.inventory_path)
    parser.add_argument("--output", type=Path, default=layout.groups_path)
    parser.set_defaults(handler=run_group)


def configure_root_parser(parser: argparse.ArgumentParser) -> None:
    subparsers = parser.add_subparsers(required=True)

    scan = subparsers.add_parser("scan")
    configure_scan_parser(scan)

    group = subparsers.add_parser("group")
    configure_group_parser(group)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="inventory")
    configure_root_parser(parser)
    return parser


def add_legacy_parser(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    parser = subparsers.add_parser("inventory")
    configure_root_parser(parser)


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
