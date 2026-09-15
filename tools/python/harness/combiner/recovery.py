"""Bind combiner rehearsal history to shared, parent-authorized source recovery."""

from __future__ import annotations

import copy
from pathlib import Path

from harness.combiner.transactions import (
    validate_transaction_history,
    verify_transaction,
)
from harness.common.deadlines import validate_deadline
from harness.common.digests import digest
from harness.common.directory import validate_repo_path

SCHEMA = "bof3.combiner-rehearsal-manifest/v1"


def prepare_recovery_manifest(
    root: Path, transaction: dict, baseline: dict, run_id: str, clock: dict
) -> dict:
    """Describe the exact PRE restoration set without granting recovery authority."""
    pre = {
        name: state["sha256"] if state is not None else None
        for name, state in transaction["pre"].items()
    }
    result = {
        "schema": SCHEMA,
        "root": str(root),
        "transaction": transaction,
        "transaction_fingerprint": transaction["fingerprint"],
        "target": transaction["preservation"]["profile"]["target"],
        "allowed_paths": transaction["allowed_paths"],
        "deletions": transaction["deletions"],
        "pre_state": pre,
        "pre_state_digest": digest(pre),
        "workspace_baseline": baseline,
        "implementation_run_id": run_id,
        "clock": clock,
        "accepted": False,
    }
    return {**result, "digest": digest(result)}


def validate_recovery_manifest(
    root: Path, value: object, *, rederive: bool = False
) -> dict:
    """Validate retained history; only explicit rederivation observes live PRE."""
    if not isinstance(value, dict):
        raise ValueError("combiner recovery requires a rehearsal manifest")
    value = copy.deepcopy(value)
    if value.get("digest") != digest(
        {name: item for name, item in value.items() if name != "digest"}
    ):
        raise ValueError("combiner recovery manifest digest differs")
    try:
        transaction = validate_transaction_history(
            value["transaction"], expected_fingerprint=value["transaction_fingerprint"]
        )
        baseline, clock, run_id = (
            value["workspace_baseline"],
            value["clock"],
            value["implementation_run_id"],
        )
        if (
            transaction["root"] != str(root)
            or not isinstance(run_id, str)
            or not run_id.strip()
            or not isinstance(baseline, dict)
            or set(baseline) != {"state", "digest", "adopted"}
            or not isinstance(baseline["state"], dict)
            or baseline["digest"] != digest(baseline["state"])
            or baseline["adopted"] is not bool(baseline["state"])
            or not isinstance(clock, dict)
            or set(clock) != {"boot_id", "deadline", "cleanup_deadline"}
            or not isinstance(clock["boot_id"], str)
            or not clock["boot_id"].strip()
        ):
            raise ValueError("combiner recovery baseline or clock differs")
        for name in baseline["state"]:
            validate_repo_path(name)
        for name in ("deadline", "cleanup_deadline"):
            validate_deadline(clock[name])
            if clock[name] is None:
                raise ValueError("combiner recovery requires original cutoffs")
        if clock["cleanup_deadline"] <= clock["deadline"]:
            raise ValueError("combiner cleanup cutoff does not follow work")
        if value != prepare_recovery_manifest(
            root, transaction, baseline, run_id, clock
        ):
            raise ValueError("combiner recovery manifest fields differ")
        if rederive:
            verify_transaction(
                root, transaction, expected_fingerprint=transaction["fingerprint"]
            )
    except (KeyError, TypeError) as error:
        raise ValueError(
            "combiner recovery manifest lacks required bindings"
        ) from error
    return value
