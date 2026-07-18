"""Shared output-detail contract for context-heavy commands."""

from __future__ import annotations

import argparse


DETAIL_LEVELS = ("minimal", "normal", "full")


def add_detail_argument(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--detail",
        choices=DETAIL_LEVELS,
        help="output detail; defaults to normal text or full JSON",
    )


def resolve_detail(*, requested: str | None, json_output: bool) -> str:
    if requested is not None:
        return requested
    return "full" if json_output else "normal"
