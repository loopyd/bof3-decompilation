from __future__ import annotations

import json
import re
import struct
from pathlib import Path
from typing import Any

from .scan import parse_psx_exe


SLOT_TABLE_VADDR = 0x80182444
PSX_EXE_HEADER_SIZE = 0x800
HEX_SUFFIX_RE = re.compile(r"^(?P<prefix>BPLD|BPLU|PLP|PL)(?P<index>[0-9A-Fa-f]{3})$")


def parse_lba_log(path: Path) -> list[dict[str, Any]]:
    if path.suffix.lower() == ".json":
        payload = json.loads(path.read_text(encoding="utf-8"))
        entries = payload.get("entries", [])
        if not isinstance(entries, list):
            raise ValueError(f"invalid disc LBA JSON: {path}")
        rows = []
        for row in entries:
            if not isinstance(row, dict):
                continue
            rows.append(
                {
                    "archive_name": str(row.get("archive_name") or ""),
                    "archive_type": str(row.get("archive_type") or ""),
                    "family": str(row.get("family") or "unknown"),
                    "lba": int(row["lba"]),
                    "source_path": str(row.get("source_path") or ""),
                }
            )
        return sorted(rows, key=lambda item: item["lba"])

    rows = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if "|" not in line or "out/extracted/" not in line:
            continue
        parts = [part.strip() for part in line.split("|")]
        if len(parts) < 7:
            continue
        row_type, name, _length, lba_text, _timecode, _bytes, source = parts[:7]
        if row_type not in {"File", "XA"}:
            continue
        source_path = Path(source)
        path_parts = source_path.parts
        family = (
            path_parts[3]
            if len(path_parts) >= 4
            and path_parts[0:3] == ("output", "extracted", "BIN")
            else "unknown"
        )
        archive_path = Path(name)
        rows.append(
            {
                "archive_name": archive_path.stem,
                "archive_type": archive_path.suffix.upper().lstrip("."),
                "family": family,
                "lba": int(lba_text),
                "source_path": source_path.as_posix(),
            }
        )
    return sorted(rows, key=lambda item: item["lba"])


def selector_hint(
    archive_name: str,
    archive_type: str,
    slot_id: int,
) -> dict[str, Any] | None:
    if archive_type != "EMI":
        return None
    match = HEX_SUFFIX_RE.match(archive_name)
    if match is None:
        return None
    prefix = match.group("prefix")
    index = int(match.group("index"), 16)
    family_selector, base_slot = {
        "BPLD": (1, 0x1DB),
        "BPLU": (2, 0x1EE),
        "PL": (0, 0x26A),
        "PLP": (3, 0x27D),
    }[prefix]
    return {
        "selector_family_hint": family_selector,
        "archive_suffix_hex": f"0x{index:x}",
        "slot_bucket_offset": slot_id - base_slot,
    }


def build_slot_rows(
    slots: list[int],
    lba_entries: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    lba_to_entry = {entry["lba"]: entry for entry in lba_entries}
    rows = []
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
                str(lba_entry["archive_name"]),
                str(lba_entry["archive_type"]),
                slot_id,
            )
            if hint is not None:
                row.update(hint)
        rows.append(row)
    return rows


def build_slot_map_artifact(
    *,
    slus_path: Path,
    disc_lba_path: Path,
    slot_table_address: int = SLOT_TABLE_VADDR,
    slot_count: int = 0,
) -> dict[str, Any]:
    slus_data = slus_path.read_bytes()
    exe_header = parse_psx_exe(slus_path)
    file_offset = PSX_EXE_HEADER_SIZE + (slot_table_address - exe_header["text_addr"])
    if file_offset < PSX_EXE_HEADER_SIZE:
        raise ValueError("slot table address resolves before the PS-X EXE payload")

    lba_entries = parse_lba_log(disc_lba_path)
    resolved_slot_count = slot_count if slot_count > 0 else len(lba_entries)
    if file_offset + resolved_slot_count * 4 > len(slus_data):
        raise ValueError(
            "slot table would read past the end of SLUS_004.22: "
            f"offset=0x{file_offset:x}, count={resolved_slot_count}"
        )

    slots = list(struct.unpack_from(f"<{resolved_slot_count}I", slus_data, file_offset))
    rows = build_slot_rows(slots, lba_entries)
    unresolved = [row for row in rows if not row["resolved"]]
    referenced_sources = {row["source_path"] for row in rows if row["resolved"]}
    unreferenced_sources = [
        entry["source_path"]
        for entry in lba_entries
        if entry["source_path"] not in referenced_sources
    ]
    return {
        "schema": "harness.inventory-slot-map/v1",
        "slot_table_address": f"0x{slot_table_address:08x}",
        "slot_count": len(slots),
        "unresolved_slot_count": len(unresolved),
        "unreferenced_source_count": len(unreferenced_sources),
        "slus": {
            "path": str(slus_path),
            "pc0": f"0x{exe_header['pc0']:08x}",
            "text_addr": f"0x{exe_header['text_addr']:08x}",
            "text_size": f"0x{exe_header['text_size']:x}",
        },
        "slots": rows,
        "unreferenced_sources": unreferenced_sources,
    }


def render_slot_map_markdown(slot_map: dict[str, Any]) -> str:
    lines = [
        "# Slot Map",
        "",
        "Machine-generated mapping from SLUS slot ids to disc LBAs and sources.",
        "",
        f"- Slot table address: `{slot_map['slot_table_address']}`",
        f"- Slot count: {slot_map['slot_count']}",
        f"- Unresolved slots: {slot_map['unresolved_slot_count']}",
        f"- Unreferenced sources: {slot_map['unreferenced_source_count']}",
        "",
        "## First 32 Slots",
        "",
        "| Slot | LBA | Archive |",
        "| ---: | ---: | --- |",
    ]
    for row in slot_map["slots"][:32]:
        archive = row["source_path"] if row["resolved"] else "<unresolved>"
        lines.append(f"| `{row['slot_id']}` | `{row['lba']}` | `{archive}` |")
    return "\n".join(lines) + "\n"
