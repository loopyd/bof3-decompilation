"""Corroborate indexed cross-image direct calls without asserting runtime ownership."""

from __future__ import annotations

import struct
from pathlib import Path

from harness.analysis.project import prepare_target
from harness.domain.ids import parse_function_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests


def cross_target_callers(
    root: Path, target: str, call_rows
) -> dict[int, tuple[str, ...]]:
    """Retain global direct-call leads for reviewed starts in the selected image.

    Invalid caller ranges remain work, so capture reports their gap. A call word
    corroborates an address only; it does not identify the active overlay.
    """
    rows = [
        (caller, address, site)
        for caller, callee, address, site in call_rows
        if callee is None and not caller.startswith(target + "@")
    ]
    if not rows:
        return {}
    manifests = load_target_manifests(root)
    selected = manifests[target]
    layout = parse_splat_layout(root / selected.splat, selected.load_address)
    starts = {b.virtual_start for b in layout.boundaries if b.is_function}
    result: dict[int, set[str]] = {}
    images: dict[str, tuple[object, bytes]] = {}
    for caller, address, site in rows:
        if address not in starts:
            continue
        identity = parse_function_id(caller)
        caller_target = identity.target.value
        # Preserve unavailable image/range leads rather than treating them as no call.
        result.setdefault(address, set()).add(str(identity))
        if caller_target not in manifests:
            continue
        if caller_target not in images:
            try:
                spec = prepare_target(
                    root, caller_target, manifest=manifests[caller_target]
                )
                images[caller_target] = spec, spec.binary.read_bytes()
            except (OSError, ValueError):
                continue
        spec, image = images[caller_target]
        offset = spec.binary_offset + site - spec.load_address
        if not spec.binary_offset <= offset <= len(image) - 8:
            continue
        word = struct.unpack_from("<I", image, offset)[0]
        destination = ((site + 4) & 0xF0000000) | ((word & 0x3FFFFFF) << 2)
        if word >> 26 != 3 or destination != address:
            # The producer validates the full body and selected call again. Keep
            # stale indexed candidates visible until that validation can refuse.
            continue
    return {address: tuple(sorted(callers)) for address, callers in result.items()}


def generated_callers(row: dict, target: str) -> frozenset[str]:
    """Return only caller identities in the caller-owned generated work plan."""
    del target
    return frozenset(
        str(parse_function_id(item["id"].split(":", 1)[1]))
        for item in row.get("required_work", [])
        if item.get("status") == "open"
        and str(item.get("id", "")).startswith("caller:")
    )
