from __future__ import annotations

import argparse
import json
import re
import struct
from pathlib import Path
from typing import Any

from ..cli import package_prog
from ..common import write_json_output, write_text_output
from ..config import ROOT
from .db.connection import connect_inventory_database
from .db.migrations import ensure_inventory_schema


SLUS_PATH = ROOT / "build" / "extracted" / "SLUS_004.22"
LBA_LOG_PATH = ROOT / "processed" / "inventory" / "disc_lba.json"
INVENTORY_DIR = ROOT / "processed" / "inventory"
DB_OUT = INVENTORY_DIR / "inventory.sqlite"

SLOT_TABLE_VADDR = 0x80182444
DEFAULT_SLOT_COUNT = 0
PSX_EXE_HEADER_SIZE = 0x800

HEX_SUFFIX_RE = re.compile(r"^(?P<prefix>BPLD|BPLU|PLP|PL)(?P<index>[0-9A-Fa-f]{3})$")


def parse_psx_exe_header(data: bytes) -> dict[str, int]:
    return {
        "pc0": struct.unpack_from("<I", data, 0x10)[0],
        "text_addr": struct.unpack_from("<I", data, 0x18)[0],
        "text_size": struct.unpack_from("<I", data, 0x1C)[0],
    }


def parse_lba_log(path: Path) -> list[dict[str, Any]]:
    if path.suffix.lower() == ".json":
        payload = json.loads(path.read_text())
        entries: list[dict[str, Any]] = []
        for row in payload.get("entries", []):
            entry: dict[str, Any] = {
                "archive_name": row.get("archive_name", ""),
                "archive_type": row.get("archive_type", ""),
                "family": row.get("family", "unknown"),
                "lba": int(row["lba"]),
                "source_path": str(row.get("source_path", "")),
            }
            manifest_path = row.get("manifest_path")
            if manifest_path:
                entry["manifest_path"] = str(manifest_path)
            entries.append(entry)
        return sorted(entries, key=lambda item: item["lba"])

    entries: list[dict[str, Any]] = []

    for line in path.read_text().splitlines():
        if "|" not in line or "build/extracted/" not in line:
            continue

        parts = [part.strip() for part in line.split("|")]
        if len(parts) < 7:
            continue

        row_type, name, _length, lba_text, _timecode, _bytes, source = parts[:7]
        if row_type not in {"File", "XA"}:
            continue

        lba = int(lba_text)
        source_path = Path(source)
        rel_source = (
            source_path.relative_to(ROOT) if source_path.is_absolute() else source_path
        )
        parts = rel_source.parts
        family = (
            parts[3]
            if len(parts) >= 4 and parts[0:3] == ("build", "extracted", "BIN")
            else "unknown"
        )
        archive_path = Path(name)
        archive_stem = archive_path.stem
        archive_type = archive_path.suffix.upper().lstrip(".")

        entry = {
            "archive_name": archive_stem,
            "archive_type": archive_type,
            "family": family,
            "lba": lba,
            "source_path": rel_source.as_posix(),
        }

        if archive_type == "EMI":
            entry["manifest_path"] = (
                rel_source.as_posix()
                .replace("build/extracted/", "processed/emi_raw/")
                .replace(".EMI", "/emi.json")
            )

        entries.append(entry)

    return sorted(entries, key=lambda item: item["lba"])


def selector_hint(
    archive_name: str, archive_type: str, slot_id: int
) -> dict[str, Any] | None:
    if archive_type != "EMI":
        return None

    match = HEX_SUFFIX_RE.match(archive_name)
    if match is None:
        return None

    prefix = match.group("prefix")
    index = int(match.group("index"), 16)
    selector = {
        "BPLD": (1, 0x1DB),
        "BPLU": (2, 0x1EE),
        "PL": (0, 0x26A),
        "PLP": (3, 0x27D),
    }[prefix]
    family_selector, base_slot = selector

    return {
        "selector_family_hint": family_selector,
        "archive_suffix_hex": f"0x{index:x}",
        "slot_bucket_offset": slot_id - base_slot,
    }


def build_slot_rows(
    slots: list[int], lba_entries: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    lba_to_entry = {entry["lba"]: entry for entry in lba_entries}
    rows: list[dict[str, Any]] = []

    for slot_id, lba in enumerate(slots):
        row: dict[str, Any] = {
            "slot_id": slot_id,
            "slot_hex": f"0x{slot_id:x}",
            "lba": lba,
            "resolved": False,
        }
        lba_entry = lba_to_entry.get(lba)
        if lba_entry is not None:
            row.update(lba_entry)
            row["resolved"] = True
            hint = selector_hint(
                lba_entry["archive_name"], lba_entry["archive_type"], slot_id
            )
            if hint is not None:
                row.update(hint)
        rows.append(row)

    return rows


def build_slot_map(
    slots: list[int], lba_entries: list[dict[str, Any]], exe_header: dict[str, int]
) -> dict[str, Any]:
    rows = build_slot_rows(slots, lba_entries)
    unresolved = [row for row in rows if not row["resolved"]]
    referenced_sources = {row["source_path"] for row in rows if row["resolved"]}
    unreferenced_sources = [
        entry["source_path"]
        for entry in lba_entries
        if entry["source_path"] not in referenced_sources
    ]

    return {
        "slot_table_address": f"0x{SLOT_TABLE_VADDR:08x}",
        "slot_count": len(slots),
        "unresolved_slot_count": len(unresolved),
        "unreferenced_source_count": len(unreferenced_sources),
        "slus": {
            "path": SLUS_PATH.relative_to(ROOT).as_posix(),
            "pc0": f"0x{exe_header['pc0']:08x}",
            "text_addr": f"0x{exe_header['text_addr']:08x}",
            "text_size": f"0x{exe_header['text_size']:x}",
        },
        "slots": rows,
        "unreferenced_sources": unreferenced_sources,
    }


def render_markdown(slot_map: dict[str, Any]) -> str:
    lines = [
        "# Slot Map",
        "",
        "Machine-generated mapping from `SLUS_004.22` slot ids to disc LBAs and extracted content sources.",
        "",
        f"- Slot table address: `{slot_map['slot_table_address']}`",
        f"- Slot count: {slot_map['slot_count']}",
        f"- Unresolved slots: {slot_map['unresolved_slot_count']}",
        f"- Unreferenced extracted sources: {slot_map['unreferenced_source_count']}",
        "",
        "## Representative Slots",
        "",
    ]

    representative_ids = [0, 1, 2, 6, 0x1DB, 0x1EE, 0x26A, 0x27D]
    representative_ids.extend(
        slot_map["slot_count"] - offset for offset in (4, 3, 2, 1)
    )
    representative_ids = list(
        dict.fromkeys(slot_id for slot_id in representative_ids if slot_id >= 0)
    )
    by_id = {row["slot_id"]: row for row in slot_map["slots"]}
    for slot_id in representative_ids:
        row = by_id.get(slot_id)
        if row is None:
            continue
        if row["resolved"]:
            lines.append(
                f"- slot `{row['slot_id']}` -> LBA `{row['lba']}` -> `{row['source_path']}`"
            )
        else:
            lines.append(f"- slot `{row['slot_id']}` -> unresolved LBA `{row['lba']}`")

    lines.extend(
        [
            "",
            "## First 32 Slots",
            "",
            "| Slot | LBA | Archive |",
            "| ---: | ---: | --- |",
        ]
    )

    for row in slot_map["slots"][:32]:
        archive = row["source_path"] if row["resolved"] else "<unresolved>"
        lines.append(f"| `{row['slot_id']}` | `{row['lba']}` | `{archive}` |")

    if slot_map["unreferenced_sources"]:
        lines.extend(
            [
                "",
                "## Unreferenced Extracted Sources",
                "",
            ]
        )
        for source in slot_map["unreferenced_sources"]:
            lines.append(f"- `{source}`")

    return "\n".join(lines) + "\n"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("inventory", "slot-map"),
        description="Recover the disc slot-to-LBA map from `SLUS_004.22` and the disc LBA log.",
    )
    parser.add_argument("--slus", type=Path, default=SLUS_PATH)
    parser.add_argument("--disc-lba", type=Path, default=LBA_LOG_PATH)
    parser.add_argument("--db", type=Path, default=DB_OUT)
    parser.add_argument(
        "--json-out", type=Path, default=None, help="optional JSON output"
    )
    parser.add_argument(
        "--md-out", type=Path, default=None, help="optional Markdown output"
    )
    parser.add_argument(
        "--slot-table-address", type=lambda text: int(text, 0), default=SLOT_TABLE_VADDR
    )
    parser.add_argument(
        "--slot-count",
        type=int,
        default=DEFAULT_SLOT_COUNT,
        help="Number of slot table entries to read; 0 means auto-size from the disc file list",
    )
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)

    slus_data = args.slus.read_bytes()
    exe_header = parse_psx_exe_header(slus_data)

    file_offset = PSX_EXE_HEADER_SIZE + (
        args.slot_table_address - exe_header["text_addr"]
    )
    if file_offset < PSX_EXE_HEADER_SIZE:
        raise SystemExit("Slot table address resolves before the PS-X EXE payload")

    lba_entries = parse_lba_log(args.disc_lba)
    slot_count = args.slot_count if args.slot_count > 0 else len(lba_entries)
    if file_offset + slot_count * 4 > len(slus_data):
        raise SystemExit(
            f"Slot table would read past the end of SLUS_004.22: offset=0x{file_offset:x}, count={slot_count}"
        )

    slots = list(struct.unpack_from(f"<{slot_count}I", slus_data, file_offset))

    slot_map = build_slot_map(slots, lba_entries, exe_header)
    persist_slot_map(args.db, lba_entries, slot_map)
    if args.json_out is not None:
        write_json_output(args.json_out, slot_map)
    if args.md_out is not None:
        write_text_output(args.md_out, render_markdown(slot_map))
    return 0


def persist_slot_map(
    db_path: Path, lba_entries: list[dict[str, Any]], slot_map: dict[str, Any]
) -> None:
    connection = connect_inventory_database(db_path)
    ensure_inventory_schema(connection)
    try:
        with connection:
            connection.execute("DELETE FROM slot_map")
            connection.execute("DELETE FROM disc_lba_entries")
            for entry in lba_entries:
                connection.execute(
                    "INSERT INTO disc_lba_entries(lba, source_path, size) VALUES (?, ?, ?)",
                    (
                        int(entry["lba"]),
                        str(entry.get("source_path") or ""),
                        None,
                    ),
                )
            for row in slot_map["slots"]:
                connection.execute(
                    "INSERT INTO slot_map(slot_index, lba, source_path) VALUES (?, ?, ?)",
                    (
                        int(row["slot_id"]),
                        int(row["lba"]),
                        None
                        if not row.get("resolved")
                        else str(row.get("source_path") or ""),
                    ),
                )
    finally:
        connection.close()
