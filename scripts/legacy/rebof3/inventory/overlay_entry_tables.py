from __future__ import annotations

import argparse
import struct
from collections import defaultdict
from pathlib import Path

from ..cli import add_logging_args, logger_from_args, package_prog
from .direct_overlay_catalog import (
    DEFAULT_EMI_ROOT,
    build_catalog,
    payload_bytes_from_candidate,
)
from ..common import (
    emit_output_summary,
    ensure_output_parents,
    write_json_output,
    write_markdown_output,
)
from .db.connection import connect_inventory_database
from .db.migrations import ensure_inventory_schema
from .layout import INVENTORY_SQLITE
from .repositories.overlays import OverlayRepository

MIN_PAYLOAD_SIZE = 0x2000
MAX_COUNT = 64
PREVIEW_POINTERS = 8


def load_entries(emi_root: Path) -> list[dict]:
    rows: list[dict] = []
    catalog = build_catalog(emi_root)
    for candidate in catalog["candidates"]:
        if candidate["size"] < MIN_PAYLOAD_SIZE:
            continue

        data = payload_bytes_from_candidate(candidate)
        if len(data) < 8:
            continue

        first_word = struct.unpack_from("<I", data, 0)[0]
        if (
            first_word == 0
            or first_word > MAX_COUNT
            or first_word != candidate["first4"]
        ):
            continue

        base = candidate["ram_ptr"]
        pointer_words = min(first_word, max(0, (len(data) // 4) - 1))
        all_pointers: list[int] = []
        in_range_total = 0
        for i in range(1, pointer_words + 1):
            word_offset = i * 4
            if word_offset + 4 > len(data):
                break
            value = struct.unpack_from("<I", data, word_offset)[0]
            all_pointers.append(value)
            if base <= value < base + len(data):
                in_range_total += 1

        pointers = [f"0x{value:08x}" for value in all_pointers[:PREVIEW_POINTERS]]
        in_range = sum(
            1
            for value in all_pointers[:PREVIEW_POINTERS]
            if base <= value < base + len(data)
        )

        if not pointers or in_range < min(4, len(pointers)):
            continue

        rows.append(
            {
                "archive_id": candidate["archive_id"],
                "candidate_name": candidate["candidate_name"],
                "emi_path": candidate["emi_path"],
                "entry_index": candidate["entry_index"],
                "entry_name": candidate["entry_name"],
                "payload_path": candidate["payload_path"],
                "ram_ptr_hex": candidate["ram_ptr_hex"],
                "size": candidate["size"],
                "first4": candidate["first4"],
                "first_word": first_word,
                "entry_count": first_word,
                "entry_addresses": [f"0x{value:08x}" for value in all_pointers],
                "entry_in_range_count": in_range_total,
                "pointer_preview": pointers,
                "preview_pointer_count": len(pointers),
                "preview_in_range_count": in_range,
            }
        )
    return rows


def build_groups(rows: list[dict]) -> list[dict]:
    grouped: dict[tuple[int, str, int], list[dict]] = defaultdict(list)
    for row in rows:
        key = (row["first4"], row["ram_ptr_hex"], row["size"])
        grouped[key].append(row)

    groups = []
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
                        "entry_index": item["entry_index"],
                        "payload_path": item["payload_path"],
                    }
                    for item in items
                ],
            }
        )
    return groups


def render_markdown(rows: list[dict], groups: list[dict]) -> str:
    lines = [
        "# Overlay Entry Tables",
        "",
        "This report lists large type-`0` EMI payloads whose first word matches the TOC `first4` value",
        "and whose first few following words look like in-range code pointers inside the same payload.",
        "The scan now reads directly from the original `build/extracted/**/*.EMI` archives.",
        "",
        f"- candidate count: `{len(rows)}`",
        f"- grouped patterns: `{len(groups)}`",
        "",
        "## Dominant Groups",
        "",
        "| Count | `first4` | Load Address | Size | Example |",
        "| ---: | ---: | --- | ---: | --- |",
    ]

    for group in groups[:20]:
        lines.append(
            f"| {group['count']} | {group['first4']} | `{group['ram_ptr_hex']}` | "
            f"{group['size']} | `{group['example_archive_id']}:{group['example_entry_index']}` |"
        )

    lines.extend(
        [
            "",
            "## Sample Candidates",
            "",
            "| Archive | Entry | Load Address | `first4` | Pointer Preview |",
            "| --- | ---: | --- | ---: | --- |",
        ]
    )

    for row in rows[:20]:
        preview = ", ".join(row["pointer_preview"][:4])
        lines.append(
            f"| `{row['archive_id']}` | {row['entry_index']} | `{row['ram_ptr_hex']}` | "
            f"{row['first4']} | `{preview}` |"
        )

    return "\n".join(lines) + "\n"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("inventory", "overlay-entry-tables"),
        description="Scan overlay candidates for entry-table-shaped pointer arrays.",
    )
    add_logging_args(parser)
    parser.add_argument("--emi-root", type=Path, default=DEFAULT_EMI_ROOT)
    parser.add_argument("--db", type=Path, default=INVENTORY_SQLITE)
    parser.add_argument(
        "--json-out", type=Path, default=None, help="optional JSON output"
    )
    parser.add_argument(
        "--md-out", type=Path, default=None, help="optional Markdown output"
    )
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "overlay_entry_tables")
    rows = load_entries(args.emi_root)
    groups = build_groups(rows)
    for row in rows:
        entry_count = int(row.get("entry_count") or row.get("first_word") or 0)
        in_range_total = int(row.get("entry_in_range_count") or 0)
        if entry_count <= 0:
            row["confidence"] = "low"
        else:
            ratio = in_range_total / entry_count
            row["confidence"] = (
                "high" if ratio >= 0.9 else "medium" if ratio >= 0.6 else "low"
            )

    connection = connect_inventory_database(args.db)
    ensure_inventory_schema(connection)
    OverlayRepository(connection).replace_entry_tables(rows)
    connection.close()

    ensure_output_parents(args.json_out, args.md_out)
    if args.json_out is not None:
        write_json_output(
            args.json_out,
            {
                "candidate_count": len(rows),
                "group_count": len(groups),
                "candidates": rows,
                "groups": groups,
            },
        )
    if args.md_out is not None:
        write_markdown_output(args.md_out, render_markdown(rows, groups))
    emit_output_summary(
        logger,
        summary=f"overlay entry-table candidates={len(rows)} groups={len(groups)}",
        json_path=args.json_out,
        md_path=args.md_out,
    )
    return 0
