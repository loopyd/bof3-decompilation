from __future__ import annotations

import argparse
import sys
from typing import Callable


ParserBuilder = Callable[[], argparse.ArgumentParser]


def run_main(
    build_parser: ParserBuilder,
    argv: list[str] | None = None,
) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
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
