"""EMI inventory and conservative target bootstrap ownership."""

from __future__ import annotations

import struct
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

from ..discovery import file_sha256
from ..io import read_json

from .catalog_verify import _payload_kind, target_slug, verify_companion_relations

CATALOG_SCHEMA = "harness.catalog.emi/v3"

def _instruction_density(payload: Path) -> float:
    data = payload.read_bytes()
    words = len(data) // 4
    if words == 0:
        return 0.0
    plausible = 0
    valid_special = {
        0,
        2,
        3,
        8,
        9,
        12,
        13,
        16,
        17,
        18,
        24,
        25,
        26,
        27,
        32,
        33,
        34,
        35,
        36,
        37,
        42,
        43,
    }
    for offset in range(0, words * 4, 4):
        word = struct.unpack_from("<I", data, offset)[0]
        if word not in {0, 0xFFFFFFFF} and (
            (word >> 26) != 0 or (word & 0x3F) in valid_special
        ):
            plausible += 1
    return plausible / words

def _entry_records(emi_root: Path) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for manifest_path in sorted(emi_root.rglob("emi.json")):
        archive_dir = manifest_path.parent
        archive_id = archive_dir.relative_to(emi_root).as_posix()
        entries = read_json(manifest_path).get("entries", [])
        if not isinstance(entries, list):
            raise ValueError(f"invalid EMI manifest: {manifest_path}")
        for raw in entries:
            if not isinstance(raw, dict):
                continue
            slot = int(raw.get("index") or 0)
            payload = archive_dir / str(raw.get("name") or f"{slot}.bin")
            if not payload.is_file():
                continue
            entry_type = int(raw.get("type") or 0)
            load_address = int(raw.get("ram_ptr") or 0)
            size = payload.stat().st_size
            density = _instruction_density(payload)
            kind = _payload_kind(entry_type, load_address, size)
            status = (
                "rejected"
                if kind != "ram" or load_address < 0x80000000
                else "candidate"
                if density >= 0.70
                else "unknown"
            )
            records.append(
                {
                    "id": f"{archive_id}#{slot}",
                    "archive_id": archive_id,
                    "family": archive_id.split("/", 1)[0],
                    "slot": slot,
                    "entry_name": payload.name,
                    "payload_path": str(payload),
                    "type": entry_type,
                    "load_address": load_address,
                    "load_address_hex": f"0x{load_address:08x}",
                    "size": size,
                    "sha256": file_sha256(payload),
                    "payload_kind": kind,
                    "code_status": status,
                    "evidence": {
                        "type": entry_type,
                        "size": size,
                        "load_address": load_address,
                        "instruction_density": round(density, 4),
                        "runtime_xref": None,
                        "reviewed_config": None,
                    },
                }
            )
    return records

def build_catalog(emi_root: Path) -> dict[str, Any]:
    entries = _entry_records(emi_root)
    root = emi_root.resolve().parents[2] if emi_root.name == "BIN" else None
    if root is not None:
        for entry in entries:
            config = (
                root / "config" / "targets" / "emi" / target_slug(entry) / "splat.yaml"
            )
            if config.is_file():
                entry["code_status"] = "confirmed"
                entry["evidence"]["reviewed_config"] = str(config.relative_to(root))
    by_content: dict[str, list[str]] = defaultdict(list)
    by_target: dict[tuple[str, int], list[str]] = defaultdict(list)
    for entry in entries:
        by_content[entry["sha256"]].append(entry["id"])
        by_target[(entry["sha256"], entry["load_address"])].append(entry["id"])
        entry["content_group"] = entry["sha256"]
        entry["build_target"] = f"{entry['sha256']}@0x{entry['load_address']:08x}:raw"
    catalog = {
        "schema": CATALOG_SCHEMA,
        "emi_root": str(emi_root),
        "entry_count": len(entries),
        "archive_count": len({entry["archive_id"] for entry in entries}),
        "type_counts": dict(
            sorted(Counter(entry["type"] for entry in entries).items())
        ),
        "payload_kind_counts": dict(
            sorted(Counter(entry["payload_kind"] for entry in entries).items())
        ),
        "code_status_counts": dict(
            sorted(Counter(entry["code_status"] for entry in entries).items())
        ),
        "content_groups": [
            {
                "sha256": digest,
                "members": sorted(members),
                "representative": sorted(members)[0],
            }
            for digest, members in sorted(by_content.items())
            if len(members) > 1
        ],
        "build_targets": [
            {
                "key": f"{digest}@0x{address:08x}:raw",
                "sha256": digest,
                "load_address": address,
                "entry_convention": "raw",
                "members": sorted(members),
                "representative": sorted(members)[0],
            }
            for (digest, address), members in sorted(by_target.items())
        ],
        "entries": entries,
    }
    if root is not None:
        catalog["companion_relations"] = verify_companion_relations(root, catalog)
    else:
        catalog["companion_relations"] = []
    return catalog

def load_catalog(root: Path) -> dict[str, Any]:
    emi_root = root / "out/extracted/BIN"
    if not emi_root.is_dir():
        raise FileNotFoundError(f"missing extracted EMI root: {emi_root}")
    return build_catalog(emi_root)

__all__ = [
    "build_catalog",
    "load_catalog",
]
