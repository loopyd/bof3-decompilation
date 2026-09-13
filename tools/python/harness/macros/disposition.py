"""Nonmutating macro inspection and independently reviewed existing dispositions."""

from __future__ import annotations

from copy import deepcopy
from pathlib import Path

from harness.common import execution
from harness.common.deadlines import bind_deadline, check_deadline
from harness.common.digests import digest
from harness.common.evidence import evidence_output_path, write_new_evidence_output
from harness.common.lease import exclude_writers
from harness.common.submodules import protect_artifacts
from harness.common.paths import require_absent
from harness.common.process import run_command
from harness.common.review import _binding, _keys, _validate_parent
from harness.common.runtime import (
    application_record,
    run_checks,
    validate_attestation,
    validate_receipts,
    write_attestation,
)
from harness.common.workspace import workspace_state
from harness.macros.assessment import validate_assessment

PREFIX = "macro-existing"
INSPECTION_SCHEMA = "bof3.macro-existing-inspection/v1"
RECEIPT_SCHEMA = "bof3.macro-existing-command-receipt/v1"
ATTESTATION_SCHEMA = "bof3.macro-existing-attestation/v1"
ENVELOPE_SCHEMA = "bof3.macro-existing-disposition/v1"


@bind_deadline
@protect_artifacts
@exclude_writers
def inspect_existing(
    root: Path,
    value: object,
    expected_manifest_digest: str,
    *,
    implementation_run_id: str,
    runner=run_command,
    output: str | None = None,
    deadline: float | None = None,
) -> dict:
    manifest = validate_assessment(root, value)
    if manifest["digest"] != expected_manifest_digest:
        raise ValueError("existing assessment manifest is not externally pinned")
    if not isinstance(implementation_run_id, str) or not implementation_run_id.strip():
        raise ValueError("existing assessment requires an actual execution identity")
    if implementation_run_id == manifest["reviewed_opportunity"]["review"]["reviewer"]:
        raise ValueError(
            "existing assessment requires an independent opportunity review"
        )
    if output is not None:
        output = evidence_output_path(root, output)
        require_absent(root, output)
    context = execution.begin(
        root, manifest, implementation_run_id, manifest["targets"]
    )
    execution.applied(root, manifest, context, {})
    receipts = []
    for check in manifest["required_checks"]:
        execution.recheck(root, manifest, context)
        current, passed = run_checks(
            root,
            [check],
            manifest["pre_state_digest"],
            runner=runner,
            start_index=len(receipts),
            receipt_schema=RECEIPT_SCHEMA,
            evidence_prefix=PREFIX,
        )
        receipts.extend(current)
        execution.checked(root, manifest, context, check, current[0])
        if context["initial_build"] != context["final_build"]:
            raise ValueError("existing assessment requires a prepared, unchanged build")
        _validate_workspace(root, manifest)
        if not passed:
            raise RuntimeError("existing macro native inspection failed")
    record = application_record(
        manifest["digest"],
        manifest["pre_state"],
        manifest["pre_state"],
        [],
        receipts,
        schema=INSPECTION_SCHEMA,
    )
    record.update(manifest=manifest, receipts=receipts, review_context=context)
    record["digest"] = digest(
        {key: item for key, item in record.items() if key != "digest"}
    )
    record["attestation"] = write_attestation(
        root, record, schema=ATTESTATION_SCHEMA, prefix=PREFIX
    )
    verify_inspection(root, record, record["digest"])
    if output is not None:
        check_deadline()
        write_new_evidence_output(root, output, record)
        execution.recheck(root, manifest, context)
        _validate_workspace(root, manifest)
    check_deadline()
    return record


def _validate_workspace(root: Path, manifest: dict) -> None:
    if workspace_state(root) != manifest["workspace_baseline"]["state"]:
        raise ValueError("existing macro inspection changed the adopted workspace")


def verify_inspection(root: Path, record: object, expected_digest: str) -> dict:
    _keys(
        record,
        "schema manifest_digest pre_state pre_state_digest post_state post_state_digest changed_paths changed_paths_digest receipt_digests digest manifest receipts review_context attestation",
    )
    facts = {
        key: item
        for key, item in record.items()
        if key not in {"digest", "attestation"}
    }
    if (
        record["schema"] != INSPECTION_SCHEMA
        or record["digest"] != expected_digest
        or digest(facts) != expected_digest
    ):
        raise ValueError("existing macro inspection is not trusted or drifted")
    validate_attestation(
        root, record, expected_digest, schema=ATTESTATION_SCHEMA, prefix=PREFIX
    )
    manifest = validate_assessment(root, record["manifest"])
    state = manifest["pre_state"]
    if (
        record["manifest_digest"] != manifest["digest"]
        or record["changed_paths"] != []
        or record["changed_paths_digest"] != digest([])
        or record["pre_state"] != state
        or record["post_state"] != state
        or record["pre_state_digest"] != digest(state)
        or record["post_state_digest"] != digest(state)
        or record["receipt_digests"]
        != [receipt["digest"] for receipt in record["receipts"]]
        or not isinstance(record["review_context"], dict)
    ):
        raise ValueError("existing macro inspection is not an unchanged complete scope")
    validate_receipts(
        root,
        record["receipts"],
        manifest["required_checks"],
        digest(state),
        receipt_schema=RECEIPT_SCHEMA,
    )
    execution.validate_context(root, record)
    context = record["review_context"]
    if context["initial_build"] != context["final_build"] or any(
        gate["build_before"] != context["initial_build"]
        or gate["build_after"] != context["initial_build"]
        for gate in context["gates"]
    ):
        raise ValueError("existing assessment build closure changed")
    _validate_workspace(root, manifest)
    return manifest


def review_existing(
    root: Path, record: object, parent: object, expected_digest: str
) -> dict:
    manifest = verify_inspection(root, record, expected_digest)
    _validate_parent(
        parent,
        "bof3.macro-existing-parent-review/v1",
        _binding(record),
        record["review_context"]["implementation_run_id"],
    )
    if manifest["reviewed_opportunity"]["review"]["reviewer"] in {
        parent["implementation_run_id"],
        parent["parent_run_id"],
    }:
        raise ValueError("existing macro review provenance is not independent")
    facts = {
        "schema": ENVELOPE_SCHEMA,
        "inspection": deepcopy(record),
        "parent_review": deepcopy(parent),
    }
    return {**facts, "digest": digest(facts)}


def verify_existing(root: Path, value: object, expected_digest: str) -> dict:
    _keys(value, "schema inspection parent_review digest")
    if value["schema"] != ENVELOPE_SCHEMA or value["digest"] != expected_digest:
        raise ValueError("existing macro disposition is not externally pinned")
    current = review_existing(
        root, value["inspection"], value["parent_review"], value["inspection"]["digest"]
    )
    if current != value:
        raise ValueError("existing macro disposition drifted")
    manifest = value["inspection"]["manifest"]
    return {
        "schema": ENVELOPE_SCHEMA,
        "digest": expected_digest,
        "accepted": True,
        "disposition": "existing_abstraction",
        "existing_abstraction_count": 1,
        "safe_application_count": 0,
        "targets": manifest["targets"],
        "candidate_id": manifest["reviewed_opportunity"]["candidate_id"],
        "candidate_fingerprint": manifest["reviewed_opportunity"][
            "candidate_fingerprint"
        ],
    }


def account_existing(root: Path, references: object) -> dict:
    if not isinstance(references, list):
        raise ValueError(
            "existing macro accounting requires explicit pinned dispositions"
        )
    from harness.common.inputs import load
    from harness.macros.review import repo_path

    rows = []
    for reference in references:
        _keys(reference, "path expected_envelope_digest")
        name = repo_path(root, reference["path"])
        rows.append(
            verify_existing(
                root, load(root / name), reference["expected_envelope_digest"]
            )
        )
    if len({row["candidate_id"] for row in rows}) != len(rows):
        raise ValueError("existing macro dispositions duplicate a candidate")
    return {
        "schema": "bof3.macro-existing-account/v1",
        "existing_abstraction_count": len(rows),
        "safe_application_count": 0,
        "rows": sorted(rows, key=lambda row: row["candidate_id"]),
    }
