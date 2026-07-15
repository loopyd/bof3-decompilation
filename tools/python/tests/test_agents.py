from __future__ import annotations

import time
from pathlib import Path
from typing import Any

from harness.agents import (
    CampaignResult,
    ToolingRepairMission,
    detect_tooling_blocker,
    get_runner,
    queue_followups,
    run_campaign,
)
from harness.reverse import plan_mission
from harness.snapshot import (
    SnapshotCall,
    SnapshotFunction,
    TargetSnapshot,
    write_snapshot,
)


# ---------------------------------------------------------------------------
# ToolingRepairMission
# ---------------------------------------------------------------------------


def test_get_runner_defaults_to_local(monkeypatch) -> None:
    monkeypatch.delenv("HARNESS_AGENT_RUNNER", raising=False)
    assert type(get_runner()).__name__ == "LocalRunner"


def test_get_runner_rejects_unknown_value(monkeypatch) -> None:
    monkeypatch.setenv("HARNESS_AGENT_RUNNER", "subagent")
    try:
        get_runner()
    except ValueError as exc:
        assert "unsupported" in str(exc)
    else:
        raise AssertionError("unknown runner was accepted")


def test_tooling_repair_mission_fields() -> None:
    mission = ToolingRepairMission(
        tool="compiler",
        error="not found",
        proposed_fix="check toolchain",
        safe_to_apply=False,
    )
    assert mission.tool == "compiler"
    assert mission.error == "not found"
    assert mission.proposed_fix == "check toolchain"
    assert mission.safe_to_apply is False


# ---------------------------------------------------------------------------
# detect_tooling_blocker
# ---------------------------------------------------------------------------


def test_detect_tooling_blocker_returns_none_stub() -> None:
    """The blocker detector is currently a stub that always returns None."""
    assert detect_tooling_blocker({"status": "compiler not found"}) is None
    assert detect_tooling_blocker({"status": "asm-diff failed"}) is None
    assert detect_tooling_blocker({"status": "splat config missing"}) is None


# ---------------------------------------------------------------------------
# queue_followups
# ---------------------------------------------------------------------------


def test_queue_followups_adds_callers(tmp_path: Path) -> None:
    """Callers and callees that are reviewed but unlifted are queued."""
    target_id = "emi_test_test_01"

    # Three functions: A calls B, B calls C
    func_a = SnapshotFunction(
        id="func_a",
        address=0x801D0000,
        analyzer_size=64,
        analyzer_name="fcn.0x801D0000",
        exact_sha256="a" * 64,
        is_reviewed=True,
        is_lifted=False,
    )
    func_b = SnapshotFunction(
        id="func_b",
        address=0x801D0100,
        analyzer_size=128,
        analyzer_name="fcn.0x801D0100",
        exact_sha256="b" * 64,
        is_reviewed=True,
        is_lifted=False,
    )
    func_c = SnapshotFunction(
        id="func_c",
        address=0x801D0200,
        analyzer_size=96,
        analyzer_name="fcn.0x801D0200",
        exact_sha256="c" * 64,
        is_reviewed=True,
        is_lifted=False,
    )

    snapshot = TargetSnapshot(
        schema="bof3.analysis-snapshot/v1",
        target=target_id,
        engine={"name": "rizin", "version": "0.7.0"},
        inputs={"binary": "deadbeef"},
        functions=(func_a, func_b, func_c),
        calls=(
            SnapshotCall(caller="func_a", callee="func_b", callsite=0x801D0004),
            SnapshotCall(caller="func_b", callee="func_c", callsite=0x801D0104),
        ),
        unresolved_calls=(),
    )

    snapshot_path = tmp_path / "out" / "reverse" / target_id / "snapshot.json"
    snapshot_path.parent.mkdir(parents=True, exist_ok=True)
    write_snapshot(snapshot, snapshot_path)

    mission = plan_mission(tmp_path, target_id, 0x801D0100, budget_depth=2)
    followups = queue_followups(tmp_path, mission, {"status": "success"})

    addresses = {f.address for f in followups}
    assert addresses == {0x801D0000, 0x801D0200}

    # All follow-ups belong to the same target
    for f in followups:
        assert f.target_id == target_id


def test_queue_followups_respects_depth_budget(tmp_path: Path) -> None:
    """Callees deeper than budget_depth are not queued."""
    target_id = "emi_test_test_01"

    func_a = SnapshotFunction(
        id="func_a",
        address=0x801D0000,
        analyzer_size=64,
        analyzer_name="fcn.0x801D0000",
        exact_sha256="a" * 64,
        is_reviewed=True,
        is_lifted=False,
    )
    func_b = SnapshotFunction(
        id="func_b",
        address=0x801D0100,
        analyzer_size=128,
        analyzer_name="fcn.0x801D0100",
        exact_sha256="b" * 64,
        is_reviewed=True,
        is_lifted=False,
    )
    func_c = SnapshotFunction(
        id="func_c",
        address=0x801D0200,
        analyzer_size=96,
        analyzer_name="fcn.0x801D0200",
        exact_sha256="c" * 64,
        is_reviewed=True,
        is_lifted=False,
    )

    snapshot = TargetSnapshot(
        schema="bof3.analysis-snapshot/v1",
        target=target_id,
        engine={"name": "rizin", "version": "0.7.0"},
        inputs={"binary": "deadbeef"},
        functions=(func_a, func_b, func_c),
        calls=(
            SnapshotCall(caller="func_a", callee="func_b", callsite=0x801D0004),
            SnapshotCall(caller="func_b", callee="func_c", callsite=0x801D0104),
        ),
        unresolved_calls=(),
    )

    snapshot_path = tmp_path / "out" / "reverse" / target_id / "snapshot.json"
    snapshot_path.parent.mkdir(parents=True, exist_ok=True)
    write_snapshot(snapshot, snapshot_path)

    # budget_depth == 0 means callees are not queued; callers still are
    mission = plan_mission(tmp_path, target_id, 0x801D0100, budget_depth=0)
    followups = queue_followups(tmp_path, mission, {"status": "success"})

    addresses = {f.address for f in followups}
    assert addresses == {0x801D0000}


def test_queue_followups_skips_lifted_functions(tmp_path: Path) -> None:
    """Already-lifted neighbours are not re-queued."""
    target_id = "emi_test_test_01"

    func_a = SnapshotFunction(
        id="func_a",
        address=0x801D0000,
        analyzer_size=64,
        analyzer_name="fcn.0x801D0000",
        exact_sha256="a" * 64,
        is_reviewed=True,
        is_lifted=True,
    )
    func_b = SnapshotFunction(
        id="func_b",
        address=0x801D0100,
        analyzer_size=128,
        analyzer_name="fcn.0x801D0100",
        exact_sha256="b" * 64,
        is_reviewed=True,
        is_lifted=False,
    )

    snapshot = TargetSnapshot(
        schema="bof3.analysis-snapshot/v1",
        target=target_id,
        engine={"name": "rizin", "version": "0.7.0"},
        inputs={"binary": "deadbeef"},
        functions=(func_a, func_b),
        calls=(SnapshotCall(caller="func_a", callee="func_b", callsite=0x801D0004),),
        unresolved_calls=(),
    )

    snapshot_path = tmp_path / "out" / "reverse" / target_id / "snapshot.json"
    snapshot_path.parent.mkdir(parents=True, exist_ok=True)
    write_snapshot(snapshot, snapshot_path)

    mission = plan_mission(tmp_path, target_id, 0x801D0100)
    followups = queue_followups(tmp_path, mission, {"status": "success"})

    # A is already lifted, so no follow-ups
    assert followups == []


def test_queue_followups_returns_empty_without_snapshot(tmp_path: Path) -> None:
    """When no snapshot exists, no follow-ups can be inferred."""
    mission = plan_mission(tmp_path, "emi_test_test_01", 0x801D0100)
    followups = queue_followups(tmp_path, mission, {"status": "success"})
    assert followups == []


# ---------------------------------------------------------------------------
# run_campaign
# ---------------------------------------------------------------------------


def test_campaign_respects_budget(monkeypatch, tmp_path: Path) -> None:
    """The campaign stops after processing ``budget_functions`` missions."""

    def _fake_score_candidates(root: Path, target_id: str) -> list[dict[str, Any]]:
        return [
            {"address": 0x801D0000, "score": 1.0},
            {"address": 0x801D0100, "score": 0.9},
            {"address": 0x801D0200, "score": 0.8},
        ]

    monkeypatch.setattr("harness.agents.score_candidates", _fake_score_candidates)

    result = run_campaign(
        tmp_path,
        "emi_test_test_01",
        budget_functions=2,
        budget_time=1800,
    )

    assert isinstance(result, CampaignResult)
    assert result.missions_completed == 0
    assert result.missions_blocked == 2
    assert len(result.decision_bundles) == 2
    assert all(b["type"] == "blocked" for b in result.decision_bundles)


def test_campaign_returns_empty_when_no_candidates(monkeypatch, tmp_path: Path) -> None:
    """When ``score_candidates`` is empty the campaign does nothing."""

    monkeypatch.setattr("harness.agents.score_candidates", lambda _r, _t: [])

    result = run_campaign(tmp_path, "emi_test_test_01")

    assert result.missions_completed == 0
    assert result.missions_blocked == 0
    assert result.decision_bundles == []


def test_campaign_respects_time_budget(monkeypatch, tmp_path: Path) -> None:
    """The campaign aborts when the time budget is exhausted."""

    def _fake_score_candidates(root: Path, target_id: str) -> list[dict[str, Any]]:
        return [{"address": 0x801D0000, "score": 1.0}]

    monkeypatch.setattr("harness.agents.score_candidates", _fake_score_candidates)

    start = time.time()
    result = run_campaign(
        tmp_path,
        "emi_test_test_01",
        budget_functions=10,
        budget_time=0,
    )
    elapsed = time.time() - start

    assert result.missions_completed == 0
    assert elapsed < 1.0
