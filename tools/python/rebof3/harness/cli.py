from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ..commands._common import run_main
from ..match.asm_diff import parse_int
from .commands import (
    run_claim,
    run_diff,
    run_export,
    run_finish,
    run_list,
    run_release,
    run_report,
    run_seed,
    run_status,
)


def configure_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--config", type=Path)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--verbose", action="store_true")
    subparsers = parser.add_subparsers(required=True)

    claim = subparsers.add_parser(
        "claim",
        help="claim next unclaimed function in a module",
    )
    claim.add_argument("target_id", nargs="?")
    claim.add_argument("--owner")
    claim.add_argument("--lease-minutes", type=int, default=120)
    claim.add_argument("--status")
    claim.add_argument("--type", default="function")
    claim.add_argument(
        "--module",
        help="claim the next target from an EMI/module filter, e.g. GAME#0",
    )
    claim.set_defaults(handler=run_claim)

    release = subparsers.add_parser("release", help="release a claim")
    release.add_argument("target_id")
    release.add_argument("--owner")
    release.set_defaults(handler=run_release)

    diff = subparsers.add_parser(
        "diff",
        help="compile and asm-diff a function against original binary",
    )
    diff.add_argument("target_id_or_source_path")
    diff.set_defaults(handler=run_diff)

    finish = subparsers.add_parser("finish", help="mark a function done")
    finish.add_argument("target_id")
    finish.add_argument(
        "--status",
        choices=("done", "blocked"),
        default="done",
    )
    finish.add_argument("--message", default="finished")
    finish.add_argument("--path", type=Path)
    finish.set_defaults(handler=run_finish)

    status = subparsers.add_parser(
        "status", help="project dashboard"
    )
    status.add_argument(
        "--module",
        help="filter by EMI/module, e.g. GAME#0",
    )
    status.set_defaults(handler=run_status)

    seed = subparsers.add_parser(
        "seed", help="seed function targets from output/analysis.sqlite3"
    )
    seed.add_argument(
        "--prune",
        action="store_true",
        help="remove queued function targets missing from the analysis DB",
    )
    seed.set_defaults(handler=run_seed)

    export = subparsers.add_parser(
        "export", help="export function context for subagent consumption"
    )
    export.add_argument("target_id")
    export.set_defaults(handler=run_export)

    list_cmd = subparsers.add_parser(
        "list", help="list programs or functions"
    )
    list_subs = list_cmd.add_subparsers(required=True)
    list_functions = list_subs.add_parser(
        "functions", help="list all functions in a module with match status"
    )
    list_functions.add_argument(
        "--module", required=True,
        help="module filter, e.g. GAME#0 or BATTLE#3",
    )
    list_functions.set_defaults(handler=run_list, kind="functions")
    list_programs = list_subs.add_parser(
        "programs", help="list all programs/binaries with decomp progress"
    )
    list_programs.add_argument(
        "--module",
        help="optional module filter",
    )
    list_programs.set_defaults(handler=run_list, kind="programs")

    report = subparsers.add_parser(
        "report", help="per-program report with match percentages"
    )
    report.add_argument("program", help="program path filter, e.g. /boot/SLUS or GAME#0")
    report.set_defaults(handler=run_report)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="harness")
    configure_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
