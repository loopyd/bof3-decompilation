from __future__ import annotations

import argparse
import json
from pathlib import Path

from ..ghidra_report import (
    build_duplicate_groups,
    context_gaps,
    function_report,
    module_report,
    queue_report,
    render_markdown,
)
from ..jsonio import write_json
from ..paths import repo_layout
from ._common import run_main


def emit(args: argparse.Namespace, payload: dict) -> None:
    text = (
        json.dumps(payload, indent=2, sort_keys=True) + "\n"
        if args.format == "json"
        else render_markdown(payload)
    )
    if args.output is None:
        print(text, end="")
        return
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.format == "json":
        write_json(args.output, payload)
    else:
        args.output.write_text(text, encoding="utf-8")
    print(f"report: {args.output}")


def run_function(args: argparse.Namespace) -> int:
    emit(args, function_report(repo_layout(), args.address, args.source))
    return 0


def run_module(args: argparse.Namespace) -> int:
    emit(args, module_report(repo_layout(), args.source_hint))
    return 0


def run_queue(args: argparse.Namespace) -> int:
    emit(args, queue_report(repo_layout(), limit=args.limit))
    return 0


def run_duplicates(args: argparse.Namespace) -> int:
    emit(args, build_duplicate_groups(repo_layout()))
    return 0


def run_context_gaps(args: argparse.Namespace) -> int:
    emit(args, context_gaps(repo_layout()))
    return 0


def add_output_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--format", choices=("markdown", "json"), default="markdown")
    parser.add_argument("--output", type=Path)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="ghidra-report")
    subparsers = parser.add_subparsers(required=True)

    function = subparsers.add_parser("function")
    function.add_argument("address")
    function.add_argument(
        "--source", help="source_hint to disambiguate duplicate addresses"
    )
    add_output_args(function)
    function.set_defaults(handler=run_function)

    module = subparsers.add_parser("module")
    module.add_argument("source_hint")
    add_output_args(module)
    module.set_defaults(handler=run_module)

    queue = subparsers.add_parser("queue")
    queue.add_argument("--limit", type=int, default=10)
    add_output_args(queue)
    queue.set_defaults(handler=run_queue)

    duplicates = subparsers.add_parser("duplicates")
    add_output_args(duplicates)
    duplicates.set_defaults(handler=run_duplicates)

    gaps = subparsers.add_parser("context-gaps")
    add_output_args(gaps)
    gaps.set_defaults(handler=run_context_gaps)

    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
