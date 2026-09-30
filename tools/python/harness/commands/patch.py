"""Expose target-scoped patch operations through the shared harness command runner."""

from __future__ import annotations

import argparse
import json

from ..common.cli import (
    add_example_argument,
    add_root_argument,
    resolved_root,
    run_main,
)
from ..io import repo_layout
from ..patches.dispatch import run_patches
from ..patches.inventory import list_patches


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness patch", description=__doc__)
    add_root_argument(parser)
    add_example_argument(parser, "bin/harness patch check --target pcsx-redux")
    actions = parser.add_subparsers(dest="action", required=True)
    for action, description in (
        ("list", "list patch files, optionally with application status"),
        ("apply", "apply selected patches"),
        ("check", "validate selected patches and current source without writes"),
        ("revert", "restore selected patches to their preceding source state"),
    ):
        command = actions.add_parser(action, help=description)
        command.add_argument(
            "--target",
            action="append",
            help="target folder name; default: all target folders",
        )
        command.add_argument(
            "--patch",
            action="append",
            help="patch filename; requires one --target; default: all its patches",
        )
        if action == "list":
            command.add_argument(
                "--status",
                action="store_true",
                help="inspect source state as well as listing files",
            )
        command.set_defaults(handler=run_patch)
    return parser


def run_patch(args: argparse.Namespace) -> int:
    layout = repo_layout(resolved_root(args))
    report = (
        list_patches(layout, args.target, args.patch, status=args.status)
        if args.action == "list"
        else run_patches(layout, args.action, args.target, args.patch)
    )
    print(json.dumps(report, indent=2))
    return 0 if report.get("valid", True) else 2


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)
