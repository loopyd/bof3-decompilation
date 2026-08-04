"""EMI catalog entry verification: companion calls and entry resolution."""

from __future__ import annotations

import re
import struct
from pathlib import Path
from typing import Any

from ..discovery import file_sha256
from ..domain import load_target_manifests, normalize_target_id
from ..io import read_json


def verify_declared_companions(
    root: Path, caller_manifest: Any
) -> list[dict[str, Any]]:
    """Verify one caller's declared companion facts without a global EMI scan."""

    caller = _direct_entry(root, caller_manifest.disc_id)
    if caller_manifest.load_address != int(caller["load_address"]):
        raise ValueError(
            f"caller catalog load address mismatch: {caller_manifest.id.value}"
        )
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
            raise ValueError(
                f"companion is not a RAM payload: {companion.target.value}"
            )
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
                {
                    "caller_address": call.caller_address,
                    "target_address": call.target_address,
                }
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


def verify_companion_relations(
    root: Path, catalog: dict[str, Any]
) -> list[dict[str, Any]]:
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
    matches = [
        entry
        for entry in entries
        if isinstance(entry, dict) and int(entry.get("index") or 0) == slot
    ]
    if len(matches) != 1:
        raise ValueError(
            f"expected one extracted EMI entry for {identifier}; found {len(matches)}"
        )
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


def target_slug(entry: dict[str, Any]) -> str:
    family, archive = _archive_parts(str(entry["archive_id"]))
    slot = int(entry["slot"])
    if not 0 <= slot <= 255:
        raise ValueError(f"invalid EMI slot: {slot}")
    return f"{family.lower()}/{archive.lower()}/{slot:02d}"


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
