from __future__ import annotations

import argparse

from ..doctor import (
    DOCTOR_PROFILES,
    DoctorProfile,
    doctor_exit_code,
    render_doctor,
    run_doctor,
)
from ..paths import repo_layout
from ._common import run_main


def run_command(args: argparse.Namespace) -> int:
    profile: DoctorProfile = "open" if bool(args.open_profile) else args.profile
    checks = run_doctor(
        layout=repo_layout(),
        profile=profile,
    )
    render_doctor(checks)
    return doctor_exit_code(checks, strict=bool(args.strict))


def configure_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--profile",
        choices=DOCTOR_PROFILES,
        default="full",
        help="doctor profile to validate: %(choices)s (default: %(default)s)",
    )
    parser.add_argument(
        "--open",
        dest="open_profile",
        action="store_true",
        help="backwards-compatible alias for --profile open",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="fail if any non-required check reports an issue",
    )
    parser.set_defaults(handler=run_command)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="doctor",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""profiles:
  open       fresh clone plus open-source setup/build prerequisites
  full       full reverse-engineering workspace, including Ghidra/decomp outputs
  decomp     decompilation/matching loop readiness after Ghidra symbol import
  ghidra     Ghidra bootstrap/project readiness without match binaries
  workspace  lightweight repo/submodule prerequisite check""",
    )
    configure_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
