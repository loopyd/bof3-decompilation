"""Emit bounded, role-specific BOF3 context in one read-only command."""

from __future__ import annotations

import argparse
from pathlib import Path

from ..context import profile_names, render_context
from ..domain.ids import FUNCTION_ID_FORMAT, FUNCTION_ID_HELP, parse_function_id
from ._common import add_root_argument, run_main


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="agent-context", description=__doc__)
    parser.add_argument(
        "role", nargs="?", default="agents", choices=sorted(profile_names())
    )
    parser.add_argument(
        "function",
        nargs="?",
        type=_selector,
        metavar=FUNCTION_ID_FORMAT,
        help=FUNCTION_ID_HELP,
    )
    parser.add_argument("--target", help="target-wide context for cleanup audit mode")
    parser.add_argument(
        "--mode",
        choices=("stable", "compatibility"),
        default="stable",
        help="bounded tracked prefill (default) or legacy-compatible full context",
    )
    add_root_argument(parser, default=_repository_root())
    parser.add_argument("--example", action="store_true", help=argparse.SUPPRESS)
    parser.set_defaults(
        handler=_run,
        argument_validator=_validate_arguments,
        error_prefix="",
        example_text="bin/agent-context worker",
    )
    return parser


def _validate_arguments(
    parser: argparse.ArgumentParser, args: argparse.Namespace
) -> None:
    if args.target and (args.role != "cleanup" or args.function is not None):
        parser.error("--target requires cleanup role and no function selector")
    if args.role in {"reverse", "review"} and args.function is None:
        parser.error(f"{args.role} requires a function selector")
    if args.role == "cleanup" and args.function is None and args.target is None:
        parser.error("cleanup requires a function selector or --target")


def _selector(value: str):
    try:
        return parse_function_id(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError(f"expected {FUNCTION_ID_HELP}") from error


def _run(args: argparse.Namespace) -> int:
    print(
        render_context(
            args.root.resolve(),
            args.role,
            args.function,
            args.target,
            args.mode,
        ),
        end="",
    )
    return 0


def _repository_root() -> Path:
    return Path(__file__).resolve().parents[4]


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
