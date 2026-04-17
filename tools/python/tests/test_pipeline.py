from __future__ import annotations

import json
import struct
from pathlib import Path

from rebof3.inventory import group_exact_duplicates, scan_inventory
from rebof3.planning import build_ghidra_manifest


def write_psx_exe(path: Path, *, text_addr: int, text_size: int, pc0: int) -> None:
    data = bytearray(0x800 + text_size)
    data[:8] = b"PS-X EXE"
    struct.pack_into("<I", data, 0x10, pc0)
    struct.pack_into("<I", data, 0x18, text_addr)
    struct.pack_into("<I", data, 0x1C, text_size)
    path.write_bytes(data)


def write_emi_archive(
    root: Path, archive_id: str, entries: list[dict[str, object]]
) -> None:
    archive_dir = root / archive_id
    archive_dir.mkdir(parents=True, exist_ok=True)
    manifest_entries: list[dict[str, object]] = []
    for entry in entries:
        filename = str(entry["name"])
        payload = bytes(entry["payload"])
        (archive_dir / filename).write_bytes(payload)
        manifest_entries.append(
            {
                "index": int(entry["index"]),
                "name": filename,
                "ram_ptr": int(entry["ram_ptr"]),
                "size": len(payload),
                "type": int(entry["type"]),
            }
        )
    (archive_dir / "emi.json").write_text(
        json.dumps({"entries": manifest_entries}, indent=2) + "\n",
        encoding="utf-8",
    )


def test_inventory_group_and_ghidra_plan(tmp_path: Path) -> None:
    slus = tmp_path / "SLUS_004.22"
    logo_dir = tmp_path / "LOGO"
    logo_dir.mkdir()
    logo = logo_dir / "LOGO.EXE"
    emi_root = tmp_path / "BIN"

    write_psx_exe(slus, text_addr=0x80096800, text_size=0x40, pc0=0x8014AA0C)
    write_psx_exe(logo, text_addr=0x80010000, text_size=0x20, pc0=0x80010100)

    shared_payload = b"\x01\x02\x03\x04"
    write_emi_archive(
        emi_root,
        "ETC/FIRST",
        [
            {
                "index": 9,
                "name": "9.bin",
                "payload": shared_payload,
                "ram_ptr": 0x80100000,
                "type": 0,
            },
            {
                "index": 10,
                "name": "10.bin",
                "payload": shared_payload,
                "ram_ptr": 0x80100000,
                "type": 0,
            },
            {
                "index": 1,
                "name": "1.t08",
                "payload": b"ignore",
                "ram_ptr": 0,
                "type": 1,
            },
        ],
    )

    snapshot = scan_inventory(slus_path=slus, logo_path=logo, emi_root=emi_root)
    assert len(snapshot.programs) == 4

    groups = group_exact_duplicates(snapshot)
    assert len(groups.groups) == 1
    assert groups.groups[0].member_program_ids == [
        "/bins/ETC/FIRST#9",
        "/bins/ETC/FIRST#10",
    ]
    assert groups.groups[0].representative_program_id == "/bins/ETC/FIRST#9"

    manifest = build_ghidra_manifest(snapshot, groups)
    assert len(manifest.imports) == 3
    assert [entry.project_folder_path for entry in manifest.imports] == [
        "/bins/ETC/FIRST",
        "/boot",
        "/boot",
    ]
    raw_import = manifest.imports[0]
    assert raw_import.loader.loader_mode == "raw"
    assert raw_import.loader.loader_args[0] == {
        "name": "-loader-baseAddr",
        "value": "0x80100000",
    }
