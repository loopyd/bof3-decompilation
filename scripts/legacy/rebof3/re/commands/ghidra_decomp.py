from __future__ import annotations

from .command import Command
from ..services.ghidra.decomp_parser import _build_ghidra_decomp_parser
from ..services.ghidra.decomp_service import _execute_args


class GhidraDecompCommand(Command):
    command_name = "ghidra-decomp"

    @classmethod
    def build_parser(cls):
        return _build_ghidra_decomp_parser()

    @classmethod
    def execute(cls, args):
        return _execute_args(args)


def build_parser():
    return GhidraDecompCommand.build_parser()


def parse_args(argv: list[str] | None = None):
    return GhidraDecompCommand.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    return GhidraDecompCommand.main(argv)
