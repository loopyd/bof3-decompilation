"""Bind combiner rehearsal history to shared, parent-authorized source recovery."""

from __future__ import annotations

import copy
from pathlib import Path

from harness.combiner.history import validate_transaction_history
from harness.combiner.transactions import verify_transaction
from harness.common.digests import digest
from harness.common.directory import validate_repo_path

SCHEMA = "bof3.combiner-rehearsal-manifest/v1"


def prepare_recovery_manifest(
    root: Path, transaction: dict, baseline: dict, run_id: str
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
        baseline, run_id = (
            value["workspace_baseline"],
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
        ):
            raise ValueError("combiner recovery baseline differs")
        for name in baseline["state"]:
            validate_repo_path(name)
        if value != prepare_recovery_manifest(root, transaction, baseline, run_id):
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
