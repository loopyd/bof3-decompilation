"""Reviewed duplicate and analyzer-only candidate index groups."""

from __future__ import annotations

import json
import sqlite3
from collections import defaultdict


def insert_duplicate_groups(connection: sqlite3.Connection) -> None:
    rows = connection.execute(
        """SELECT reviewed_sha256, reviewed_size, id, target_id,
                  lift_status, source, analyzer_sha256, size, trivial_kind,
                  compiled_symbol
           FROM functions WHERE reviewed_sha256 IS NOT NULL
           ORDER BY reviewed_sha256, reviewed_size, id"""
    ).fetchall()
    groups: dict[tuple[str, int], list[tuple]] = defaultdict(list)
    for row in rows:
        groups[(row[0], row[1])].append(row)
    for (digest, size), members in groups.items():
        if len(members) < 2:
            continue
        exact = [member for member in members if member[4] == "exact"]
        representative = (exact or members)[0]
        unlifted = sum(member[4] == "unlifted" for member in members)
        # Shared templates save lift effort, not runtime instructions: a
        # reusable representative removes one lift per still-unlifted member.
        effort = unlifted if exact else 0
        blockers = [] if exact else ["no exact member"]
        # Default-excluded from `rev-query duplicates` unless every member
        # is a classified return-only stub.
        trivial = int(all(member[8] is not None for member in members))
        connection.execute(
            "INSERT INTO duplicate_groups VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
            (
                digest,
                size,
                len(members),
                unlifted,
                len({member[3] for member in members}),
                representative[2],
                "reusable" if exact else "analysis_representative",
                effort,
                json.dumps(blockers),
                trivial,
            ),
        )
        connection.executemany(
            "INSERT INTO duplicate_members VALUES (?, ?, ?, ?, ?, ?, ?)",
            (
                (
                    digest,
                    size,
                    member[2],
                    member[4],
                    member[5],
                    member[9],
                    int(member[6] == digest and member[7] == size),
                )
                for member in members
            ),
        )


def insert_unconfirmed_candidates(connection: sqlite3.Connection) -> None:
    candidates: dict[tuple[str, int], list[str]] = defaultdict(list)
    for digest, size, function_id in connection.execute(
        """SELECT analyzer_sha256, size, id FROM functions
           WHERE reviewed_sha256 IS NULL
           ORDER BY id"""
    ):
        candidates[(digest, size)].append(function_id)
    for (digest, size), members in candidates.items():
        if len(members) > 1:
            connection.execute(
                "INSERT INTO unconfirmed_candidates VALUES (?, ?, ?, ?)",
                (digest, size, len(members), json.dumps(members)),
            )
