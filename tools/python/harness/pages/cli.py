"""Command-line interface for progress pages and the main-push hook."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from harness.common.cli import add_example_argument, add_root_argument, run_main
from harness.pages.build import build_site
from harness.pages.publish import build_revision, install_hook, pre_push, publish


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness pages")
    add_example_argument(parser, "bin/harness pages install-hook")
    commands = parser.add_subparsers(dest="action", required=True)
    for action in ("build", "preview", "publish", "pre-push", "install-hook"):
        command = commands.add_parser(action)
        add_root_argument(command)
        command.set_defaults(handler=run)
        if action in {"build", "preview", "publish"}:
            command.add_argument("--revision", required=True, help="source commit")
        if action in {"build", "preview"}:
            command.add_argument("--output", type=Path, required=True)
        if action == "build":
            command.add_argument("--cache", type=Path)
        if action == "publish":
            command.add_argument("--remote", default="origin")
        if action == "pre-push":
            command.add_argument("remote")
            command.add_argument("url")
    return parser


def run(args) -> int:
    root = args.root.resolve()
    if args.action == "install-hook":
        install_hook(root)
    elif args.action == "pre-push":
        pre_push(root, args.remote, sys.stdin)
    elif args.action == "publish":
        publish(root, args.remote, args.revision)
    elif args.action == "preview":
        build_revision(root, args.revision, args.output)
    elif args.action == "build":
        build_site(root, args.output, args.revision, args.cache)
    return 0


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
