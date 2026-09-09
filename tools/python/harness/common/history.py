"""Internal retained private proof integrity, never current-state or sharing authority."""

from __future__ import annotations

from pathlib import Path

from harness.common import execution as execution_context
from harness.common.digests import digest
from harness.common.inputs import file_state
from harness.common.runtime import validate_attestation, validate_receipts


def validate_application_history(
    root: Path, value: object, expected: str, owner: str, manifest_validator
) -> dict:
    validate_attestation(
        root,
        value,
        expected,
        schema=f"bof3.{owner}-application-attestation/v1",
        prefix=owner,
    )
    if value.get("schema") != f"bof3.{owner}-application/v1" or not value.get(
        "applied"
    ):
        raise ValueError(f"{owner} application proof is invalid")
    facts = {k: v for k, v in value.items() if k not in {"attestation", "digest"}}
    if value.get("digest") != digest(facts):
        raise ValueError(f"{owner} application proof drifted")
    manifest = manifest_validator(root, value.get("manifest"))
    if (
        value["manifest_digest"] != manifest["digest"]
        or value["pre_state"] != manifest["pre_state"]
    ):
        raise ValueError(f"{owner} application proof does not match manifest")
    validate_receipts(
        root,
        value.get("receipts"),
        manifest["required_checks"],
        value["post_state_digest"],
        receipt_schema=f"bof3.{owner}-command-receipt/v1",
    )
    if value.get("changed_paths_digest") != digest(value.get("changed_paths")) or value[
        "receipt_digests"
    ] != [r["digest"] for r in value["receipts"]]:
        raise ValueError(f"{owner} application receipt or changed-path set drifted")
    if "review_context" in value:
        execution_context._validate_context_history(value)
    return manifest


def validate_reviewed_history(
    root: Path, value: object, expected: str, owner: str, manifest_validator
) -> None:
    """Check externally pinned original acceptance without accepting subsequent drift."""
    from harness.common.review import _binding, _keys, _validate_parent

    _keys(value, "schema application parent_review digest")
    if (
        value["schema"] != f"bof3.{owner}-reviewed-application/v1"
        or not isinstance(expected, str)
        or value["digest"] != expected
        or digest({k: v for k, v in value.items() if k != "digest"}) != expected
    ):
        raise ValueError("reviewed application envelope is not trusted or drifted")
    application = value["application"]
    if application.get("concern") in {"shared", "shared_template"}:
        raise ValueError("shared reviewed transition is not supported")
    _validate_parent(
        value["parent_review"],
        f"bof3.{owner}-parent-review/v1",
        _binding(application),
        application["review_context"]["implementation_run_id"],
    )
    manifest = validate_application_history(
        root, application, application["digest"], owner, manifest_validator
    )
    # Retained provenance remains live evidence, even when authoritative source
    # inputs are historical. Never reconstruct the old closure from today's tree.
    context = application["review_context"]
    for name in execution_context._evidence_paths(manifest):
        if file_state(root / name) != context["final_state"]["inputs"][name]:
            raise ValueError("retained historical prerequisite replaced")
