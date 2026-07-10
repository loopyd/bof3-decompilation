"""Durable inventory for BOF3 executable images and extracted EMI payloads.

An EMI archive is a container, not a linkable binary.  This module deliberately
keeps that distinction in the catalog: only a reviewed, explicitly promoted
payload becomes a decompilation target.
"""

from __future__ import annotations

import hashlib
import re
import shutil
import struct
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

from .inventory.scan import file_sha256, parse_psx_exe
from .jsonio import read_json, write_json


CATALOG_SCHEMA = "rebof3.catalog.emi/v2"
LIFTS_SCHEMA = "rebof3.catalog.lifts/v1"
PSX_EXE_HEADER_SIZE = 0x800
CODE_TYPES = {0}
AUDIO_TYPES = {6, 7, 8, 10}
TARGET_RE = re.compile(r"^(?P<archive>.+?)(?:\.EMI)?#(?P<slot>\d+)$", re.I)


def parse_number(value: str) -> int:
    return int(value, 0)


def normalize_executable(source: Path, destination: Path) -> dict[str, Any]:
    """Extract a PS-X EXE load image and retain header metadata beside it."""
    header = parse_psx_exe(source)
    data = source.read_bytes()
    image_size = header["text_size"]
    image = data[PSX_EXE_HEADER_SIZE : PSX_EXE_HEADER_SIZE + image_size]
    if len(image) != image_size:
        raise ValueError(f"truncated PS-X EXE load image: {source}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(image)
    metadata = {
        "schema": "rebof3.normalized-exe/v1",
        "source": str(source),
        "source_sha256": file_sha256(source),
        "image": str(destination),
        "image_sha256": hashlib.sha256(image).hexdigest(),
        "header_size": PSX_EXE_HEADER_SIZE,
        "pc0": header["pc0"],
        "load_address": header["text_addr"],
        "image_size": image_size,
    }
    write_json(destination.with_suffix(destination.suffix + ".json"), metadata)
    return metadata


def set_splat_expected_hash(config_path: Path, image_path: Path) -> None:
    """Record the expected raw-image SHA-1 in the tracked Splat configuration."""
    digest = hashlib.sha1(image_path.read_bytes()).hexdigest()
    text = config_path.read_text(encoding="utf-8")
    if re.search(r"^sha1:.*$", text, flags=re.M) is None:
        raise ValueError(f"missing sha1 field in Splat config: {config_path}")
    updated = re.sub(r"^sha1:.*$", f"sha1: {digest}", text, count=1, flags=re.M)
    config_path.write_text(updated, encoding="utf-8")


def _mips_instruction_density(payload: Path) -> float:
    data = payload.read_bytes()
    words = len(data) // 4
    if words == 0:
        return 0.0
    plausible = 0
    for offset in range(0, words * 4, 4):
        word = struct.unpack_from("<I", data, offset)[0]
        opcode = word >> 26
        funct = word & 0x3F
        # This is intentionally conservative: it is evidence for review, never
        # proof that a type-0 payload is executable.
        if word not in {0, 0xFFFFFFFF} and (
            opcode != 0
            or funct
            in {
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
        ):
            plausible += 1
    return plausible / words


def _payload_kind(entry_type: int, ram_ptr: int, size: int) -> str:
    if entry_type == 3:
        return "image"
    if entry_type in AUDIO_TYPES:
        return "audio"
    if (
        entry_type == 0
        and 0x80033000 <= ram_ptr <= 0x8003AFFF
        and size in {0x40, 0x200, 0x400, 0x1000}
    ):
        return "image"
    if entry_type == 0:
        return "ram"
    return "unresolved"


def _code_status(entry_type: int, ram_ptr: int, density: float, kind: str) -> str:
    if kind != "ram" or entry_type not in CODE_TYPES or ram_ptr < 0x80000000:
        return "rejected"
    if density >= 0.70:
        return "candidate"
    return "unknown"


def _entry_records(emi_root: Path) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for manifest_path in sorted(emi_root.rglob("emi.json")):
        archive_dir = manifest_path.parent
        archive_id = archive_dir.relative_to(emi_root).as_posix()
        raw_entries = read_json(manifest_path).get("entries", [])
        if not isinstance(raw_entries, list):
            raise ValueError(f"invalid EMI manifest: {manifest_path}")
        for entry in raw_entries:
            if not isinstance(entry, dict):
                continue
            slot = int(entry.get("index") or 0)
            name = str(entry.get("name") or f"{slot}.bin")
            payload = archive_dir / name
            if not payload.is_file():
                continue
            entry_type = int(entry.get("type") or 0)
            ram_ptr = int(entry.get("ram_ptr") or 0)
            size = payload.stat().st_size
            density = _mips_instruction_density(payload)
            kind = _payload_kind(entry_type, ram_ptr, size)
            status = _code_status(entry_type, ram_ptr, density, kind)
            digest = file_sha256(payload)
            records.append(
                {
                    "id": f"{archive_id}#{slot}",
                    "archive_id": archive_id,
                    "family": archive_id.split("/", 1)[0],
                    "slot": slot,
                    "entry_name": name,
                    "payload_path": str(payload),
                    "type": entry_type,
                    "load_address": ram_ptr,
                    "load_address_hex": f"0x{ram_ptr:08x}",
                    "size": size,
                    "sha256": digest,
                    "payload_kind": kind,
                    "code_status": status,
                    "evidence": {
                        "type": entry_type,
                        "size": size,
                        "load_address": ram_ptr,
                        "instruction_density": round(density, 4),
                        "runtime_xref": None,
                        "reviewed_config": None,
                    },
                }
            )
    return records


def build_emi_catalog(emi_root: Path) -> dict[str, Any]:
    entries = _entry_records(emi_root)
    root = emi_root.resolve().parents[2] if emi_root.name == "BIN" else None
    if root is not None:
        for entry in entries:
            config = root / "config" / "splat" / "emi" / (target_slug(entry) + ".yaml")
            if config.is_file():
                entry["code_status"] = "confirmed"
                entry["evidence"]["reviewed_config"] = str(config.relative_to(root))
    by_content: dict[str, list[str]] = defaultdict(list)
    by_target: dict[tuple[str, int, str], list[str]] = defaultdict(list)
    for entry in entries:
        by_content[entry["sha256"]].append(entry["id"])
        by_target[(entry["sha256"], entry["load_address"], "raw")].append(entry["id"])
    content_groups = [
        {
            "sha256": digest,
            "members": sorted(members),
            "representative": sorted(members)[0],
        }
        for digest, members in sorted(by_content.items())
        if len(members) > 1
    ]
    build_targets = [
        {
            "key": f"{digest}@0x{address:08x}:{convention}",
            "sha256": digest,
            "load_address": address,
            "entry_convention": convention,
            "members": sorted(members),
            "representative": sorted(members)[0],
        }
        for (digest, address, convention), members in sorted(by_target.items())
    ]
    for entry in entries:
        entry["content_group"] = entry["sha256"]
        entry["build_target"] = f"{entry['sha256']}@0x{entry['load_address']:08x}:raw"
    return {
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
        "content_groups": content_groups,
        "build_targets": build_targets,
        "entries": entries,
    }


def write_catalog(emi_root: Path, catalog_path: Path) -> dict[str, Any]:
    catalog = build_emi_catalog(emi_root)
    write_json(catalog_path, catalog)
    return catalog


def resolve_entry(catalog: dict[str, Any], identifier: str) -> dict[str, Any]:
    match = TARGET_RE.match(identifier.replace("\\", "/"))
    if match is None:
        raise ValueError(
            "target must be an archive slot such as BIN/BATTLE/BATTLE.EMI#3"
        )
    archive = match.group("archive").removesuffix(".EMI")
    # User-facing disc identifiers include BIN/, whereas unpacked archive IDs
    # are rooted inside BIN/ already.
    if archive.upper().startswith("BIN/"):
        archive = archive[4:]
    slot = int(match.group("slot"))
    matches = [
        entry
        for entry in catalog["entries"]
        if entry["archive_id"] == archive and entry["slot"] == slot
    ]
    if len(matches) != 1:
        raise ValueError(f"no extracted EMI entry for {identifier}")
    return matches[0]


def target_slug(entry: dict[str, Any]) -> str:
    return (
        "/".join(part.lower() for part in entry["archive_id"].split("/"))
        + f"/{entry['slot']:02d}"
    )


def splat_config_text(entry: dict[str, Any], root: Path) -> str:
    source_path = Path(entry["payload_path"]).resolve()
    source = source_path.relative_to(root).as_posix()
    slug = target_slug(entry)
    return "\n".join(
        [
            "name: " + slug.replace("/", "_"),
            "sha1: "
            + hashlib.sha1(Path(entry["payload_path"]).read_bytes()).hexdigest(),
            "options:",
            "  platform: psx",
            "  compiler: psyq",
            "  base_path: .",
            "  target_path: " + source,
            "  asm_path: out/splat/" + slug + "/asm",
            "  src_path: src/emi/" + slug,
            "  ld_script_path: out/splat/" + slug + "/linker.ld",
            "  symbol_addrs_path:",
            "    - config/symbols/psyq.txt",
            "    - config/symbols/shared.txt",
            "segments:",
            "  - [0x0, bin]",
            "",
        ]
    )


def promote_entry(
    *, catalog_path: Path, identifier: str, root: Path, confirm_code: bool
) -> tuple[Path, Path]:
    if not confirm_code:
        raise ValueError(
            "promotion requires --confirm-code; type-0 is not proof of executable code"
        )
    catalog = read_json(catalog_path)
    entry = resolve_entry(catalog, identifier)
    if entry["payload_kind"] not in {"ram", "unresolved"}:
        raise ValueError(
            f"{identifier} is cataloged as {entry['payload_kind']}, not code or mixed code/data"
        )
    slug = target_slug(entry)
    config_path = root / "config" / "splat" / "emi" / (slug + ".yaml")
    source_dir = root / "src" / "emi" / slug
    if config_path.exists():
        raise ValueError(f"target already promoted: {config_path}")
    config_path.parent.mkdir(parents=True, exist_ok=True)
    source_dir.mkdir(parents=True, exist_ok=True)
    config_path.write_text(splat_config_text(entry, root), encoding="utf-8")
    (source_dir / "internal.h").write_text(
        "#ifndef BOF3_INTERNAL_H\n#define BOF3_INTERNAL_H\n\n#endif\n", encoding="utf-8"
    )
    entry["code_status"] = "confirmed"
    entry["evidence"]["reviewed_config"] = str(config_path.relative_to(root))
    catalog["code_status_counts"] = dict(
        sorted(Counter(item["code_status"] for item in catalog["entries"]).items())
    )
    write_json(catalog_path, catalog)
    return config_path, source_dir


def record_lift(*, root: Path, catalog_path: Path, target: str, address: int) -> Path:
    catalog = read_json(catalog_path)
    entry = resolve_entry(catalog, target)
    if entry["code_status"] != "confirmed":
        raise ValueError("lift requires a confirmed promoted target")
    start = entry["load_address"]
    end = start + entry["size"]
    if not start <= address < end:
        raise ValueError(f"address 0x{address:08x} is outside {target}")
    lifts_path = root / "out" / "catalog" / "lifts.json"
    lifts = (
        read_json(lifts_path)
        if lifts_path.exists()
        else {"schema": LIFTS_SCHEMA, "lifts": []}
    )
    if any(
        item["target"] == target and item["address"] == address
        for item in lifts["lifts"]
    ):
        raise ValueError(f"function already lifted at 0x{address:08x}")
    slug = target_slug(entry)
    source = root / "src" / "emi" / slug / f"func_{address:08x}.c"
    if source.exists():
        raise ValueError(f"source already exists: {source}")
    source.write_text(
        '#include "internal.h"\n\nvoid func_%08x(void) {\n}\n' % address,
        encoding="utf-8",
    )
    work = root / "out" / "work" / slug / f"func_{address:08x}"
    work.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(entry["payload_path"], work / "original.bin")
    lifts["lifts"].append(
        {"target": target, "address": address, "source": str(source.relative_to(root))}
    )
    write_json(lifts_path, lifts)
    return source
