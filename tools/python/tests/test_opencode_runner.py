from __future__ import annotations

import json
from pathlib import Path
from subprocess import CompletedProcess

from harness.opencode_runner import (
    RESULT_SCHEMA,
    _extract_result,
    _opencode_command,
    _validate_result,
    run_opencode_mission,
)
from harness.reverse import plan_mission


def _mission() -> object:
    return plan_mission(
        Path("."), "exe/slus_004_22", 0x80096800, budget_time_seconds=60
    )


def _result(mission_id: str) -> dict[str, object]:
    return {
        "schema": RESULT_SCHEMA,
        "mission_id": mission_id,
        "target": "exe/slus_004_22",
        "address": 0x80096800,
        "status": "complete",
        "summary": "matched",
        "changed_paths": ["src/exe/slus_004_22/func_80096800.c"],
        "checks": [],
        "exact_match": True,
        "instruction_match_percent": 100,
        "byte_match_percent": 100,
        "blockers": [],
    }


def _target_root(tmp_path: Path) -> None:
    (tmp_path / "config" / "targets" / "exe").mkdir(parents=True)
    (tmp_path / "config" / "splat").mkdir(parents=True)
    (tmp_path / "src" / "exe" / "slus_004_22").mkdir(parents=True)
    (tmp_path / "out").mkdir()
    (tmp_path / "out" / "slus.bin").write_bytes(b"test")
    (tmp_path / "config" / "splat" / "slus.yaml").write_text("segments: []\n")
    (tmp_path / "config" / "targets" / "exe" / "slus_004_22.toml").write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/slus_004_22"\n'
        'disc_id = "SLUS_004.22"\n'
        'kind = "executable"\n'
        'status = "active"\n'
        'source_dir = "src/exe/slus_004_22"\n'
        'binary = "out/slus.bin"\n'
        'splat = "config/splat/slus.yaml"\n'
        'load_address = 0x80096800\n'
        'profile = "native/test"\n'
    )


def test_extract_result_from_json_event_text() -> None:
    result = _result("exe/slus_004_22_0x80096800")
    event = json.dumps({"type": "text", "text": json.dumps(result)})
    assert _extract_result(event) == result


def test_validate_result_rejects_protected_path(tmp_path: Path) -> None:
    mission = _mission()
    result = _result(mission.mission_id)
    result["changed_paths"] = ["inputs/disc/game.bin"]
    try:
        _validate_result(tmp_path, mission, result)
    except ValueError as exc:
        assert "protected" in str(exc)
    else:
        raise AssertionError("protected path was accepted")


def test_opencode_command_has_no_auto_approval(monkeypatch, tmp_path: Path) -> None:
    mission = _mission()
    monkeypatch.setenv("HARNESS_OPENCODE_BIN", "/usr/bin/opencode")
    command = _opencode_command(tmp_path, mission, "prompt")
    assert command[:7] == ["/usr/bin/opencode", "run", "--format", "json", "--dir", str(tmp_path), "--agent"]
    assert "--auto" not in command
    assert "--continue" not in command


def test_run_opencode_mission_writes_result(monkeypatch, tmp_path: Path) -> None:
    _target_root(tmp_path)
    mission = plan_mission(tmp_path, "exe/slus_004_22", 0x80096800, budget_time_seconds=60)
    result = _result(mission.mission_id)
    monkeypatch.setenv("HARNESS_OPENCODE_BIN", "/usr/bin/opencode")
    monkeypatch.setattr(
        "harness.opencode_runner.subprocess.run",
        lambda *args, **kwargs: CompletedProcess(args[0], 0, json.dumps(result), ""),
    )

    completed = run_opencode_mission(tmp_path, mission)

    assert completed.exit_code == 0
    assert (completed.artifact_dir / "mission.json").is_file()
    assert json.loads((completed.artifact_dir / "result.json").read_text()) == result
