"""Provenance-rich reverse-index macro query payloads."""

from __future__ import annotations

import json
import sqlite3
from typing import Any


def _decoded(row: sqlite3.Row) -> dict[str, Any]:
    result = dict(row)
    for key in ("parameters", "conditional_context", "restrictions"):
        if key in result:
            result[key] = json.loads(result[key])
    return result


def macros_payload(
    connection: sqlite3.Connection,
    *,
    target: str | None,
    pattern: str | None,
    classification: str | None,
    limit: int,
) -> list[dict[str, Any]]:
    clauses, params = [], []
    if target:
        clauses.append("(owner_target = ? OR owner_target = '__shared__')")
        params.append(target)
    if pattern:
        clauses.append("name LIKE ?")
        params.append(f"%{pattern}%")
    if classification:
        clauses.append("classification = ?")
        params.append(classification)
    where = "WHERE " + " AND ".join(clauses) if clauses else ""
    params.append(-1 if limit == 0 else limit)
    return [
        _decoded(row)
        for row in connection.execute(
            f"SELECT id, owner_target, name, source_path, source_line, parameters, body, "
            f"conditional_context, classification, provenance, restrictions, generated, "
            f"candidate_status, source_sha256, diagnostic FROM macro_definitions {where} "
            "ORDER BY owner_target, name, source_path, source_line LIMIT ?",
            params,
        )
    ]


def macro_uses_payload(
    connection: sqlite3.Connection,
    *,
    target: str | None,
    pattern: str | None,
    limit: int,
) -> list[dict[str, Any]]:
    clauses, params = [], []
    if target:
        clauses.append("u.target_id = ?")
        params.append(target)
    if pattern:
        clauses.append("u.name LIKE ?")
        params.append(f"%{pattern}%")
    where = "WHERE " + " AND ".join(clauses) if clauses else ""
    params.append(-1 if limit == 0 else limit)
    return [
        _decoded(row)
        for row in connection.execute(
            f"SELECT u.target_id, u.name, u.source_path, u.source_line, u.source_column, "
            f"u.arguments, u.conditional_context, u.use_context, u.function_id, "
            f"u.generated, u.candidate_status, u.restrictions, "
            f"d.owner_target AS definition_owner, d.source_path AS definition_path, "
            f"d.source_line AS definition_line, d.classification FROM macro_uses u "
            f"JOIN macro_definitions d ON d.id = u.definition_id {where} "
            "ORDER BY u.target_id, u.name, u.source_path, u.source_line, "
            "u.source_column, d.owner_target, d.source_path, d.source_line LIMIT ?",
            params,
        )
    ]


__all__ = ["macro_uses_payload", "macros_payload"]
