"""Bulk naming-audit inventory and atomic report-set publication."""

from __future__ import annotations

import shutil
from pathlib import Path
from typing import Any

from ..domain.naming_debt import collect_naming_debt


def expected_inventories(
    root: Path, manifests: dict[str, Any]
) -> dict[str, set[tuple[str, str]]]:
    """Collect naming debt once and partition it by target."""

    expected: dict[str, set[tuple[str, str]]] = {target: set() for target in manifests}
    debt = collect_naming_debt(root, manifests)
    for kind, values in (
        ("function", debt.raw_functions),
        ("data", debt.raw_data),
    ):
        for value in values:
            target, name = value.split(":", 1)
            expected[target].add((kind, name))
    return expected


def publish_reports(staging: Path, output: Path) -> None:
    """Atomically replace the complete report set, restoring it on swap failure."""

    output.parent.mkdir(parents=True, exist_ok=True)
    backup = output.parent / f".{output.name}.backup"
    if backup.exists():
        shutil.rmtree(backup)
    moved = False
    try:
        if output.exists():
            output.replace(backup)
            moved = True
        staging.replace(output)
    except BaseException:
        if moved and not output.exists():
            backup.replace(output)
        raise
    finally:
        shutil.rmtree(backup, ignore_errors=True)


__all__ = ["expected_inventories", "publish_reports"]
