"""Describe reviewed storage boundaries and available indexed access widths."""

from __future__ import annotations

from pathlib import Path
from typing import Any

from harness.domain.ids import normalize_target_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.psx import binary_offset_for
from harness.domain.registry import payload_end_for
from harness.domain.symbols import load_target_symbols

from .index import connect

_KINDS = {"bss", "rodata", "data"}


def canonical_storage(root: Path, target: str, address: int) -> dict[str, Any]:
    """Derive storage from reviewed layout and original binary presence."""

    target = normalize_target_id(target).value
    root = root.resolve()
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    manifest = manifests[target]
    binary = root / manifest.binary
    if not binary.is_file():
        raise ValueError(f"target binary not found: {manifest.binary}")
    binary_offset = binary_offset_for(binary.read_bytes())
    payload_end = payload_end_for(root, manifest)
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    boundary = layout.find_containing_boundary(address)
    mapped = manifest.load_address <= address < payload_end
    splat_kind = boundary.kind if boundary is not None else None
    start = address
    symbols = load_target_symbols(root, target, psyq_space=manifest.psyq_space)
    later = sorted(symbol.address for symbol in symbols if symbol.address > address)
    boundary_end = boundary.virtual_end if boundary is not None else None
    end = min(
        later + ([boundary_end] if boundary_end is not None else []),
        default=address + 1,
    )
    widths = {"lb": 1, "lbu": 1, "sb": 1, "lh": 2, "lhu": 2, "sh": 2, "lw": 4, "sw": 4}
    try:
        connection = connect(root)
        try:
            opcodes = [
                row[0]
                for row in connection.execute(
                    "SELECT DISTINCT opcode FROM data_references WHERE target_id = ? AND address = ?",
                    (target, address),
                )
                if row[0] in widths
            ]
        finally:
            connection.close()
        if opcodes:
            end = min(end, address + max(widths[opcode] for opcode in opcodes))
    except (FileNotFoundError, ValueError):
        pass
    if end <= address:
        end = address + 1
    authority = []
    if boundary is not None:
        authority.append("reviewed_splat")
    if splat_kind in _KINDS:
        kind = splat_kind
    elif mapped:
        kind = "data"
    else:
        kind = "unknown"
    if mapped:
        authority.append("original_binary")
    return {
        "kind": kind,
        "start": f"0x{start:08X}",
        "end": f"0x{end:08X}",
        "file_offset": f"0x{binary_offset + address - manifest.load_address:X}"
        if mapped
        else None,
        "present_in_binary": mapped and kind != "bss",
        "authority": authority,
    }
