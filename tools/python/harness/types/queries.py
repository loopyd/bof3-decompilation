"""Provenance-rich reverse-index type query payloads."""

from __future__ import annotations

import json
import sqlite3
from typing import Any

from harness.types.representation import (
    resolve_field_declaration,
    resolve_identity_declaration,
)


def types_payload(
    connection: sqlite3.Connection,
    *,
    target: str | None,
    pattern: str | None,
    untyped: bool,
    limit: int,
    detail: str = "full",
) -> list[dict[str, Any]]:
    clauses, params = [], []
    if target:
        clauses.append("d.target_id = ?")
        params.append(target)
    if pattern:
        clauses.append("d.name LIKE ?")
        params.append(f"%{pattern}%")
    if untyped:
        clauses.append(
            "NOT EXISTS (SELECT 1 FROM type_usages u WHERE u.target_id = d.target_id "
            "AND u.type_name = CASE WHEN d.namespace = 'tag' THEN d.kind || ' ' || d.name "
            "ELSE d.name END AND u.use_kind != 'lexical')"
        )
    where = "WHERE " + " AND ".join(clauses) if clauses else ""
    params.append(-1 if limit == 0 else limit)
    rows = [
        dict(row)
        for row in connection.execute(
            f"SELECT d.* FROM type_declarations d {where} ORDER BY d.target_id, d.name, d.namespace, "
            "d.source_path LIMIT ?",
            params,
        )
    ]
    field_owners = {}
    representations: dict[str, str] = {}
    for row in rows:
        representation = resolve_identity_declaration(
            connection, row["id"], identities=representations
        )
        row["byte_size"] = connection.execute(
            "SELECT byte_size FROM type_declarations WHERE id = ?", (representation,)
        ).fetchone()[0]
        field_owner = resolve_field_declaration(
            connection, row["id"], identities=representations
        )
        field_owners[row["id"]] = field_owner
        row["field_count"] = connection.execute(
            "SELECT COUNT(*) FROM type_fields WHERE declaration_id = ?",
            (field_owner,),
        ).fetchone()[0]
    if detail != "full":
        keys = (
            "id",
            "target_id",
            "name",
            "namespace",
            "complete",
            "alias_target",
            "alias_kind",
            "kind",
            "source_path",
            "provenance",
            "review_status",
            "byte_size",
            "field_count",
            "diagnostic",
        )
        return [
            {
                **{key: row[key] for key in keys},
                "evidence_truncated": True,
                "full_evidence": "rerun with --detail full",
            }
            for row in rows
        ]
    for row in rows:
        row["fields"] = [
            dict(field)
            for field in connection.execute(
                "SELECT ordinal, name, type_name, byte_offset, byte_width, array_extent, "
                "qualifiers, semantic_status, provenance FROM type_fields WHERE declaration_id = ? "
                "ORDER BY ordinal",
                (field_owners[row["id"]],),
            )
        ]
        row["constraints"] = [
            dict(item)
            for item in connection.execute(
                "SELECT field_name, constraint_kind, value, expression, provenance, evidence_class, "
                "source_path, namespace, resolution FROM type_constraints WHERE target_id = ? "
                "AND (declaration_id = ? OR representation_id = ?) "
                "ORDER BY source_path, constraint_kind, field_name",
                (row["target_id"], row["id"], representations[row["id"]]),
            )
        ]
        row["conflicts"] = [
            dict(item)
            for item in connection.execute(
                "SELECT left_value, right_value, source_path, conflict_kind FROM type_conflicts "
                "WHERE target_id = ? AND (declaration_id = ? OR representation_id = ?) ORDER BY source_path",
                (row["target_id"], row["id"], representations[row["id"]]),
            )
        ]
        row["occurrences"] = [
            dict(item)
            for item in connection.execute(
                "SELECT occurrence.*, link.role FROM type_occurrences occurrence "
                "JOIN type_declaration_occurrences link ON link.occurrence_id = occurrence.id "
                "WHERE link.declaration_id = ? ORDER BY occurrence.source_path, occurrence.ordinal",
                (row["id"],),
            )
        ]
    return rows


def type_usages_payload(
    connection: sqlite3.Connection,
    *,
    target: str | None,
    pattern: str | None,
    limit: int,
) -> list[dict[str, Any]]:
    clauses, params = [], []
    if target:
        clauses.append("target_id = ?")
        params.append(target)
    if pattern:
        clauses.append("(type_name LIKE ? OR subject LIKE ?)")
        params.extend((f"%{pattern}%", f"%{pattern}%"))
    where = "WHERE " + " AND ".join(clauses) if clauses else ""
    params.append(-1 if limit == 0 else limit)
    return [
        dict(row)
        for row in connection.execute(
            f"SELECT target_id, source_path, subject, function_id, type_name, use_kind, "
            f"storage_kind, provenance, evidence FROM type_usages {where} ORDER BY target_id, "
            "source_path, subject, type_name, function_id, use_kind LIMIT ?",
            params,
        )
    ]


def type_candidates_payload(
    connection: sqlite3.Connection,
    *,
    target: str | None,
    status: str | None,
    kind: str | None = None,
    limit: int,
) -> list[dict[str, Any]]:
    clauses, params = [], []
    if target:
        clauses.append("target_id = ?")
        params.append(target)
    if status:
        clauses.append("status = ?")
        params.append(status)
    if kind:
        clauses.append("kind = ?")
        params.append(kind)
    where = "WHERE " + " AND ".join(clauses) if clauses else ""
    params.append(-1 if limit == 0 else limit)
    result = [
        dict(row)
        for row in connection.execute(
            f"SELECT id, target_id, printf('0x%08X', address) address, "
            f"CASE WHEN end IS NULL THEN NULL ELSE printf('0x%08X', end) END end, kind, "
            f"evidence_class, width, signedness, status, representation_status, semantic_status, "
            f"evidence, blocker FROM type_candidates "
            f"{where} ORDER BY target_id, address, kind LIMIT ?",
            params,
        )
    ]
    for row in result:
        row["evidence"] = json.loads(row["evidence"])
    return result
