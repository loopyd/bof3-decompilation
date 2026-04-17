from __future__ import annotations

import argparse
from pathlib import Path

from ....cli import add_logging_args, package_prog
from .decomp_runtime import DEFAULT_PROJECT_NAME


def _build_ghidra_decomp_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("re", "ghidra-decomp"),
        description=(
            "Build a Ghidra-first decomp bundle for one function from a shipped "
            "PS-X EXE, EMI#entry, or extracted .bin payload."
        ),
    )
    add_logging_args(parser)
    parser.add_argument(
        "input",
        help=(
            "Path to a PS-X EXE, EMI entry ref like "
            "build/extracted/BIN/ETC/GAME.EMI#1, or extracted .bin payload"
        ),
    )
    parser.add_argument("address", help="Function address to export")
    parser.add_argument(
        "--project-dir",
        type=Path,
        help=(
            "Override the shared Ghidra project directory; pass an explicit "
            "path to opt into an isolated project"
        ),
    )
    parser.add_argument("--project-name", default=DEFAULT_PROJECT_NAME)
    parser.add_argument("--program-name", help="Override the imported program name")
    parser.add_argument(
        "--artifacts-dir", type=Path, help="Override output directory for bundle files"
    )
    parser.add_argument(
        "--base-addr",
        type=lambda value: int(value, 0),
        help="Override raw import base address",
    )
    parser.add_argument(
        "--loader-mode", choices=("auto", "raw", "psx", "psx-exe"), default="auto"
    )
    parser.add_argument(
        "--asm-backend",
        choices=("ghidra", "spimdisasm"),
        default="ghidra",
        help="Select which asm backend becomes the canonical func.s input for m2c",
    )
    parser.add_argument(
        "--no-spimdisasm",
        action="store_true",
        help="Skip the alternative spimdisasm asm artifact lane",
    )
    parser.add_argument(
        "--no-m2c", action="store_true", help="Skip the automatic m2c sidecar attempt"
    )
    parser.add_argument("--noanalysis", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    return parser


def build_parser() -> argparse.ArgumentParser:
    from ...commands.ghidra_decomp import build_parser as command_build_parser

    return command_build_parser()


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    from ...commands.ghidra_decomp import parse_args as command_parse_args

    return command_parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    from ...commands.ghidra_decomp import main as command_main

    return command_main(argv)


__all__ = ["_build_ghidra_decomp_parser", "build_parser", "main", "parse_args"]
