"""Rebuild the generated cross-target reverse index."""

from __future__ import annotations

import argparse

from ..analysis.index import rebuild
from ..analysis.project import analyze_project, status
from ..domain import load_target_manifests
from ._common import add_root_argument, run_main


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    if args.recover:
        targets = sorted(load_target_manifests(root))
        stale = [target for target in targets if not status(root, target)["fresh"]]
        for target in stale:
            analyze_project(root, target, timeout=args.timeout)
        remaining = [target for target in targets if not status(root, target)["fresh"]]
        if remaining:
            raise ValueError(
                "analysis recovery left stale targets: " + ", ".join(remaining)
            )
    path = rebuild(root)
    print(path.relative_to(root))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="index")
    add_root_argument(parser)
    parser.add_argument(
        "--recover",
        action="store_true",
        help="reanalyze every stale generated snapshot before rebuilding",
    )
    parser.add_argument("--timeout", type=int, default=120)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
