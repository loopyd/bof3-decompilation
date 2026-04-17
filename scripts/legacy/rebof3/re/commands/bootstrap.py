from __future__ import annotations

from .command import Command
from ..services import bootstrap as bootstrap_module


class BootstrapCommand(Command):
    command_name = "bootstrap"

    @classmethod
    def build_parser(cls):
        return bootstrap_module._build_bootstrap_parser()

    @classmethod
    def execute(cls, args):
        return bootstrap_module._execute_args(args)


def build_parser():
    return BootstrapCommand.build_parser()


def parse_args(argv: list[str] | None = None):
    return BootstrapCommand.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    return BootstrapCommand.main(argv)
