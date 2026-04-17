from __future__ import annotations

import argparse
from dataclasses import dataclass

from .logger import Rebof3Logger, make_logger


CliLogger = Rebof3Logger
PACKAGE_ENTRYPOINT = ("python3", "-m", "scripts.rebof3")


@dataclass(frozen=True, slots=True)
class CliOptions:
    quiet: bool = False
    verbose: bool = False


@dataclass(frozen=True, slots=True)
class CommandContext:
    logger: Rebof3Logger
    cli: CliOptions


def package_prog(*parts: str) -> str:
    return " ".join((*PACKAGE_ENTRYPOINT, *parts))


def add_logging_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "-q",
        "--quiet",
        action="store_true",
        default=argparse.SUPPRESS,
        help="suppress non-error output",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        default=argparse.SUPPRESS,
        help="show detailed per-item output",
    )


def add_program_entry_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "-p", "--program", required=True, help="program path, name, or slug"
    )
    parser.add_argument("-e", "--entry", required=True, help="function entry address")


def cli_options_from_args(args: argparse.Namespace) -> CliOptions:
    return CliOptions(
        quiet=bool(getattr(args, "quiet", False)),
        verbose=bool(getattr(args, "verbose", False)),
    )


def logger_from_args(args: argparse.Namespace, tool_name: str) -> Rebof3Logger:
    options = cli_options_from_args(args)
    return make_logger(
        tool_name,
        quiet=options.quiet,
        verbose=options.verbose,
    )


def context_from_args(args: argparse.Namespace, tool_name: str) -> CommandContext:
    options = cli_options_from_args(args)
    return CommandContext(
        logger=make_logger(tool_name, quiet=options.quiet, verbose=options.verbose),
        cli=options,
    )
