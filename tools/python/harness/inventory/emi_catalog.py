from __future__ import annotations

import json
import struct
from collections import Counter
from pathlib import Path
from typing import Any

from ..jsonio import write_json
from .scan import file_sha256


def read_manifest_entries(manifest_path: Path) -> list[dict[str, Any]]:
    payload = json.loads(manifest_path.read_text(encoding="utf-8"))
    entries = payload.get("entries", [])
    if not isinstance(entries, list):
        raise ValueError(f"invalid EMI manifest: {manifest_path}")
    return [entry for entry in entries if isinstance(entry, dict)]


def read_first_word(payload_path: Path) -> int:
    data = payload_path.read_bytes()
    if len(data) < 4:
        return 0
    return struct.unpack_from("<I", data, 0)[0]


def build_emi_manifest_catalog(emi_root: Path) -> dict[str, Any]:
    entries: list[dict[str, Any]] = []
    type_counts: Counter[int] = Counter()
    family_counts: Counter[str] = Counter()
    archive_counts: Counter[str] = Counter()

    for manifest_path in sorted(emi_root.rglob("emi.json")):
        archive_dir = manifest_path.parent
        archive_id = archive_dir.relative_to(emi_root).as_posix()
        archive_name = archive_dir.name
        family = archive_dir.relative_to(emi_root).parts[0] if archive_id else "unknown"
        manifest_entries = read_manifest_entries(manifest_path)
        archive_counts[family] += 1
        for entry in manifest_entries:
            entry_index = int(entry.get("index") or 0)
            entry_name = str(entry.get("name") or f"{entry_index}.bin")
            payload_path = archive_dir / entry_name
            if not payload_path.is_file():
                continue
            entry_type = int(entry.get("type") or 0)
            ram_ptr = int(entry.get("ram_ptr") or 0)
            size = payload_path.stat().st_size
            first4 = read_first_word(payload_path)
            audio_bundle_id = (
                ram_ptr
                if entry_type in {6, 7, 8, 10} and 0 <= ram_ptr < 0x100
                else None
            )
            record = {
                "archive_id": archive_id,
                "archive_name": archive_name,
                "entry_index": entry_index,
                "entry_name": entry_name,
                "family": family,
                "manifest_path": str(manifest_path),
                "payload_path": str(payload_path),
                "type": entry_type,
                "ram_ptr": ram_ptr,
                "ram_ptr_hex": f"0x{ram_ptr:08x}",
                "size": size,
                "first4": first4,
                "sha256": file_sha256(payload_path),
                "code_candidate": entry_type == 0 and ram_ptr >= 0x80000000,
                "image_candidate": entry_type == 3,
                "palette_candidate": (
                    entry_type == 0
                    and 0x80033000 <= ram_ptr <= 0x8003AFFF
                    and size in {0x40, 0x200, 0x400, 0x1000}
                ),
                "audio_bundle_id": audio_bundle_id,
            }
            entries.append(record)
            type_counts[entry_type] += 1
            family_counts[family] += 1

    return {
        "schema": "harness.inventory-emi-catalog/v1",
        "generated_from": str(emi_root),
        "entry_count": len(entries),
        "archive_count": sum(archive_counts.values()),
        "code_candidate_count": sum(1 for entry in entries if entry["code_candidate"]),
        "image_candidate_count": sum(
            1 for entry in entries if entry["image_candidate"]
        ),
        "palette_candidate_count": sum(
            1 for entry in entries if entry["palette_candidate"]
        ),
        "type_counts": dict(sorted(type_counts.items())),
        "family_counts": dict(sorted(family_counts.items())),
        "archive_counts": dict(sorted(archive_counts.items())),
        "entries": entries,
    }


def render_emi_catalog_markdown(catalog: dict[str, Any]) -> str:
    lines = [
        "# EMI Catalog",
        "",
        "Machine-generated inventory of unpacked EMI entries.",
        "",
        f"- Entry count: {catalog['entry_count']}",
        f"- Archive count: {catalog['archive_count']}",
        f"- Code candidates: {catalog['code_candidate_count']}",
        f"- Image candidates: {catalog['image_candidate_count']}",
        f"- Palette candidates: {catalog['palette_candidate_count']}",
        "",
        "## Families",
        "",
    ]
    for family, count in catalog["family_counts"].items():
        lines.append(f"- {family}: {count}")
    lines.extend(["", "## Types", ""])
    for type_id, count in catalog["type_counts"].items():
        lines.append(f"- type `{type_id}`: {count}")
    lines.extend(["", "## Representative Code Candidates", ""])
    shown = 0
    for entry in catalog["entries"]:
        if not entry["code_candidate"]:
            continue
        lines.append(
            f"- `{entry['archive_id']}#{entry['entry_index']}` -> `{entry['ram_ptr_hex']}` size `{entry['size']}`"
        )
        shown += 1
        if shown == 20:
            break
    return "\n".join(lines) + "\n"


def build_emi_catalog(
    emi_root: Path, *, json_out: Path, md_out: Path
) -> dict[str, Path]:
    catalog = build_emi_manifest_catalog(emi_root)
    write_json(json_out, catalog)
    md_out.parent.mkdir(parents=True, exist_ok=True)
    md_out.write_text(render_emi_catalog_markdown(catalog), encoding="utf-8")
    return {"json": json_out, "markdown": md_out}
