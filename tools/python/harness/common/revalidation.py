"""Fresh native evidence for independently reviewed private results and exact deltas."""

from __future__ import annotations

import copy
import json
from pathlib import Path

from harness.common import execution as execution_context
from harness.common.process import run_command
from harness.common.digests import digest
from harness.common.deadlines import bind_deadline, check_deadline
from harness.common.evidence import evidence_output_path
from harness.common.files import atomic_write
from harness.common.paths import require_absent
from harness.common.lease import exclude_writers, verify_writer
from harness.common.submodules import protect_artifacts
from harness.common.runtime import run_checks, validate_receipts
from harness.common.workspace import workspace_baseline


def _prerequisite(
    root, envelope, expected, verify, owner, intervening, *, manifest_validator
):
    if intervening:
        from harness.common.transition import validate_transition

        validate_transition(
            root,
            envelope,
            expected,
            intervening,
            owner,
            manifest_validator=manifest_validator,
        )
    else:
        verify(root, envelope, expected)


def _manifest(
    root: Path,
    envelope: dict,
    expected: str,
    run_id: str,
    adopted: str,
    verify,
    owner: str,
    intervening: list,
    *,
    manifest_validator,
) -> dict:
    _prerequisite(
        root,
        envelope,
        expected,
        verify,
        owner,
        intervening,
        manifest_validator=manifest_validator,
    )
    baseline = workspace_baseline(root)
    if adopted != baseline["digest"]:
        raise ValueError("revalidation requires explicit current adopted baseline")
    return _retained_manifest(envelope, expected, run_id, baseline, intervening)


def _retained_manifest(envelope, expected, run_id, baseline, intervening):
    origins = envelope["parent_review"]
    if (
        not isinstance(run_id, str)
        or not run_id.strip()
        or run_id != run_id.strip()
        or run_id
        in {
            origins[key]
            for key in ("implementation_run_id", "reviewer_run_id", "parent_run_id")
        }
    ):
        raise ValueError("revalidation requires a new distinct execution run ID")
    if any(
        run_id
        in {
            pin["envelope"]["parent_review"][key]
            for key in ("implementation_run_id", "reviewer_run_id", "parent_run_id")
        }
        for pin in intervening
    ):
        raise ValueError("revalidation requires a new distinct execution run ID")
    manifest = copy.deepcopy(envelope["application"]["manifest"])
    manifest.update(
        pre_state=copy.deepcopy(envelope["application"]["post_state"]),
        pre_state_digest=envelope["application"]["post_state_digest"],
        workspace_baseline=baseline,
        request={
            "expected_envelope_digest": expected,
            "execution_run_id": run_id,
            "adopted_baseline": baseline["digest"],
        },
    )
    manifest["digest"] = digest({k: v for k, v in manifest.items() if k != "digest"})
    return manifest


def _context_record(record: dict) -> dict:
    return {
        "manifest": record["manifest"],
        "review_context": record["review_context"],
        "pre_state": record["manifest"]["pre_state"],
        "post_state": record["manifest"]["pre_state"],
        "changed_paths": [],
        "receipt_digests": [receipt["digest"] for receipt in record["receipts"]],
    }


@bind_deadline
@protect_artifacts
@exclude_writers
def revalidate(
    root: Path,
    envelope: object,
    expected: str,
    *,
    execution_run_id: str,
    adopted_baseline: str,
    owner: str,
    verify,
    manifest_validator,
    runner=run_command,
    output: str | None = None,
    intervening: list | None = None,
    deadline: float | None = None,
) -> dict:
    if intervening is None:
        intervening = []
    if not isinstance(intervening, list):
        raise ValueError("intervening private pins must be a list")
    if output is not None:
        output = evidence_output_path(root, output)
        require_absent(root, output)
    manifest = _manifest(
        root,
        envelope,
        expected,
        execution_run_id,
        adopted_baseline,
        verify,
        owner,
        intervening,
        manifest_validator=manifest_validator,
    )
    context = execution_context.begin(
        root,
        manifest,
        execution_run_id,
        envelope["application"]["review_context"]["initial_state"].get(
            "participating_targets"
        ),
    )
    execution_context.applied(root, manifest, context, {})
    receipts = []

    def unchanged() -> None:
        execution_context.recheck(root, manifest, context)
        if workspace_baseline(root) != manifest["workspace_baseline"]:
            raise ValueError("check-only revalidation mutated workspace")
        _prerequisite(
            root,
            envelope,
            expected,
            verify,
            owner,
            intervening,
            manifest_validator=manifest_validator,
        )

    for check in manifest["required_checks"]:
        unchanged()
        current, passed = run_checks(
            root,
            [check],
            manifest["pre_state_digest"],
            runner=runner,
            start_index=len(receipts),
            receipt_schema=f"bof3.{owner}-command-receipt/v1",
            evidence_prefix=f"{owner}-revalidation",
        )
        receipts.extend(current)
        execution_context.checked(root, manifest, context, check, current[0])
        # No mutation authority: unexpected gate writes require explicit recovery,
        # never overwrite them using a speculative rollback ownership assumption.
        unchanged()
        if not passed:
            raise RuntimeError("check-only revalidation native gate failed")
    facts = {
        "schema": f"bof3.{owner}-application-revalidation/v1",
        "prerequisite": copy.deepcopy(envelope),
        "expected_envelope_digest": expected,
        "manifest": manifest,
        "review_context": context,
        "receipts": receipts,
        "checked": True,
    }
    if intervening:
        facts["intervening"] = copy.deepcopy(intervening)
    record = {**facts, "digest": digest(facts)}
    verify_revalidation(
        root,
        record,
        record["digest"],
        owner=owner,
        verify=verify,
        manifest_validator=manifest_validator,
    )
    if output is not None:
        check_deadline()
        verify_writer(root)
        atomic_write(
            root,
            output,
            (json.dumps(record, indent=2, sort_keys=True) + "\n").encode(),
            exclusive=True,
        )
        verify_revalidation(
            root,
            record,
            record["digest"],
            owner=owner,
            verify=verify,
            manifest_validator=manifest_validator,
        )
    check_deadline()
    return record


def verify_revalidation(
    root: Path, value: object, expected: str, *, owner: str, verify, manifest_validator
) -> dict:
    return _verify_record(
        root,
        value,
        expected,
        owner=owner,
        verify=verify,
        manifest_validator=manifest_validator,
    )


def _verify_record(
    root, value, expected, *, owner, verify, manifest_validator, historical=False
):
    if (
        not isinstance(value, dict)
        or set(value) - {"intervening"}
        != {
            "schema",
            "prerequisite",
            "expected_envelope_digest",
            "manifest",
            "review_context",
            "receipts",
            "checked",
            "digest",
        }
        or value["schema"] != f"bof3.{owner}-application-revalidation/v1"
        or value["checked"] is not True
        or not isinstance(expected, str)
        or value["digest"] != expected
        or digest({k: v for k, v in value.items() if k != "digest"}) != expected
    ):
        raise ValueError("revalidation evidence is not trusted or drifted")
    intervening = value.get("intervening", [])
    if not isinstance(intervening, list) or (
        "intervening" in value and not intervening
    ):
        raise ValueError("invalid intervening private pins")
    context = value["review_context"]
    if not isinstance(context, dict) or not isinstance(
        context.get("adopted_baseline"), dict
    ):
        raise ValueError("invalid revalidation execution context")
    if historical:
        from harness.common.transition import validate_endpoint, validate_sequence

        applications = validate_sequence(
            root,
            value["prerequisite"],
            value["expected_envelope_digest"],
            intervening,
            owner,
            manifest_validator=manifest_validator,
        )
        validate_endpoint(
            applications,
            value,
            context["initial_state"],
            context["initial_build"],
            context["adopted_baseline"]["state"],
        )
        baseline = context["adopted_baseline"]
        if baseline != {
            "state": baseline["state"],
            "digest": digest(baseline["state"]),
            "adopted": bool(baseline["state"]),
        }:
            raise ValueError("historical revalidation baseline drifted")
        manifest = _retained_manifest(
            value["prerequisite"],
            value["expected_envelope_digest"],
            context.get("implementation_run_id"),
            baseline,
            intervening,
        )
    else:
        manifest = _manifest(
            root,
            value["prerequisite"],
            value["expected_envelope_digest"],
            context.get("implementation_run_id"),
            context["adopted_baseline"].get("digest"),
            verify,
            owner,
            intervening,
            manifest_validator=manifest_validator,
        )
    if context["initial_state"].get("participating_targets") != value["prerequisite"][
        "application"
    ]["review_context"]["initial_state"].get("participating_targets"):
        raise ValueError("revalidation participating targets drifted")
    if manifest != value["manifest"]:
        raise ValueError("revalidation manifest drifted")
    validate_receipts(
        root,
        value["receipts"],
        manifest["required_checks"],
        manifest["pre_state_digest"],
        receipt_schema=f"bof3.{owner}-command-receipt/v1",
    )
    if historical:
        execution_context._validate_context_history(_context_record(value))
        if context["initial_build"] != context["final_build"]:
            raise ValueError("historical check-only build drifted")
    else:
        execution_context.validate_context(root, _context_record(value))
    return {
        "schema": value["schema"],
        "digest": expected,
        "checked": True,
        "target": manifest["target"],
    }


def _review_binding(value: dict) -> dict:
    return {
        "revalidation_digest": value["digest"],
        "prerequisite_envelope_digest": value["expected_envelope_digest"],
        "manifest_digest": digest(value["manifest"]),
        "review_context_digest": digest(value["review_context"]),
        "pre_state_digest": value["manifest"]["pre_state_digest"],
        "native_receipts_digest": digest(value["receipts"]),
        "adopted_baseline_digest": digest(value["review_context"]["adopted_baseline"]),
    }


def review_revalidation(
    root: Path,
    value: object,
    parent: object,
    expected: str,
    *,
    owner: str,
    verify,
    manifest_validator,
) -> dict:
    verify_revalidation(
        root,
        value,
        expected,
        owner=owner,
        verify=verify,
        manifest_validator=manifest_validator,
    )
    _validate_revalidation_parent(value, parent, owner)
    facts = {
        "schema": f"bof3.{owner}-reviewed-revalidation/v1",
        "revalidation": copy.deepcopy(value),
        "parent_review": copy.deepcopy(parent),
    }
    return {**facts, "digest": digest(facts)}


def _validate_revalidation_parent(value, parent, owner):
    from harness.common.review import _validate_parent

    _validate_parent(
        parent,
        f"bof3.{owner}-revalidation-parent-review/v1",
        _review_binding(value),
        value["review_context"]["implementation_run_id"],
    )
    originals = [
        value["prerequisite"]["parent_review"],
        *(pin["envelope"]["parent_review"] for pin in value.get("intervening", [])),
    ]
    if any(
        parent["reviewer_run_id"]
        in {
            original[key]
            for key in ("parent_run_id", "implementation_run_id", "reviewer_run_id")
        }
        or parent["review_artifact"] == original["review_artifact"]
        for original in originals
    ):
        raise ValueError("revalidation requires fresh independent review")


def verify_reviewed_revalidation(
    root: Path, value: object, expected: str, *, owner: str, verify, manifest_validator
) -> dict:
    from harness.common.review import _keys

    _keys(value, "schema revalidation parent_review digest")
    if (
        value["schema"] != f"bof3.{owner}-reviewed-revalidation/v1"
        or not isinstance(expected, str)
        or value["digest"] != expected
        or digest({k: v for k, v in value.items() if k != "digest"}) != expected
    ):
        raise ValueError("reviewed revalidation envelope is not trusted or drifted")
    record = value["revalidation"]
    if not isinstance(record, dict):
        raise ValueError("invalid reviewed revalidation")
    review_revalidation(
        root,
        record,
        value["parent_review"],
        record.get("digest"),
        owner=owner,
        verify=verify,
        manifest_validator=manifest_validator,
    )
    return {
        "schema": value["schema"],
        "digest": expected,
        "accepted": True,
        "target": record["manifest"]["target"],
    }
