from __future__ import annotations

import argparse
from pathlib import Path

from ....cli import add_logging_args, package_prog


def _build_bootstrap_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("re", "bootstrap"),
        description="Bootstrap the reusable binary-first headless Ghidra project.",
    )
    add_logging_args(parser)
    parser.add_argument("--noanalysis", action="store_true")
    parser.add_argument("--no-restore-metadata", action="store_true")
    parser.add_argument("--restore-metadata-from", type=Path)
    parser.add_argument("--strict-restore", action="store_true")
    return parser


def build_parser() -> argparse.ArgumentParser:
    from ...commands.bootstrap import (
        build_parser as command_build_parser,
    )

    return command_build_parser()


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    from ...commands.bootstrap import parse_args as command_parse_args

    return command_parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    from ...commands.bootstrap import main as command_main

    return command_main(argv)


__all__ = ["_build_bootstrap_parser", "build_parser", "main", "parse_args"]
