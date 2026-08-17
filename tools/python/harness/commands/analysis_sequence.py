"""Target-scoped analysis sequence: freshness check -> index rebuild -> rev-query."""

from __future__ import annotations

import argparse
import sys

from ..domain import FUNCTION_ID_FORMAT, FUNCTION_ID_HELP
from ..analysis.index import rebuild
from ..analysis.project import status
from ._common import add_example_argument, add_root_argument, resolved_root, run_main


_root = resolved_root


def run_sequence(args: argparse.Namespace) -> int:
    """Check snapshot freshness, rebuild the index, then run a ranking."""
    root = _root(args)
    target = args.target

    # Step 1: Check target snapshot freshness.
    snap = status(root, target)
    if not snap["fresh"]:
        print(f"stale snapshot for {target}: stage=snapshot", file=sys.stderr)
        return 1

    # Step 2: Rebuild only the derived index after freshness succeeds.
    print(f"index rebuilt: {rebuild(root).relative_to(root)}")

    # Step 3: Run rev-query without touching snapshots or reviewed maps.
    if not args.ranking:
        print(
            "no ranking specified; use --ranking <command> "
            "(metrics|hotspots|leafs|quick-wins|pareto|duplicates)",
            file=sys.stderr,
        )
        return 1

    # Import rev-query locally to avoid circular deps at import time
    from ..commands.rev_query import build_parser, run_query

    parser = build_parser()
    # Parse only the ranking-specific arguments
    argv = [args.ranking]
    if args.json:
        argv.append("--json")
    if args.detail:
        argv.extend(["--detail", args.detail])
    if args.limit is not None:
        argv.extend(["--limit", str(args.limit)])
    if getattr(args, "exclusions", False):
        argv.append("--exclusions")
    if getattr(args, "unlifted", False):
        argv.append("--unlifted")
    if getattr(args, "include_trivial", False):
        argv.append("--include-trivial")
    if args.function:
        argv.append(args.function)

    parsed = parser.parse_args(argv)
    parsed.root = root  # Ensure root is set on parsed args
    return run_query(parsed)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="analysis-sequence",
        description=(
            "Target-scoped analysis sequence: check rz-project snapshot freshness, "
            "rebuild index when stale, then run a rev-query ranking."
        ),
    )
    add_root_argument(parser)
    parser.add_argument("target", help="target ID for the sequence")
    parser.add_argument(
        "--ranking",
        required=True,
        help="rev-query ranking command (metrics|hotspots|leafs|quick-wins|pareto|duplicates)",
    )
    parser.add_argument("--timeout", type=int, default=120)
    parser.add_argument("--json", action="store_true", default=False)
    parser.add_argument(
        "--detail",
        choices=["minimal", "normal", "full"],
        default=None,
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=None,
        help="maximum rows; 0 means all",
    )
    parser.add_argument("--exclusions", action="store_true", default=False)
    parser.add_argument("--unlifted", action="store_true", default=False)
    parser.add_argument(
        "--include-trivial",
        action="store_true",
        default=False,
        help="include classified return-only stubs in rankings",
    )
    parser.add_argument(
        "function",
        nargs="?",
        default=None,
        metavar=FUNCTION_ID_FORMAT,
        help=FUNCTION_ID_HELP,
    )
    add_example_argument(
        parser, "bin/analysis-sequence exe/slus_004_22 --ranking quick-wins"
    )
    parser.set_defaults(handler=run_sequence)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
