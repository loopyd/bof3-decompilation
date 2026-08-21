"""Fresh exactly-once accounting for current macro opportunity leads."""

from __future__ import annotations

import hashlib
import json
import sqlite3
from pathlib import Path
from typing import Any

from ..discovery import file_sha256
from .index import connect
from .macro_opportunities import macro_opportunities_payload
from .near_duplicates import near_duplicates_payload

ACCOUNT_SCHEMA = "bof3.macro-candidate-account/v1"
_ALLOWED_STATUSES = frozenset({"blocked", "accepted"})


def _digest(value: object) -> str:
    encoded = json.dumps(value, sort_keys=True, separators=(",", ":"))
    return "v1:" + hashlib.sha256(encoded.encode()).hexdigest()


def _fresh_file(root: Path, relative: str, expected: str, identity: str) -> None:
    path = (root / relative).resolve()
    if (
        not path.is_relative_to(root.resolve())
        or not path.is_file()
        or file_sha256(path) != expected
    ):
        raise ValueError(f"stale macro account input: {identity}")


def _input_fingerprints(
    connection: sqlite3.Connection, root: Path
) -> list[dict[str, Any]]:
    inputs: list[dict[str, Any]] = []
    for target, path, sha256 in connection.execute(
        "SELECT id,binary,binary_sha256 FROM targets ORDER BY id"
    ):
        identity = f"binary:{target}"
        _fresh_file(root, path, sha256, identity)
        inputs.append(
            {
                "id": identity,
                "kind": "binary",
                "target": target,
                "path": path,
                "sha256": sha256,
                "fresh": True,
            }
        )
    for target, path, sha256, input_kind, owner in connection.execute(
        "SELECT target_id,source_path,sha256,input_kind,owner_target "
        "FROM macro_input_fingerprints "
        "ORDER BY target_id,source_path,owner_target"
    ):
        identity = f"source:{target}:{owner}:{path}"
        _fresh_file(root, path, sha256, identity)
        inputs.append(
            {
                "id": identity,
                "kind": "source",
                "target": target,
                "owner": owner,
                "path": path,
                "provenance": input_kind,
                "sha256": sha256,
                "fresh": True,
            }
        )
    if len({item["id"] for item in inputs}) != len(inputs):
        raise ValueError("macro account inputs are not exactly once")
    return inputs


def _candidate_rows(candidates: list[dict[str, Any]]) -> list[dict[str, Any]]:
    if len({item.get("id") for item in candidates}) != len(candidates):
        raise ValueError("macro candidate accounting is not exactly once")
    rows = []
    for candidate in candidates:
        candidate_id = candidate.get("id")
        status = candidate.get("status")
        blockers = candidate.get("blockers")
        if not isinstance(candidate_id, str) or not candidate_id:
            raise ValueError("macro candidate lacks a stable ID")
        if status not in _ALLOWED_STATUSES:
            raise ValueError(f"unaccounted macro candidate state: {status}")
        if status == "blocked" and (
            not isinstance(blockers, list)
            or not blockers
            or any(not isinstance(item, str) or not item for item in blockers)
        ):
            raise ValueError(
                f"blocked macro candidate lacks explicit reason: {candidate_id}"
            )
        rows.append(
            {
                "id": candidate_id,
                "kind": candidate["kind"],
                "status": status,
                "blocked_reason": "; ".join(blockers) if status == "blocked" else None,
                "candidate_fingerprint": _digest(candidate),
            }
        )
    return sorted(rows, key=lambda item: item["id"])


def candidate_account(root: Path) -> dict[str, Any]:
    """Account once for every fresh macro and near-duplicate opportunity."""

    connection = connect(root)
    try:
        candidates = macro_opportunities_payload(
            connection, root, target=None, kind=None, limit=0
        ) + near_duplicates_payload(connection, root, target=None, limit=0)
        inputs = _input_fingerprints(connection, root)
    finally:
        connection.close()
    rows = _candidate_rows(candidates)
    counts = {
        status: sum(row["status"] == status for row in rows)
        for status in sorted(_ALLOWED_STATUSES)
        if any(row["status"] == status for row in rows)
    }
    return {
        "schema": ACCOUNT_SCHEMA,
        "complete": True,
        "fresh": True,
        "candidate_count": len(rows),
        "safe_application_count": counts.get("accepted", 0),
        "counts": counts,
        "source_input_fingerprint": _digest(inputs),
        "inputs": inputs,
        "rows": rows,
    }


def validate_account(root: Path, report: object) -> dict[str, Any]:
    """Require a report to equal fresh current opportunity accounting."""

    current = candidate_account(root)
    if report != current:
        raise ValueError("macro candidate account is stale, incomplete, or duplicated")
    return current


__all__ = ["ACCOUNT_SCHEMA", "candidate_account", "validate_account"]
