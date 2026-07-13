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


CATALOG_SCHEMA = "harness.catalog.emi/v2"
LIFTS_SCHEMA = "harness.catalog.lifts/v1"
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
        "schema": "harness.normalized-exe/v1",
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


def materialize_promoted_emi_targets(
    *, root: Path, catalog: dict[str, Any]
) -> list[Path]:
    """Restore normalized images for every tracked EMI target manifest."""
    from .domain import load_target_manifests

    images: list[Path] = []
    manifests = sorted(
        load_target_manifests(root).values(), key=lambda manifest: manifest.id.value
    )
    for manifest in manifests:
        if manifest.kind != "emi":
            continue
        entry = resolve_entry(catalog, manifest.disc_id)
        source = Path(entry["payload_path"])
        if not source.is_file():
            raise FileNotFoundError(
                f"unpacked payload missing for {manifest.disc_id}: {source}"
            )
        payload = source.read_bytes()
        image = root / manifest.binary
        image.parent.mkdir(parents=True, exist_ok=True)
        if not image.is_file() or image.read_bytes() != payload:
            image.write_bytes(payload)
        digest = hashlib.sha256(payload).hexdigest()
        write_json(
            image.with_suffix(".bin.json"),
            {
                "schema": "harness.normalized-emi/v1",
                "source": str(source),
                "source_sha256": digest,
                "image": str(image),
                "image_sha256": digest,
                "load_address": manifest.load_address,
            },
        )
        images.append(image)
    return images


def resolve_entry(catalog: dict[str, Any], identifier: str) -> dict[str, Any]:
    from .domain import normalize_target_id

    try:
        normalized = normalize_target_id(identifier)
    except ValueError:
        normalized = None
    if normalized is not None and normalized.value.startswith("emi/"):
        _, family, archive, slot_text = normalized.value.split("/", 3)
        archive = f"{family.upper()}/{archive.upper()}"
        slot = int(slot_text)
        matches = [
            entry
            for entry in catalog["entries"]
            if entry["archive_id"].upper() == archive and entry["slot"] == slot
        ]
        if len(matches) == 1:
            return matches[0]
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


def internal_header_guard(slug: str) -> str:
    return "BOF3_" + re.sub(r"[^A-Za-z0-9]", "_", slug).upper() + "_INTERNAL_H"


def target_details(entry: dict[str, Any], root: Path) -> dict[str, Any]:
    """Return the small, derived target view shared by CLI consumers."""
    slug = target_slug(entry)
    config = root / "config" / "splat" / "emi" / f"{slug}.yaml"
    source = root / "src" / "emi" / slug
    payload = Path(entry["payload_path"])
    try:
        payload = payload.resolve().relative_to(root.resolve())
    except ValueError:
        pass
    artifact = _artifact_for_entry(entry, root)
    return {
        "id": entry["id"],
        "kind": "emi",
        "payload": payload.as_posix(),
        "sha256": entry["sha256"],
        "load_address": entry["load_address"],
        "size": entry["size"],
        "code_status": entry["code_status"],
        "splat": config.relative_to(root).as_posix() if config.is_file() else None,
        "source": source.relative_to(root).as_posix() if source.is_dir() else None,
        "build": artifact,
        "progress": target_progress(entry, root),
    }


SPLAT_FUNCTION_SUBSEGMENT_RE = re.compile(
    r"^\s*-\s*\[\s*(?P<offset>0x[0-9a-fA-F]+|[0-9]+)\s*,\s*(?:asm|c)(?:\s*,[^]]*)?\]"
)


def reviewed_function_addresses(entry: dict[str, Any], root: Path) -> list[int]:
    """Return reviewed function starts declared as C subsegments in Splat."""
    config = root / "config" / "splat" / "emi" / f"{target_slug(entry)}.yaml"
    if not config.is_file():
        return []
    addresses: list[int] = []
    for line in config.read_text(encoding="utf-8").splitlines():
        match = SPLAT_FUNCTION_SUBSEGMENT_RE.match(line)
        if match is not None:
            addresses.append(entry["load_address"] + int(match.group("offset"), 0))
    return sorted(set(addresses))


def target_progress(entry: dict[str, Any], root: Path) -> dict[str, Any]:
    """Summarize reviewed, lifted, matched, and whole-payload target progress."""
    reviewed = reviewed_function_addresses(entry, root)
    source_dir = root / "src" / "emi" / target_slug(entry)
    lifted = (
        {
            int(match.group(1), 16)
            for path in source_dir.glob("func_*.c")
            if (match := re.fullmatch(r"func_([0-9a-fA-F]{8})\.c", path.name))
        }
        if source_dir.is_dir()
        else set()
    )
    matched: set[int] = set()
    for address in lifted:
        summary = (
            root
            / "out"
            / "matching"
            / "emi"
            / target_slug(entry)
            / f"func_{address:08x}"
            / "asm-differ"
            / "summary.json"
        )
        if not summary.is_file():
            continue
        payload = read_json(summary)
        if payload.get("instruction_count", {}).get("match_percent") == 100:
            matched.add(address)

    artifact = _artifact_for_entry(entry, root)
    output = (
        Path(str(artifact.get("output")))
        if artifact and artifact.get("output")
        else None
    )
    whole_payload_match = False
    if (
        output is not None
        and output.is_file()
        and output.stat().st_size == entry["size"]
    ):
        whole_payload_match = (
            hashlib.sha256(output.read_bytes()).hexdigest() == entry["sha256"]
        )

    remaining = [address for address in reviewed if address not in lifted]
    return {
        "layout": "reviewed" if reviewed else "unsegmented",
        "reviewed_functions": len(reviewed),
        "lifted_functions": len(lifted & set(reviewed)),
        "matched_functions": len(matched & set(reviewed)),
        "next_function": remaining[0] if remaining else None,
        "whole_payload_match": whole_payload_match,
    }


def _artifact_for_entry(entry: dict[str, Any], root: Path) -> dict[str, Any] | None:
    manifest_path = (
        root / "build" / "default" / "artifacts" / "metadata" / "artifacts.json"
    )
    if not manifest_path.is_file():
        return None
    manifest = read_json(manifest_path)
    suffix = f"BIN/{entry['archive_id']}.EMI#{entry['slot']}"
    matches = [
        row
        for row in manifest.get("artifacts", [])
        if isinstance(row, dict) and str(row.get("source_hint", "")).endswith(suffix)
    ]
    if len(matches) != 1:
        return None
    row = matches[0]
    output = str(row.get("output") or "")
    output_path = Path(output) if output else None
    return {
        "target": row.get("target"),
        "stage": row.get("build_stage"),
        "output": output or None,
        "exists": output_path.is_file() if output_path else False,
    }


def splat_config_text(
    entry: dict[str, Any], root: Path, *, target_path: Path | None = None
) -> str:
    source_path = Path(entry["payload_path"]).resolve()
    normalized_path = target_path or source_path
    source = normalized_path.resolve().relative_to(root).as_posix()
    slug = target_slug(entry)
    return "\n".join(
        [
            "name: " + slug.replace("/", "_"),
            "sha1: " + hashlib.sha1(normalized_path.read_bytes()).hexdigest(),
            "options:",
            "  platform: psx",
            "  compiler: psyq",
            "  base_path: .",
            "  target_path: " + source,
            "  asm_path: out/splat/emi/" + slug + "/asm",
            "  src_path: src/emi/" + slug,
            "  ld_script_path: out/splat/emi/" + slug + "/linker.ld",
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
    # Keep the normalized image addressable by its canonical target slug.  The
    # slot is already the final component of ``slug`` (for example ``03``),
    # so the generated artifact is ``.../03.bin`` rather than a misleading
    # ``.../03/bin`` directory.
    normalized_path = root / "out" / "binaries" / "emi" / f"{slug}.bin"
    normalized_path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(entry["payload_path"], normalized_path)
    write_json(
        normalized_path.with_suffix(".bin.json"),
        {
            "schema": "harness.normalized-emi/v1",
            "source": entry["payload_path"],
            "source_sha256": entry["sha256"],
            "image": str(normalized_path),
            "image_sha256": hashlib.sha256(normalized_path.read_bytes()).hexdigest(),
            "load_address": entry["load_address"],
        },
    )
    config_path.parent.mkdir(parents=True, exist_ok=True)
    source_dir.mkdir(parents=True, exist_ok=True)
    config_path.write_text(
        splat_config_text(entry, root, target_path=normalized_path), encoding="utf-8"
    )
    header_path = source_dir / "internal.h"
    if not header_path.exists():
        guard = internal_header_guard(slug)
        header_path.write_text(
            f"#ifndef {guard}\n#define {guard}\n\n#endif\n", encoding="utf-8"
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
        (
            '#include "internal.h"\n\n'
            "/* @behavior Pending analysis.\n"
            f" * @source 0x{address:08x} func_{address:08x}\n"
            " */\n"
            f"void func_{address:08x}(void) {{\n}}\n"
        ),
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
