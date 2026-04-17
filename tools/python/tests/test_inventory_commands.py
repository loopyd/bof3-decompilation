from __future__ import annotations

import json
import struct
from pathlib import Path

from rebof3.commands import inventory as inventory_command
from rebof3.jsonio import read_json
from rebof3.paths import repo_layout


def write_psx_exe(path: Path, *, text_addr: int, text_size: int, pc0: int) -> None:
    data = bytearray(0x800 + text_size)
    data[:8] = b"PS-X EXE"
    struct.pack_into("<I", data, 0x10, pc0)
    struct.pack_into("<I", data, 0x18, text_addr)
    struct.pack_into("<I", data, 0x1C, text_size)
    path.write_bytes(data)


def write_psx_exe_with_slot_table(
    path: Path,
    *,
    text_addr: int,
    text_size: int,
    pc0: int,
    table_addr: int,
    slots: list[int],
) -> None:
    data = bytearray(0x800 + text_size)
    data[:8] = b"PS-X EXE"
    struct.pack_into("<I", data, 0x10, pc0)
    struct.pack_into("<I", data, 0x18, text_addr)
    struct.pack_into("<I", data, 0x1C, text_size)
    file_offset = 0x800 + (table_addr - text_addr)
    struct.pack_into(f"<{len(slots)}I", data, file_offset, *slots)
    path.write_bytes(data)


def write_emi_archive(
    root: Path,
    archive_id: str,
    entries: list[dict[str, object]],
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


def write_entry_table_overlay(*, entry_count: int, base_addr: int) -> bytes:
    data = bytearray(0x3000)
    struct.pack_into("<I", data, 0, entry_count)
    for index in range(entry_count):
        struct.pack_into(
            "<I", data, 4 + (index * 4), base_addr + 0x100 + (index * 0x10)
        )
    return bytes(data)


def test_inventory_build_writes_artifact_family(tmp_path: Path) -> None:
    root = tmp_path / "workspace"
    layout = repo_layout(root)
    layout.extracted_dir.mkdir(parents=True)
    (layout.extracted_dir / "LOGO").mkdir(parents=True)
    layout.emi_root.mkdir(parents=True)

    write_psx_exe(
        layout.slus_path, text_addr=0x80096800, text_size=0x400, pc0=0x8014AA0C
    )
    write_psx_exe(
        layout.logo_path, text_addr=0x80010000, text_size=0x200, pc0=0x80010100
    )

    shared_payload = b"\x01\x02\x03\x04"
    entry_table_payload = write_entry_table_overlay(entry_count=8, base_addr=0x80110000)
    write_emi_archive(
        layout.emi_root,
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
        ],
    )
    write_emi_archive(
        layout.emi_root,
        "SCENARIO/ACTOR",
        [
            {
                "index": 3,
                "name": "3.bin",
                "payload": entry_table_payload,
                "ram_ptr": 0x80110000,
                "type": 0,
            }
        ],
    )

    result = inventory_command.main(
        [
            "build",
            "--root",
            str(root),
            "--slus",
            str(layout.slus_path),
            "--logo",
            str(layout.logo_path),
            "--emi-root",
            str(layout.emi_root),
        ]
    )

    assert result == 0

    inventory_payload = read_json(layout.inventory_path)
    groups_payload = read_json(layout.groups_path)
    overlay_catalog = read_json(layout.inventory_overlay_catalog_path)
    entry_tables = read_json(layout.inventory_entry_tables_path)
    project_plan = read_json(layout.inventory_project_plan_path)
    artifact_index = read_json(layout.inventory_artifact_index_path)

    assert len(inventory_payload["programs"]) == 5
    assert len(groups_payload["groups"]) == 1
    assert overlay_catalog["candidate_count"] == 3
    assert entry_tables["candidate_count"] == 1
    assert project_plan["function_candidate_count"] == 1
    assert len(artifact_index["artifacts"]) >= 10
    assert layout.inventory_render_metadata_md_path.is_file()


def test_inventory_slot_map_command_reads_disc_lba_json(tmp_path: Path) -> None:
    slus = tmp_path / "SLUS_004.22"
    disc_lba = tmp_path / "disc_lba.json"
    slot_map_json = tmp_path / "slot_map.json"
    slot_map_md = tmp_path / "slot_map.md"

    write_psx_exe_with_slot_table(
        slus,
        text_addr=0x80096800,
        text_size=0x100000,
        pc0=0x8014AA0C,
        table_addr=0x80182444,
        slots=[111, 222, 999],
    )
    disc_lba.write_text(
        json.dumps(
            {
                "entries": [
                    {
                        "archive_name": "FIRST",
                        "archive_type": "EMI",
                        "family": "ETC",
                        "lba": 111,
                        "source_path": "build/extracted/BIN/ETC/FIRST.EMI",
                    },
                    {
                        "archive_name": "SECOND",
                        "archive_type": "EMI",
                        "family": "SCENARIO",
                        "lba": 222,
                        "source_path": "build/extracted/BIN/SCENARIO/SECOND.EMI",
                    },
                ]
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )

    result = inventory_command.main(
        [
            "slot-map",
            "--slus",
            str(slus),
            "--disc-lba",
            str(disc_lba),
            "--json-out",
            str(slot_map_json),
            "--md-out",
            str(slot_map_md),
            "--slot-count",
            "3",
        ]
    )

    assert result == 0
    payload = read_json(slot_map_json)
    assert payload["slot_count"] == 3
    assert payload["unresolved_slot_count"] == 1
    assert payload["slots"][0]["resolved"] is True
    assert payload["slots"][2]["resolved"] is False
    assert slot_map_md.is_file()


def test_inventory_import_ghidra_symbols_writes_indexes_and_program_files(
    tmp_path: Path,
) -> None:
    raw_export = tmp_path / "raw.json"
    index_out = tmp_path / "index.json"
    function_index_out = tmp_path / "functions.json"
    function_index_tsv_out = tmp_path / "functions.tsv"
    md_out = tmp_path / "index.md"
    program_output_dir = tmp_path / "programs"

    raw_export.write_text(
        json.dumps(
            {
                "project_name": "bof3_main",
                "rows": [
                    {
                        "kind": "function",
                        "program_path": "/SLUS_004.22.17",
                        "address": "80162d00",
                        "name": "emi_ready",
                        "type_spec": "bool emi_ready(void)",
                        "body_min": "80162d00",
                        "body_max": "80162d1f",
                        "namespace": "Global",
                        "name_source": "USER_DEFINED",
                        "is_thunk": False,
                    },
                    {
                        "kind": "function",
                        "program_path": "/bins/ETC/FIRST#9",
                        "address": "80100000",
                        "name": "overlay_start",
                        "type_spec": "void overlay_start(void)",
                        "body_min": "80100000",
                        "body_max": "8010003f",
                        "namespace": "Global",
                        "name_source": "USER_DEFINED",
                        "is_thunk": False,
                    },
                ],
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )

    result = inventory_command.main(
        [
            "import-ghidra-symbols",
            str(raw_export),
            "--index-out",
            str(index_out),
            "--function-index-out",
            str(function_index_out),
            "--function-index-tsv-out",
            str(function_index_tsv_out),
            "--md-out",
            str(md_out),
            "--program-output-dir",
            str(program_output_dir),
        ]
    )

    assert result == 0

    index_payload = read_json(index_out)
    function_payload = read_json(function_index_out)
    assert index_payload["program_count"] == 2
    assert index_payload["function_count"] == 2
    assert index_payload["programs"][0]["program_path"] == "/bins/ETC/FIRST#9"
    assert index_payload["programs"][1]["program_path"] == "/boot/SLUS_004.22"
    assert len(function_payload["rows"]) == 2
    assert "emi_ready" in function_index_tsv_out.read_text(encoding="utf-8")
    assert md_out.is_file()
    assert len(list(program_output_dir.glob("*_ghidra_symbols.json"))) == 2
