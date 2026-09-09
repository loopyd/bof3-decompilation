"""Owner-neutral parent acceptance and externally pinned application replay."""

from __future__ import annotations

import copy
from pathlib import Path

from harness.common.digests import digest
from harness.common.inputs import file_state, load
from harness.common.workspace import workspace_state

PRESERVATION = dict.fromkeys(
    ("scope", "body", "abi", "range", "index", "adopted_baseline"), True
)


def _keys(value, names: str) -> None:
    if not isinstance(value, dict) or set(value) != set(names.split()):
        raise ValueError("invalid parent review keys")


def _binding(application: dict) -> dict:
    if not isinstance(application, dict):
        raise ValueError("invalid reviewed application")
    context = application.get("review_context")
    if not isinstance(context, dict):
        raise ValueError("independent review requires captured implementation context")
    return {
        "application_digest": application["digest"],
        "application_proof_digest": digest(application),
        "manifest_digest": application["manifest_digest"],
        "request_digest": digest(application["manifest"]["request"]),
        "review_context_digest": digest(context),
        "post_state_digest": application["post_state_digest"],
        "native_receipts_digest": digest(application["receipts"]),
        "adopted_baseline_digest": digest(context["adopted_baseline"]),
    }


def _validate_parent(
    parent: object, schema: str, binding: dict, implementation_run_id: str
) -> None:
    _keys(
        parent,
        "schema accepted parent_run_id implementation_run_id reviewer_run_id review_artifact binding preservation",
    )
    if parent["schema"] != schema or parent["accepted"] is not True:
        raise ValueError("explicit accepted owner parent review required")
    origins = [
        parent[key]
        for key in ("parent_run_id", "implementation_run_id", "reviewer_run_id")
    ]
    if (
        any(not isinstance(v, str) or not v.strip() or v != v.strip() for v in origins)
        or len(set(origins)) != 3
    ):
        raise ValueError("parent review requires distinct actual run identities")
    _keys(parent["binding"], " ".join(binding))
    if parent["binding"] != binding or origins[1] != implementation_run_id:
        raise ValueError("parent review application binding mismatch")
    _keys(parent["preservation"], " ".join(PRESERVATION))
    if any(value is not True for value in parent["preservation"].values()):
        raise ValueError("parent review requires baseline preservation")
    ref = parent["review_artifact"]
    _keys(ref, "path sha256")
    if not isinstance(ref["path"], str):
        raise ValueError("review artifact path must be canonical absolute")
    path = Path(ref["path"])
    if not path.is_absolute() or str(path.resolve()) != ref["path"]:
        raise ValueError("review artifact path must be canonical absolute")
    state = file_state(path)
    if (
        state is None
        or state["sha256"] != ref["sha256"]
        or not path.read_bytes().strip()
    ):
        raise ValueError("retained accepted reviewer artifact replaced or empty")


def _validate(
    root: Path, application: dict, parent, owner: str, verify, *, manifest_validator
) -> None:
    if (
        isinstance(application, dict)
        and application.get("concern") in {"shared", "shared_template"}
        and "shared_pre" not in application.get("manifest", {})
    ):
        raise ValueError("shared reviewed transition is not supported")
    binding = _binding(application)
    _validate_parent(
        parent,
        f"bof3.{owner}-parent-review/v1",
        binding,
        application["review_context"]["implementation_run_id"],
    )
    # Original local proofs remain integrity evidence; parent acceptance adds no
    # substitute receipts, captured context, or inferred verdict from prose.
    load(root / application["attestation"]["path"])
    verify(root, application, binding["application_digest"])
    manifest = application["manifest"]
    if application.get("concern") in {"shared", "shared_template"}:
        from harness.common.promotion import validate_post

        validate_post(
            root, application, owner, parent, manifest_validator=manifest_validator
        )
    allowed = set(manifest["allowed_paths"])
    baseline = manifest["workspace_baseline"]["state"]
    current = workspace_state(root)
    if {p: v for p, v in current.items() if p not in allowed} != {
        p: v for p, v in baseline.items() if p not in allowed
    }:
        raise ValueError("adopted unrelated workspace baseline drifted")


def _review(
    root: Path,
    application: object,
    parent: object,
    expected: str,
    owner: str,
    verify,
    *,
    manifest_validator,
) -> dict:
    if not isinstance(application, dict) or application.get("digest") != expected:
        raise ValueError("application digest is not trusted")
    _validate(
        root, application, parent, owner, verify, manifest_validator=manifest_validator
    )
    facts = {
        "schema": f"bof3.{owner}-reviewed-application/v1",
        "application": copy.deepcopy(application),
        "parent_review": copy.deepcopy(parent),
    }
    return {**facts, "digest": digest(facts)}


def _verify_reviewed(
    root: Path,
    value: object,
    expected: str,
    owner: str,
    verify,
    *,
    manifest_validator,
) -> dict:
    _keys(value, "schema application parent_review digest")
    facts = {key: item for key, item in value.items() if key != "digest"}
    if (
        value["schema"] != f"bof3.{owner}-reviewed-application/v1"
        or not isinstance(expected, str)
        or value["digest"] != expected
        or digest(facts) != expected
    ):
        raise ValueError("reviewed application envelope is not trusted or drifted")
    _validate(
        root,
        value["application"],
        value["parent_review"],
        owner,
        verify,
        manifest_validator=manifest_validator,
    )
    return {
        "schema": value["schema"],
        "digest": expected,
        "accepted": True,
        "target": value["application"]["target"],
    }
