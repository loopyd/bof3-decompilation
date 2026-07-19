"""Read-only PsyQ provenance discovery command."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from ..domain import normalize_target_id
from ..io import repo_layout
from ..psyq.discovery import discover
from ._common import run_main


def _example(_args: argparse.Namespace) -> int:
    print("psyq-find exe/logo --json -o out/analysis/psyq-logo.json")
    return 0


def _run(args: argparse.Namespace) -> int:
    targets = [normalize_target_id(value).value for value in args.targets]
    payload = discover(args.root.resolve(), targets or None)
    rendered = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    if args.out is not None:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(rendered, encoding="utf-8")
    if args.json:
        print(rendered, end="")
    else:
        print(
            f"targets={len(payload['targets'])} sdk_functions={payload['sdk_function_count']} "
            f"matches={len(payload['matches'])}"
        )
        if args.out is not None:
            print(args.out)
    return 0 if payload["matches"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="psyq-find",
        description="scan staged PsyQ archives against target binaries",
    )
    parser.add_argument("targets", nargs="*", metavar="TARGET")
    parser.add_argument(
        "--json", action="store_true", help="emit evidence JSON to stdout"
    )
    parser.add_argument("-o", "--out", type=Path, help="write JSON evidence to FILE")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.add_argument("--example", action="store_true")
    parser.set_defaults(
        handler=lambda args: _example(args) if args.example else _run(args)
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
