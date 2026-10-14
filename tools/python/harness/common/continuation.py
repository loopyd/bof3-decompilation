"""Resolve current, request-bound resume dispositions without applying work."""

from __future__ import annotations

from pathlib import Path
from typing import Any, Callable

from harness.common.digests import digest


def resolve_disposition(
    root: Path,
    manifest: object,
    expected_manifest_digest: str,
    implementation_run_id: str,
    application: object,
    expected_application_digest: str,
    *,
    owner: str,
    manifest_validator: Callable[..., dict[str, Any]],
    verify_application: Callable[..., dict[str, Any]],
    verify_reviewed: Callable[..., dict[str, Any]],
    envelope: object = None,
    expected_envelope_digest: str | None = None,
) -> dict[str, Any]:
    if owner not in {"type", "macro"}:
        raise ValueError("unsupported resume owner")
    if (
        not isinstance(manifest, dict)
        or manifest.get("digest") != expected_manifest_digest
        or not isinstance(implementation_run_id, str)
        or not implementation_run_id.strip()
    ):
        raise ValueError("resume requires the pinned original manifest and run")
    original = manifest_validator(root, manifest, rederive=False)
    if (
        not isinstance(application, dict)
        or application.get("digest") != expected_application_digest
        or application.get("manifest") != original
        or digest(application.get("manifest")) != digest(original)
    ):
        raise ValueError("resume application belongs to another original request")
    context = application.get("review_context")
    if (
        not isinstance(context, dict)
        or context.get("implementation_run_id") != implementation_run_id
        or context.get("manifest_digest") != expected_manifest_digest
        or context.get("request_digest") != digest(original["request"])
    ):
        raise ValueError("resume requires the captured original implementation context")
    if (envelope is None) != (expected_envelope_digest is None):
        raise ValueError("resume acceptance requires both envelope and independent pin")
    if envelope is None:
        verified = verify_application(root, application, expected_application_digest)
        if (
            verified.get("applied") is not True
            or verified.get("digest") != expected_application_digest
        ):
            raise ValueError(
                "resume application verification did not establish applied POST"
            )
        disposition = "needs-review"
    else:
        if (
            not isinstance(envelope, dict)
            or envelope.get("application") != application
            or digest(envelope.get("application")) != digest(application)
        ):
            raise ValueError(
                "resume acceptance belongs to another complete application"
            )
        verified = verify_reviewed(root, envelope, expected_envelope_digest)
        if (
            verified.get("accepted") is not True
            or verified.get("digest") != expected_envelope_digest
        ):
            raise ValueError(
                "resume acceptance verification did not establish current acceptance"
            )
        disposition = "skip-accepted"
    return {
        "schema": "bof3.transaction-resume/v1",
        "owner": owner,
        "disposition": disposition,
        "manifest_digest": expected_manifest_digest,
        "request_digest": digest(original["request"]),
        "implementation_run_id": implementation_run_id,
        "application_digest": expected_application_digest,
        "application_proof_digest": digest(application),
        "envelope_digest": expected_envelope_digest,
        "application_started": False,
        "retry_authorized": False,
        "writer_termination_verified": False,
        "writer_exclusion_verified": False,
        "observation_atomic": False,
        "durable_skip_token": False,
    }
