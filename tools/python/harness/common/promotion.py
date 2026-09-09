"""Frozen common PRE binding for two freshly reviewed private owner results."""

from __future__ import annotations

from pathlib import Path

from harness.common import execution as context
from harness.common.transition import _state_equal


def private_application(
    root: Path, envelope: dict, expected: str, owner: str, verify, *, manifest_validator
):
    if envelope.get("schema") == f"bof3.{owner}-reviewed-revalidation/v1":
        from harness.common.revalidation import verify_reviewed_revalidation

        verified = verify_reviewed_revalidation(
            root,
            envelope,
            expected,
            owner=owner,
            verify=verify,
            manifest_validator=manifest_validator,
        )
        return envelope["revalidation"]["prerequisite"]["application"], verified
    verified = verify(root, envelope, expected)
    return envelope["application"], verified


def freeze(root: Path, manifest: dict, proofs: list[dict]) -> dict:
    fresh = [proof.get("reviewed_envelope", {}).get("revalidation") for proof in proofs]
    if not any(record is not None for record in fresh):
        return {}  # Existing preparation-only route retains its acceptance guard.
    if (
        len(fresh) != 2
        or any(record is None for record in fresh)
        or len(set(manifest["targets"])) != 2
        or {proof["target"] for proof in proofs} != set(manifest["targets"])
    ):
        raise ValueError("shared PRE requires exactly two fresh private revalidations")
    pre = {
        "state": context.capture(root, manifest, manifest["targets"]),
        "build": context.build_state(root),
        "workspace_baseline": manifest["workspace_baseline"],
    }
    _validate_common(manifest, fresh, pre)
    return {"shared_pre": pre}


def _validate_common(manifest, fresh, pre):
    if (
        len(fresh) != 2
        or {r["manifest"]["target"] for r in fresh} != set(manifest["targets"])
        or len(set(manifest["targets"])) != 2
    ):
        raise ValueError("shared PRE requires two distinct private targets")
    states = [record["review_context"] for record in fresh]
    if any(
        state["initial_state"].get("participating_targets") != manifest["targets"]
        for state in states
    ):
        raise ValueError("shared PRE requires identical participating targets")
    if len({state["implementation_run_id"] for state in states}) != 2:
        raise ValueError("shared PRE requires distinct fresh check executions")
    for record, state in zip(fresh, states):
        _state_equal(record, state["final_state"], {"manifest": manifest}, pre["state"])
        if (
            state["initial_build"] != pre["build"]
            or state["final_build"] != pre["build"]
            or state["adopted_baseline"] != pre["workspace_baseline"]
        ):
            raise ValueError("shared PRE build or workspace mismatch")


def validate_entry(manifest: dict, execution) -> None:
    if "shared_pre" not in manifest:
        return
    pre = manifest["shared_pre"]
    if (
        execution is None
        or execution["initial_state"] != pre["state"]
        or execution["initial_build"] != pre["build"]
        or execution["adopted_baseline"] != pre["workspace_baseline"]
    ):
        raise ValueError("shared PRE execution binding mismatch")


def validate_post(
    root: Path, application: dict, owner: str, parent=None, *, manifest_validator
) -> None:
    """Replay retained private checks at frozen PRE, never against shared POST."""
    from harness.common.digests import digest
    from harness.common.inputs import file_state, load
    from harness.common.revalidation import (
        _validate_revalidation_parent,
        _verify_record,
    )
    from harness.common.review import _keys

    manifest = application["manifest"]
    if "shared_pre" not in manifest:
        raise ValueError("shared reviewed transition is not supported")
    proofs = manifest[
        "private_transaction_proofs" if owner == "type" else "exact_function_proofs"
    ]
    validate_entry(manifest, application.get("review_context"))
    records = []
    origins = []
    for proof in proofs:
        envelope = proof["reviewed_envelope"]
        _keys(envelope, "schema revalidation parent_review digest")
        if (
            envelope["schema"] != f"bof3.{owner}-reviewed-revalidation/v1"
            or envelope["digest"] != proof["expected_envelope_digest"]
            or digest({k: v for k, v in envelope.items() if k != "digest"})
            != envelope["digest"]
            or load(root / proof["path"]) != envelope
        ):
            raise ValueError("shared historical envelope drifted")
        record = envelope["revalidation"]
        _verify_record(
            root,
            record,
            record["digest"],
            owner=owner,
            verify=None,
            historical=True,
            manifest_validator=manifest_validator,
        )
        _validate_revalidation_parent(record, envelope["parent_review"], owner)
        if (
            proof["application"] != record["prerequisite"]["application"]
            or proof["target"] != record["manifest"]["target"]
        ):
            raise ValueError("shared private application binding drifted")
        records.append(record)
        origins.extend(
            [
                envelope["parent_review"],
                record["prerequisite"]["parent_review"],
                *(
                    pin["envelope"]["parent_review"]
                    for pin in record.get("intervening", [])
                ),
            ]
        )
    pre = manifest["shared_pre"]
    _validate_common(manifest, records, pre)
    # Every retained manifest artifact remains required, including nested receipts.
    for name in context._evidence_paths(manifest):
        if file_state(root / name) != pre["state"]["inputs"][name]:
            raise ValueError("shared historical evidence drifted")
    if parent is not None:
        execution = application["review_context"]["implementation_run_id"]
        if any(
            execution
            in {
                origin[k]
                for k in ("implementation_run_id", "reviewer_run_id", "parent_run_id")
            }
            or parent["reviewer_run_id"]
            in {
                origin[k]
                for k in ("implementation_run_id", "reviewer_run_id", "parent_run_id")
            }
            or parent["review_artifact"] == origin["review_artifact"]
            for origin in origins
        ):
            raise ValueError(
                "shared transition requires fresh execution and independent review"
            )
