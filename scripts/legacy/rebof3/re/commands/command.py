from __future__ import annotations

import argparse
from abc import ABC, abstractmethod


class Command(ABC):
    command_name = "command"

    @classmethod
    @abstractmethod
    def build_parser(cls) -> argparse.ArgumentParser:
        raise NotImplementedError

    @classmethod
    def parse_args(cls, argv: list[str] | None = None) -> argparse.Namespace:
        return cls.build_parser().parse_args(argv)

    @classmethod
    @abstractmethod
    def execute(cls, args: argparse.Namespace) -> int:
        raise NotImplementedError

    @classmethod
    def main(cls, argv: list[str] | None = None) -> int:
        return cls.execute(cls.parse_args(argv))
