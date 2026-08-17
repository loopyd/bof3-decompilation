"""PsyQ SDK map operations: provenance import, bindings, and reference reports."""

from __future__ import annotations

import json
from pathlib import Path
import re
from typing import Any

from ..domain import FUNCTION_ID_FORMAT, normalize_target_id
from ..domain.symbols import (
    MapSymbol,
    format_map,
    load_map,
    sdk_map_path,
    weak_bindings_c,
    write_map,
)


def parse_psyq_find(path: Path) -> list[dict[str, object]]:
    """Return the exact external matches of one psyq-find proposal."""
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict) or payload.get("schema") != "bof3.psyq-find/v1":
        raise ValueError(f"not a psyq-find proposal: {path}")
    rows = payload.get("matches")
    if not isinstance(rows, list):
        raise ValueError(f"invalid psyq-find matches: {path}")
    valid: list[dict[str, object]] = []
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError(f"invalid psyq-find match: {path}")
        if not all(
            isinstance(row.get(key), str) for key in ("target", "address", "name")
        ):
            raise ValueError(f"invalid psyq-find match: {path}")
        if row.get("confidence") == "exact" and row.get("external") is True:
            valid.append(row)
    return valid


def _selector(row: dict[str, object]) -> str:
    return f"{row['target']}@0x{int(str(row['address']), 16):08X}"


def select_import_rows(
    rows: list[dict[str, object]],
    selectors: set[str],
    all_qualified: bool,
) -> list[dict[str, object]]:
    """Filter proposal rows to the requested selectors or --all-qualified."""
    if all_qualified and selectors:
        raise ValueError(
            f"--all-qualified cannot be combined with {FUNCTION_ID_FORMAT}"
        )
    if not all_qualified and not selectors:
        raise ValueError(
            f"select at least one {FUNCTION_ID_FORMAT} or pass --all-qualified"
        )
    selected = (
        rows if all_qualified else [row for row in rows if _selector(row) in selectors]
    )
    found = {_selector(row) for row in selected}
    missing = sorted(selectors - found)
    if missing:
        raise ValueError(f"proposal has no exact PsyQ match for: {missing[0]}")
    return selected


def apply_psyq_provenance(
    root: Path,
    manifests: dict[str, Any],
    selected: list[dict[str, object]],
    *,
    write: bool,
) -> tuple[bool, list[str]]:
    """Apply reviewed exact PsyQ provenance to the shared SDK maps.

    PsyQ symbols are owned by the SDK space (slus/logo), not by one target, so
    candidates are grouped by the proposing target's space and written to
    config/sdk/psyq-<space>.txt where every target in that space picks them up.
    Returns (changed, display messages).
    """
    by_space: dict[str, list[dict[str, object]]] = {}
    for row in selected:
        target = normalize_target_id(str(row["target"])).value
        if target not in manifests:
            raise ValueError(f"proposal names an unknown target: {target}")
        by_space.setdefault(manifests[target].psyq_space, []).append(row)
    changed = False
    messages: list[str] = []
    for space, rows in sorted(by_space.items()):
        path = sdk_map_path(root, space)
        existing = load_map(path)
        by_address = {symbol.address: symbol for symbol in existing}
        by_name = {symbol.canonical_name: symbol for symbol in existing}
        replacement: dict[int, MapSymbol] = {}
        for row in sorted(
            rows, key=lambda item: (str(item["address"]), str(item["name"]))
        ):
            address = int(str(row["address"]), 16)
            candidate = MapSymbol(address, str(row["name"]))
            current = by_address.get(address)
            same_name = by_name.get(candidate.canonical_name)
            if same_name is not None and same_name.address != address:
                raise ValueError(
                    f"{space}: PsyQ name already belongs to "
                    f"0x{same_name.address:08X}: {candidate.canonical_name}"
                )
            if (
                current is not None
                and current.canonical_name == candidate.canonical_name
            ):
                continue
            if current is not None and not current.is_raw:
                raise ValueError(
                    f"{space}: address 0x{address:08X} already has semantic name "
                    f"{current.canonical_name}"
                )
            replacement[address] = candidate
            by_address[address] = candidate
            by_name[candidate.canonical_name] = candidate
        if not replacement:
            messages.append(f"unchanged {path.relative_to(root)}")
            continue
        changed = True
        updated = [replacement.get(symbol.address, symbol) for symbol in existing]
        for address, symbol in replacement.items():
            if not any(old.address == address for old in existing):
                updated.append(symbol)
        format_map(updated)
        if write:
            write_map(path, updated)
            messages.append(f"wrote {path.relative_to(root)}")
        else:
            messages.append(f"would write {path.relative_to(root)}")
    return changed, messages


def sdk_weak_bindings(root: Path, manifest: Any) -> str:
    """Render one target's claimed PsyQ weak-binding source from the SDK map.

    Every target must declare ``psyq_source``; the output path is never
    guessed from ``source_dir`` or a filename stem.
    """
    space = manifest.psyq_space
    content = weak_bindings_c(load_map(sdk_map_path(root, space)))
    if not manifest.psyq_source:
        raise ValueError(
            f"target {manifest.id.value} must declare psyq_source in its manifest"
        )
    return content


def sdk_references(
    root: Path, manifests: dict[str, Any], target: str
) -> tuple[str, list[MapSymbol], int]:
    """Return (psyq space, sorted referenced SDK symbols, total) for one target.

    Scans metadata-owned lift sources (recursive) plus target headers, not
    the generated bindings.
    """
    manifest = manifests[target]
    space = manifest.psyq_space
    sdk = load_map(sdk_map_path(root, space))
    source_dir = root / manifest.source_dir
    from ..domain.claims import (
        collect_manifest_source_addresses,
        manifest_header_paths,
    )

    haystack_paths = {
        source for source, _ in collect_manifest_source_addresses(root, manifest)
    }
    haystack_paths.update(
        path
        for path in manifest_header_paths(root, manifest)
        if path.name == "internal.h"
    )
    public_dir = source_dir / "public"
    if public_dir.is_dir():
        haystack_paths.update(sorted(public_dir.glob("*.h")))
    haystacks = sorted(haystack_paths)
    text = "".join(path.read_text(encoding="utf-8") for path in haystacks)
    referenced = sorted(
        (
            symbol
            for symbol in sdk
            if re.search(rf"\b{re.escape(symbol.canonical_name)}\b", text)
        ),
        key=lambda symbol: symbol.address,
    )
    return space, referenced, len(sdk)
