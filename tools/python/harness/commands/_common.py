from __future__ import annotations

import argparse
from pathlib import Path
import sys
from typing import Callable

from ..io import repo_layout


ParserBuilder = Callable[[], argparse.ArgumentParser]


def add_root_argument(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--root", type=Path, default=repo_layout().root)


def add_example_argument(parser: argparse.ArgumentParser, text: str) -> None:
    """Add a --example flag whose text run_main prints before dispatch."""
    parser.add_argument(
        "--example", action="store_true", help="print a minimal invocation"
    )
    parser.set_defaults(example_text=text)


def run_main(
    build_parser: ParserBuilder,
    argv: list[str] | None = None,
) -> int:
    parser = build_parser()
    raw = sys.argv[1:] if argv is None else argv
    example = parser.get_default("example_text")
    if example is not None and "--example" in raw:
        print(example)
        return 0
    args = parser.parse_args(raw)
    handler = getattr(args, "handler", None)
    if handler is None:
        parser.error("missing command handler")
    try:
        return handler(args)
    except BrokenPipeError:
        return 0
    except (FileNotFoundError, RuntimeError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
