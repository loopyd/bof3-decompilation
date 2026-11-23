"""PsyQ SDK map operations: provenance import, bindings, and reference reports."""

from __future__ import annotations

import json
from pathlib import Path
import re
from typing import Any

from ..domain.ids import FUNCTION_ID_FORMAT, normalize_target_id
from ..domain.sdk_map import (
    SdkSymbol,
    load_sdk_map,
    project_symbols,
    sdk_map_path,
    write_sdk_map,
)
from ..domain.symbols import (
    MapSymbol,
    weak_bindings_c,
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
    """Apply reviewed exact PsyQ provenance to the combined SDK map.

    Each symbol line lists every verified ``target@address`` location, so an
    overlay that carries its own copy of a library routine records that copy
    beside the resident executable's. One name may not hold two addresses inside
    the same target, and a semantic name is never replaced at an occupied
    location.
    """
    path = sdk_map_path(root)
    existing = load_sdk_map(path) if path.is_file() else []
    by_location = {(symbol.target, symbol.address): symbol for symbol in existing}
    by_name_target = {(symbol.name, symbol.target): symbol for symbol in existing}
    added: list[SdkSymbol] = []
    changed = False
    for row in selected:
        target = normalize_target_id(str(row["target"])).value
        if target not in manifests:
            raise ValueError(f"proposal names an unknown target: {target}")
        address = int(str(row["address"]), 16)
        name = str(row["name"])
        current = by_location.get((target, address))
        same_name = by_name_target.get((name, target))
        if same_name is not None and same_name.address != address:
            raise ValueError(
                f"{target}: PsyQ name already belongs to "
                f"0x{same_name.address:08X}: {name}"
            )
        if current is not None and current.name == name:
            continue
        if current is not None and not MapSymbol(address, current.name).is_raw:
            raise ValueError(
                f"{target}: address 0x{address:08X} already has semantic name "
                f"{current.name}"
            )
        if current is not None:
            existing = [
                symbol
                for symbol in existing
                if (symbol.target, symbol.address) != (target, address)
            ]
        added.append(SdkSymbol(name, target, address))
        by_location[(target, address)] = SdkSymbol(name, target, address)
        by_name_target[(name, target)] = SdkSymbol(name, target, address)
        changed = True
    if not changed:
        return False, [f"unchanged {path.relative_to(root)}"]
    if write:
        write_sdk_map(path, existing + added)
        return True, [f"wrote {path.relative_to(root)}"]
    return True, [f"would write {path.relative_to(root)}"]


def sdk_weak_bindings(root: Path, manifest: Any) -> str:
    """Render one target's claimed PsyQ weak-binding source from the SDK map.

    Every target must declare ``psyq_source``; the output path is never
    guessed from ``source_dir`` or a filename stem.
    """
    content = weak_bindings_c(
        project_symbols(
            load_sdk_map(sdk_map_path(root)),
            target=manifest.id.value,
            space=manifest.psyq_space,
        )
    )
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
    sdk = project_symbols(load_sdk_map(sdk_map_path(root)), target=target, space=space)
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
