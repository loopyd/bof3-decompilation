"""Assess or remove known disposable repository outputs."""

from __future__ import annotations

import argparse
import sys

from harness.cleanup.inventory import Options, assess, estimate_bytes
from harness.cleanup.operations import execute
from harness.common.cli import add_example_argument, add_root_argument, run_main


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/harness cleanup",
        description="Remove stale generated outputs; preview with --dry-run.",
    )
    add_root_argument(parser)
    add_example_argument(parser, "bin/harness cleanup --dry-run")
    parser.add_argument(
        "--dry-run", action="store_true", help="print operations without writes"
    )
    parser.add_argument(
        "--with-worktrees",
        action="store_true",
        help="include clean, merged, inactive local linked worktrees",
    )
    parser.add_argument(
        "--with-session-history",
        action="store_true",
        help="include inactive session history older than 24 hours",
    )
    parser.add_argument(
        "--with-evidence",
        action="store_true",
        help="include eligible historical out/ evidence older than 24 hours",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="include all optional categories, retaining safeguards",
    )
    parser.set_defaults(handler=run)
    return parser


def space(value: int) -> str:
    return f"{value / 1024**3:.3f} GiB ({value:,} bytes)"


def run(args: argparse.Namespace) -> int:
    options = Options(
        worktrees=args.all or args.with_worktrees,
        session_history=args.all or args.with_session_history,
        evidence=args.all or args.with_evidence,
    )
    plan = assess(args.root, options)
    for operation in plan.operations:
        print(f"PLAN [{operation.category}] {operation.path}")
        print(
            f"  {operation.reason}; allocated estimate: {space(estimate_bytes([operation]))}"
        )
    for skipped in plan.skipped:
        print(f"SKIP {skipped.path}: {skipped.reason}")
    if plan.unreadable_processes:
        print(
            "Process visibility gap: cannot inspect same-user PIDs "
            + ", ".join(map(str, plan.unreadable_processes))
        )
    if args.dry_run:
        print(f"DRY RUN: {len(plan.operations)} planned operations; no changes made.")
        print(f"Estimated savings: {space(plan.estimated_bytes)}")
        return 0
    result = execute(plan)
    if result.error:
        print(f"Cleanup stopped: {result.error}", file=sys.stderr)
    print(f"Removed {len(result.completed)} operation(s).")
    print(f"Estimated savings: {space(result.estimated_bytes)}")
    print(f"Observed free-space change: {space(result.observed_bytes)}")
    return 1 if result.error else 0


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
