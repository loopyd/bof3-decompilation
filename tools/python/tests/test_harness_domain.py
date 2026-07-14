from __future__ import annotations

from pathlib import Path

import pytest

from harness.domain import load_profiles, load_target_manifests, normalize_target_id
from harness.evidence import EvidenceRepository, build_index


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


def test_repository_schema_contains_graph_tables(tmp_path: Path) -> None:
    database = tmp_path / "harness.sqlite"
    with EvidenceRepository(database) as repository:
        repository.initialize()
        tables = {
            row[0]
            for row in repository.execute(
                "SELECT name FROM sqlite_master WHERE type = 'table'"
            )
        }
    assert {"targets", "functions", "edges", "evidence"} <= tables


def test_repository_index_uses_canonical_manifests() -> None:
    root = Path(__file__).resolve().parents[3]
    database = root / "out" / "index" / "test-harness.sqlite"
    manifests = load_target_manifests(root)
    profiles = load_profiles(root)
    summary = build_index(root, database)
    assert "exe/slus_004_22" in manifests
    assert "native/capcom97" in profiles
    assert summary["targets"] == len(manifests)
    with EvidenceRepository(database) as repository:
        indexed = repository.execute(
            "SELECT address FROM symbols WHERE target_id = ? AND name = ?",
            ("exe/slus_004_22", "CdSync"),
        ).fetchone()
    assert indexed is not None
    assert indexed[0] == 0x80175640
