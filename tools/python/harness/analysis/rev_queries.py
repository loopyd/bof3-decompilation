"""Cross-target reverse-index query SQL (rev-query command surface).

SQL and its domain context live here, not in the CLI adapter: commands own
parsing and presentation only, the analysis layer owns queries.
"""

from __future__ import annotations

import sqlite3
from typing import Any

from ..domain import FunctionId
from ..domain.layout import parse_splat_layout
from .index import rows
from .naming_readiness import canonical_storage
from .project import prepare_target
from .type_index import (
    type_candidates_payload,
    type_usages_payload,
    types_payload,
)
from .macro_opportunities import macro_opportunities_payload
from .macro_queries import macro_uses_payload, macros_payload
from .near_duplicates import near_duplicates_payload


def known_target(connection: sqlite3.Connection, target: str) -> bool:
    return (
        connection.execute("SELECT 1 FROM targets WHERE id = ?", (target,)).fetchone()
        is not None
    )


def symbol_at(
    connection: sqlite3.Connection, target: str, address: int
) -> tuple[str, str] | None:
    return connection.execute(
        "SELECT name, kind FROM symbols WHERE target_id = ? AND address = ?",
        (target, address),
    ).fetchone()


def describe_payload(
    connection: sqlite3.Connection,
    function: FunctionId,
    *,
    root: Any,
    manifests: dict[str, Any],
    limit: int,
) -> list[dict[str, Any]]:
    manifest = manifests[function.target.value]
    target_spec = prepare_target(root, function.target.value)
    binary = target_spec.binary
    binary_offset = target_spec.binary_offset
    payload_size = binary.stat().st_size - binary_offset
    offset = function.address - manifest.load_address
    mapped = 0 <= offset < payload_size
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    boundary = layout.find_containing_boundary(function.address)
    symbol = symbol_at(connection, function.target.value, function.address)
    sql_limit = -1 if limit == 0 else limit
    return [
        {
            "target": function.target.value,
            "address": f"0x{function.address:08X}",
            "payload": {
                "contained": mapped,
                "payload_offset": f"0x{offset:X}" if mapped else None,
                "file_offset": f"0x{binary_offset + offset:X}" if mapped else None,
                "remaining_bytes": payload_size - offset if mapped else 0,
            },
            "splat": None
            if boundary is None
            else {
                "kind": boundary.kind,
                "start": f"0x{boundary.virtual_start:08X}",
                "end": None
                if boundary.virtual_end is None
                else f"0x{boundary.virtual_end:08X}",
                "name": boundary.name,
            },
            "symbol": None
            if symbol is None
            else {"name": symbol[0], "kind": symbol[1]},
            "storage": (
                None
                if symbol is None or symbol[1] != "data"
                else canonical_storage(root, function.target.value, function.address)
            ),
            "references": rows(
                connection,
                "SELECT printf('0x%08X', source) source, function_id, access_kind, opcode FROM data_references WHERE target_id = ? AND address = ? ORDER BY source LIMIT ?",
                (function.target.value, function.address, sql_limit),
            ),
        }
    ]


def xrefs_payload(
    connection: sqlite3.Connection, function: FunctionId, *, limit: int
) -> list[dict[str, Any]]:
    sql_limit = -1 if limit == 0 else limit
    call_rows = rows(
        connection,
        """SELECT x.target_id,
                  printf('0x%08X', x.source) AS source,
                  printf('0x%08X', x.destination) AS destination,
                  x.kind,
                  f.id AS function_id,
                  NULL AS opcode
             FROM xrefs x
             LEFT JOIN functions f
               ON f.target_id = x.target_id
              AND x.source >= f.address
              AND x.source < f.address + f.size
            WHERE x.target_id = ? AND x.destination = ?
            ORDER BY x.source LIMIT ?""",
        (function.target.value, function.address, sql_limit),
    )
    data_rows = rows(
        connection,
        """SELECT target_id,
                  printf('0x%08X', source) AS source,
                  printf('0x%08X', address) AS destination,
                  access_kind AS kind,
                  function_id,
                  opcode
             FROM data_references
            WHERE target_id = ? AND address = ?
            ORDER BY source LIMIT ?""",
        (function.target.value, function.address, sql_limit),
    )
    return call_rows + data_rows


def owners_payload(
    connection: sqlite3.Connection, function: FunctionId, *, limit: int
) -> list[dict[str, Any]]:
    sql_limit = -1 if limit == 0 else limit
    return rows(
        connection,
        """SELECT target_id,
                  printf('0x%08X', address) AS function_address,
                  CASE WHEN end IS NULL THEN NULL ELSE printf('0x%08X', end) END AS function_end,
                  name,
                  CASE WHEN end IS NULL THEN NULL ELSE end - address END AS size,
                  CASE WHEN address = ? THEN 'entry' ELSE 'contains' END AS match,
                  provenance,
                  confidence,
                  payload_contained
             FROM function_candidates
            WHERE target_id != ?
              AND ((end IS NOT NULL AND ? >= address AND ? < end)
                   OR (end IS NULL AND ? = address))
            ORDER BY CASE WHEN address = ? THEN 0 ELSE 1 END,
                     CASE provenance
                       WHEN 'reviewed_range' THEN 0
                       WHEN 'analyzer_range' THEN 1
                       ELSE 2
                     END,
                     CASE WHEN target_id LIKE 'exe/%' THEN 0 ELSE 1 END,
                     target_id, address
            LIMIT ?""",
        (
            function.address,
            function.target.value,
            function.address,
            function.address,
            function.address,
            function.address,
            sql_limit,
        ),
    )


def symbols_payload(
    connection: sqlite3.Connection, pattern: str | None, *, limit: int
) -> list[dict[str, Any]]:
    sql_limit = -1 if limit == 0 else limit
    pattern = f"%{pattern}%" if pattern else "%"
    return rows(
        connection,
        "SELECT target_id, printf('0x%08X', address) AS address, name, kind FROM symbols WHERE name LIKE ? ORDER BY target_id, address, name LIMIT ?",
        (pattern, sql_limit),
    )


def variables_payload(
    connection: sqlite3.Connection, pattern: str | None, *, limit: int
) -> list[dict[str, Any]]:
    sql_limit = -1 if limit == 0 else limit
    pattern = f"%{pattern}%" if pattern else "%"
    return rows(
        connection,
        "SELECT target_id, printf('0x%08X', address) AS address, name FROM symbols WHERE kind = 'data' AND name LIKE ? ORDER BY target_id, address LIMIT ?",
        (pattern, sql_limit),
    )


def calls_payload(
    connection: sqlite3.Connection, function_id: str, *, limit: int
) -> list[dict[str, Any]]:
    sql_limit = -1 if limit == 0 else limit
    return rows(
        connection,
        "SELECT caller, callee, printf('0x%08X', callsite) AS callsite FROM calls WHERE caller = ? OR callee = ? ORDER BY caller, callsite LIMIT ?",
        (function_id, function_id, sql_limit),
    )


def status_payload(
    connection: sqlite3.Connection,
) -> list[dict[str, Any]]:
    return rows(
        connection,
        "SELECT t.id, t.engine, COUNT(DISTINCT f.id) AS functions, COUNT(DISTINCT s.address) AS symbols FROM targets t LEFT JOIN functions f ON f.target_id = t.id LEFT JOIN symbols s ON s.target_id = t.id GROUP BY t.id ORDER BY t.id",
    )


def duplicates_payload(
    connection: sqlite3.Connection,
    *,
    target: str | None,
    function: str | None,
    unlifted: bool,
    include_trivial: bool,
) -> list[dict[str, Any]]:
    clauses, params = [], []
    if target:
        clauses.append("f.target_id = ?")
        params.append(target)
    if function:
        clauses.append("f.id = ?")
        params.append(function)
    if unlifted:
        clauses.append("dg.unlifted_members > 0")
    if not include_trivial:
        clauses.append("dg.trivial_group = 0")
    where = "WHERE " + " AND ".join(clauses) if clauses else ""
    return rows(
        connection,
        f"""
        SELECT dg.reviewed_sha256 AS hash, dg.reviewed_size AS size,
               dg.members, dg.unlifted_members, dg.targets,
               dg.representative, dg.representative_kind,
               dg.effort_saved_instructions AS remaining_effort_instructions,
               dg.promotion_blockers,
               json_group_array(json_object(
                   'id', dm.function_id,
                   'lift_status', dm.lift_status,
                   'source', dm.source_path,
                   'compiled_symbol', dm.compiled_symbol,
                   'agrees_with_analyzer', json(dm.agrees_with_analyzer)
               )) AS functions,
               json_group_array(
                   CASE WHEN dm.compiled_symbol IS NOT NULL
                        THEN json_object('id', dm.function_id,
                                        'compiled', dm.compiled_symbol)
                   END
               ) FILTER (WHERE dm.compiled_symbol IS NOT NULL)
                   AS compiled
        FROM duplicate_groups dg
        JOIN duplicate_members dm
          ON dm.reviewed_sha256 = dg.reviewed_sha256
         AND dm.reviewed_size = dg.reviewed_size
        JOIN functions f ON f.id = dm.function_id
        {where}
        GROUP BY dg.reviewed_sha256, dg.reviewed_size
        ORDER BY remaining_effort_instructions DESC, dg.members DESC,
                 dg.reviewed_sha256
    """,
        params,
    )


def analyzer_candidates_payload(
    connection: sqlite3.Connection,
    *,
    target: str | None,
    function: str | None,
    unlifted: bool,
) -> list[dict[str, Any]]:
    # Unconfirmed analyzer-equality candidates: (analyzer hash, size)
    # groups that the reviewed identity has not confirmed. Reviewed-
    # range groups are never relabelled here; a group whose members
    # carry a reviewed identity is excluded from this listing.
    clauses, params = [], []
    if target:
        clauses.append("f.target_id = ?")
        params.append(target)
    if function:
        clauses.append("f.id = ?")
        params.append(function)
    if unlifted:
        clauses.append("f.lifted = 0")
    where = "WHERE " + " AND ".join(clauses) if clauses else ""
    return rows(
        connection,
        f"""
        SELECT uc.analyzer_sha256 AS hash, uc.size,
               uc.members, uc.function_ids AS functions,
               COALESCE(json_group_array(
                   CASE WHEN f.compiled_symbol IS NOT NULL
                        THEN json_object('id', f.id,
                                        'compiled', f.compiled_symbol)
                   END
               ) FILTER (WHERE f.compiled_symbol IS NOT NULL), '[]')
                   AS compiled
        FROM unconfirmed_candidates uc
        JOIN functions f
          ON f.analyzer_sha256 = uc.analyzer_sha256
         AND f.size = uc.size
        {where}
        GROUP BY uc.analyzer_sha256, uc.size
        ORDER BY uc.members DESC, uc.analyzer_sha256
    """,
        params,
    )


__all__ = [
    "analyzer_candidates_payload",
    "calls_payload",
    "describe_payload",
    "duplicates_payload",
    "known_target",
    "macro_opportunities_payload",
    "macro_uses_payload",
    "macros_payload",
    "near_duplicates_payload",
    "owners_payload",
    "status_payload",
    "symbol_at",
    "type_candidates_payload",
    "type_usages_payload",
    "types_payload",
    "symbols_payload",
    "variables_payload",
    "xrefs_payload",
]
