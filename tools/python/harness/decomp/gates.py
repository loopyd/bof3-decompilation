"""Parent-owned native evidence for a retained, not-yet-accepted lift candidate."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Callable

from harness.build.operations import cmake_target_for_source
from harness.common.checks import check_evidence, validate_partial_evidence
from harness.common.deadlines import check_deadline, resolve_deadline
from harness.common.digests import digest
from harness.common.lease import require_writer
from harness.common.process import run_bounded
from harness.context.capabilities import verify_policy
from harness.domain.ids import parse_function_id
from harness.domain.sources import reviewed_function_name
from harness.io import unique_object

from .inventory import verify_scope
from .execution import build_gate_command
from .missions import inspect_candidate, parse_result, validate_mission_record


def audit_candidate(
    root: Path,
    record: dict,
    proposal: str,
    *,
    expected_mission_digest: str,
    policy: dict,
    expected_policy_digest: str,
    publish: Callable[[str, dict], None],
    stream_gate: Callable[[str, str, bytes], None],
) -> dict:
    require_writer(root)
    validate_mission_record(record, expected_mission_digest)
    report = parse_result(proposal, record["request"]["selector"])
    if (
        report["acceptance"]["pre_mission"]
        != {
            "mission_digest": expected_mission_digest,
            "baseline_digest": record["request"]["adopted_baseline"],
        }
        or report["acceptance"]["retained_candidate"] != record["request"]["source"]
    ):
        raise ValueError(
            "lift acceptance pre-mission or retained-source binding drifted"
        )
    checks = check_candidate(
        root,
        record,
        expected_mission_digest=expected_mission_digest,
        policy=policy,
        expected_policy_digest=expected_policy_digest,
        publish=publish,
        stream_gate=stream_gate,
    )
    after = checks["post"]
    if report["mission"]["match_percent"] != checks["match_percent"]:
        raise ValueError("lift match percentage differs from parent native evidence")
    if report["mission"]["status"] == "exact" and not checks["byte_exact"]:
        raise ValueError("lift byte-match claim differs from parent native gates")
    if sorted(report["mission"]["files_changed"]) != after["changed_paths"]:
        raise ValueError("lift changed-file claim differs from retained candidate")
    if (
        after["changed_paths"]
        and not report["acceptance"]["snapshot_index_refresh_required"]
    ):
        raise ValueError(
            "lift changed indexed inputs without requesting parent refresh"
        )
    if report["acceptance"]["staged_index_changed"]:
        raise ValueError(
            "lift report claims staged-index mutation; parent inspection required"
        )
    check_deadline()
    require_writer(root)
    result = {
        **{key: value for key, value in checks.items() if key != "digest"},
        "schema": "bof3.lift-audit/v1",
        "report": report,
        "status": "needs-parent-restoration-review"
        if report["acceptance"]["parent_restore_required"]
        else "needs-independent-review",
    }
    return {**result, "digest": digest(result)}


def check_candidate(
    root: Path,
    record: dict,
    *,
    expected_mission_digest: str,
    policy: dict,
    expected_policy_digest: str,
    publish: Callable[[str, dict], None],
    stream_gate: Callable[[str, str, bytes], None],
) -> dict:
    require_writer(root)
    validate_mission_record(record, expected_mission_digest)
    verify_policy(root, policy, expected_policy_digest, before_dispatch=False)
    if policy["paths"] != record["request"]["paths"]:
        raise ValueError("lift native policy differs from the pinned source scope")
    check_deadline()
    deadline = resolve_deadline()
    if deadline is None:
        raise ValueError("lift gates require the original work cutoff")
    selector = record["request"]["selector"]
    identity = parse_function_id(selector)
    target, address = identity.target.value, identity.address
    gate_selector = f"{target}@0x{address:08X}"
    function = reviewed_function_name(root, target, address)
    before = inspect_candidate(root, record)

    def run_gate(label: str, command: list[str]) -> dict:
        require_writer(root)
        check_deadline()
        verify_scope(
            before["inventory"], inspect_candidate(root, record)["inventory"], []
        )
        wrapped = build_gate_command(
            root,
            policy,
            expected_policy_digest,
            record["execution"]["tools"]["bwrap"],
            command,
        )
        publish(label + "-started", {"command": command, "sandbox": wrapped})
        gate = run_bounded(
            root,
            wrapped,
            timeout=120,
            deadline=deadline,
            output_limit=1024 * 1024,
            on_output=lambda stream, chunk: stream_gate(label, stream, chunk),
            on_spawn=lambda process: publish(
                label + "-spawn",
                {"supervisor_pid": process.pid, "termination_authority": False},
            ),
        )
        publish(label + "-terminal", gate)
        require_writer(root)
        verify_policy(root, policy, expected_policy_digest, before_dispatch=False)
        verify_scope(
            before["inventory"], inspect_candidate(root, record)["inventory"], []
        )
        return gate

    metadata_changed = bool(
        set(before["changed_paths"])
        & {
            record["facts"]["manifest"]["splat"],
            f"config/targets/{target}/symbols.txt",
        }
    )
    layout_gates = []
    if metadata_changed:
        for position, command in enumerate(
            (
                ["bin/symbols", "check", target],
                ["bin/splat", target],
            ),
            1,
        ):
            gate = run_gate(f"layout-{position}", command)
            layout_gates.append(gate)
            if gate["failure"] or gate["exit_code"] != 0:
                raise ValueError(
                    "lift layout gate failed; retain candidate for inspection"
                )
    tools = record["execution"]["tools"]
    build_tree = str(root / "build/cmake")
    source = record["request"]["source"]
    cold_gates = []
    for position, command in enumerate(
        (
            [
                tools["cmake"]["path"],
                "--fresh",
                "-S",
                str(root),
                "-B",
                build_tree,
                "-G",
                "Ninja",
            ],
            [
                tools["ninja"]["path"],
                "-C",
                build_tree,
                "-t",
                "clean",
                cmake_target_for_source(root, root / source),
            ],
        ),
        1,
    ):
        gate = run_gate(f"cold-{position}", command)
        cold_gates.append(gate)
        if gate["failure"] or gate["exit_code"] != 0:
            raise ValueError("lift cold build preparation failed; retain candidate")
    object_path = root / "build" / Path(source).with_suffix(".o")
    if object_path.exists() or object_path.is_symlink():
        raise ValueError("lift cold build retained a prior object")
    gates = []
    for position, command in enumerate(
        (
            ["bin/asm-diff", gate_selector, "--json", "--detail", "full"],
            ["bin/byte-match", gate_selector, "--json"],
        ),
        1,
    ):
        gate = run_gate(f"gate-{position}", command)
        gates.append(gate)
        if gate["failure"]:
            raise ValueError("lift native gate failed; retain candidate for inspection")
        check = {
            "argv": command,
            "target": target,
            "selector": gate_selector,
            "function": function,
            "source": record["request"]["source"],
        }
        if gate["exit_code"] == 1:
            validate_partial_evidence(
                root,
                tool=command[0].removeprefix("bin/"),
                target=target,
                selector=gate_selector,
                function=function,
                source=record["request"]["source"],
                exit_code=gate["exit_code"],
                output=gate["stdout"],
            )
        elif not check_evidence(check, gate["exit_code"], gate["stdout"], root)[
            "passed"
        ]:
            raise ValueError("lift native gate evidence is invalid")
        payload = json.loads(gate["stdout"], object_pairs_hook=unique_object)
        original_path = Path(payload["original_binary"])
        expected_path = Path(record["facts"]["original"]["path"])
        if original_path != (
            root / expected_path if original_path.is_absolute() else expected_path
        ):
            raise ValueError("lift native gate changed the original binary path")
        if payload["original_size"] != record["facts"]["original"]["size"]:
            raise ValueError("lift native gate changed the pinned original span")
        gate["payload"] = payload
    after = inspect_candidate(root, record)
    verify_scope(before["inventory"], after["inventory"], [])
    outcomes = {gate["payload"]["byte_match"] for gate in gates}
    if len(outcomes) != 1:
        raise ValueError("lift native byte gates disagree")
    check_deadline()
    require_writer(root)
    result = {
        "schema": "bof3.lift-check/v1",
        "mission_digest": record["digest"],
        "selector": selector,
        "gates": gates,
        "layout_gates": layout_gates,
        "cold_gates": cold_gates,
        "policy_digest": expected_policy_digest,
        "post": after,
        "status": "measured-not-accepted",
        "byte_exact": outcomes == {True},
        "match_percent": gates[0]["payload"]["instruction_count"]["match_percent"],
        "source_accepted": False,
        "restoration_authorized": False,
        "retry_authorized": False,
    }
    return {**result, "digest": digest(result)}
