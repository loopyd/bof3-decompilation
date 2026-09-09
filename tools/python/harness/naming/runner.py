"""``bin/naming-evidence-run``: one receipt-backed target evidence run.

Native collection of the ``required_work`` for a single target v3 report:
one process, one bulk context, one shared index connection, bounded
read-only concurrency, digest-bound evidence files and append-only row
checkpoints.  Bounded row selection defaults to a deterministic shard of at
most ten still-open rows; ``--all-rows`` runs under the same budgeted
collection.  Query success never closes a semantic rung; only the owning
validator may move a row to a terminal state.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.common.cli import (
    add_example_argument,
    add_root_argument,
    resolved_root,
    run_main,
)
from harness.naming.execution import (
    DEFAULT_CONCURRENCY,
    DEFAULT_DEADLINE,
    DEFAULT_ROWS_BUDGET,
    MAX_CONCURRENCY,
    SHARD_ROWS,
    run_evidence,
)


class _SingleValue(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        if getattr(namespace, self.dest, None) is not None:
            parser.error(f"{option_string} may be specified once")
        setattr(namespace, self.dest, values)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="naming-evidence-run")
    add_root_argument(parser)
    add_example_argument(
        parser,
        "bin/naming-evidence-run exe/test out/reviews/reports/exe__test.json",
    )
    parser.add_argument("target")
    parser.add_argument("report", type=Path)
    parser.add_argument(
        "--rows",
        action=_SingleValue,
        help="comma-separated KIND:NAME row selectors (maximum 10); "
        "may be specified once; omitted selects the default pending shard",
    )
    parser.add_argument(
        "--all-rows",
        action="store_true",
        help="select every still-open row under the run deadline",
    )
    parser.add_argument(
        "--shard",
        type=int,
        default=SHARD_ROWS,
        help=f"default pending shard size (default {SHARD_ROWS})",
    )
    parser.add_argument(
        "--rows-budget",
        type=int,
        default=DEFAULT_ROWS_BUDGET,
        help="explicit maximum rows permitted with --all-rows",
    )
    parser.add_argument(
        "--concurrency",
        type=int,
        default=DEFAULT_CONCURRENCY,
        help=f"read-only indexed query workers (1-{MAX_CONCURRENCY}, default {DEFAULT_CONCURRENCY})",
    )
    parser.add_argument(
        "--evidence-root",
        action=_SingleValue,
        help="one canonically spelled absolute parent owning all evidence artifacts",
    )
    parser.add_argument(
        "--terminalize",
        action="store_true",
        help="attempt semantic terminalization; fail closed without report mutation when evidence is insufficient",
    )
    parser.add_argument(
        "--deadline",
        type=int,
        default=DEFAULT_DEADLINE,
        help=f"relative per-operation/index-request limit in seconds (default {DEFAULT_DEADLINE})",
    )
    parser.add_argument(
        "--instructions",
        action="store_true",
        help="capture complete original instructions for function rows and their direct work",
    )
    parser.add_argument(
        "--work-deadline",
        type=float,
        action=_SingleValue,
        help="original absolute monotonic work cutoff; --deadline remains a relative per-operation cap",
    )
    parser.set_defaults(handler=_run)
    return parser


def _run(args: argparse.Namespace) -> int:
    rows = None if args.rows is None else [row for row in args.rows.split(",") if row]
    from harness.domain.receipts import reset_receipt_root, set_receipt_root
    from harness.naming.namespace import canonical_evidence_root
    from harness.naming.namespace import reset_evidence_root
    from harness.naming.namespace import set_evidence_root

    root = resolved_root(args)
    evidence_root = (
        None
        if args.evidence_root is None
        else canonical_evidence_root(args.evidence_root)
    )
    from harness.naming.instructions import INSTRUCTION_CAPTURE

    instruction_token = INSTRUCTION_CAPTURE.set(args.instructions)
    token = set_evidence_root(evidence_root)
    receipt_token = set_receipt_root(evidence_root)
    try:
        payload = run_evidence(
            root,
            args.target,
            args.report,
            rows=rows,
            all_rows=args.all_rows,
            shard=args.shard,
            concurrency=args.concurrency,
            deadline=args.deadline,
            rows_budget=args.rows_budget,
            terminalize=args.terminalize,
            **(
                {"work_deadline": args.work_deadline}
                if getattr(args, "work_deadline", None) is not None
                else {}
            ),
        )
    finally:
        reset_receipt_root(receipt_token)
        reset_evidence_root(token)
        INSTRUCTION_CAPTURE.reset(instruction_token)
    print(json.dumps(payload, indent=2, sort_keys=True))
    return 1 if payload["errors"] else 0


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())


__all__ = ["build_parser", "main"]
