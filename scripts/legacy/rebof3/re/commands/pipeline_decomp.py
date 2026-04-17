from __future__ import annotations

from ...cli import package_prog
from .command import Command
from ..services.ghidra.decomp_parser import _build_ghidra_decomp_parser
from ..services.ghidra.decomp_service import _execute_args


class PipelineDecompCommand(Command):
    command_name = "pipeline-decomp"

    @classmethod
    def build_parser(cls):
        parser = _build_ghidra_decomp_parser()
        parser.prog = package_prog("re", "pipeline-decomp")
        parser.description = "Run the concrete decomp pipeline for one function and write the bundle artifacts."
        return parser

    @classmethod
    def execute(cls, args):
        return _execute_args(args)


def build_parser():
    return PipelineDecompCommand.build_parser()


def parse_args(argv: list[str] | None = None):
    return PipelineDecompCommand.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    return PipelineDecompCommand.main(argv)
