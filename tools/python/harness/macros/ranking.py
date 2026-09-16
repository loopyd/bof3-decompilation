"""Fresh human-value assessments and pinned top-N macro work selection."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from harness.common.digests import digest
from harness.common.deadlines import check_deadline
from harness.common.files import read_file
from harness.common.directory import validate_repo_path
from harness.io import unique_object
from harness.macros.blocks import HUMAN_CRITERIA, block_report, validate_minimum

REQUEST_SCHEMA = "bof3.macro-ranking-request/v1"
SCHEMA = "bof3.macro-ranking/v1"
_POLICY_KEYS = {"min_instructions", "target", "pool_size", "top_n", "require_source"}
_ROW_KEYS = {
    "id",
    "candidate_fingerprint",
    "decision",
    "priority",
    "rationale",
    "criteria",
}


def _policy(value: object) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != _POLICY_KEYS:
        raise ValueError("macro ranking policy has invalid fields")
    validate_minimum(value["min_instructions"])
    for name in ("pool_size", "top_n"):
        if type(value[name]) is not int or value[name] < 1:
            raise ValueError(f"macro ranking {name} must be a positive integer")
    if value["top_n"] > value["pool_size"]:
        raise ValueError("macro ranking top_n cannot exceed pool_size")
    if type(value["require_source"]) is not bool:
        raise ValueError("macro ranking require_source must be boolean")
    if value["target"] is not None and not isinstance(value["target"], str):
        raise ValueError("macro ranking target must be a canonical target or null")
    return dict(value)


def _pool(root: Path, policy: dict) -> dict:
    check_deadline()
    report = block_report(
        root,
        min_instructions=policy["min_instructions"],
        target=policy["target"],
        limit=0,
    )
    available = [
        row
        for row in report["candidates"]
        if not policy["require_source"]
        or all(member["source_path"] for member in row["members"])
    ]
    check_deadline()
    return {
        "policy": policy,
        "population": {
            "visible": len(report["candidates"]),
            "eligible": len(available),
            "source_missing": len(report["candidates"]) - len(available),
        },
        "candidates": available[: policy["pool_size"]],
    }


def create_ranking_request(root: Path, policy: object) -> dict:
    check_deadline()
    pool = _pool(root, _policy(policy))
    result = {
        "schema": REQUEST_SCHEMA,
        **pool,
        "pool_digest": digest(pool),
        "assessor": "",
        "assessments": [
            {
                "id": row["id"],
                "candidate_fingerprint": digest(row),
                "decision": "unreviewed",
                "priority": None,
                "rationale": "",
                "criteria": {
                    name: {"verdict": "unknown", "evidence": ""}
                    for name in HUMAN_CRITERIA
                },
            }
            for row in pool["candidates"]
        ],
    }
    check_deadline()
    return result


def _text(value: object) -> bool:
    return isinstance(value, str) and bool(value.strip())


def _assessments(value: object, candidates: list[dict]) -> list[dict]:
    if not isinstance(value, list) or len(value) != len(candidates):
        raise ValueError(
            "macro ranking must assess every candidate in the pool exactly once"
        )
    by_id = {}
    for row in value:
        if (
            not isinstance(row, dict)
            or set(row) != _ROW_KEYS
            or not isinstance(row["id"], str)
            or row["id"] in by_id
        ):
            raise ValueError(
                "macro ranking assessment has invalid or duplicate identity"
            )
        by_id[row["id"]] = row
    result = []
    for machine_rank, candidate in enumerate(candidates, 1):
        check_deadline()
        row = by_id.get(candidate["id"])
        if row is None or row["candidate_fingerprint"] != digest(candidate):
            raise ValueError("macro ranking candidate fingerprint drifted")
        if (
            not isinstance(row["decision"], str)
            or row["decision"] not in {"try", "defer", "reject"}
            or not _text(row["rationale"])
        ):
            raise ValueError(
                "macro ranking requires a decision and rationale for every candidate"
            )
        criteria = row["criteria"]
        if not isinstance(criteria, dict) or set(criteria) != set(HUMAN_CRITERIA):
            raise ValueError("macro ranking requires all five human-value criteria")
        verdicts = []
        for observation in criteria.values():
            if (
                not isinstance(observation, dict)
                or set(observation) != {"verdict", "evidence"}
                or not isinstance(observation["verdict"], str)
                or observation["verdict"] not in {"promising", "poor", "unknown"}
                or not _text(observation["evidence"])
            ):
                raise ValueError("macro ranking human-value evidence is incomplete")
            verdicts.append(observation["verdict"])
        if row["decision"] == "try":
            if (
                any(verdict != "promising" for verdict in verdicts)
                or type(row["priority"]) is not int
                or row["priority"] < 1
            ):
                raise ValueError(
                    "macro ranking try requires five promising criteria and positive priority"
                )
        elif row["priority"] is not None:
            raise ValueError("deferred/rejected macro rankings cannot have a priority")
        if row["decision"] == "reject" and "poor" not in verdicts:
            raise ValueError("macro ranking rejection requires a poor criterion")
        result.append(
            {
                **row,
                "machine_rank": machine_rank,
                "instruction_count": candidate["instruction_count"],
                "support": len(candidate["members"]),
            }
        )
    priorities = sorted(row["priority"] for row in result if row["decision"] == "try")
    if priorities != list(range(1, len(priorities) + 1)):
        raise ValueError(
            "macro ranking try priorities must be unique and contiguous from one"
        )
    return result


def rank_candidates(root: Path, request: object, *, expected_pool_digest: str) -> dict:
    check_deadline()
    required = {
        "schema",
        "policy",
        "population",
        "candidates",
        "pool_digest",
        "assessor",
        "assessments",
    }
    if (
        not isinstance(request, dict)
        or set(request) != required
        or request["schema"] != REQUEST_SCHEMA
    ):
        raise ValueError("macro ranking request schema/fields are invalid")
    policy = _policy(request["policy"])
    pool = _pool(root, policy)
    if (
        request["pool_digest"] != expected_pool_digest
        or expected_pool_digest != digest(pool)
        or digest({key: request[key] for key in pool}) != expected_pool_digest
    ):
        raise ValueError("macro ranking pool or external pin is stale")
    if not _text(request["assessor"]):
        raise ValueError("macro ranking requires a named assessor")
    rows = _assessments(request["assessments"], pool["candidates"])
    selected = sorted(
        (row for row in rows if row["decision"] == "try"),
        key=lambda row: row["priority"],
    )[: policy["top_n"]]
    candidates = {row["id"]: row for row in pool["candidates"]}
    facts = {
        "schema": SCHEMA,
        "request": request,
        "pool_digest": expected_pool_digest,
        "assessor": request["assessor"],
        "top_n": policy["top_n"],
        "decisions": rows,
        "queue": [
            {
                "position": position,
                "id": row["id"],
                "candidate_fingerprint": row["candidate_fingerprint"],
                "priority": row["priority"],
                "machine_rank": row["machine_rank"],
                "members": candidates[row["id"]]["members"],
                "status": "selected_for_review",
            }
            for position, row in enumerate(selected, 1)
        ],
        "counts": {
            decision: sum(row["decision"] == decision for row in rows)
            for decision in ("try", "defer", "reject")
        },
        "budget_deferred": [
            row["id"]
            for row in rows
            if row["decision"] == "try" and row["priority"] > policy["top_n"]
        ],
        "safe_application_count": 0,
        "execution_started": False,
    }
    result = {**facts, "digest": digest(facts)}
    check_deadline()
    return result


def validate_ranking(
    root: Path, value: object, *, expected_ranking_digest: str
) -> dict:
    check_deadline()
    if not isinstance(value, dict) or value.get("schema") != SCHEMA:
        raise ValueError("macro ranking report schema is invalid")
    facts = {key: item for key, item in value.items() if key != "digest"}
    if value.get(
        "digest"
    ) != expected_ranking_digest or expected_ranking_digest != digest(facts):
        raise ValueError("macro ranking report or external pin drifted")
    current = rank_candidates(
        root, value.get("request"), expected_pool_digest=value.get("pool_digest")
    )
    if current != value or current["digest"] != expected_ranking_digest:
        raise ValueError("macro ranking report is not canonical")
    check_deadline()
    return current


def require_ranked_candidate(
    root: Path, reference: object, reviewed: dict, minimum: int, target: str
) -> None:
    if not isinstance(reference, dict) or set(reference) != {
        "path",
        "expected_ranking_digest",
    }:
        raise ValueError(
            "assembly block transaction requires a pinned ranking reference"
        )
    name = validate_repo_path(reference["path"])
    if not name.startswith("out/reviews/evidence/"):
        raise ValueError("macro ranking reference must be under out/reviews/evidence")
    report = json.loads(read_file(root, name), object_pairs_hook=unique_object)
    current = validate_ranking(
        root, report, expected_ranking_digest=reference["expected_ranking_digest"]
    )
    policy = current["request"]["policy"]
    if policy["min_instructions"] != minimum or policy["target"] not in {None, target}:
        raise ValueError("macro ranking discovery policy differs from the transaction")
    if not any(
        row["id"] == reviewed["candidate_id"]
        and row["candidate_fingerprint"] == reviewed["candidate_fingerprint"]
        for row in current["queue"]
    ):
        raise ValueError("macro candidate is outside the pinned top-N selection")
