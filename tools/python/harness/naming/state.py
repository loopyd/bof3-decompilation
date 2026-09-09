"""Public checkpoint paths and deterministic row selection."""

from __future__ import annotations

import json
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import Any

from harness.naming.collection import _select_rows
from harness.naming.journal import load_journal


def load_checkpoint(
    root: Path, report: Path, target: str | None = None
) -> dict[str, dict[str, Any]]:
    """Completed row operations keyed by ``KIND:NAME`` (torn tails discarded)."""

    actual_target = target or str(json.loads(report.read_text())["target"])
    entries, _torn = load_journal(root, report, actual_target)
    return entries


def select_rows(
    report: dict[str, Any],
    *,
    rows: Sequence[str] | None,
    all_rows: bool,
    shard: int,
    checkpoint: Mapping[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    """Deterministic, bounded row selection; completed rows never replay."""

    report_rows = report.get("rows")
    if not isinstance(report_rows, list):
        raise ValueError("report rows must be an array")
    return _select_rows(
        report_rows, rows=rows, all_rows=all_rows, shard=shard, checkpoint=checkpoint
    )


def checkpoint_path(root: Path, report: Path, target: str | None = None) -> Path:
    from harness.naming.namespace import journal_path

    actual_target = target or str(json.loads(report.read_text())["target"])
    return journal_path(root, report, actual_target)


def evidence_dir(root: Path, report: Path, target: str) -> Path:
    from harness.naming.namespace import journal_dir

    return journal_dir(root, report, target)


def manifest_path(root: Path, report: Path, target: str | None = None) -> Path:
    """Return the digest-bound manifest path for callers."""

    from harness.naming.namespace import manifest_path as path_for

    actual_target = target or str(json.loads(report.read_text())["target"])
    return path_for(root, report, actual_target)
