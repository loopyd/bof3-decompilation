"""SQLite schema primitives shared by the evidence repository and builder."""

from __future__ import annotations

from pathlib import Path
import sqlite3


TABLES = (
    "targets",
    "containers",
    "entries",
    "functions",
    "symbols",
    "types",
    "fields",
    "values",
    "declarations",
    "calls",
    "unresolved_calls",
    "references",
    "artifacts",
    "fingerprints",
    "duplicate_groups",
    "psyq_versions",
    "psyq_libraries",
    "psyq_members",
    "psyq_functions",
    "psyq_occurrences",
    "edges",
    "evidence",
)


def _table_sql(table: str) -> str:
    return f'"{table}"' if table in {"values", "references"} else table


def connect(path: Path) -> sqlite3.Connection:
    path.parent.mkdir(parents=True, exist_ok=True)
    connection = sqlite3.connect(path)
    connection.row_factory = sqlite3.Row
    connection.execute("PRAGMA foreign_keys = ON")
    return connection


def create_schema(connection: sqlite3.Connection) -> None:
    connection.execute(
        "CREATE TABLE IF NOT EXISTS metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL)"
    )
    definitions = {
        "targets": "id TEXT PRIMARY KEY, kind TEXT NOT NULL, disc_id TEXT NOT NULL, source_dir TEXT, binary TEXT, splat TEXT, load_address INTEGER, profile TEXT",
        "containers": "id TEXT PRIMARY KEY, path TEXT NOT NULL, sha256 BLOB",
        "entries": "id TEXT PRIMARY KEY, container_id TEXT, slot INTEGER, path TEXT, sha256 BLOB, load_address INTEGER, size INTEGER, payload_kind TEXT, code_status TEXT",
        "functions": "id TEXT PRIMARY KEY, target_id TEXT NOT NULL, address INTEGER NOT NULL, source TEXT, source_sha256 BLOB, behavior TEXT",
        "symbols": "id TEXT PRIMARY KEY, name TEXT NOT NULL, address INTEGER, target_id TEXT",
        "types": "id TEXT PRIMARY KEY, name TEXT NOT NULL, layout_hash BLOB",
        "fields": "id TEXT PRIMARY KEY, type_id TEXT, name TEXT, offset INTEGER, field_type TEXT",
        "values": "id TEXT PRIMARY KEY, name TEXT, value TEXT, declaration_id TEXT",
        "declarations": "id TEXT PRIMARY KEY, name TEXT, kind TEXT, source TEXT",
        "calls": "caller_id TEXT, callee_id TEXT, PRIMARY KEY(caller_id, callee_id)",
        "unresolved_calls": "caller_id TEXT NOT NULL, target_address INTEGER NOT NULL, callsite INTEGER, kind TEXT NOT NULL, symbol TEXT, PRIMARY KEY(caller_id, target_address, callsite)",
        "references": "function_id TEXT, symbol_id TEXT, PRIMARY KEY(function_id, symbol_id)",
        "artifacts": "id TEXT PRIMARY KEY, kind TEXT, path TEXT, sha256 BLOB, provenance TEXT",
        "fingerprints": "id TEXT PRIMARY KEY, subject_id TEXT, kind TEXT, value BLOB",
        "duplicate_groups": "id TEXT PRIMARY KEY, kind TEXT, fingerprint BLOB",
        "psyq_versions": "id TEXT PRIMARY KEY, version TEXT, source TEXT, sha256 BLOB",
        "psyq_libraries": "id TEXT PRIMARY KEY, version_id TEXT, name TEXT",
        "psyq_members": "id TEXT PRIMARY KEY, library_id TEXT, path TEXT, sha256 BLOB",
        "psyq_functions": "id TEXT PRIMARY KEY, member_id TEXT, name TEXT, address INTEGER",
        "psyq_occurrences": "id TEXT PRIMARY KEY, psyq_function_id TEXT, function_id TEXT, confidence REAL",
        "edges": "id INTEGER PRIMARY KEY AUTOINCREMENT, source_id TEXT NOT NULL, relation TEXT NOT NULL, target_id TEXT NOT NULL, UNIQUE(source_id, relation, target_id)",
        "evidence": "id TEXT PRIMARY KEY, subject_id TEXT NOT NULL, kind TEXT NOT NULL, strength REAL, detail TEXT",
    }
    for table, definition in definitions.items():
        connection.execute(
            f"CREATE TABLE IF NOT EXISTS {_table_sql(table)} ({definition})"
        )
    connection.execute(
        "CREATE INDEX IF NOT EXISTS idx_edges_source ON edges(source_id)"
    )
    connection.execute(
        "CREATE INDEX IF NOT EXISTS idx_edges_target ON edges(target_id)"
    )
    connection.execute(
        "CREATE INDEX IF NOT EXISTS idx_functions_target ON functions(target_id)"
    )
    connection.execute("CREATE INDEX IF NOT EXISTS idx_entries_hash ON entries(sha256)")
    connection.execute(
        "CREATE INDEX IF NOT EXISTS idx_fingerprints_value ON fingerprints(value)"
    )


def clear(connection: sqlite3.Connection) -> None:
    for table in reversed(TABLES):
        connection.execute(f"DELETE FROM {_table_sql(table)}")
