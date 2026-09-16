"""Reviewed, live-bound type candidate artifacts; the derived index never authorizes."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from harness.common.digests import digest
from harness.common.deadlines import check_deadline
from harness.common.directory import validate_repo_path
from harness.common.inputs import InputBatch
from harness.domain.receipts import validate_candidate
from harness.io import unique_object

SCHEMA = "bof3.reviewed-type-candidate/v1"
_LIMIT = 4 * 1024 * 1024
_KIND_FOR_CONCERN = {
    "alias": {"typedef"},
    "layout": {"aggregate"},
    "field": {"field"},
    "prototype": {"prototype"},
    "shared": {"aggregate", "field"},
}
_INDEX_FOR_CONCERN = {
    "alias": lambda value: value == "storage",
    "layout": lambda value: value in {"aggregate_region", "array_stride"},
    "field": lambda value: value.startswith("field_offset_"),
    "prototype": lambda value: value == "prototype",
    "shared": lambda value: (
        value in {"aggregate_region", "array_stride"}
        or value.startswith("field_offset_")
    ),
}


def _record(content: bytes, artifact: str) -> dict[str, Any]:
    check_deadline()
    try:
        value = json.loads(content.decode("utf-8"), object_pairs_hook=unique_object)
    except (json.JSONDecodeError, UnicodeDecodeError, RecursionError) as error:
        raise ValueError(
            f"reviewed type candidate is not bounded JSON: {artifact}"
        ) from error
    if not isinstance(value, dict):
        raise ValueError("reviewed type candidate must be an object")
    check_deadline()
    return value


def validate_reviewed_candidate(
    root: Path,
    artifact: str,
    concern: str,
    index_row: dict[str, Any],
) -> dict[str, Any]:
    """Validate independent review plus live repository and index fingerprints."""

    artifact = validate_repo_path(artifact)
    with InputBatch(root) as batch:
        state, content = batch.read(root / artifact, max_bytes=_LIMIT)
    if state is None:
        raise ValueError(f"reviewed type candidate artifact missing: {artifact}")
    value = _record(content, artifact)
    facts = {key: item for key, item in value.items() if key != "digest"}
    if (
        facts.get("schema") != SCHEMA
        or set(value)
        != {
            "schema",
            "index_id",
            "candidate",
            "representation",
            "semantics",
            "review",
            "index_row_digest",
            "digest",
        }
        or value.get("digest") != digest(facts)
    ):
        raise ValueError("reviewed type candidate artifact drifted")
    candidate = validate_candidate(value["candidate"], root)
    if (
        candidate["status"] != "accepted"
        or candidate["kind"] not in _KIND_FOR_CONCERN[concern]
        or candidate["target"] != index_row["target_id"]
        or candidate["address"] != index_row["address"]
        or candidate["missing_facts"]
        or len(candidate["observations"]) < 2
        or candidate["authority"] not in {"original", "reviewed", "authored"}
    ):
        raise ValueError("reviewed type candidate has unresolved evidence")
    if (
        value["index_id"] != index_row["id"]
        or not _INDEX_FOR_CONCERN[concern](index_row["kind"])
        or value["index_row_digest"] != digest(index_row)
    ):
        raise ValueError("reviewed type candidate index fingerprint drifted")
    representation = value["representation"]
    semantics = value["semantics"]
    review = value["review"]
    if (
        not isinstance(representation, dict)
        or representation.get("status") != "resolved"
        or not isinstance(representation.get("contract"), dict)
        or not representation["contract"]
        or not isinstance(semantics, dict)
        or semantics.get("status") != "resolved"
        or not isinstance(semantics.get("contract"), dict)
        or not semantics["contract"]
        or not isinstance(review, dict)
        or review.get("verdict") != "accepted"
        or not isinstance(review.get("reviewer"), str)
        or not review["reviewer"].strip()
    ):
        raise ValueError("reviewed type candidate has unresolved review contract")
    check_deadline()
    return {
        "artifact": artifact,
        "artifact_sha256": state["sha256"],
        "index_id": index_row["id"],
        "candidate": candidate,
        "representation": representation["contract"],
        "semantics": semantics["contract"],
        "review": review,
    }


def candidate_account(
    root: Path, connect, target: str | None = None, *, close: bool = True
) -> dict[str, Any]:
    connection = connect(root)
    try:
        where = "" if target is None else " WHERE target_id = ?"
        parameters = () if target is None else (target,)
        rows = [
            dict(row)
            for row in connection.execute(
                "SELECT id,target_id target,printf('0x%08X',address) address,kind,status,blocker FROM type_candidates"
                + where
                + " ORDER BY target_id,address,kind,id",
                parameters,
            )
        ]
        total = connection.execute(
            "SELECT COUNT(*) FROM type_candidates" + where, parameters
        ).fetchone()[0]
    finally:
        if close:
            connection.close()
    if len(rows) != total or len({row["id"] for row in rows}) != total:
        raise ValueError("type candidate accounting is not exactly once")
    counts: dict[str, int] = {}
    allowed = {"blocked", "proposed", "accepted", "rejected", "stale"}
    for row in rows:
        if row["status"] not in allowed:
            raise ValueError(f"unaccounted type candidate state: {row['status']}")
        if row["status"] == "blocked" and not row["blocker"]:
            raise ValueError(
                f"blocked type candidate lacks explicit blocker: {row['id']}"
            )
        counts[row["status"]] = counts.get(row["status"], 0) + 1
    return {
        "schema": "bof3.type-candidate-account/v1",
        "complete": True,
        "candidate_count": total,
        "safe_application_count": 0,
        "counts": dict(sorted(counts.items())),
        "rows": rows,
    }


def artifact_paths(value: object) -> list[str]:
    if (
        not isinstance(value, list)
        or not value
        or any(not isinstance(item, str) or not item for item in value)
        or len(set(value)) != len(value)
    ):
        raise ValueError("candidate_artifacts must be unique repo-relative paths")
    return [validate_repo_path(path) for path in value]


__all__ = [
    "SCHEMA",
    "artifact_paths",
    "candidate_account",
    "digest",
    "validate_reviewed_candidate",
]
