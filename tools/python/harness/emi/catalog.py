"""EMI inventory and conservative target bootstrap ownership."""

from __future__ import annotations

import hashlib
import json
import os
import re
import struct
import tempfile
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

from ..canonical import load_map
from ..discovery import file_sha256
from ..domain import load_target_manifests, normalize_target_id
from ..io import read_json, write_json


CATALOG_SCHEMA = "harness.catalog.emi/v3"
TARGET_RE = re.compile(r"^(?P<archive>.+?)(?:\.EMI)?#(?P<slot>\d+)$", re.I)
AUDIO_TYPES = {6, 7, 8, 10}
ARCHIVE_PART_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9_.-]*$")


def _archive_parts(archive_id: str) -> tuple[str, str]:
    parts = archive_id.replace("\\", "/").split("/")
    if len(parts) != 2 or any(
        part in {".", ".."} or ARCHIVE_PART_RE.fullmatch(part) is None for part in parts
    ):
        raise ValueError(f"invalid EMI archive id: {archive_id!r}")
    return parts[0], parts[1]


def target_slug(entry: dict[str, Any]) -> str:
    family, archive = _archive_parts(str(entry["archive_id"]))
    slot = int(entry["slot"])
    if not 0 <= slot <= 255:
        raise ValueError(f"invalid EMI slot: {slot}")
    return f"{family.lower()}/{archive.lower()}/{slot:02d}"


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


def _payload_kind(entry_type: int, load_address: int, size: int) -> str:
    if entry_type == 3:
        return "image"
    if entry_type in AUDIO_TYPES:
        return "audio"
    if (
        entry_type == 0
        and 0x80033000 <= load_address <= 0x8003AFFF
        and size
        in {
            0x40,
            0x200,
            0x400,
            0x1000,
        }
    ):
        return "image"
    return "ram" if entry_type == 0 else "unresolved"


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


def resolve_entry(catalog: dict[str, Any], identifier: str) -> dict[str, Any]:
    try:
        normalized = normalize_target_id(identifier)
    except ValueError:
        normalized = None
    if normalized is not None and normalized.value.startswith("emi/"):
        _, family, archive, slot_text = normalized.value.split("/", 3)
        archive_id, slot = f"{family}/{archive}".upper(), int(slot_text)
    else:
        match = TARGET_RE.fullmatch(identifier.replace("\\", "/"))
        if match is None:
            raise ValueError("entry must look like BIN/BATTLE/BATTLE.EMI#3")
        archive_id = match.group("archive").removesuffix(".EMI")
        archive_id = (
            archive_id[4:] if archive_id.upper().startswith("BIN/") else archive_id
        )
        archive_id, slot = archive_id.upper(), int(match.group("slot"))
    matches = [
        entry
        for entry in catalog.get("entries", [])
        if str(entry["archive_id"]).upper() == archive_id and int(entry["slot"]) == slot
    ]
    if len(matches) != 1:
        raise ValueError(
            f"expected one extracted EMI entry for {identifier}; found {len(matches)}"
        )
    return matches[0]


def _jal_target(word: int, caller_address: int) -> int | None:
    if word >> 26 != 3:
        return None
    return ((caller_address + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def _direct_entry(root: Path, identifier: str) -> dict[str, Any]:
    """Read one declared extracted EMI entry without inventorying every archive."""

    match = TARGET_RE.fullmatch(identifier.replace("\\", "/"))
    if match is None:
        raise ValueError(f"invalid EMI entry id: {identifier}")
    archive_id = match.group("archive").removesuffix(".EMI")
    archive_id = archive_id[4:] if archive_id.upper().startswith("BIN/") else archive_id
    slot = int(match.group("slot"))
    archive_dir = root / "out" / "extracted" / "BIN" / archive_id
    manifest_path = archive_dir / "emi.json"
    if not manifest_path.is_file():
        raise FileNotFoundError(f"missing extracted EMI manifest: {manifest_path}")
    entries = read_json(manifest_path).get("entries", [])
    if not isinstance(entries, list):
        raise ValueError(f"invalid EMI manifest: {manifest_path}")
    matches = [entry for entry in entries if isinstance(entry, dict) and int(entry.get("index") or 0) == slot]
    if len(matches) != 1:
        raise ValueError(f"expected one extracted EMI entry for {identifier}; found {len(matches)}")
    raw = matches[0]
    payload = archive_dir / str(raw.get("name") or f"{slot}.bin")
    if not payload.is_file():
        raise FileNotFoundError(f"missing extracted EMI payload: {payload}")
    entry_type = int(raw.get("type") or 0)
    load_address = int(raw.get("ram_ptr") or 0)
    size = payload.stat().st_size
    return {
        "id": f"{archive_id}#{slot}",
        "payload_path": str(payload),
        "sha256": file_sha256(payload),
        "load_address": load_address,
        "size": size,
        "payload_kind": _payload_kind(entry_type, load_address, size),
    }


def verify_declared_companions(
    root: Path, caller_manifest: Any
) -> list[dict[str, Any]]:
    """Verify one caller's declared companion facts without a global EMI scan."""

    caller = _direct_entry(root, caller_manifest.disc_id)
    if caller_manifest.load_address != int(caller["load_address"]):
        raise ValueError(f"caller catalog load address mismatch: {caller_manifest.id.value}")
    caller_payload = Path(caller["payload_path"]).read_bytes()
    relations: list[dict[str, Any]] = []
    for companion in caller_manifest.companions:
        target = _direct_entry(root, companion.disc_id)
        if (
            target["sha256"] != companion.payload_sha256
            or int(target["load_address"]) != companion.load_address
            or int(target["size"]) != companion.size
        ):
            raise ValueError(
                f"companion catalog identity mismatch: {caller_manifest.id.value} -> "
                f"{companion.target.value}"
            )
        if target["payload_kind"] != "ram":
            raise ValueError(f"companion is not a RAM payload: {companion.target.value}")
        calls: list[dict[str, int]] = []
        for call in companion.static_calls:
            offset = call.caller_address - int(caller["load_address"])
            if offset < 0 or offset + 4 > int(caller["size"]):
                raise ValueError(
                    f"companion call outside caller payload: {caller_manifest.id.value}"
                )
            word = struct.unpack_from("<I", caller_payload, offset)[0]
            if _jal_target(word, call.caller_address) != call.target_address:
                raise ValueError(
                    f"companion call bytes differ: {caller_manifest.id.value} at "
                    f"0x{call.caller_address:08X}"
                )
            calls.append(
                {"caller_address": call.caller_address, "target_address": call.target_address}
            )
        relations.append(
            {
                "caller": caller_manifest.id.value,
                "companion": companion.target.value,
                "disc_id": companion.disc_id,
                "payload_sha256": companion.payload_sha256,
                "load_address": companion.load_address,
                "size": companion.size,
                "static_calls": calls,
                "evidence": companion.evidence,
            }
        )
    return sorted(relations, key=lambda item: (item["caller"], item["companion"]))


def verify_companion_relations(root: Path, catalog: dict[str, Any]) -> list[dict[str, Any]]:
    """Return declared companion calls whose immutable catalog evidence agrees.

    This deliberately verifies only a static call and payload identity. It does
    not establish ABI, load order, concurrent residency, or source ownership.
    """

    relations: list[dict[str, Any]] = []
    for manifest in load_target_manifests(root).values():
        for companion in manifest.companions:
            caller = resolve_entry(catalog, manifest.disc_id)
            target = resolve_entry(catalog, companion.disc_id)
            if (
                target["sha256"] != companion.payload_sha256
                or int(target["load_address"]) != companion.load_address
                or int(target["size"]) != companion.size
            ):
                raise ValueError(
                    f"companion catalog identity mismatch: {manifest.id.value} -> "
                    f"{companion.target.value}"
                )
            caller_start = int(caller["load_address"])
            caller_size = int(caller["size"])
            if manifest.load_address != caller_start:
                raise ValueError(
                    f"caller catalog load address mismatch: {manifest.id.value}"
                )
            if target["payload_kind"] != "ram":
                raise ValueError(
                    f"companion is not a RAM payload: {companion.target.value}"
                )
            payload = Path(caller["payload_path"]).read_bytes()
            calls: list[dict[str, int]] = []
            for call in companion.static_calls:
                offset = call.caller_address - caller_start
                if offset < 0 or offset + 4 > caller_size:
                    raise ValueError(
                        f"companion call outside caller payload: {manifest.id.value}"
                    )
                word = struct.unpack_from("<I", payload, offset)[0]
                if _jal_target(word, call.caller_address) != call.target_address:
                    raise ValueError(
                        f"companion call bytes differ: {manifest.id.value} at "
                        f"0x{call.caller_address:08X}"
                    )
                calls.append(
                    {
                        "caller_address": call.caller_address,
                        "target_address": call.target_address,
                    }
                )
            relations.append(
                {
                    "caller": manifest.id.value,
                    "companion": companion.target.value,
                    "disc_id": companion.disc_id,
                    "payload_sha256": companion.payload_sha256,
                    "load_address": companion.load_address,
                    "size": companion.size,
                    "static_calls": calls,
                    "evidence": companion.evidence,
                }
            )
    return sorted(relations, key=lambda item: (item["caller"], item["companion"]))


def _eligibility(root: Path, entry: dict[str, Any]) -> list[str]:
    reasons: list[str] = []
    payload = Path(entry["payload_path"])
    if not payload.is_file():
        reasons.append("missing-payload")
    elif file_sha256(payload) != entry["sha256"]:
        reasons.append("payload-sha256-mismatch")
    if entry.get("payload_kind") != "ram":
        reasons.append(f"not-ram:{entry.get('payload_kind', 'unknown')}")
    if entry.get("code_status") == "rejected":
        reasons.append("rejected")
    address, size = int(entry.get("load_address", 0)), int(entry.get("size", 0))
    if not 0x80000000 <= address < 0x80200000 or address + size > 0x80200000:
        reasons.append("invalid-ram-range")
    if size <= 0:
        reasons.append("empty-payload")
    slug = target_slug(entry)
    tracked = [
        root / f"config/targets/emi/{slug}/target.toml",
        root / f"config/targets/emi/{slug}/splat.yaml",
        root / f"config/targets/emi/{slug}/symbols.txt",
        root / f"src/emi/{slug}",
    ]
    if any(path.exists() for path in tracked):
        reasons.append("existing-reviewed-target")
    return reasons


def _base_path(config_path: Path) -> str:
    return "/".join(".." for _ in config_path.parts[:-1]) or "."


def bootstrap_plan(
    root: Path, catalog: dict[str, Any], identifier: str
) -> dict[str, Any]:
    entry = resolve_entry(catalog, identifier)
    reasons = _eligibility(root, entry)
    if reasons:
        raise ValueError(f"ineligible EMI entry {entry['id']}: {', '.join(reasons)}")
    slug = target_slug(entry)
    target = f"emi/{slug}"
    binary = f"out/binaries/emi/{slug}.bin"
    manifest = f"config/targets/emi/{slug}/target.toml"
    splat = f"config/targets/emi/{slug}/splat.yaml"
    symbols = f"config/targets/emi/{slug}/symbols.txt"
    basename = slug.replace("/", "_")
    payload = Path(entry["payload_path"])
    source = payload.relative_to(root).as_posix()
    metadata = (
        json.dumps(
            {
                "schema": "harness.normalized-emi/v1",
                "source": source,
                "source_sha256": entry["sha256"],
                "image": binary,
                "image_sha256": entry["sha256"],
                "load_address": entry["load_address"],
            },
            indent=2,
            sort_keys=True,
        )
        + "\n"
    )
    manifest_text = (
        'schema = "harness.target/v2"\n'
        f'id = "{target}"\n'
        f'disc_id = "BIN/{entry["archive_id"].upper()}.EMI#{entry["slot"]}"\n'
        'kind = "emi"\n'
        f'source_dir = "src/emi/{slug}"\n'
        f'binary = "{binary}"\n'
        f'splat = "{splat}"\n'
        f"load_address = 0x{entry['load_address']:08X}\n"
    )
    splat_text = (
        f"name: {basename}\nsha1: {hashlib.sha1(payload.read_bytes()).hexdigest()}\n"
        "options:\n  platform: psx\n  compiler: psyq\n"
        f"  basename: {basename}\n  base_path: {_base_path(Path(splat))}\n"
        f"  target_path: {binary}\n  asm_path: out/splat/emi/{slug}/asm\n"
        f"  src_path: src/emi/{slug}\n  ld_script_path: out/splat/emi/{slug}/linker.ld\n"
        "  symbol_addrs_path:\n"
        "  - config/targets/shared/symbols.txt\n"
        "  - config/sdk/psyq-slus.txt\n"
        f"  - {symbols}\n"
        f"segments:\n- [0x0, bin]\n- [0x{entry['size']:X}]\n"
    )
    return {
        "schema": "harness.emi-bootstrap/v1",
        "entry": entry["id"],
        "target": target,
        "identity": {
            "payload_sha256": entry["sha256"],
            "load_address": entry["load_address"],
            "size": entry["size"],
        },
        "files": [
            {"path": binary, "kind": "payload"},
            {"path": f"{binary}.json", "kind": "text", "content": metadata},
            {"path": manifest, "kind": "text", "content": manifest_text},
            {"path": splat, "kind": "text", "content": splat_text},
            {"path": symbols, "kind": "text", "content": ""},
        ],
    }


def _write_new(path: Path, content: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        raise ValueError(f"refusing to overwrite: {path}")
    descriptor, temporary_name = tempfile.mkstemp(
        dir=path.parent, prefix=f".{path.name}."
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(content)
            stream.flush()
            os.fsync(stream.fileno())
        os.link(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def _destination(root: Path, relative: str) -> Path:
    path = (root / relative).resolve()
    if not path.is_relative_to(root.resolve()):
        raise ValueError(f"target path escapes repository: {relative}")
    return path


def apply_bootstrap(
    root: Path, catalog: dict[str, Any], plan: dict[str, Any]
) -> list[Path]:
    fresh = bootstrap_plan(root, catalog, str(plan.get("entry", "")))
    if fresh != plan:
        raise ValueError("bootstrap plan is stale")
    created: list[Path] = []
    try:
        payload = Path(
            resolve_entry(catalog, fresh["entry"])["payload_path"]
        ).read_bytes()
        for item in fresh["files"]:
            path = _destination(root, item["path"])
            content = payload if item["kind"] == "payload" else item["content"].encode()
            _write_new(path, content)
            created.append(path)
        manifests = load_target_manifests(root)
        if fresh["target"] not in manifests:
            raise ValueError("created target manifest did not load")
        load_map(root / f"config/targets/{fresh['target']}/symbols.txt")
        return created
    except BaseException:
        for path in reversed(created):
            path.unlink(missing_ok=True)
        raise


def materialize_reviewed_targets(*, root: Path, catalog: dict[str, Any]) -> list[Path]:
    """Restore ignored binaries for already-reviewed EMI manifests."""

    verify_companion_relations(root, catalog)
    images: list[Path] = []
    for manifest in sorted(
        load_target_manifests(root).values(), key=lambda item: item.id.value
    ):
        if manifest.kind != "emi":
            continue
        entry = resolve_entry(catalog, manifest.disc_id)
        source = Path(entry["payload_path"])
        if not source.is_file() or file_sha256(source) != entry["sha256"]:
            raise ValueError(f"missing or stale payload for {manifest.disc_id}")
        if manifest.load_address != int(entry["load_address"]):
            raise ValueError(
                f"load address differs from {manifest.disc_id}: "
                f"manifest=0x{manifest.load_address:08X}, "
                f"payload=0x{int(entry['load_address']):08X}"
            )
        image = root / manifest.binary
        if image.is_file() and file_sha256(image) != entry["sha256"]:
            raise ValueError(
                f"normalized binary differs from {manifest.disc_id}: {image}"
            )
        if not image.is_file():
            _write_new(image, source.read_bytes())
        write_json(
            image.with_suffix(".bin.json"),
            {
                "schema": "harness.normalized-emi/v1",
                "source": str(source),
                "source_sha256": entry["sha256"],
                "image": str(image),
                "image_sha256": entry["sha256"],
                "load_address": manifest.load_address,
            },
        )
        images.append(image)
    return images


__all__ = [
    "apply_bootstrap",
    "bootstrap_plan",
    "build_catalog",
    "load_catalog",
    "materialize_reviewed_targets",
    "resolve_entry",
    "target_slug",
    "verify_companion_relations",
]
