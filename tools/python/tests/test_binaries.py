from __future__ import annotations

import json
import struct
from pathlib import Path

import pytest

from rebof3.binaries import (
    build_emi_catalog,
    normalize_executable,
    promote_entry,
    resolve_entry,
    write_catalog,
)


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
