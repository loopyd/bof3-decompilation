"""Shared native dispatch budget arguments and retained input loading."""

from __future__ import annotations

import argparse
from pathlib import Path

from harness.context.journal import read_record


def add_dispatch_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--budget", required=True)
    parser.add_argument("--consumption", action="append", required=True)
    parser.add_argument("--expected-budget-digest", required=True)
    parser.add_argument("--expected-checkpoint-digest", required=True)
    parser.add_argument("--expected-sequence", required=True, type=int)
    parser.add_argument(
        "--expected-result-digest",
        help="external prior completed-dispatch pin; required after sequence zero",
    )


def load_dispatch_inputs(
    root: Path, args: argparse.Namespace
) -> tuple[dict, list[dict], dict]:
    budget = read_record(root, args.budget)
    try:
        deadline = budget["clock"]["deadline_ns"] / 1e9
    except (KeyError, TypeError, OverflowError) as error:
        raise ValueError("invalid execution budget clock") from error
    return (
        budget,
        [read_record(root, name) for name in args.consumption],
        {
            "expected_budget": args.expected_budget_digest,
            "expected_checkpoint": args.expected_checkpoint_digest,
            "expected_sequence": args.expected_sequence,
            "expected_result": args.expected_result_digest,
            "deadline": deadline,
        },
    )
