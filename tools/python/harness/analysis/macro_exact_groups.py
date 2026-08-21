"""Reviewed exact-group macro opportunity leads."""

from __future__ import annotations

import json
import sqlite3
from collections import defaultdict
from typing import Any, Callable


def exact_group_opportunities(
    connection: sqlite3.Connection,
    target: str | None,
    candidate_id: Callable[[str, str], str],
    guards: Callable[..., dict[str, dict[str, str]]],
) -> list[dict[str, Any]]:
    """Return evidence-blocked opportunities from reviewed exact groups."""

    clauses = ["dg.trivial_group = 0"]
    params: list[object] = []
    if target:
        clauses.append("f.target_id = ?")
        params.append(target)
    rows = connection.execute(
        "SELECT dg.reviewed_sha256, dg.reviewed_size, dg.members, dg.targets, "
        "dg.representative, dg.promotion_blockers, dm.function_id, dm.lift_status, "
        "dm.agrees_with_analyzer FROM duplicate_groups dg "
        "JOIN duplicate_members dm ON dm.reviewed_sha256 = dg.reviewed_sha256 "
        "AND dm.reviewed_size = dg.reviewed_size JOIN functions f ON f.id = dm.function_id "
        f"WHERE {' AND '.join(clauses)} ORDER BY dg.reviewed_sha256, dm.function_id",
        params,
    ).fetchall()
    groups: dict[tuple[str, int], list[tuple[Any, ...]]] = defaultdict(list)
    for row in rows:
        groups[(row[0], row[1])].append(row)
    result: list[dict[str, Any]] = []
    for (digest, size), members in groups.items():
        if len(members) < 2:
            continue
        function_members = [
            {
                "function": row[6],
                "lift_status": row[7],
                "agrees_with_analyzer": bool(row[8]),
            }
            for row in members
        ]
        result.append(
            {
                "id": candidate_id("exact_group", f"{digest}:{size}"),
                "kind": "exact_group",
                "status": "blocked",
                "rank": size * len(members),
                "target_scope": target or "cross_target",
                "pattern": digest,
                "members": function_members,
                "evidence": {
                    "reviewed_sha256": digest,
                    "reviewed_size": size,
                    "representative": members[0][4],
                    "targets": members[0][3],
                    "registry_blockers": json.loads(members[0][5]),
                },
                "counterexamples": [
                    {"function": row[6], "kind": "analyzer_boundary_disagreement"}
                    for row in members
                    if not row[8]
                ],
                "semantic_guards": guards(),
                "blockers": [
                    "source_shape_equivalence_unproven",
                    "two_independent_exact_c_members_required",
                    "read_only_analysis_only",
                ],
            }
        )
    return result


__all__ = ["exact_group_opportunities"]
