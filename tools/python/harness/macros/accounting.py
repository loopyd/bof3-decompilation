"""Fresh exactly-once accounting for current macro opportunity leads."""

from __future__ import annotations

import hashlib
import json
import sqlite3
from pathlib import Path
from typing import Any

from harness.analysis.index import connect
from harness.io import file_sha256
from harness.macros.blocks import block_opportunities_payload, validate_minimum
from harness.macros.opportunities import macro_opportunities_payload
from harness.macros.similarity import near_duplicates_payload

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
    connection: sqlite3.Connection, root: Path, target_filter: str | None = None
) -> list[dict[str, Any]]:
    inputs: list[dict[str, Any]] = []
    target_where = "" if target_filter is None else " WHERE id = ?"
    parameters = () if target_filter is None else (target_filter,)
    for target, path, sha256 in connection.execute(
        "SELECT id,binary,binary_sha256 FROM targets" + target_where + " ORDER BY id",
        parameters,
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
    source_where = (
        ""
        if target_filter is None
        else "WHERE target_id = ? OR owner_target = '__shared__' "
    )
    for target, path, sha256, input_kind, owner in connection.execute(
        "SELECT target_id,source_path,sha256,input_kind,owner_target "
        "FROM macro_input_fingerprints "
        + source_where
        + "ORDER BY target_id,source_path,owner_target",
        parameters,
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


def _candidate_targets(candidate: dict[str, Any]) -> list[str]:
    """Derive canonical target membership from the global candidate shape."""

    targets = {
        target
        for member in candidate.get("members", [])
        if isinstance(member, dict)
        for target in (
            member.get("target"),
            str(member.get("function", "")).rsplit("@", 1)[0],
        )
        if isinstance(target, str) and target and "/" in target
    }
    scope = candidate.get("target_scope")
    if isinstance(scope, str) and "/" in scope:
        targets.add(scope)
    declared = candidate.get("targets", [])
    if isinstance(declared, list):
        targets.update(
            item for item in declared if isinstance(item, str) and "/" in item
        )
    return sorted(targets)


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
                "targets": _candidate_targets(candidate),
                "shared": candidate.get("target_scope") == "__shared__",
                "blocked_reason": (
                    "; ".join(blockers)
                    if status == "blocked" and isinstance(blockers, list)
                    else None
                ),
                "candidate_fingerprint": _digest(candidate),
            }
        )
    return sorted(rows, key=lambda item: item["id"])


def candidate_account(
    root: Path,
    connection=None,
    target: str | None = None,
    *,
    block_min_instructions: int | None = None,
) -> dict[str, Any]:
    """Account once for every fresh macro and near-duplicate opportunity."""

    owned = connection is None
    if owned:
        connection = connect(root)
    try:
        candidates = macro_opportunities_payload(
            connection, root, target=target, kind=None, limit=0
        ) + near_duplicates_payload(connection, root, target=target, limit=0)
        if block_min_instructions is not None:
            blocks = block_opportunities_payload(
                connection, root, min_instructions=block_min_instructions
            )
            candidates += [
                row
                for row in blocks
                if target is None or target in _candidate_targets(row)
            ]
        inputs = _input_fingerprints(connection, root, target)
    finally:
        if owned:
            connection.close()
    rows = _candidate_rows(candidates)
    counts = {
        status: sum(row["status"] == status for row in rows)
        for status in sorted(_ALLOWED_STATUSES)
        if any(row["status"] == status for row in rows)
    }
    report = {
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
    if block_min_instructions is not None:
        report["block_discovery"] = {
            "min_instructions": block_min_instructions,
            "minimum_uses": 4,
        }
    return report


def validate_account(root: Path, report: object) -> dict[str, Any]:
    """Require a report to equal fresh current opportunity accounting."""

    options = {}
    if isinstance(report, dict) and "block_discovery" in report:
        policy = report["block_discovery"]
        if (
            not isinstance(policy, dict)
            or set(policy) != {"min_instructions", "minimum_uses"}
            or type(policy["minimum_uses"]) is not int
            or policy["minimum_uses"] != 4
        ):
            raise ValueError("invalid macro block discovery policy")
        options["block_min_instructions"] = validate_minimum(policy["min_instructions"])
    current = candidate_account(root, **options)
    if report != current:
        raise ValueError("macro candidate account is stale, incomplete, or duplicated")
    return current


__all__ = ["ACCOUNT_SCHEMA", "candidate_account", "validate_account"]
