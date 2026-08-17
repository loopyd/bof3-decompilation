from __future__ import annotations

import argparse
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path
import sys
from typing import Any, Callable, Generic, TypeVar

from ..domain import FunctionId, resolve_function
from ..domain.manifests import TargetManifest
from ..io import repo_layout


ParserBuilder = Callable[[], argparse.ArgumentParser]

T = TypeVar("T")


@dataclass(frozen=True)
class Check(Generic[T]):
    """One ordered setup/doctor-style check and its runner."""

    label: str
    run: Callable[[T], str]


def register_check(
    label: str, tasks: list[Check[T]]
) -> Callable[[Callable[[T], str]], Callable[[T], str]]:
    """Register a check runner in a command's ordered task list."""

    def register(run: Callable[[T], str]) -> Callable[[T], str]:
        tasks.append(Check(label, run))
        return run

    return register


def resolve_function_selector(
    value: str,
) -> tuple[FunctionId, TargetManifest, Path | None]:
    """Return one known target and its authored source path, or None.

    Facts come from the domain registry; this tuple shape is kept for
    command ergonomics.  No path is fabricated for a missing source; callers
    report the explicit absence.
    """

    resolved = resolve_function(repo_layout().root, value)
    return resolved.id, resolved.manifest, resolved.source


def add_root_argument(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--root", type=Path, default=repo_layout().root)


def resolved_root(args: argparse.Namespace) -> Path:
    """Resolve the shared --root argument once for command handlers."""
    return args.root.resolve()


def render_task(status: str, label: str, detail: str, tasks: Sequence[Any]) -> None:
    """Render one setup/doctor-style task line with aligned labels."""
    print(f"[{status}] {label:<{max(len(task.label) for task in tasks)}}  {detail}")


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
