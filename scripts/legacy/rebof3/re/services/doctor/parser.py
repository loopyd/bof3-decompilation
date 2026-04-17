from __future__ import annotations

import argparse

from ....cli import add_logging_args, package_prog


def _build_doctor_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("re", "doctor"),
        description="Check native dependencies and BOF3 workflow state.",
    )
    add_logging_args(parser)
    parser.add_argument(
        "--json", action="store_true", help="Emit machine-readable output"
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Treat optional missing checks as failures too",
    )
    return parser


def build_parser() -> argparse.ArgumentParser:
    from ...commands.doctor import build_parser as command_build_parser

    return command_build_parser()


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    from ...commands.doctor import parse_args as command_parse_args

    return command_parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    from ...commands.doctor import main as command_main

    return command_main(argv)


__all__ = ["_build_doctor_parser", "build_parser", "main", "parse_args"]
