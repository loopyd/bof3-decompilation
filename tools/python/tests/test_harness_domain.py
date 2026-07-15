from __future__ import annotations

from pathlib import Path

import pytest

from harness.targets import load_target_manifests, normalize_target_id


def test_target_ids_preserve_shipped_spelling() -> None:
    target = normalize_target_id("BIN/BATTLE/BATTLE.EMI#3")
    assert target.value == "emi/battle/battle/03"
    assert target.shipped == "BIN/BATTLE/BATTLE.EMI#3"


def test_target_manifest_loads_reviewed_matching_section_placement() -> None:
    root = Path(__file__).resolve().parents[3]

    manifest = load_target_manifests(root)["emi/etc/game/00"]
    placement = manifest.section_placements[0x80197378][0]

    assert placement.section == ".rodata"
    assert placement.address == 0x80195830
    assert placement.size == 0x28


def test_target_manifest_rejects_duplicate_matching_section_placement(
    tmp_path: Path,
) -> None:
    target = tmp_path / "config" / "targets" / "test.toml"
    target.parent.mkdir(parents=True)
    target.write_text(
        'schema = "harness.target/v2"\n'
        'id = "emi/test/test/00"\n'
        'kind = "emi"\n'
        'source_dir = "src/emi/test/test/00"\n'
        'binary = "out/test.bin"\n'
        'splat = "config/splat/test.yaml"\n'
        "load_address = 0x80100000\n"
        'profile = "native/test"\n'
        "[[matching.section_placements]]\n"
        "function = 0x80100100\n"
        'section = ".rodata"\n'
        "address = 0x80100020\n"
        "size = 0x10\n"
        "[[matching.section_placements]]\n"
        "function = 0x80100100\n"
        'section = ".rodata"\n'
        "address = 0x80100040\n"
        "size = 0x10\n",
        encoding="utf-8",
    )

    with pytest.raises(ValueError, match="duplicate matching section placement"):
        load_target_manifests(tmp_path)
