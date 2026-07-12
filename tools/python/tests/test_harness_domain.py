from __future__ import annotations

from pathlib import Path

from harness.domain import load_profiles, load_target_manifests, normalize_target_id
from harness.evidence import EvidenceRepository, build_index


def test_target_ids_preserve_shipped_spelling() -> None:
    target = normalize_target_id("BIN/BATTLE/BATTLE.EMI#3")
    assert target.value == "emi/battle/battle/03"
    assert target.shipped == "BIN/BATTLE/BATTLE.EMI#3"


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
    manifests = load_target_manifests(root)
    profiles = load_profiles(root)
    summary = build_index(root, root / "out" / "index" / "test-harness.sqlite")
    assert "exe/slus_004_22" in manifests
    assert "native/capcom97" in profiles
    assert summary["targets"] == len(manifests)
