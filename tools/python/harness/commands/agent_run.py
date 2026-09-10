"""Run deterministic parent-owned lift diagnosis and candidate audit tools."""

from __future__ import annotations

import argparse

from harness.common.cli import (
    add_example_argument,
    add_root_argument,
    run_main,
)
from harness.decomp.cli import register_commands


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    add_root_argument(parser)
    add_example_argument(
        parser,
        "bin/agent-run diagnose out/mission-request.json --expected-request-digest PIN --output out/reviews/lift-diagnosis/BEFORE --deadline ORIGINAL_MONOTONIC_CUTOFF",
    )
    commands = parser.add_subparsers(dest="command", required=True)
    register_commands(commands)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
