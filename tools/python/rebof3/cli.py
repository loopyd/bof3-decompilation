from __future__ import annotations

import argparse

from .commands import doctor as doctor_command
from .commands import ghidra as ghidra_command
from .commands import inventory as inventory_command
from .commands import setup as setup_command
from .commands import toolchain as toolchain_command


COMPATIBILITY_DESCRIPTION = (
    "Compatibility entrypoint for the legacy aggregate CLI. "
    "Prefer the dedicated bin/* commands for normal use."
)


def add_compatibility_parsers(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    inventory_command.add_legacy_parser(subparsers)

    plan = subparsers.add_parser("plan")
    ghidra_command.add_legacy_plan_parser(plan.add_subparsers(required=True))

    pipeline = subparsers.add_parser("pipeline")
    ghidra_command.add_legacy_pipeline_parser(pipeline.add_subparsers(required=True))

    ghidra = subparsers.add_parser("ghidra")
    ghidra_command.add_legacy_ghidra_parser(ghidra.add_subparsers(required=True))

    doctor_command.add_legacy_parser(subparsers)
    toolchain_command.add_legacy_parser(subparsers)
    setup_command.add_legacy_parser(subparsers)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bof3",
        description=COMPATIBILITY_DESCRIPTION,
    )
    subparsers = parser.add_subparsers(required=True)
    add_compatibility_parsers(subparsers)

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    handler = getattr(args, "handler", None)
    if handler is None:
        parser.error("missing command handler")
    return handler(args)


if __name__ == "__main__":
    raise SystemExit(main())
