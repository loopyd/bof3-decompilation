from __future__ import annotations

import argparse

from ..doctor import doctor_exit_code, render_doctor, run_doctor
from ..paths import repo_layout
from ._common import run_main


def run_command(args: argparse.Namespace) -> int:
    checks = run_doctor(
        layout=repo_layout(),
        include_local_inputs=not bool(args.open_profile),
        include_generated_outputs=not bool(args.open_profile),
    )
    render_doctor(checks)
    return doctor_exit_code(checks, strict=bool(args.strict))


def configure_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--open",
        dest="open_profile",
        action="store_true",
        help="check only the fresh-clone open setup path",
    )
    parser.add_argument("--strict", action="store_true")
    parser.set_defaults(handler=run_command)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="doctor")
    configure_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
