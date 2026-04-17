from __future__ import annotations

import argparse
import importlib
import sys

from ..cli import package_prog
from ..config import ROOT
from .db.connection import connect_inventory_database
from .db.migrations import ensure_inventory_schema
from .layout import INVENTORY_DIR, INVENTORY_SQLITE


def build_main() -> int:
    INVENTORY_DIR.mkdir(parents=True, exist_ok=True)
    connection = connect_inventory_database(INVENTORY_SQLITE)
    try:
        ensure_inventory_schema(connection)
    finally:
        connection.close()
    return 0


COMMAND_IMPORTS = {
    "slot-map": "scripts.rebof3.inventory.slot_map",
    "emi-catalog": "scripts.rebof3.inventory.emi_catalog",
    "overlay-catalog": "scripts.rebof3.inventory.overlay_catalog",
    "overlay-clusters": "scripts.rebof3.inventory.overlay_clusters",
    "unique-overlay-map": "scripts.rebof3.inventory.unique_overlay_map",
    "overlay-entry-tables": "scripts.rebof3.inventory.overlay_entry_tables",
    "ghidra-symbols": "scripts.rebof3.inventory.ghidra_symbols",
}

COMMAND_HELP = {
    "build": "initialize or migrate the canonical inventory sqlite",
    "slot-map": "refresh the slot map",
    "emi-catalog": "catalog EMI archives",
    "overlay-catalog": "catalog code-bearing overlay candidates",
    "overlay-clusters": "cluster duplicate overlay payloads",
    "unique-overlay-map": "build the representative overlay map",
    "overlay-entry-tables": "extract overlay entry table candidates",
    "ghidra-symbols": "reshape whole-project Ghidra symbol exports",
}


def build_command_parser() -> argparse.ArgumentParser:
    return argparse.ArgumentParser(
        prog=package_prog("inventory", "build"),
        description="Initialize or migrate the canonical BOF3 inventory SQLite database.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="\n".join(
            [
                "Output:",
                f"  {INVENTORY_SQLITE.relative_to(ROOT).as_posix()}",
                "",
                "Example:",
                f"  {package_prog('inventory', 'build')}",
            ]
        ),
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="python3 -m scripts.rebof3 inventory",
        description="Canonical BOF3 inventory workflow entrypoint.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="\n".join(
            [
                "Commands:",
                *[f"  {name:<22} {text}" for name, text in COMMAND_HELP.items()],
                "",
                "Examples:",
                "  python3 -m scripts.rebof3 inventory build",
                "  python3 -m scripts.rebof3 inventory overlay-catalog",
            ]
        ),
    )
    parser.add_argument("command", nargs="?", default="build")
    parser.add_argument("args", nargs=argparse.REMAINDER)
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    if args.command in {"-h", "--help"}:
        build_parser().print_help()
        return 0
    if args.command == "build" and any(arg in {"-h", "--help"} for arg in args.args):
        build_command_parser().print_help()
        return 0
    if args.command == "build":
        return build_main()

    module_name = COMMAND_IMPORTS.get(args.command)
    if module_name is None:
        print(f"unknown command: {args.command}", file=sys.stderr)
        return 1

    module = importlib.import_module(module_name)
    forwarded_args = list(args.args)
    if forwarded_args and forwarded_args[0] == "--":
        forwarded_args = forwarded_args[1:]
    return int(module.main(forwarded_args))
