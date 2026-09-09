"""Discover and inspect target-local raw symbol naming leads without mutation."""

from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Any

from harness.common.digests import digest
from harness.domain.ids import normalize_target_id
from harness.domain.manifests import load_target_manifests
from harness.domain.symbols import map_path, parse_map
from harness.naming.debt import classify_raw_symbol, collect_symbol_debt


def collect_inventory(root: Path, target: str) -> list[dict[str, Any]]:
    """Preserve the complete legacy raw-function/data inventory projection."""
    target = normalize_target_id(target).value
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    raw_functions, raw_data = collect_symbol_debt(root, {target: manifests[target]})
    return [
        {"kind": kind, "name": row.split(":", 1)[1]}
        for kind, entries in (
            ("function", raw_functions),
            ("data", raw_data),
        )
        for row in sorted(entries)
        if row.startswith(f"{target}:")
    ]


def collect_opportunities(root: Path, target: str) -> list[dict[str, Any]]:
    """Bind deterministic leads to one parsed target-map byte snapshot."""
    target = normalize_target_id(target).value
    if target not in load_target_manifests(root):
        raise ValueError(f"unknown target: {target}")
    path = map_path(root, target)
    original = path.read_bytes()
    source = path.relative_to(root).as_posix()
    map_sha256 = hashlib.sha256(original).hexdigest()
    rows = []
    for symbol in parse_map(
        original.decode("utf-8"), source=source, canonicalize=False
    ):
        kind = classify_raw_symbol(symbol.name)
        if kind is None:
            continue
        row = {
            "schema": "bof3.naming-opportunity/v1",
            "id": f"{target}@{kind}:{symbol.name}",
            "target": target,
            "kind": kind,
            "name": symbol.name,
            "address": f"0x{symbol.address:08X}",
            "map": source,
            "map_sha256": map_sha256,
        }
        rows.append({**row, "fingerprint": digest(row)})
    return sorted(rows, key=lambda row: (row["kind"] != "function", row["name"]))


def describe_opportunity(
    root: Path, target: str, identifier: str, *, expected_fingerprint: str | None = None
) -> dict[str, Any]:
    """Require exact current lead membership and an optional caller-retained pin."""
    rows = collect_opportunities(root, target)
    matches = [row for row in rows if row["id"] == identifier]
    if len(matches) != 1:
        raise ValueError("unknown naming opportunity for target")
    row = matches[0]
    if expected_fingerprint is not None and row["fingerprint"] != expected_fingerprint:
        raise ValueError("naming opportunity fingerprint drifted")
    return row
