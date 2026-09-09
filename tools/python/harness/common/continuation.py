"""Resolve current, request-bound resume dispositions without applying work."""

from __future__ import annotations

from datetime import datetime, timedelta
from pathlib import Path
import re
import time
from typing import Any, Callable

from harness.common.digests import digest


def _validate_budget_fields(value: object, fields: str) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != set(fields.split()):
        raise ValueError("invalid execution budget fields")
    return value


def _validate_budget_integer(value: object) -> int:
    if type(value) is not int or value < 0:
        raise ValueError("execution budget counters must be nonnegative integers")
    return value


def _validate_budget_pin(value: dict[str, Any], expected: str) -> None:
    if (
        not isinstance(expected, str)
        or re.fullmatch(r"v1:[0-9a-f]{64}", expected) is None
        or value.get("digest") != expected
        or digest({key: item for key, item in value.items() if key != "digest"})
        != expected
    ):
        raise ValueError("execution budget or consumption pin drifted")


def check_budget(
    root: Path,
    contract: object,
    checkpoints: object,
    expected_budget_digest: str,
    expected_checkpoint_digest: str,
    expected_sequence: int,
) -> dict[str, Any]:
    contract = _validate_budget_fields(
        contract, "schema id root queue clock limits digest"
    )
    _validate_budget_pin(contract, expected_budget_digest)
    if (
        contract["schema"] != "bof3.execution-budget/v1"
        or not isinstance(contract["id"], str)
        or not contract["id"].strip()
    ):
        raise ValueError("invalid execution budget identity")
    root = root.resolve()
    if not root.is_dir():
        raise ValueError("execution budget root must be a directory")
    metadata = root.stat()
    binding = {"path": str(root), "device": metadata.st_dev, "inode": metadata.st_ino}
    if contract["root"] != binding or digest(contract["root"]) != digest(binding):
        raise ValueError("execution budget root drifted")
    queue = contract["queue"]
    if not isinstance(queue, list) or not queue:
        raise ValueError("execution budget needs the original ordered queue")
    identities = []
    for item in queue:
        item = _validate_budget_fields(item, "id fingerprint")
        if (
            not isinstance(item["id"], str)
            or not item["id"].strip()
            or not isinstance(item["fingerprint"], str)
            or re.fullmatch(r"v1:[0-9a-f]{64}", item["fingerprint"]) is None
        ):
            raise ValueError("invalid execution budget queue identity")
        identities.append(item["id"])
    if len(set(identities)) != len(identities):
        raise ValueError("execution budget queue contains duplicates")
    clock = _validate_budget_fields(
        contract["clock"], "boot_id started_ns deadline_ns started_utc"
    )
    started = _validate_budget_integer(clock["started_ns"])
    deadline = _validate_budget_integer(clock["deadline_ns"])
    try:
        audit_time = datetime.fromisoformat(clock["started_utc"])
    except (TypeError, ValueError) as error:
        raise ValueError("execution budget audit time must be UTC ISO-8601") from error
    if (
        deadline <= started
        or clock["boot_id"]
        != Path("/proc/sys/kernel/random/boot_id").read_text().strip()
        or not isinstance(clock["started_utc"], str)
        or not clock["started_utc"].strip()
        or audit_time.utcoffset() != timedelta(0)
    ):
        raise ValueError("execution budget boot or original deadline is invalid")
    limits = _validate_budget_fields(contract["limits"], "launches repairs")
    launch_limit = _validate_budget_integer(limits["launches"])
    repairs = limits["repairs"]
    if not isinstance(repairs, dict) or set(repairs) != set(identities):
        raise ValueError("execution budget repair limits omit queue entries")
    for limit in repairs.values():
        _validate_budget_integer(limit)
    sequence = _validate_budget_integer(expected_sequence)
    if not isinstance(checkpoints, list) or len(checkpoints) != sequence + 1:
        raise ValueError(
            "execution budget requires the complete latest checkpoint chain"
        )
    previous = None
    for position, checkpoint in enumerate(checkpoints):
        checkpoint = _validate_budget_fields(
            checkpoint,
            "schema budget_digest sequence previous_digest launches repairs digest",
        )
        _validate_budget_pin(checkpoint, checkpoint["digest"])
        if (
            checkpoint["schema"] != "bof3.execution-consumption/v1"
            or checkpoint["budget_digest"] != expected_budget_digest
            or _validate_budget_integer(checkpoint["sequence"]) != position
            or checkpoint["previous_digest"]
            != (previous["digest"] if previous else None)
        ):
            raise ValueError("execution budget checkpoint ancestry drifted")
        counters = []
        before = []
        for kind in ("launches", "repairs"):
            values = checkpoint[kind]
            if not isinstance(values, dict) or set(values) != set(identities):
                raise ValueError("execution budget checkpoint omits queue entries")
            for identity in identities:
                counters.append(_validate_budget_integer(values[identity]))
                before.append(previous[kind][identity] if previous else 0)
        deltas = [current - earlier for current, earlier in zip(counters, before)]
        if (
            (previous is None and any(counters))
            or (
                previous is not None
                and (sum(deltas) != 1 or any(change not in {0, 1} for change in deltas))
            )
            or sum(checkpoint["launches"].values()) > launch_limit
            or any(
                checkpoint["repairs"][identity] > repairs[identity]
                for identity in identities
            )
        ):
            raise ValueError(
                "execution budget consumption was refunded, reset or exceeded"
            )
        previous = checkpoint
    _validate_budget_pin(checkpoints[-1], expected_checkpoint_digest)
    now = time.monotonic_ns()
    if now < started:
        raise ValueError("execution budget clock precedes the original start")
    remaining = launch_limit - sum(previous["launches"].values())
    return {
        "schema": "bof3.execution-budget-status/v1",
        "budget_digest": expected_budget_digest,
        "checkpoint_digest": expected_checkpoint_digest,
        "sequence": sequence,
        "queue_digest": digest(queue),
        "deadline_ns": deadline,
        "remaining_ns": max(0, deadline - now),
        "remaining_launches": remaining,
        "remaining_repairs": {
            identity: repairs[identity] - previous["repairs"][identity]
            for identity in identities
        },
        "exhausted": now >= deadline or remaining == 0,
        "launch_authorized": False,
        "latestness_basis": "parent-pinned high-water checkpoint; not independently discovered",
    }


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
