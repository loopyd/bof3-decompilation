"""Scoped prepared-row repair for the ``naming-audit`` readiness preflight.

``prepare`` is the readiness preflight: it classifies lift metadata debt and,
behind evidence-atomic live exact proof, rewrites only proven-exact progress
metadata. A supplied row selector list (sorted, unique, already classified
``safe_metadata_repair`` by the unscoped preflight) restricts the repair to
those rows; all selected proofs must pass before any write.
"""

from __future__ import annotations

import os
import re
from collections.abc import Sequence
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from harness.domain.claims import manifest_source_paths
from harness.naming.debt import address_of
from harness.domain.tags import (
    STATUS_TAG_RE,
    canonical_exact_progress,
    parse_source_tag,
)

ROW_SELECTOR_RE = re.compile(r"^(function:func_[0-9A-F]{8}|data:D_[0-9A-F]{8})$")


class RollbackError(RuntimeError):
    """Rollback itself failed; ``snapshot`` keeps every file recoverable."""

    def __init__(self, message: str, snapshot: dict[str, str]) -> None:
        super().__init__(message)
        self.snapshot = snapshot


@dataclass
class _RepairTransaction:
    """One bounded repair batch: proven rows plus the file pre-images."""

    entries: list[dict[str, Any]]
    snapshots: dict[str, str] = field(default_factory=dict)
    written: list[str] = field(default_factory=list)

    def restore(self, root: Path) -> None:
        """Restore written files atomically; a failure keeps the snapshots."""

        for relative in self.written:
            _atomic_write(root, relative, self.snapshots[relative])
        self.written = []


def _atomic_write(root: Path, relative: str, text: str) -> None:
    """Write one file via same-directory temp plus atomic rename."""

    target = root / relative
    temp = target.with_name(f"{target.name}.rollback-tmp")
    temp.write_text(text, encoding="utf-8")
    os.replace(temp, target)


def validate_row_selectors(rows: list[str]) -> None:
    """Reject malformed, duplicate, or unsorted prepared-row selectors."""

    for row in rows:
        if not ROW_SELECTOR_RE.fullmatch(row):
            raise ValueError(f"invalid row selector: {row}")
    if len(rows) != len(set(rows)):
        raise ValueError(f"duplicate row selector in: {rows!r}")
    if rows != sorted(rows):
        raise ValueError(f"row selectors must be sorted: {rows!r}")


def select_rows_findings(rows: list[str], findings: list[dict[str, Any]]) -> None:
    """Reject selectors that are unknown or not safe metadata repairs."""

    by_row = {str(finding["row"]): finding for finding in findings}
    for row in rows:
        finding = by_row.get(row)
        if finding is None:
            raise ValueError(f"unknown target row: {row}")
        if finding.get("class") != "safe_metadata_repair":
            raise ValueError(f"row is not a safe metadata repair: {row}")


def source_addresses(root: Path, manifest: Any) -> dict[int, Path]:
    """Map each claimed lift's @source address to its file, once."""

    by_address: dict[int, Path] = {}
    try:
        sources = manifest_source_paths(root, manifest)
    except ValueError:
        return by_address
    for source in sources:
        if source.suffix != ".c":
            continue
        address = parse_source_tag(source.read_text(encoding="utf-8"))
        if address is not None:
            by_address[address] = source
    return by_address


def merge_findings_notes(
    findings: list[dict[str, Any]], notes: dict[str, dict[str, Any]]
) -> list[dict[str, Any]]:
    """Carry live-proof and rollback notes across the post-repair recompute."""

    if not notes:
        return findings
    return [
        {**finding, **notes[str(finding["row"])]}
        if str(finding["row"]) in notes
        else finding
        for finding in findings
    ]


def repair_selected_rows(
    root: Path,
    findings: list[dict[str, Any]],
    by_address: dict[int, Path],
    *,
    rows: Sequence[str] | None,
    live_exact: Any,
) -> tuple[list[dict[str, Any]], dict[str, dict[str, Any]]]:
    """Prove, repair, and report the selected rows.

    ``rows is None`` keeps the legacy repair of every safe row; a supplied
    list (even empty) restricts the repair to exactly those rows. Both modes
    are evidence-atomic across selected rows. Returns
    the ``repaired`` records and the re-classified findings that keep a
    live-proof or rollback note across the post-repair recompute.
    """

    selected = None if rows is None else set(rows)
    transaction = _RepairTransaction(entries=[])
    notes: dict[str, dict[str, Any]] = {}
    for finding in findings:
        if finding.get("class") != "safe_metadata_repair":
            continue
        row = str(finding["row"])
        if selected is not None and row not in selected:
            continue
        source = by_address.get(address_of(row))
        if source is None:
            continue
        exact, commands = live_exact(address_of(row))
        if not exact:
            finding["class"] = "review_required"
            finding["reason"] = "live exact proof failed; no selected row was written"
            finding["validation"] = commands
            notes[row] = finding
            continue
        relative = source.relative_to(root).as_posix()
        text = source.read_text(encoding="utf-8")
        transaction.entries.append(
            {"row": row, "file": relative, "validation": commands}
        )
        transaction.snapshots.setdefault(relative, text)
    if notes:
        return [], notes
    repaired: list[dict[str, Any]] = []
    current: dict[str, Any] | None = None
    try:
        for entry in transaction.entries:
            current = entry
            relative = entry["file"]
            before = transaction.snapshots[relative]
            fixed, changed = canonical_exact_progress(before)
            if changed:
                _atomic_write(root, relative, fixed)
                transaction.written.append(relative)
            if changed:
                entry["repair"] = "repaired"
            elif STATUS_TAG_RE.search(before):
                entry["repair"] = "already-canonical"
            else:
                entry["repair"] = "no-status-tags"
            repaired.append(entry)
    except (OSError, ValueError, UnicodeError):
        try:
            transaction.restore(root)
        except OSError as error:
            raise RollbackError(
                f"repair failed and its rollback failed on "
                f"{error.filename or 'a repair file'}; recover from the "
                f"error snapshot ({len(transaction.snapshots)} file pre-images)",
                transaction.snapshots,
            ) from error
        if current is None:
            raise
        failed = current
        notes[failed["row"]] = {
            "row": failed["row"],
            "class": "review_required",
            "reason": f"repair failed; rolled back: {failed['row']}",
            "validation": failed["validation"],
        }
        repaired = []
    return repaired, notes
