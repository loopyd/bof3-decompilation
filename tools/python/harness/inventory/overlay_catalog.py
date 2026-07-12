from __future__ import annotations

import struct
from collections import Counter
from pathlib import Path
from typing import Any

from ..models import DuplicateGroups, InventoryProgram, InventorySnapshot


def sanitize(text: str) -> str:
    cleaned = []
    for char in text:
        cleaned.append(char.lower() if char.isalnum() else "_")
    collapsed = "".join(cleaned).strip("_")
    while "__" in collapsed:
        collapsed = collapsed.replace("__", "_")
    return collapsed or "unknown"


def candidate_name(program: InventoryProgram) -> str:
    archive_name = (
        program.archive_id.split("/")[-1]
        if program.archive_id
        else program.program_name
    )
    entry_index = program.entry_index if program.entry_index is not None else 0
    base_addr = program.base_addr if program.base_addr is not None else 0
    family = program.family or "unknown"
    return (
        f"ovl_{sanitize(family)}_{sanitize(archive_name)}"
        f"_e{entry_index:02d}_{base_addr:08x}"
    )


def read_first_word(path: Path) -> int:
    data = path.read_bytes()
    if len(data) < 4:
        return 0
    return struct.unpack_from("<I", data, 0)[0]


def build_duplicate_group_sizes(groups: DuplicateGroups | None) -> dict[str, int]:
    if groups is None:
        return {}
    sizes: dict[str, int] = {}
    for group in groups.groups:
        group_size = len(group.member_program_ids)
        for member_id in group.member_program_ids:
            sizes[member_id] = group_size
    return sizes


def overlay_candidate_record(
    program: InventoryProgram,
    *,
    duplicate_group_size: int,
) -> dict[str, Any]:
    payload_path = Path(program.payload_path)
    base_addr = program.base_addr if program.base_addr is not None else 0
    entry_index = program.entry_index if program.entry_index is not None else 0
    archive_name = (
        program.archive_id.split("/")[-1]
        if program.archive_id
        else payload_path.parent.name
    )
    return {
        "archive_id": program.archive_id,
        "archive_name": archive_name,
        "candidate_name": candidate_name(program),
        "entry_index": entry_index,
        "family": program.family,
        "first4": read_first_word(payload_path),
        "payload_path": program.payload_path,
        "program_id": program.program_id,
        "program_name": program.program_name,
        "project_folder_path": program.project_folder_path,
        "ram_ptr": base_addr,
        "ram_ptr_hex": f"0x{base_addr:08x}",
        "sha256": program.sha256,
        "size": program.size,
        "duplicate_group_size": duplicate_group_size,
    }


def build_overlay_catalog(
    snapshot: InventorySnapshot,
    groups: DuplicateGroups | None = None,
) -> dict[str, Any]:
    duplicate_group_sizes = build_duplicate_group_sizes(groups)
    candidates: list[dict[str, Any]] = []
    family_counts: Counter[str] = Counter()
    load_address_counts: Counter[str] = Counter()
    unique_hashes: set[str] = set()

    for program in sorted(snapshot.programs, key=lambda item: item.program_id):
        if program.kind != "overlay":
            continue
        record = overlay_candidate_record(
            program,
            duplicate_group_size=duplicate_group_sizes.get(program.program_id, 1),
        )
        candidates.append(record)
        family_counts[str(record["family"] or "unknown")] += 1
        load_address_counts[record["ram_ptr_hex"]] += 1
        unique_hashes.add(program.sha256)

    return {
        "schema": "harness.inventory-overlay-catalog/v1",
        "candidate_count": len(candidates),
        "family_counts": dict(sorted(family_counts.items())),
        "load_address_counts": dict(
            sorted(load_address_counts.items(), key=lambda item: (-item[1], item[0]))
        ),
        "unique_payload_hashes": len(unique_hashes),
        "candidates": candidates,
    }


def render_overlay_catalog_markdown(catalog: dict[str, Any]) -> str:
    lines = [
        "# Overlay Catalog",
        "",
        "Machine-generated catalog of overlay candidates from the scanned inventory.",
        "",
        f"- Candidate count: {catalog['candidate_count']}",
        f"- Unique payload hashes: {catalog['unique_payload_hashes']}",
        "",
        "## Families",
        "",
    ]
    for family, count in catalog["family_counts"].items():
        lines.append(f"- {family}: {count}")
    lines.extend(["", "## Representative Candidates", ""])
    for candidate in catalog["candidates"][:20]:
        lines.append(
            f"- `{candidate['candidate_name']}` from `{candidate['archive_id']}`"
            f" at `{candidate['ram_ptr_hex']}`"
        )
    return "\n".join(lines) + "\n"
