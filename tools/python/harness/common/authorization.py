"""Validate externally pinned parent authority for source-only restoration."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from harness.common.digests import digest
from harness.common.directory import validate_repo_path
from harness.common.files import read_file
from harness.common.observation import observe_file
from harness.io import unique_object

AUTHORIZATION_SCHEMA = "bof3.source-recovery-authorization/v1"


def _require_authorization_fields(value: object, names: str) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != set(names.split()):
        raise ValueError("invalid source recovery authorization fields")
    return value


def _validate_recovery_artifact(root: Path, value: object) -> str:
    ref = _require_authorization_fields(value, "path sha256")
    path = validate_repo_path(ref["path"])
    if not path.startswith("out/reviews/evidence/"):
        raise ValueError("recovery authority artifacts must be retained as evidence")
    observed = observe_file(root, path)
    content = read_file(root, path)
    if (
        observed is None
        or observed["links"] != 1
        or observed["sha256"] != ref["sha256"]
        or hashlib.sha256(content).hexdigest() != ref["sha256"]
        or not content.strip()
        or observe_file(root, path) != observed
    ):
        raise ValueError("recovery authority artifact drifted or is empty")
    return path


def load_recovery_authorization(
    root: Path, name: str, expected: str, recovery_name: str, record: dict[str, Any]
) -> dict[str, Any]:
    name = validate_repo_path(name)
    if not name.startswith("out/reviews/evidence/"):
        raise ValueError("recovery authorization must be retained as evidence")
    observed = observe_file(root, name)
    if observed is None or observed["links"] != 1:
        raise ValueError("recovery authorization must be a single-link regular file")
    content = read_file(root, name)
    authorization = _require_authorization_fields(
        json.loads(content, object_pairs_hook=unique_object),
        "schema approved parent_run_id reviewer_run_id review_artifact binding termination digest",
    )
    facts = {key: value for key, value in authorization.items() if key != "digest"}
    if (
        authorization["schema"] != AUTHORIZATION_SCHEMA
        or authorization["approved"] is not True
        or authorization["digest"] != expected
        or digest(facts) != expected
        or hashlib.sha256(content).hexdigest() != observed["sha256"]
    ):
        raise ValueError("recovery authorization or independent pin drifted")
    binding = record["binding"]
    run_ids = [
        binding["implementation_run_id"],
        authorization["parent_run_id"],
        authorization["reviewer_run_id"],
    ]
    if (
        any(not isinstance(value, str) or not value.strip() for value in run_ids)
        or len(set(run_ids)) != 3
    ):
        raise ValueError(
            "recovery requires distinct implementation, parent and reviewer"
        )
    completion = (
        f"out/reviews/evidence/{binding['owner']}-restoration-{record['nonce']}.json"
    )
    expected_binding = {
        "action": "restore-owned-pre",
        "owner": binding["owner"],
        "root": record["root"],
        "implementation_run_id": binding["implementation_run_id"],
        "recovery": {"path": recovery_name, "digest": record["digest"]},
        "manifest_digest": binding["manifest"]["digest"],
        "paths": sorted(record["files"]),
        "publication": binding["output"],
        "completion": completion,
    }
    if authorization["binding"] != expected_binding or digest(
        authorization["binding"]
    ) != digest(expected_binding):
        raise ValueError("recovery authorization does not bind the exact restoration")
    termination = _require_authorization_fields(
        authorization["termination"], "tool handle state writer_tree_stopped artifact"
    )
    if (
        termination["state"] != "terminal"
        or termination["writer_tree_stopped"] is not True
        or any(
            not isinstance(termination[key], str) or not termination[key].strip()
            for key in ("tool", "handle")
        )
    ):
        raise ValueError(
            "recovery requires explicit parent-attested writer termination"
        )
    terminal_path = _validate_recovery_artifact(root, termination["artifact"])
    review_path = _validate_recovery_artifact(root, authorization["review_artifact"])
    paths = [
        name,
        recovery_name,
        completion,
        binding["output"],
        terminal_path,
        review_path,
    ]
    if len(set(paths)) != len(paths):
        raise ValueError("recovery evidence and publication paths must be distinct")
    if observe_file(root, name) != observed:
        raise ValueError("recovery authorization changed during validation")
    return authorization
