from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable, Protocol

from harness.reverse import MissionState, plan_mission, score_candidates


class Runner(Protocol):
    """Provider-independent runner interface."""

    def run(self, fn: Callable[..., Any], *args: Any, **kwargs: Any) -> Any:
        ...


class LocalRunner:
    """Calls Python functions directly in-process."""

    def run(self, fn: Callable[..., Any], *args: Any, **kwargs: Any) -> Any:
        return fn(*args, **kwargs)


def get_runner() -> Runner:
    """Return a runner based on ``HARNESS_AGENT_RUNNER``.

    Defaults to ``"subagent"`` and currently always returns a
    :class:`LocalRunner`.
    """
    runner_name = os.environ.get("HARNESS_AGENT_RUNNER", "subagent")
    if runner_name in ("local", "subagent"):
        return LocalRunner()
    return LocalRunner()


DiffFn = Callable[[Any], dict[str, Any]]


class Planner:
    """Validate mission, gather narrow context, choose tactics."""

    def __init__(self, runner: Runner | None = None) -> None:
        self.runner = runner or get_runner()

    def plan(self, root: Path, mission: dict[str, Any]) -> dict[str, Any]:
        target_id = mission.get("target_id")
        if not target_id:
            return {
                "validated": False,
                "context": {},
                "tactic": "",
                "estimated_time": 0,
                "blockers": ["missing target_id"],
            }

        context: dict[str, Any] = {
            "target_id": target_id,
            "address": mission.get("address"),
            "goal": mission.get("goal", "lift"),
        }

        from harness.snapshot import snapshot_path as get_snapshot_path

        snapshot_file = get_snapshot_path(root, target_id)
        context["snapshot_available"] = snapshot_file.is_file()

        from harness.domain import load_target_manifests, normalize_target_id

        splat_dir = root / "config" / "splat"
        target_splat = None
        splat_count = len(tuple(splat_dir.rglob("*.yaml"))) if splat_dir.is_dir() else 0
        try:
            normalized_target = normalize_target_id(target_id).value
        except ValueError:
            normalized_target = target_id
        manifest = load_target_manifests(root).get(normalized_target)
        if manifest is not None:
            target_splat = root / manifest.splat
        elif splat_dir.is_dir():
            for p in splat_dir.rglob("*.yaml"):
                if target_id in p.name:
                    target_splat = p
                    break
        context["splat_configs_count"] = splat_count
        context["target_splat"] = str(target_splat) if target_splat else None

        source_dir = (
            root / manifest.source_dir
            if manifest is not None
            else root / "src" / target_id
        )
        context["source_dir_exists"] = source_dir.is_dir()

        blockers: list[str] = []
        if not context["source_dir_exists"]:
            blockers.append(f"source directory not found: {source_dir}")
        if not context["target_splat"]:
            blockers.append("no splat config found for target")

        return {
            "validated": len(blockers) == 0,
            "context": context,
            "tactic": "decompile_and_match" if not blockers else "none",
            "estimated_time": 300 if not blockers else 0,
            "blockers": blockers,
        }


class Implementer:
    """Reverse, compile, diff, iterate, and prepare changes."""

    def __init__(
        self,
        runner: Runner | None = None,
        diff_fn: DiffFn | None = None,
    ) -> None:
        self.runner = runner or get_runner()
        self._diff_fn = diff_fn

    def _get_diff_fn(self) -> DiffFn | None:
        if self._diff_fn is not None:
            return self._diff_fn
        try:
            from harness.match.asm_diff import run_asm_diff_one
            return run_asm_diff_one
        except Exception:
            return None

    def execute(
        self,
        root: Path,
        mission: dict[str, Any],
        plan: dict[str, Any],
    ) -> dict[str, Any]:
        target_id = mission.get("target_id", "unknown")
        address = mission.get("address")

        print(f"[Implementer] Compiling candidate for {target_id}")

        if plan.get("blockers"):
            print("[Implementer] Blocked by plan blockers")
            return {
                "status": "blocked",
                "match_percent": 0.0,
                "exact_match": False,
            }

        from harness.domain import load_target_manifests, normalize_target_id

        try:
            normalized_target = normalize_target_id(target_id).value
        except ValueError:
            normalized_target = target_id
        manifest = load_target_manifests(root).get(normalized_target)
        source_dir = (
            root / manifest.source_dir
            if manifest is not None
            else root / "src" / target_id
        )
        if address is not None:
            source_path = source_dir / f"func_{address:08x}.c"
        else:
            source_path = source_dir / "unknown.c"

        print(f"[Implementer] Source path: {source_path}")

        diff_fn = self._get_diff_fn()
        if diff_fn is not None and source_path.is_file():
            from harness.match.asm_diff import AsmDiffRequest
            request = AsmDiffRequest(
                source_path=source_path,
                address=address,
                output_root=root / "out" / "matching",
            )
            print("[Implementer] Calling harness.match.asm_diff.run_asm_diff_one")
            try:
                diff_result = self.runner.run(diff_fn, request)
                exact = diff_result.get("exact_match", False)
                percent = (
                    diff_result.get("instruction_count", {}).get(
                        "match_percent", 0.0
                    )
                    or 0.0
                )
                return {
                    "status": "complete" if exact else "progress",
                    "match_percent": percent,
                    "exact_match": exact,
                }
            except Exception as exc:
                print(f"[Implementer] Diff call failed: {exc}")
                return {
                    "status": "blocked",
                    "match_percent": 0.0,
                    "exact_match": False,
                }

        if address is not None and not source_path.is_file():
            return {
                "status": "not_lifted",
                "match_percent": 0.0,
                "exact_match": False,
            }
        if address is None:
            return {
                "status": "invalid_target",
                "match_percent": 0.0,
                "exact_match": False,
            }
        return {
            "status": "no_diff_tool",
            "match_percent": 0.0,
            "exact_match": False,
        }


class Reviewer:
    """Independently check boundaries, claims, matching, and scope."""

    def __init__(self, runner: Runner | None = None) -> None:
        self.runner = runner or get_runner()

    def review(
        self,
        root: Path,
        mission: dict[str, Any],
        result: dict[str, Any],
    ) -> dict[str, Any]:
        target_id = mission.get("target_id", "unknown")
        findings: list[str] = []

        from harness.domain import load_target_manifests, normalize_target_id

        target_splat = None
        try:
            normalized_target = normalize_target_id(target_id).value
        except ValueError:
            normalized_target = target_id
        manifest = load_target_manifests(root).get(normalized_target)
        if manifest is not None:
            target_splat = root / manifest.splat

        if target_splat is None:
            findings.append("No splat config to validate boundaries against")

        if result.get("status") == "blocked":
            findings.append("Implementer reported blocked status")

        if not result.get("exact_match", False):
            if result.get("match_percent", 0.0) < 50.0:
                findings.append("Low match rate suggests possible ABI or type mismatch")

        approved = (
            result.get("exact_match", False) and result.get("status") != "blocked"
        )

        return {
            "approved": approved,
            "findings": findings,
        }


def create_decision_bundle(
    mission: dict[str, Any],
    plan: dict[str, Any],
    result: dict[str, Any],
    review: dict[str, Any],
) -> dict[str, Any]:
    """Assemble a decision bundle from mission artifacts."""
    human_required = not review.get("approved", False) or result.get(
        "status"
    ) == "blocked"
    return {
        "mission": mission,
        "plan": plan,
        "result": result,
        "review": review,
        "human_decision_required": human_required,
    }


def run_mission(
    root: Path,
    mission: dict[str, Any],
    *,
    max_cycles: int = 3,
    planner: Planner | None = None,
    implementer: Implementer | None = None,
    reviewer: Reviewer | None = None,
) -> dict[str, Any]:
    """Run the full agent orchestration loop for a mission.

    Steps:
    1. Planner validates mission and gathers context.
    2. Implementer compiles and diffs the candidate.
    3. Reviewer checks boundaries and claims.
    4. If reviewer rejects but progress was made, retry up to
       ``max_cycles``.
    """
    planner = planner or Planner()
    implementer = implementer or Implementer()
    reviewer = reviewer or Reviewer()

    plan = planner.plan(root, mission)
    if not plan["validated"]:
        result = {
            "status": "blocked",
            "match_percent": 0.0,
            "exact_match": False,
        }
        review = reviewer.review(root, mission, result)
        return create_decision_bundle(mission, plan, result, review)

    result: dict[str, Any] = {}
    review: dict[str, Any] = {}

    for cycle in range(1, max_cycles + 1):
        result = implementer.execute(root, mission, plan)
        review = reviewer.review(root, mission, result)

        if review["approved"]:
            break

        if result["status"] == "blocked":
            break

        if result["status"] == "complete":
            break

        if result["status"] == "progress" and cycle < max_cycles:
            print(
                f"[Orchestrator] Reviewer rejected, retrying "
                f"(cycle {cycle}/{max_cycles})"
            )
            continue

    return create_decision_bundle(mission, plan, result, review)


@dataclass(frozen=True)
class CampaignResult:
    missions_completed: int
    missions_blocked: int
    decision_bundles: list[dict[str, Any]]


@dataclass(frozen=True)
class ToolingRepairMission:
    tool: str
    error: str
    proposed_fix: str
    safe_to_apply: bool


def detect_tooling_blocker(result: dict[str, Any]) -> ToolingRepairMission | None:
    return None


def queue_followups(
    root: Path, mission: MissionState, result: dict[str, Any]
) -> list[MissionState]:
    from harness.snapshot import read_snapshot

    from harness.snapshot import snapshot_path as get_snapshot_path

    snapshot_file = get_snapshot_path(root, mission.target_id)
    if not snapshot_file.is_file():
        return []

    if mission.address is None:
        return []

    snapshot = read_snapshot(snapshot_file)

    target_func = None
    for f in snapshot.functions:
        if f.address == mission.address:
            target_func = f
            break

    if target_func is None:
        return []

    func_by_id = {f.id: f for f in snapshot.functions}

    caller_ids: set[str] = set()
    callee_ids: set[str] = set()
    for call in snapshot.calls:
        if call.callee == target_func.id:
            caller_ids.add(call.caller)
        if call.caller == target_func.id:
            callee_ids.add(call.callee)

    followups: list[MissionState] = []
    seen: set[int] = set()

    def try_enqueue(func_id: str) -> None:
        func = func_by_id.get(func_id)
        if func is None:
            return
        if func.is_reviewed and not func.is_lifted:
            if func.address not in seen:
                seen.add(func.address)
                followups.append(
                    plan_mission(
                        root,
                        mission.target_id,
                        func.address,
                        budget_depth=max(0, mission.budget_depth - 1),
                    )
                )

    for cid in caller_ids:
        try_enqueue(cid)

    if mission.budget_depth > 0:
        for cid in callee_ids:
            try_enqueue(cid)

    return followups


def run_campaign(
    root: Path,
    target_id: str,
    *,
    budget_functions: int = 5,
    budget_time: int = 3600,
    planner: Planner | None = None,
    implementer: Implementer | None = None,
    reviewer: Reviewer | None = None,
) -> CampaignResult:
    import time

    candidates = score_candidates(root, target_id)
    candidates.sort(key=lambda c: c.get("score", 0.0), reverse=True)

    completed = 0
    blocked = 0
    bundles: list[dict[str, Any]] = []
    start_time = time.time()

    for candidate in candidates:
        if completed + blocked >= budget_functions:
            break
        if time.time() - start_time >= budget_time:
            break

        address = candidate.get("address")
        mission = plan_mission(root, target_id, address)
        bundle = run_mission(
            root,
            mission.to_dict(),
            planner=planner,
            implementer=implementer,
            reviewer=reviewer,
        )
        result = bundle.get("result", {})
        review = bundle.get("review", {})
        if result.get("status") == "complete" and review.get("approved"):
            bundle["type"] = "completed"
            completed += 1
        else:
            bundle["type"] = "blocked"
            blocked += 1
        bundles.append(bundle)

    return CampaignResult(
        missions_completed=completed,
        missions_blocked=blocked,
        decision_bundles=bundles,
    )


__all__ = [
    "CampaignResult",
    "ToolingRepairMission",
    "detect_tooling_blocker",
    "queue_followups",
    "run_campaign",
]
