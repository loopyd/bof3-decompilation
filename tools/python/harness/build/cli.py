"""Internal build adapters for metadata-owned compiler output."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import sys

from harness.common.cli import add_root_argument, run_main
from harness.common.deadlines import check_deadline, use_deadline
from harness.build.driver import run_compiler
from harness.build.inventory import capture_inventory, check_inventory
from harness.build.receipts import run_producer, verify_production

from .translation import prepare_translation


def run_translation(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    source = args.source.resolve()
    sys.stdout.write(prepare_translation(root, source, sys.stdin.read()))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="build-adapter")
    add_root_argument(parser)
    commands = parser.add_subparsers(dest="command", required=True)
    compiler = commands.add_parser(
        "compiler", help="run the guarded native compiler adapter"
    )
    compiler.add_argument("arguments", nargs=argparse.REMAINDER)
    compiler.set_defaults(handler=_run_compiler)
    producer = commands.add_parser(
        "producer",
        help="always compile a grouped unit and record configured provenance",
    )
    producer.add_argument("arguments", nargs=argparse.REMAINDER)
    producer.set_defaults(handler=_run_producer)
    receipt = commands.add_parser(
        "receipt",
        help="check pinned producer provenance; never authorizes object reuse",
    )
    receipt.add_argument("--expected-sha256", required=True)
    receipt.add_argument("arguments", nargs=argparse.REMAINDER)
    receipt.set_defaults(handler=_run_receipt)
    inventory = commands.add_parser(
        "inventory", help="capture or check configured C topology"
    )
    inventory.add_argument("--snapshot", type=Path)
    inventory.add_argument("--expected-sha256")
    inventory.set_defaults(handler=_run_inventory)
    translation = commands.add_parser("translation")
    translation.add_argument("source", type=Path)
    translation.set_defaults(handler=run_translation)
    return parser


def _run_compiler(args: argparse.Namespace) -> int:
    arguments = args.arguments[1:] if args.arguments[:1] == ["--"] else args.arguments
    return run_compiler(args.root.resolve(), arguments)


def _run_producer(args: argparse.Namespace) -> int:
    arguments = args.arguments[1:] if args.arguments[:1] == ["--"] else args.arguments
    try:
        return run_producer(args.root.resolve(), arguments)
    except BrokenPipeError:
        return 1


def _run_receipt(args: argparse.Namespace) -> int:
    arguments = args.arguments[1:] if args.arguments[:1] == ["--"] else args.arguments
    return _run_report(
        lambda: verify_production(args.root.resolve(), arguments, args.expected_sha256)
    )


def _run_inventory(args: argparse.Namespace) -> int:
    if args.snapshot is None and args.expected_sha256 is None:
        return _run_report(lambda: capture_inventory(args.root.resolve()))
    if args.snapshot is None or args.expected_sha256 is None:
        raise ValueError("inventory checking requires both snapshot and external pin")
    return _run_report(
        lambda: check_inventory(
            args.root.resolve(), args.snapshot, args.expected_sha256
        )
    )


def _run_report(operation) -> int:
    deadline = os.environ.get("BOF3_WORK_DEADLINE")
    try:
        with use_deadline(float(deadline) if deadline else None):
            result = operation()
            if result is not None:
                rendered = json.dumps(result, sort_keys=True)
                check_deadline()
                print(rendered)
                sys.stdout.flush()
            check_deadline()
        return 0
    except BrokenPipeError:
        return 1


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
