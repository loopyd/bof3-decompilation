"""Rebuild the generated cross-target reverse index."""

from __future__ import annotations

import argparse

from harness.common.cli import add_root_argument, run_main

from ..analysis.index import connect, index_path, rebuild
from ..analysis.project import analyze_project, status
from ..domain.manifests import load_target_manifests


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    if args.recover:
        manifests = load_target_manifests(root)
        states = {
            target: status(root, target, manifest=manifest)
            for target, manifest in sorted(manifests.items())
        }
        stale = [target for target, state in states.items() if not state["fresh"]]
        if not stale:
            path = index_path(root)
            try:
                connection = connect(root, manifests=manifests)
            except (FileNotFoundError, ValueError):
                pass
            else:
                connection.close()
                print(path.relative_to(root))
                return 0
        for target in stale:
            analyze_project(root, target, timeout=args.timeout)
        remaining = [
            target
            for target in stale
            if not status(root, target, manifest=manifests[target])["fresh"]
        ]
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
