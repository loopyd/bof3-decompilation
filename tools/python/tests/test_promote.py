"""Tests for the promote workflow and its overwrite protection."""

from __future__ import annotations

import json
import re
import struct
from pathlib import Path

import pytest

from harness.binaries import splat_config_text
from harness.targets import (
    promote_entry,
    write_catalog,
)


def _payload() -> bytes:
    return struct.pack("<4I", 0x3C018000, 0x34210000, 0x03E00008, 0)


def _write_entry(emi_root: Path, archive: str, slot: int, address: int) -> None:
    directory = emi_root / archive
    directory.mkdir(parents=True, exist_ok=True)
    (directory / f"{slot}.bin").write_bytes(_payload())
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


def test_splat_config_text_emits_complete_splat_041_layout(tmp_path: Path) -> None:
    payload = _payload()
    payload_path = tmp_path / "payload.bin"
    payload_path.write_bytes(payload)
    entry = {
        "archive_id": "BATTLE/BATTLE",
        "slot": 3,
        "payload_path": str(payload_path),
    }
    text = splat_config_text(entry, tmp_path, target_path=payload_path)
    assert "basename: battle_battle_03" in text
    assert "base_path: ../../../../.." in text
    assert "target_path: payload.bin" in text
    assert "symbol_addrs_path:" in text
    assert "config/symbols/psyq.txt" in text
    assert "config/symbols/shared.txt" in text
    # Splat 0.41 needs an explicit EOF sentinel after the bin segment.
    assert re.search(r"-\s*\[0x[0-9a-fA-F]+\]\s*$", text, re.M) is not None


def test_promote_writes_manifest_and_tracked_splat(tmp_path: Path) -> None:
    emi_root = tmp_path / "out" / "extracted" / "BIN"
    _write_entry(emi_root, "BATTLE/BATTLE", 3, 0x801D0C00)
    catalog_path = tmp_path / "out" / "catalog" / "emi.json"
    write_catalog(emi_root, catalog_path)

    config, source = promote_entry(
        catalog_path=catalog_path,
        identifier="BIN/BATTLE/BATTLE.EMI#3",
        root=tmp_path,
        confirm_code=True,
    )

    manifest_path = (
        tmp_path
        / "config"
        / "targets"
        / "emi"
        / "battle"
        / "battle"
        / "03.toml"
    )
    assert manifest_path.is_file()
    manifest = manifest_path.read_text(encoding="utf-8")
    assert 'id = "emi/battle/battle/03"' in manifest
    assert 'disc_id = "BIN/BATTLE/BATTLE.EMI#3"' in manifest
    assert 'kind = "emi"' in manifest
    assert 'status = "quarantined"' in manifest
    assert 'source_dir = "src/emi/battle/battle/03"' in manifest
    assert 'splat = "config/splat/emi/battle/battle/03.yaml"' in manifest
    assert "load_address = 0x801d0c00" in manifest
    assert 'profile = "compat/capcom97"' in manifest

    splat = config.read_text(encoding="utf-8")
    assert "basename: battle_battle_03" in splat
    assert "base_path: ../../../../.." in splat

    normalized = tmp_path / "out" / "binaries" / "emi" / "battle" / "battle" / "03.bin"
    assert normalized.is_file()
    metadata = json.loads(normalized.with_suffix(".bin.json").read_text(encoding="utf-8"))
    assert metadata["load_address"] == 0x801D0C00
    assert metadata["source_sha256"] == json.loads(
        catalog_path.read_text(encoding="utf-8")
    )["entries"][0]["sha256"]
    assert source == tmp_path / "src" / "emi" / "battle" / "battle" / "03"


def test_promote_refuses_to_overwrite_existing_target(tmp_path: Path) -> None:
    emi_root = tmp_path / "out" / "extracted" / "BIN"
    _write_entry(emi_root, "BATTLE/BATTLE", 3, 0x801D0C00)
    catalog_path = tmp_path / "out" / "catalog" / "emi.json"
    write_catalog(emi_root, catalog_path)

    config, _ = promote_entry(
        catalog_path=catalog_path,
        identifier="BIN/BATTLE/BATTLE.EMI#3",
        root=tmp_path,
        confirm_code=True,
    )
    # A second promotion must refuse to overwrite the existing tracked
    # bootstrap or canonical manifest.
    with pytest.raises(ValueError, match="already promoted"):
        promote_entry(
            catalog_path=catalog_path,
            identifier="BIN/BATTLE/BATTLE.EMI#3",
            root=tmp_path,
            confirm_code=True,
        )


def test_promote_refuses_to_overwrite_existing_manifest(tmp_path: Path) -> None:
    emi_root = tmp_path / "out" / "extracted" / "BIN"
    _write_entry(emi_root, "BATTLE/BATTLE", 3, 0x801D0C00)
    catalog_path = tmp_path / "out" / "catalog" / "emi.json"
    write_catalog(emi_root, catalog_path)

    # Pre-create only the manifest, leaving other artifacts untouched.
    manifest_path = (
        tmp_path
        / "config"
        / "targets"
        / "emi"
        / "battle"
        / "battle"
        / "03.toml"
    )
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text('id = "emi/battle/battle/03"\n', encoding="utf-8")

    with pytest.raises(ValueError, match="already promoted"):
        promote_entry(
            catalog_path=catalog_path,
            identifier="BIN/BATTLE/BATTLE.EMI#3",
            root=tmp_path,
            confirm_code=True,
        )


def test_promote_preserves_existing_internal_header(tmp_path: Path) -> None:
    emi_root = tmp_path / "out" / "extracted" / "BIN"
    _write_entry(emi_root, "BATTLE/BATTLE", 3, 0x801D0C00)
    catalog_path = tmp_path / "out" / "catalog" / "emi.json"
    write_catalog(emi_root, catalog_path)

    source_dir = tmp_path / "src" / "emi" / "battle" / "battle" / "03"
    source_dir.mkdir(parents=True)
    header = source_dir / "internal.h"
    header.write_text("/* reviewed declarations */\n", encoding="utf-8")

    promote_entry(
        catalog_path=catalog_path,
        identifier="BIN/BATTLE/BATTLE.EMI#3",
        root=tmp_path,
        confirm_code=True,
    )

    assert header.read_text(encoding="utf-8") == "/* reviewed declarations */\n"
