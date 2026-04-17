from __future__ import annotations

from .command import Command
from ..services import doctor as doctor_module


class DoctorCommand(Command):
    command_name = "doctor"

    @classmethod
    def build_parser(cls):
        return doctor_module._build_doctor_parser()

    @classmethod
    def execute(cls, args):
        return doctor_module._execute_args(args)


def build_parser():
    return DoctorCommand.build_parser()


def parse_args(argv: list[str] | None = None):
    return DoctorCommand.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    return DoctorCommand.main(argv)
