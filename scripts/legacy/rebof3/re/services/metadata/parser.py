from __future__ import annotations

import argparse


def build_parser() -> argparse.ArgumentParser:
    from ...commands.metadata import build_parser as command_build_parser

    return command_build_parser()


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    from ...commands.metadata import parse_args as command_parse_args

    return command_parse_args(argv)


def execute(args: argparse.Namespace) -> int:
    from ...commands.metadata import execute as command_execute

    return command_execute(args)


def main(argv: list[str] | None = None) -> int:
    from ...commands.metadata import main as command_main

    return command_main(argv)
