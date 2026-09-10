"""CLI adapters for explicit read-only consolidation inspection."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import add_root_argument, resolved_root, run_main
from harness.combiner.inspection import inspect_source


def _inspect_source(args: argparse.Namespace) -> int:
    print(json.dumps(inspect_source(resolved_root(args), args.source), indent=2))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="combiner")
    add_root_argument(parser)
    commands = parser.add_subparsers(dest="command", required=True)
    inspect = commands.add_parser(
        "inspect-source", help="inspect leading function metadata"
    )
    inspect.add_argument("source")
    inspect.set_defaults(handler=_inspect_source)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
