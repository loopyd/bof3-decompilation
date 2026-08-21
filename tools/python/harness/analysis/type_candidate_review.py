"""Reviewed, live-bound type candidate artifacts; the derived index never authorizes."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from ..domain.receipts import validate_candidate

SCHEMA = "bof3.reviewed-type-candidate/v1"
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


def digest(value: object) -> str:
    encoded = json.dumps(value, sort_keys=True, separators=(",", ":"))
    return "v1:" + hashlib.sha256(encoded.encode()).hexdigest()


def _record(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, UnicodeDecodeError) as error:
        raise ValueError(f"reviewed type candidate is not JSON: {path}") from error
    if not isinstance(value, dict):
        raise ValueError("reviewed type candidate must be an object")
    return value


def validate_reviewed_candidate(
    root: Path,
    artifact: str,
    concern: str,
    index_row: dict[str, Any],
) -> dict[str, Any]:
    """Validate independent review plus live repository and index fingerprints."""

    path = Path(artifact)
    if path.is_absolute() or not (root / path).is_file():
        raise ValueError(f"reviewed type candidate artifact missing: {artifact}")
    value = _record(root / path)
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
    return {
        "artifact": artifact,
        "artifact_sha256": hashlib.sha256((root / path).read_bytes()).hexdigest(),
        "index_id": index_row["id"],
        "candidate": candidate,
        "representation": representation["contract"],
        "semantics": semantics["contract"],
        "review": review,
    }


def candidate_account(root: Path, connect) -> dict[str, Any]:
    connection = connect(root)
    try:
        rows = [
            dict(row)
            for row in connection.execute(
                "SELECT id,target_id target,printf('0x%08X',address) address,kind,status,blocker FROM type_candidates ORDER BY target_id,address,kind,id"
            )
        ]
        total = connection.execute("SELECT COUNT(*) FROM type_candidates").fetchone()[0]
    finally:
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
    return value


__all__ = [
    "SCHEMA",
    "artifact_paths",
    "candidate_account",
    "digest",
    "validate_reviewed_candidate",
]
