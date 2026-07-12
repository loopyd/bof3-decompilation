from __future__ import annotations

import struct
from collections import defaultdict
from pathlib import Path
from typing import Any


MIN_PAYLOAD_SIZE = 0x2000
MAX_COUNT = 64
PREVIEW_POINTERS = 8


def payload_bytes(candidate: dict[str, Any]) -> bytes:
    return Path(str(candidate["payload_path"])).read_bytes()


def entry_table_row(candidate: dict[str, Any]) -> dict[str, Any] | None:
    if int(candidate["size"]) < MIN_PAYLOAD_SIZE:
        return None

    data = payload_bytes(candidate)
    if len(data) < 8:
        return None

    first_word = struct.unpack_from("<I", data, 0)[0]
    if (
        first_word == 0
        or first_word > MAX_COUNT
        or first_word != int(candidate["first4"])
    ):
        return None

    base_addr = int(candidate["ram_ptr"])
    pointer_words = min(first_word, max(0, (len(data) // 4) - 1))
    all_pointers: list[int] = []
    in_range_total = 0
    for index in range(1, pointer_words + 1):
        word_offset = index * 4
        if word_offset + 4 > len(data):
            break
        value = struct.unpack_from("<I", data, word_offset)[0]
        all_pointers.append(value)
        if base_addr <= value < base_addr + len(data):
            in_range_total += 1

    preview = [f"0x{value:08x}" for value in all_pointers[:PREVIEW_POINTERS]]
    preview_in_range = sum(
        1
        for value in all_pointers[:PREVIEW_POINTERS]
        if base_addr <= value < base_addr + len(data)
    )
    if not preview or preview_in_range < min(4, len(preview)):
        return None

    return {
        "archive_id": candidate["archive_id"],
        "candidate_name": candidate["candidate_name"],
        "entry_count": first_word,
        "entry_in_range_count": in_range_total,
        "entry_index": candidate["entry_index"],
        "entry_addresses": [f"0x{value:08x}" for value in all_pointers],
        "family": candidate["family"],
        "first4": candidate["first4"],
        "payload_path": candidate["payload_path"],
        "program_id": candidate["program_id"],
        "preview_in_range_count": preview_in_range,
        "pointer_preview": preview,
        "preview_pointer_count": len(preview),
        "ram_ptr_hex": candidate["ram_ptr_hex"],
        "size": candidate["size"],
    }


def confidence_for_row(row: dict[str, Any]) -> str:
    entry_count = int(row.get("entry_count") or 0)
    if entry_count <= 0:
        return "low"
    ratio = int(row.get("entry_in_range_count") or 0) / entry_count
    if ratio >= 0.9:
        return "high"
    if ratio >= 0.6:
        return "medium"
    return "low"


def build_groups(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    grouped: dict[tuple[int, str, int], list[dict[str, Any]]] = defaultdict(list)
    for row in rows:
        grouped[(int(row["first4"]), str(row["ram_ptr_hex"]), int(row["size"]))].append(
            row
        )

    groups: list[dict[str, Any]] = []
    for (first4, ram_ptr_hex, size), items in sorted(
        grouped.items(),
        key=lambda item: (-len(item[1]), item[0][1], item[0][2], item[0][0]),
    ):
        groups.append(
            {
                "first4": first4,
                "ram_ptr_hex": ram_ptr_hex,
                "size": size,
                "count": len(items),
                "example_archive_id": items[0]["archive_id"],
                "example_entry_index": items[0]["entry_index"],
                "example_pointer_preview": items[0]["pointer_preview"],
                "members": [
                    {
                        "archive_id": item["archive_id"],
                        "candidate_name": item["candidate_name"],
                        "entry_index": item["entry_index"],
                        "payload_path": item["payload_path"],
                    }
                    for item in items
                ],
            }
        )
    return groups


def build_entry_tables_catalog(catalog: dict[str, Any]) -> dict[str, Any]:
    rows = []
    for candidate in catalog["candidates"]:
        row = entry_table_row(candidate)
        if row is None:
            continue
        row["confidence"] = confidence_for_row(row)
        rows.append(row)
    groups = build_groups(rows)
    return {
        "schema": "harness.inventory-entry-tables/v1",
        "candidate_count": len(rows),
        "group_count": len(groups),
        "candidates": rows,
        "groups": groups,
    }


def render_entry_tables_markdown(catalog: dict[str, Any]) -> str:
    lines = [
        "# Overlay Entry Tables",
        "",
        "Machine-generated report for overlay payloads that look like entry-table roots.",
        "",
        f"- Candidate count: {catalog['candidate_count']}",
        f"- Group count: {catalog['group_count']}",
        "",
        "## Dominant Groups",
        "",
    ]
    for group in catalog["groups"][:20]:
        lines.append(
            f"- `{group['ram_ptr_hex']}` size `{group['size']}` first4 `{group['first4']}`: {group['count']} members"
        )
    return "\n".join(lines) + "\n"
