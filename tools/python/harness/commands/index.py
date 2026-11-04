"""Rebuild the generated cross-target reverse index."""

from __future__ import annotations

import argparse

from harness.common.cli import add_root_argument, run_main

from ..analysis.index import connect, index_path, rebuild
from ..analysis.project import analyze_project, status
from ..domain.manifests import load_target_manifests


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
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
    # A stale snapshot is regenerated here rather than handed back to the caller:
    # refusing to index until someone passes a flag turns a self-healing step into
    # a ritual, and every caller that forgot it read a stale board.
    for target in stale:
        analyze_project(root, target)
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
    parser = argparse.ArgumentParser(
        prog="bin/harness analysis index", description=__doc__
    )
    add_root_argument(parser)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
