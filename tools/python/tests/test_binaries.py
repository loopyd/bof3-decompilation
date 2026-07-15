from __future__ import annotations

import json
import struct
from pathlib import Path

import pytest

from harness.binaries import (
    build_emi_catalog,
    normalize_executable,
    resolve_entry,
    target_details,
    target_progress,
)
from harness.targets import (
    materialize_promoted_emi_targets,
    promote_entry,
    write_catalog,
)


def test_materialize_promoted_emi_targets_restores_deleted_binary(
    tmp_path: Path,
) -> None:
    emi_root = tmp_path / "out" / "extracted" / "BIN"
    payload = b"promoted EMI payload"
    write_entry(emi_root, "ETC/GAME", 1, payload, 0x801D0C00)
    target = tmp_path / "config" / "targets" / "emi" / "etc" / "game" / "01.toml"
    target.parent.mkdir(parents=True)
    target.write_text(
        "\n".join(
            [
                'schema = "harness.target/v2"',
                'id = "emi/etc/game/01"',
                'disc_id = "BIN/ETC/GAME.EMI#1"',
                'kind = "emi"',
                'source_dir = "src/emi/etc/game/01"',
                'binary = "out/binaries/emi/etc/game/01.bin"',
                'splat = "config/splat/emi/etc/game/01.yaml"',
                "load_address = 0x801D0C00",
                'profile = "compat/capcom97"',
                "",
            ]
        ),
        encoding="utf-8",
    )
    catalog = build_emi_catalog(emi_root)

    images = materialize_promoted_emi_targets(root=tmp_path, catalog=catalog)

    image = tmp_path / "out" / "binaries" / "emi" / "etc" / "game" / "01.bin"
    assert images == [image]
    assert image.read_bytes() == payload
    image.unlink()
    materialize_promoted_emi_targets(root=tmp_path, catalog=catalog)
    assert image.read_bytes() == payload
    metadata = json.loads(image.with_suffix(".bin.json").read_text(encoding="utf-8"))
    assert metadata["load_address"] == 0x801D0C00


def write_entry(
    root: Path, archive: str, slot: int, payload: bytes, address: int
) -> None:
    directory = root / archive
    directory.mkdir(parents=True, exist_ok=True)
    (directory / f"{slot}.bin").write_bytes(payload)
    (directory / "emi.json").write_text(
        json.dumps(
            {
                "entries": [
                    {
                        "index": slot,
                        "name": f"{slot}.bin",
                        "type": 0,
                        "ram_ptr": address,
                    }
                ]
            }
        ),
        encoding="utf-8",
    )


def test_catalog_keeps_content_and_build_target_identity_separate(
    tmp_path: Path,
) -> None:
    root = tmp_path / "out" / "extracted" / "BIN"
    payload = struct.pack("<4I", 0x3C018000, 0x34210000, 0x03E00008, 0)
    write_entry(root, "BATTLE/BATTLE", 3, payload, 0x801D0C00)
    write_entry(root, "ETC/OTHER", 1, payload, 0x801E0000)

    catalog = build_emi_catalog(root)

    assert len(catalog["content_groups"]) == 1
    assert len(catalog["content_groups"][0]["members"]) == 2
    assert len(catalog["build_targets"]) == 2
    assert resolve_entry(catalog, "BIN/BATTLE/BATTLE.EMI#3")["id"] == "BATTLE/BATTLE#3"


def test_promotion_requires_explicit_code_confirmation(tmp_path: Path) -> None:
    emi_root = tmp_path / "out" / "extracted" / "BIN"
    write_entry(
        emi_root,
        "BATTLE/BATTLE",
        3,
        struct.pack("<4I", 0x3C018000, 0x34210000, 0x03E00008, 0),
        0x801D0C00,
    )
    catalog_path = tmp_path / "out" / "catalog" / "emi.json"
    write_catalog(emi_root, catalog_path)
    (tmp_path / "config" / "symbols").mkdir(parents=True)
    existing_header = (
        tmp_path / "src" / "emi" / "battle" / "battle" / "03" / "internal.h"
    )
    existing_header.parent.mkdir(parents=True)
    existing_header.write_text("/* reviewed declarations */\n", encoding="utf-8")

    with pytest.raises(ValueError, match="--confirm-code"):
        promote_entry(
            catalog_path=catalog_path,
            identifier="BIN/BATTLE/BATTLE.EMI#3",
            root=tmp_path,
            confirm_code=False,
        )

    config, source = promote_entry(
        catalog_path=catalog_path,
        identifier="BIN/BATTLE/BATTLE.EMI#3",
        root=tmp_path,
        confirm_code=True,
    )

    assert (
        config
        == tmp_path / "config" / "splat" / "emi" / "battle" / "battle" / "03.yaml"
    )
    assert (source / "internal.h").is_file()
    assert (source / "internal.h").read_text(encoding="utf-8") == (
        "/* reviewed declarations */\n"
    )
    normalized = tmp_path / "out" / "binaries" / "emi" / "battle" / "battle" / "03.bin"
    assert (
        normalized.read_bytes()
        == (emi_root / "BATTLE" / "BATTLE" / "3.bin").read_bytes()
    )
    assert "target_path: out/binaries/emi/battle/battle/03.bin" in config.read_text()


def test_normalize_executable_extracts_only_the_load_image(tmp_path: Path) -> None:
    executable = tmp_path / "TEST.EXE"
    data = bytearray(0x800 + 4)
    data[:8] = b"PS-X EXE"
    struct.pack_into("<I", data, 0x10, 0x80010000)
    struct.pack_into("<I", data, 0x18, 0x80010000)
    struct.pack_into("<I", data, 0x1C, 4)
    data[0x800:] = b"test"
    executable.write_bytes(data)

    metadata = normalize_executable(executable, tmp_path / "out" / "test.bin")

    assert (tmp_path / "out" / "test.bin").read_bytes() == b"test"
    assert metadata["load_address"] == 0x80010000


def test_target_details_derives_promoted_paths_without_duplicating_metadata(
    tmp_path: Path,
) -> None:
    payload = tmp_path / "out" / "extracted" / "BIN" / "BATTLE" / "BATTLE" / "3.bin"
    payload.parent.mkdir(parents=True)
    payload.write_bytes(b"target")
    config = tmp_path / "config" / "splat" / "emi" / "battle" / "battle" / "03.yaml"
    config.parent.mkdir(parents=True)
    config.write_text("name: battle\n", encoding="utf-8")
    source = tmp_path / "src" / "emi" / "battle" / "battle" / "03"
    source.mkdir(parents=True)

    details = target_details(
        {
            "id": "BATTLE/BATTLE#3",
            "archive_id": "BATTLE/BATTLE",
            "slot": 3,
            "payload_path": str(payload),
            "sha256": "abc",
            "load_address": 0x801D0C00,
            "size": 6,
            "code_status": "confirmed",
        },
        tmp_path,
    )

    assert details["payload"] == "out/extracted/BIN/BATTLE/BATTLE/3.bin"
    assert details["splat"] == "config/splat/emi/battle/battle/03.yaml"
    assert details["source"] == "src/emi/battle/battle/03"
    assert details["build"] is None
    assert details["progress"] == {
        "layout": "unsegmented",
        "reviewed_functions": 0,
        "lifted_functions": 0,
        "matched_functions": 0,
        "next_function": None,
        "whole_payload_match": False,
    }


def test_target_progress_uses_reviewed_splat_c_subsegments(tmp_path: Path) -> None:
    entry = {
        "id": "WORLD00/AREA008#13",
        "archive_id": "WORLD00/AREA008",
        "slot": 13,
        "payload_path": str(tmp_path / "13.bin"),
        "sha256": "abc",
        "load_address": 0x801F2C00,
        "size": 0x100,
        "code_status": "confirmed",
    }
    config = tmp_path / "config" / "splat" / "emi" / "world00" / "area008" / "13.yaml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "segments:\n"
        "  - name: main\n"
        "    type: code\n"
        "    subsegments:\n"
        "      - [0x14, c, func_801f2c14]\n"
        "      - [0x58, c, func_801f2c58]\n",
        encoding="utf-8",
    )
    source = tmp_path / "src" / "emi" / "world00" / "area008" / "13"
    source.mkdir(parents=True)
    (source / "func_801f2c14.c").write_text("void func_801f2c14(void) {}\n")

    assert target_progress(entry, tmp_path) == {
        "layout": "reviewed",
        "reviewed_functions": 2,
        "lifted_functions": 1,
        "matched_functions": 0,
        "next_function": 0x801F2C58,
        "whole_payload_match": False,
    }
