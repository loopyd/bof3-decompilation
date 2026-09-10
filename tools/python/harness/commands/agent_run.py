"""Run pinned parent lift gates or explicitly budgeted native Codex proposals."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import (
    add_example_argument,
    add_root_argument,
    resolved_root,
    run_main,
)
from harness.context.dispatch import run_dispatch
from harness.context.journal import read_record
from harness.context.arguments import add_dispatch_arguments, load_dispatch_inputs
from harness.decomp.cli import register_commands


def dispatch_review(args: argparse.Namespace) -> int:
    args.root = resolved_root(args)
    budget, chain, options = load_dispatch_inputs(args.root, args)
    result = run_dispatch(
        args.root,
        read_record(args.root, args.request),
        budget,
        chain,
        **options,
    )
    print(json.dumps(result), flush=True)
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    add_root_argument(parser)
    add_example_argument(
        parser,
        "bin/agent-run review out/review-request.json --budget out/budget.json --consumption out/initial-consumption.json --expected-budget-digest PIN --expected-checkpoint-digest PIN --expected-sequence 0",
    )
    commands = parser.add_subparsers(dest="command", required=True)
    register_commands(commands)
    review = commands.add_parser(
        "review", help="launch one native-read-only Codex review"
    )
    review.add_argument("request")
    add_dispatch_arguments(review)
    review.set_defaults(handler=dispatch_review)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
