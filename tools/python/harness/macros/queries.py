"""Provenance-rich reverse-index macro query payloads."""

from __future__ import annotations

import json
import sqlite3
from typing import Any


def _decoded(row: sqlite3.Row) -> dict[str, Any]:
    result = dict(row)
    for key in (
        "parameters",
        "conditional_context",
        "definition_conditions",
        "restrictions",
    ):
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
    definition_ids: list[str] | None = None,
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
    if definition_ids is not None:
        if not definition_ids:
            return []
        clauses.append("id IN (" + ",".join("?" for _ in definition_ids) + ")")
        params.extend(definition_ids)
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


_DEFINITION_COLUMNS = (
    "id, owner_target, name, source_path, source_line, parameters, body, "
    "conditional_context, classification, provenance, restrictions, generated, "
    "candidate_status, source_sha256, diagnostic"
)

_USE_COLUMNS = (
    "u.target_id, u.name, u.source_path, u.source_line, u.source_column, "
    "u.arguments, u.conditional_context, u.use_context, u.function_id, "
    "u.generated, u.candidate_status, u.restrictions, u.definition_id, "
    "d.conditional_context AS definition_conditions, "
    "d.owner_target AS definition_owner, d.source_path AS definition_path, "
    "d.source_line AS definition_line, d.classification"
)


def _file_filter(
    files: list[tuple[str, str]],
    owner_column: str = "owner_target",
    path_column: str = "source_path",
) -> tuple[str, list[str]]:
    """Build an owner/path OR-clause for a bounded definition or use lookup."""
    clauses, params = [], []
    for owner, path in files:
        clauses.append(f"({owner_column} = ? AND {path_column} = ?)")
        params.extend((owner, path))
    return "(" + " OR ".join(clauses) + ")", params


def macros_in_files(
    connection: sqlite3.Connection, files: list[tuple[str, str]]
) -> list[dict[str, Any]]:
    """Load the definitions declared in exactly these owner/path pairs."""
    if not files:
        return []
    clause, params = _file_filter(files)
    return [
        _decoded(row)
        for row in connection.execute(
            f"SELECT {_DEFINITION_COLUMNS} FROM macro_definitions WHERE {clause} "
            "ORDER BY owner_target, name, source_path, source_line",
            params,
        )
    ]


def macro_uses_in_files(
    connection: sqlite3.Connection, files: list[tuple[str, str]]
) -> list[dict[str, Any]]:
    """Load the definition-body references written in exactly these files."""
    if not files:
        return []
    clause, params = _file_filter(files, "u.target_id", "u.source_path")
    return [
        _decoded(row)
        for row in connection.execute(
            f"SELECT {_USE_COLUMNS} FROM macro_uses u "
            "JOIN macro_definitions d ON d.id = u.definition_id "
            f"WHERE u.use_context = 'definition_body' AND {clause} "
            "ORDER BY u.target_id, u.source_path, u.source_line, u.source_column",
            params,
        )
    ]


def macro_uses_payload(
    connection: sqlite3.Connection,
    *,
    target: str | None,
    pattern: str | None,
    limit: int,
    function_ids: list[str] | None = None,
    definition_ids: list[str] | None = None,
    generated: bool | None = None,
) -> list[dict[str, Any]]:
    clauses, params = [], []
    if target:
        clauses.append("u.target_id = ?")
        params.append(target)
    if generated is not None:
        clauses.append("u.generated = ?")
        params.append(1 if generated else 0)
    if definition_ids is not None:
        if not definition_ids:
            return []
        clauses.append(
            "u.definition_id IN (" + ",".join("?" for _ in definition_ids) + ")"
        )
        params.extend(definition_ids)
    if pattern:
        clauses.append("u.name LIKE ?")
        params.append(f"%{pattern}%")
    if function_ids is not None:
        if not function_ids:
            return []
        clauses.append("u.function_id IN (" + ",".join("?" for _ in function_ids) + ")")
        params.extend(function_ids)
    where = "WHERE " + " AND ".join(clauses) if clauses else ""
    params.append(-1 if limit == 0 else limit)
    return [
        _decoded(row)
        for row in connection.execute(
            f"SELECT {_USE_COLUMNS} FROM macro_uses u "
            f"JOIN macro_definitions d ON d.id = u.definition_id {where} "
            "ORDER BY u.target_id, u.name, u.source_path, u.source_line, "
            "u.source_column, d.owner_target, d.source_path, d.source_line LIMIT ?",
            params,
        )
    ]


__all__ = [
    "macro_uses_in_files",
    "macro_uses_payload",
    "macros_in_files",
    "macros_payload",
]
