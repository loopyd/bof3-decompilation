from __future__ import annotations

import json
import time
from pathlib import Path

from harness.reverse import (
    EVIDENCE_LADDER,
    MODULE_MILESTONES,
    MissionState,
    infer_goal,
    load_mission,
    plan_mission,
    preview_mission,
    save_mission,
    score_candidates,
    select_next_function,
)


def test_plan_mission_creates_correct_paths(tmp_path: Path) -> None:
    mission = plan_mission(tmp_path, "emi_battle_battle_03", 0x801D0C00)
    assert mission.mission_id == "emi_battle_battle_03_0x801d0c00"
    assert mission.target_id == "emi_battle_battle_03"
    assert mission.address == 0x801D0C00
    assert mission.goal == "lift"
    assert mission.strategy == "balanced"
    assert mission.status == "pending"
    assert mission.attempts == 0
    assert mission.max_attempts == 3
    assert mission.budget_functions == 1
    assert mission.budget_time_seconds == 1800
    assert mission.budget_depth == 1
    assert mission.created_at <= time.time()

    path = save_mission(tmp_path, mission)
    expected = (
        tmp_path
        / "out"
        / "reverse"
        / "emi_battle_battle_03"
        / "functions"
        / "func_801d0c00"
        / "mission.json"
    )
    assert path == expected
    assert path.exists()


def test_save_and_load_mission_round_trip(tmp_path: Path) -> None:
    mission = plan_mission(
        tmp_path, "emi_battle_battle_03", 0x801D0C00, strategy="hot"
    )
    save_mission(tmp_path, mission)
    loaded = load_mission(tmp_path, mission.mission_id)
    assert loaded is not None
    assert loaded.mission_id == mission.mission_id
    assert loaded.target_id == mission.target_id
    assert loaded.address == mission.address
    assert loaded.goal == mission.goal
    assert loaded.strategy == mission.strategy
    assert loaded.status == mission.status
    assert loaded.attempts == mission.attempts
    assert loaded.max_attempts == mission.max_attempts
    assert loaded.budget_functions == mission.budget_functions
    assert loaded.budget_time_seconds == mission.budget_time_seconds
    assert loaded.budget_depth == mission.budget_depth


def test_preview_mission_returns_expected_keys(tmp_path: Path) -> None:
    mission = plan_mission(
        tmp_path, "emi_battle_battle_03", 0x801D0C00, strategy="root"
    )
    preview = preview_mission(tmp_path, mission)
    assert set(preview.keys()) == {
        "target",
        "address",
        "inferred_goal",
        "strategy",
        "budget",
        "exclusions",
        "alternatives",
    }
    assert preview["target"] == "emi_battle_battle_03"
    assert preview["address"] == 0x801D0C00
    assert preview["inferred_goal"] == "lift"
    assert preview["strategy"] == "root"
    assert preview["budget"] == {
        "functions": 1,
        "time_seconds": 1800,
        "depth": 1,
    }
    assert preview["exclusions"] == []
    assert preview["alternatives"] == []


def test_preview_mission_with_none_address_suggests_next(tmp_path: Path) -> None:
    target = "emi_battle_battle_03"
    config = tmp_path / "config" / "splat" / f"{target}.yaml"
    config.parent.mkdir(parents=True, exist_ok=True)
    config.write_text("segments:\n  - [0x100, c]\n  - [0x200, c]\n", encoding="utf-8")

    # Function at 0x100 is not lifted → highest score
    mission = plan_mission(tmp_path, target, None)
    preview = preview_mission(tmp_path, mission)
    assert preview["address"] == 0x100
    assert preview["inferred_goal"] == "lift"
    assert preview["suggested_function"]["address"] == 0x100
    assert preview["suggested_function"]["goal"] == "lift"


def test_mission_state_defaults() -> None:
    now = time.time()
    mission = MissionState(
        mission_id="test_mission",
        target_id="test_target",
        address=0x80010000,
        goal="lift",
        strategy="balanced",
        status="pending",
        attempts=0,
    )
    assert mission.max_attempts == 3
    assert mission.budget_functions == 1
    assert mission.budget_time_seconds == 1800
    assert mission.budget_depth == 1
    assert mission.created_at >= now
    assert mission.updated_at >= now


def test_evidence_ladder_constant() -> None:
    assert EVIDENCE_LADDER[0] == "draft"
    assert EVIDENCE_LADDER[-1] == "byte-exact"


def test_module_milestones_constant() -> None:
    assert MODULE_MILESTONES[0] == "mapped"
    assert MODULE_MILESTONES[-1] == "module-complete"


def test_infer_goal_no_source_returns_lift(tmp_path: Path) -> None:
    assert infer_goal(tmp_path, "emi_battle_battle_03", 0x801D0C00) == "lift"


def test_infer_goal_source_no_match_returns_improve(tmp_path: Path) -> None:
    source = tmp_path / "src" / "emi_battle_battle_03" / "func_801d0c00.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text('/* stub */\n', encoding="utf-8")
    assert infer_goal(tmp_path, "emi_battle_battle_03", 0x801D0C00) == "improve"


def test_infer_goal_match_not_100_returns_match(tmp_path: Path) -> None:
    source = tmp_path / "src" / "emi_battle_battle_03" / "func_801d0c00.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text('/* stub */\n', encoding="utf-8")

    summary = (
        tmp_path
        / "out"
        / "matching"
        / "emi_battle_battle_03"
        / "func_801d0c00"
        / "asm-differ"
        / "summary.json"
    )
    summary.parent.mkdir(parents=True, exist_ok=True)
    summary.write_text(
        json.dumps(
            {"instruction_count": {"match_percent": 85}, "bytes": {"match_percent": 70}}
        ),
        encoding="utf-8",
    )
    assert infer_goal(tmp_path, "emi_battle_battle_03", 0x801D0C00) == "match"


def test_infer_goal_byte_exact_returns_next(tmp_path: Path) -> None:
    source = tmp_path / "src" / "emi_battle_battle_03" / "func_801d0c00.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text('/* stub */\n', encoding="utf-8")

    summary = (
        tmp_path
        / "out"
        / "matching"
        / "emi_battle_battle_03"
        / "func_801d0c00"
        / "asm-differ"
        / "summary.json"
    )
    summary.parent.mkdir(parents=True, exist_ok=True)
    summary.write_text(
        json.dumps(
            {
                "instruction_count": {"match_percent": 100},
                "bytes": {"match_percent": 100},
            }
        ),
        encoding="utf-8",
    )
    assert (
        infer_goal(tmp_path, "emi_battle_battle_03", 0x801D0C00)
        == "select the next eligible function"
    )


def test_score_candidates_orders_by_score(tmp_path: Path) -> None:
    target = "emi_battle_battle_03"
    config = tmp_path / "config" / "splat" / f"{target}.yaml"
    config.parent.mkdir(parents=True, exist_ok=True)
    config.write_text(
        "segments:\n  - [0x100, c]\n  - [0x200, c]\n  - [0x300, c]\n  - [0x400, c]\n",
        encoding="utf-8",
    )

    # 0x100: not lifted (score 100)
    # 0x200: lifted, no match (score 50)
    # 0x300: matching, not exact (score 25)
    # 0x400: byte exact (score 0, skipped)
    for addr in (0x200, 0x300, 0x400):
        src = tmp_path / "src" / target / f"func_{addr:08x}.c"
        src.parent.mkdir(parents=True, exist_ok=True)
        src.write_text("/* stub */\n", encoding="utf-8")

    # match summary for 0x300 (not exact)
    s300 = (
        tmp_path
        / "out"
        / "matching"
        / target
        / "func_00000300"
        / "asm-differ"
        / "summary.json"
    )
    s300.parent.mkdir(parents=True, exist_ok=True)
    s300.write_text(
        json.dumps(
            {"instruction_count": {"match_percent": 90}, "bytes": {"match_percent": 80}}
        ),
        encoding="utf-8",
    )

    # match summary for 0x400 (exact)
    s400 = (
        tmp_path
        / "out"
        / "matching"
        / target
        / "func_00000400"
        / "asm-differ"
        / "summary.json"
    )
    s400.parent.mkdir(parents=True, exist_ok=True)
    s400.write_text(
        json.dumps(
            {
                "instruction_count": {"match_percent": 100},
                "bytes": {"match_percent": 100},
            }
        ),
        encoding="utf-8",
    )

    candidates = score_candidates(tmp_path, target)
    assert len(candidates) == 3
    assert candidates[0]["address"] == 0x100
    assert candidates[0]["score"] == 100
    assert candidates[1]["address"] == 0x200
    assert candidates[1]["score"] == 50
    assert candidates[2]["address"] == 0x300
    assert candidates[2]["score"] == 25


def test_score_candidates_empty_when_no_splat(tmp_path: Path) -> None:
    assert score_candidates(tmp_path, "unknown_target") == []


def test_score_candidates_uses_manifest_load_address(tmp_path: Path) -> None:
    manifest = tmp_path / "config" / "targets" / "slus.toml"
    manifest.parent.mkdir(parents=True, exist_ok=True)
    manifest.write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/slus_004_22"\n'
        'disc_id = "SLUS_004.22"\n'
        'kind = "executable"\n'
        'status = "active"\n'
        'source_dir = "src/exe/slus_004_22"\n'
        'binary = "out/slus.bin"\n'
        'splat = "config/splat/slus.yaml"\n'
        'load_address = 0x80096800\n'
        'profile = "native/test"\n',
        encoding="utf-8",
    )
    splat = tmp_path / "config" / "splat" / "slus.yaml"
    splat.parent.mkdir(parents=True, exist_ok=True)
    splat.write_text("segments:\n  - [0xb42c8, c, func_8014aac8]\n", encoding="utf-8")

    candidates = score_candidates(tmp_path, "exe/slus_004_22")
    assert candidates[0]["address"] == 0x8014AAC8


def test_select_next_function_returns_highest(tmp_path: Path) -> None:
    target = "emi_battle_battle_03"
    config = tmp_path / "config" / "splat" / f"{target}.yaml"
    config.parent.mkdir(parents=True, exist_ok=True)
    config.write_text(
        "segments:\n  - [0x100, c]\n  - [0x200, c]\n",
        encoding="utf-8",
    )

    # 0x100 not lifted, 0x200 lifted
    src = tmp_path / "src" / target / "func_00000200.c"
    src.parent.mkdir(parents=True, exist_ok=True)
    src.write_text("/* stub */\n", encoding="utf-8")

    result = select_next_function(tmp_path, target)
    assert result is not None
    assert result[0] == 0x100
    assert result[1] == "lift"


def test_select_next_function_none_when_all_exact(tmp_path: Path) -> None:
    target = "emi_battle_battle_03"
    config = tmp_path / "config" / "splat" / f"{target}.yaml"
    config.parent.mkdir(parents=True, exist_ok=True)
    config.write_text("segments:\n  - [0x100, c]\n", encoding="utf-8")

    src = tmp_path / "src" / target / "func_00000100.c"
    src.parent.mkdir(parents=True, exist_ok=True)
    src.write_text("/* stub */\n", encoding="utf-8")

    summary = (
        tmp_path
        / "out"
        / "matching"
        / target
        / "func_00000100"
        / "asm-differ"
        / "summary.json"
    )
    summary.parent.mkdir(parents=True, exist_ok=True)
    summary.write_text(
        json.dumps(
            {
                "instruction_count": {"match_percent": 100},
                "bytes": {"match_percent": 100},
            }
        ),
        encoding="utf-8",
    )

    assert select_next_function(tmp_path, target) is None


def test_plan_mission_with_none_address(tmp_path: Path) -> None:
    mission = plan_mission(tmp_path, "emi_field_field_01", None)
    assert mission.mission_id == "emi_field_field_01_none"
    assert mission.address is None
    path = save_mission(tmp_path, mission)
    expected = (
        tmp_path
        / "out"
        / "reverse"
        / "emi_field_field_01"
        / "functions"
        / "unknown"
        / "mission.json"
    )
    assert path == expected
    loaded = load_mission(tmp_path, mission.mission_id)
    assert loaded is not None
    assert loaded.address is None


def test_plan_mission_accepts_opts(tmp_path: Path) -> None:
    mission = plan_mission(
        tmp_path,
        "emi_battle_battle_03",
        0x801D0C00,
        strategy="leaf",
        max_attempts=5,
        budget_functions=3,
        budget_time_seconds=3600,
        budget_depth=2,
    )
    assert mission.strategy == "leaf"
    assert mission.max_attempts == 5
    assert mission.budget_functions == 3
    assert mission.budget_time_seconds == 3600
    assert mission.budget_depth == 2


def test_load_missing_mission_returns_none(tmp_path: Path) -> None:
    result = load_mission(tmp_path, "nonexistent_mission")
    assert result is None
