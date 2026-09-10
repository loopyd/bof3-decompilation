"""Audit retained lift proposals against pinned diagnosis without resetting its clock."""

from __future__ import annotations

import hashlib
import math
from pathlib import Path

from harness.common.deadlines import bind_deadline, check_deadline, validate_work_clock
from harness.common.digests import digest
from harness.common.directory import validate_repo_path
from harness.common.files import read_file
from harness.common.lease import acquire_writer, require_writer
from harness.common.paths import leaf_stat
from harness.context.journal import read_record, write_private, write_record
from harness.decomp.evidence import (
    capture_streams,
    hash_files,
    reserve_directory,
    retain_failure,
)
from harness.decomp.gates import audit_candidate
from harness.decomp.inventory import verify_scope
from harness.decomp.missions import inspect_candidate, validate_mission_record
from harness.decomp.writers import verify_writer_result


def validate_diagnosis(diagnosis: dict, expected_digest: str) -> float:
    if (
        diagnosis.get("digest") != expected_digest
        or digest({key: value for key, value in diagnosis.items() if key != "digest"})
        != expected_digest
        or diagnosis.get("schema") != "bof3.lift-diagnosis/v1"
        or diagnosis.get("status") != "diagnosed-not-accepted"
        or any(
            diagnosis.get(name) is not False
            for name in (
                "source_accepted",
                "source_write_authorized",
                "model_dispatched",
                "restoration_authorized",
                "retry_authorized",
            )
        )
        or type(diagnosis.get("byte_exact")) is not bool
        or type(diagnosis.get("match_percent")) not in (int, float)
        or not 0 <= diagnosis["match_percent"] <= 100
        or not math.isfinite(diagnosis["match_percent"])
        or not isinstance(diagnosis.get("post"), dict)
        or diagnosis["post"].get("changed_paths") != []
        or diagnosis.get("layout_gates") != []
        or any(
            not isinstance(diagnosis.get(name), list) or len(diagnosis[name]) != 2
            for name in ("gates", "cold_gates")
        )
    ):
        raise ValueError("audit diagnosis pin or disposition is invalid")
    return validate_work_clock(diagnosis.get("clock"))


def load_diagnosis(
    root: Path, directory: str, expected_digest: str
) -> tuple[dict, dict, dict]:
    validate_repo_path(directory)
    if (
        not directory.startswith("out/reviews/lift-diagnosis/")
        or len(Path(directory).parts) != 4
    ):
        raise ValueError("audit requires a retained lift diagnosis directory")
    if leaf_stat(root, directory + "/failure.json") is not None:
        raise ValueError("diagnosis failed; parent inspection required")
    diagnosis = read_record(root, directory + "/result.json")
    validate_diagnosis(diagnosis, expected_digest)
    names = ["mission.json", "policy.json", "invocation.json"]
    for label in ("cold-1", "cold-2", "gate-1", "gate-2"):
        names.extend(
            f"{label}-{suffix}"
            for suffix in (
                "started.json",
                "spawn.json",
                "terminal.json",
                "stdout.log",
                "stderr.log",
            )
        )
    if diagnosis.get("artifacts") != hash_files(root, directory, names):
        raise ValueError("diagnosis evidence coverage or retained bytes drifted")
    mission = read_record(root, directory + "/mission.json")
    validate_mission_record(mission, diagnosis.get("mission_digest"))
    invocation = read_record(root, directory + "/invocation.json")
    if invocation != {
        "request_digest": digest(mission["request"]),
        "work_deadline": diagnosis["clock"]["deadline"],
        "budget_debit_performed": False,
        "model_dispatched": False,
    }:
        raise ValueError(
            "diagnosis invocation differs from the retained request or clock"
        )
    wrapper = read_record(root, directory + "/policy.json")
    if (
        set(wrapper) != {"policy", "digest"}
        or wrapper["digest"] != diagnosis.get("policy_digest")
        or digest(wrapper["policy"]) != wrapper["digest"]
        or diagnosis.get("selector") != mission["request"]["selector"]
        or diagnosis.get("post", {}).get("changed_paths") != []
    ):
        raise ValueError("diagnosis mission, policy or PRE binding drifted")
    verify_scope(mission["inventory"], diagnosis["post"]["inventory"], [])
    return diagnosis, mission, wrapper["policy"]


def run_audit(
    root: Path,
    directory: str,
    proposal_path: str,
    *,
    expected_diagnosis_digest: str,
    expected_proposal_sha256: str,
    output: str,
    unmeasured: bool = False,
    expected_writer_digest: str | None = None,
) -> dict:
    diagnosis = read_record(root, validate_repo_path(directory) + "/result.json")
    deadline = validate_diagnosis(diagnosis, expected_diagnosis_digest)
    return execute_audit(
        root,
        directory,
        proposal_path,
        expected_diagnosis_digest=expected_diagnosis_digest,
        expected_proposal_sha256=expected_proposal_sha256,
        output=output,
        deadline=deadline,
        unmeasured=unmeasured,
        expected_writer_digest=expected_writer_digest,
    )


@bind_deadline
def execute_audit(
    root: Path,
    directory: str,
    proposal_path: str,
    *,
    expected_diagnosis_digest: str,
    expected_proposal_sha256: str,
    output: str,
    deadline: float,
    unmeasured: bool = False,
    expected_writer_digest: str | None = None,
) -> dict:
    reserved = False
    try:
        with acquire_writer(root):
            diagnosis, mission, policy = load_diagnosis(
                root, directory, expected_diagnosis_digest
            )
            writer = verify_writer_result(
                root, directory, diagnosis, mission, expected_writer_digest
            )
            if writer is not None:
                if (
                    not unmeasured
                    or expected_proposal_sha256 != writer["artifacts"]["proposal.md"]
                ):
                    raise ValueError(
                        "managed lift audit must use its original unmeasured proposal"
                    )
                verify_scope(
                    writer["post"]["inventory"],
                    inspect_candidate(root, mission)["inventory"],
                    [],
                )
            if deadline != validate_work_clock(diagnosis["clock"]):
                raise ValueError("audit cannot replace the original diagnosis cutoff")
            proposal = read_file(root, proposal_path)
            if (
                len(proposal) > 131072
                or hashlib.sha256(proposal).hexdigest() != expected_proposal_sha256
            ):
                raise ValueError(
                    "audit proposal exceeds 128 KiB or its external pin drifted"
                )
            text = proposal.decode("utf-8")
            reserve_directory(root, output, kind="audit")
            reserved = True
            write_private(root, output + "/proposal.md", proposal)
            with capture_streams(root, output) as (publish, stream_gate, artifacts):
                checked = audit_candidate(
                    root,
                    mission,
                    text,
                    expected_mission_digest=diagnosis["mission_digest"],
                    policy=policy,
                    expected_policy_digest=diagnosis["policy_digest"],
                    publish=publish,
                    stream_gate=stream_gate,
                    unmeasured=unmeasured,
                )
            load_diagnosis(root, directory, expected_diagnosis_digest)
            verify_writer_result(
                root, directory, diagnosis, mission, expected_writer_digest
            )
            if read_file(root, proposal_path) != proposal:
                raise ValueError("audit proposal changed during native checks")
            regression = checked["match_percent"] < diagnosis["match_percent"] or (
                diagnosis["byte_exact"] and not checked["byte_exact"]
            )
            result = {
                **{key: value for key, value in checked.items() if key != "digest"},
                "schema": "bof3.lift-audit/v2",
                "diagnosis": {
                    "directory": directory,
                    "digest": expected_diagnosis_digest,
                },
                "proposal_sha256": expected_proposal_sha256,
                "writer_digest": expected_writer_digest,
                "proposal_format": "unmeasured" if unmeasured else "mission",
                "clock": diagnosis["clock"],
                "comparison": {
                    "before_match_percent": diagnosis["match_percent"],
                    "after_match_percent": checked["match_percent"],
                    "regressed": regression,
                },
                "status": "needs-parent-restoration-review"
                if regression
                else checked["status"],
                "source_write_authorized": False,
                "model_dispatched": False,
                "budget_debit_performed": False,
                "artifacts": hash_files(root, output, ["proposal.md", *artifacts]),
            }
            result["digest"] = digest(result)
            check_deadline()
            require_writer(root)
            write_record(root, output + "/result.json", result)
            check_deadline()
            require_writer(root)
        return {
            "directory": output,
            "audit_digest": result["digest"],
            "status": result["status"],
            "byte_exact": result["byte_exact"],
            "match_percent": result["match_percent"],
            "regressed": regression,
            "source_accepted": False,
            "model_dispatched": False,
        }
    except BaseException as error:
        if reserved:
            retain_failure(root, output, error)
        raise
