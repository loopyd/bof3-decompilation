from __future__ import annotations

import hashlib
import json
import struct
from pathlib import Path

from ..models import InventoryProgram, InventorySnapshot

DEFAULT_PSX_PROCESSOR = "PSX:LE:32:default"
DEFAULT_COMPILER = "default"


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def parse_psx_exe(path: Path) -> dict[str, int]:
    data = path.read_bytes()
    if len(data) < 0x20 or data[:8] != b"PS-X EXE":
        raise ValueError(f"not a PS-X EXE: {path}")
    return {
        "pc0": struct.unpack_from("<I", data, 0x10)[0],
        "text_addr": struct.unpack_from("<I", data, 0x18)[0],
        "text_size": struct.unpack_from("<I", data, 0x1C)[0],
    }


def scan_boot_program(
    *,
    path: Path,
    program_id: str,
    project_folder_path: str,
) -> InventoryProgram:
    header = parse_psx_exe(path)
    return InventoryProgram(
        program_id=program_id,
        kind="boot",
        source_path=str(path),
        payload_path=str(path),
        project_folder_path=project_folder_path,
        program_name=path.name,
        loader_mode="psx",
        processor=DEFAULT_PSX_PROCESSOR,
        compiler=DEFAULT_COMPILER,
        base_addr=header["text_addr"],
        file_offset=0x800,
        length=header["text_size"],
        block_name=path.name,
        size=path.stat().st_size,
        sha256=file_sha256(path),
    )


def scan_emi_root(root: Path) -> list[InventoryProgram]:
    programs: list[InventoryProgram] = []
    for manifest_path in sorted(root.rglob("emi.json")):
        archive_dir = manifest_path.parent
        archive_id = archive_dir.relative_to(root).as_posix()
        family = archive_dir.relative_to(root).parts[0] if archive_id else None
        payload = json.loads(manifest_path.read_text(encoding="utf-8"))
        entries = payload.get("entries", [])
        if not isinstance(entries, list):
            raise ValueError(f"invalid EMI manifest: {manifest_path}")
        archive_name = archive_dir.name
        for entry in entries:
            if not isinstance(entry, dict):
                continue
            entry_type = int(entry.get("type") or 0)
            ram_ptr = int(entry.get("ram_ptr") or 0)
            if entry_type != 0 or ram_ptr < 0x80000000:
                continue
            entry_index = int(entry.get("index") or 0)
            entry_name = str(entry.get("name") or f"{entry_index}.bin")
            payload_path = archive_dir / entry_name
            if not payload_path.is_file():
                continue
            program_name = f"{archive_name}_e{entry_index:02d}_{ram_ptr:08x}.bin"
            size = payload_path.stat().st_size
            programs.append(
                InventoryProgram(
                    program_id=f"/bins/{archive_id}#{entry_index}",
                    kind="overlay",
                    source_path=str(payload_path),
                    payload_path=str(payload_path),
                    project_folder_path=f"/bins/{archive_id}",
                    program_name=program_name,
                    loader_mode="raw",
                    processor=DEFAULT_PSX_PROCESSOR,
                    compiler=DEFAULT_COMPILER,
                    base_addr=ram_ptr,
                    file_offset=0,
                    length=size,
                    block_name=program_name[:60],
                    size=size,
                    sha256=file_sha256(payload_path),
                    family=family,
                    archive_id=archive_id,
                    entry_index=entry_index,
                )
            )
    return programs


def scan_inventory(
    *,
    slus_path: Path | None,
    logo_path: Path | None,
    emi_root: Path | None,
) -> InventorySnapshot:
    programs: list[InventoryProgram] = []
    if slus_path is not None:
        programs.append(
            scan_boot_program(
                path=slus_path.resolve(),
                program_id="/boot/SLUS_004.22",
                project_folder_path="/boot",
            )
        )
    if logo_path is not None:
        programs.append(
            scan_boot_program(
                path=logo_path.resolve(),
                program_id="/boot/LOGO.EXE",
                project_folder_path="/boot",
            )
        )
    if emi_root is not None:
        programs.extend(scan_emi_root(emi_root.resolve()))
    return InventorySnapshot(programs=programs)
