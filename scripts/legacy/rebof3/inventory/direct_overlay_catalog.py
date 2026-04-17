from __future__ import annotations

import hashlib
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

from ..assets.emi_archive import EmiArchive
from ..config import ROOT


DEFAULT_EMI_ROOT = ROOT / "build" / "extracted"


def repo_relative_or_absolute(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return str(path)


def file_sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sanitize(text: str) -> str:
    cleaned = []
    for char in text:
        if char.isalnum():
            cleaned.append(char.lower())
        else:
            cleaned.append("_")
    collapsed = "".join(cleaned).strip("_")
    while "__" in collapsed:
        collapsed = collapsed.replace("__", "_")
    return collapsed or "unknown"


def archive_id_parts(emi_path: Path, emi_root: Path) -> tuple[str, ...]:
    repo_rel = None
    try:
        repo_rel = emi_path.relative_to(ROOT)
    except ValueError:
        repo_rel = None

    if repo_rel is not None:
        parts = repo_rel.with_suffix("").parts
        if len(parts) >= 3 and parts[0] == "build" and parts[1] == "extracted":
            return parts[2:]

    try:
        return emi_path.relative_to(emi_root).with_suffix("").parts
    except ValueError:
        return (emi_path.stem,)


def archive_id_from_emi_path(emi_path: Path, emi_root: Path) -> str:
    return "/".join(archive_id_parts(emi_path, emi_root))


def family_from_emi_path(emi_path: Path, emi_root: Path) -> str:
    parts = archive_id_parts(emi_path, emi_root)
    if parts and parts[0] == "BIN" and len(parts) >= 2:
        return parts[1]
    if parts:
        return parts[0]
    return "unknown"


def candidate_name(
    family: str, archive_name: str, entry_index: int, ram_ptr: int
) -> str:
    return f"ovl_{sanitize(family)}_{sanitize(archive_name)}_e{entry_index:02d}_{ram_ptr:08x}"


def virtual_payload_path(emi_path: Path, entry_index: int) -> str:
    return f"{repo_relative_or_absolute(emi_path)}#{entry_index}"


def iter_emi_archives(emi_root: Path) -> list[Path]:
    if emi_root.is_file():
        return [emi_root]
    return sorted(emi_root.rglob("*.EMI"))


def is_overlay_candidate(entry_type: int, load_arg: int) -> bool:
    return entry_type == 0 and load_arg >= 0x80000000


def build_catalog(emi_root: Path = DEFAULT_EMI_ROOT) -> dict[str, Any]:
    candidates: list[dict[str, Any]] = []
    address_counts: Counter[str] = Counter()
    family_counts: Counter[str] = Counter()
    by_hash: defaultdict[str, list[int]] = defaultdict(list)

    for emi_path in iter_emi_archives(emi_root):
        archive = EmiArchive(emi_path)
        archive_id = archive_id_from_emi_path(emi_path, emi_root)
        archive_name = emi_path.stem
        family = family_from_emi_path(emi_path, emi_root)
        emi_path_ref = repo_relative_or_absolute(emi_path)

        for entry in archive.entries:
            if not is_overlay_candidate(entry.type_id, entry.load_arg):
                continue

            payload = archive.payload(entry.index)
            sha256 = file_sha256_bytes(payload)
            candidate = {
                "archive_id": archive_id,
                "archive_name": archive_name,
                "candidate_name": candidate_name(
                    family, archive_name, entry.index, entry.load_arg
                ),
                "emi_path": emi_path_ref,
                "entry_index": entry.index,
                "entry_name": entry.default_name,
                "family": family,
                "first4": entry.first_word,
                "payload_offset": entry.payload_offset,
                "payload_path": virtual_payload_path(emi_path, entry.index),
                "ram_ptr": entry.load_arg,
                "ram_ptr_hex": f"0x{entry.load_arg:08x}",
                "sha256": sha256,
                "size": entry.size,
                "type": entry.type_id,
            }
            by_hash[sha256].append(len(candidates))
            candidates.append(candidate)
            address_counts[candidate["ram_ptr_hex"]] += 1
            family_counts[family] += 1

    for indexes in by_hash.values():
        group_size = len(indexes)
        for idx in indexes:
            candidates[idx]["duplicate_group_size"] = group_size

    return {
        "generated_from": repo_relative_or_absolute(emi_root),
        "candidate_count": len(candidates),
        "family_counts": dict(sorted(family_counts.items())),
        "load_address_counts": dict(
            sorted(address_counts.items(), key=lambda item: (-item[1], item[0]))
        ),
        "unique_payload_hashes": len(by_hash),
        "candidates": candidates,
    }


def parse_virtual_payload_ref(payload_path: str) -> tuple[Path, int] | None:
    path_text, separator, entry_text = payload_path.rpartition("#")
    if separator == "" or not entry_text.isdigit():
        return None
    return ROOT / path_text, int(entry_text)


def payload_bytes_from_candidate(candidate: dict[str, Any]) -> bytes:
    emi_path_text = candidate.get("emi_path")
    entry_index = candidate.get("entry_index")
    if isinstance(emi_path_text, str) and isinstance(entry_index, int):
        archive = EmiArchive(ROOT / emi_path_text)
        return archive.payload(entry_index)

    payload_path_text = candidate.get("payload_path")
    if isinstance(payload_path_text, str):
        parsed = parse_virtual_payload_ref(payload_path_text)
        if parsed is not None:
            emi_path, parsed_entry_index = parsed
            archive = EmiArchive(emi_path)
            return archive.payload(parsed_entry_index)

        payload_path = ROOT / payload_path_text
        if payload_path.is_file():
            return payload_path.read_bytes()

    raise ValueError(f"candidate does not resolve to a payload source: {candidate}")


def materialize_candidate_payload(candidate: dict[str, Any], output_dir: Path) -> Path:
    payload_path_text = candidate.get("payload_path")
    if isinstance(payload_path_text, str) and "#" not in payload_path_text:
        payload_path = ROOT / payload_path_text
        if payload_path.is_file():
            return payload_path

    output_dir.mkdir(parents=True, exist_ok=True)
    candidate_name_text = candidate.get("candidate_name", "overlay_candidate")
    staged_path = output_dir / f"{candidate_name_text}.bin"
    payload = payload_bytes_from_candidate(candidate)

    if not staged_path.is_file() or staged_path.read_bytes() != payload:
        staged_path.write_bytes(payload)

    return staged_path
