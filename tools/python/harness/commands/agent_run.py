"""Dispatch one explicitly budgeted read-only BOF3 Codex review."""

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


def dispatch_review(args: argparse.Namespace) -> int:
    args.root = resolved_root(args)
    budget = read_record(args.root, args.budget)
    try:
        deadline = budget["clock"]["deadline_ns"] / 1e9
    except (KeyError, TypeError, OverflowError) as error:
        raise ValueError("invalid execution budget clock") from error
    result = run_dispatch(
        args.root,
        read_record(args.root, args.request),
        budget,
        [read_record(args.root, name) for name in args.consumption],
        expected_budget=args.expected_budget_digest,
        expected_checkpoint=args.expected_checkpoint_digest,
        expected_sequence=args.expected_sequence,
        expected_result=args.expected_result_digest,
        deadline=deadline,
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
    review = commands.add_parser(
        "review", help="launch one native-read-only Codex review"
    )
    review.add_argument("request")
    review.add_argument("--budget", required=True)
    review.add_argument("--consumption", action="append", required=True)
    review.add_argument("--expected-budget-digest", required=True)
    review.add_argument("--expected-checkpoint-digest", required=True)
    review.add_argument("--expected-sequence", required=True, type=int)
    review.add_argument(
        "--expected-result-digest",
        help="external prior completed-dispatch pin; required after sequence zero",
    )
    review.set_defaults(handler=dispatch_review)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
