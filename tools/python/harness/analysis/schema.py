"""SQLite schema for the reverse-engineering index."""

from __future__ import annotations

import sqlite3
from functools import lru_cache

from harness.types.schema import SCHEMA as TYPE_SCHEMA
from harness.macros.schema import SCHEMA as MACRO_SCHEMA

SchemaRows = tuple[tuple[object, ...], ...]


def collect_indexes(connection: sqlite3.Connection) -> dict[str, SchemaRows]:
    """Collect exact stored index definitions, grouped by owning table."""

    indexes: dict[str, list[tuple[object, ...]]] = {}
    for table, *definition in connection.execute(
        'SELECT owner.tbl_name, owner.name, owner.sql, flags."unique", '
        "flags.origin, flags.partial, detail.seqno, detail.cid, detail.name, "
        'detail."desc", detail.coll, detail.key FROM sqlite_master AS owner '
        "LEFT JOIN pragma_index_list(owner.tbl_name) AS flags "
        "ON flags.name = owner.name "
        "LEFT JOIN pragma_index_xinfo(owner.name) AS detail ON 1 "
        "WHERE owner.type = 'index' ORDER BY owner.tbl_name, owner.name, detail.seqno"
    ):
        indexes.setdefault(table, []).append(tuple(definition))
    return {table: tuple(rows) for table, rows in indexes.items()}


@lru_cache(maxsize=1)
def required_schema() -> dict[str, tuple[SchemaRows, SchemaRows, SchemaRows]]:
    """Return columns, foreign keys and indexes from the canonical schema."""

    connection = sqlite3.connect(":memory:")
    try:
        create_schema(connection)
        indexes = collect_indexes(connection)
        tables = [
            row[0]
            for row in connection.execute(
                "SELECT name FROM sqlite_master WHERE type = 'table'"
            )
        ]
        return {
            table: (
                tuple(
                    tuple(row[1:7])
                    for row in connection.execute(f'PRAGMA table_xinfo("{table}")')
                ),
                tuple(
                    tuple(row)
                    for row in connection.execute(f'PRAGMA foreign_key_list("{table}")')
                ),
                indexes.get(table, ()),
            )
            for table in tables
        }
    finally:
        connection.close()


def create_schema(connection: sqlite3.Connection, *, atomic: bool = False) -> None:
    """Create the reverse-index tables and foreign-key enforcement."""
    if not isinstance(atomic, bool):
        raise TypeError("atomic schema option must be boolean")
    if atomic and connection.in_transaction:
        raise ValueError("atomic schema creation requires an idle connection")
    script = (
        """
        CREATE TABLE metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL);
        CREATE TABLE targets (
            id TEXT PRIMARY KEY,
            binary TEXT NOT NULL,
            binary_sha256 TEXT NOT NULL,
            load_address INTEGER NOT NULL,
            engine TEXT NOT NULL,
            engine_version TEXT NOT NULL,
            snapshot TEXT NOT NULL,
            snapshot_sha256 TEXT NOT NULL
        );
        CREATE TABLE selected_map_fingerprints (
            target_id TEXT NOT NULL REFERENCES targets(id) ON DELETE CASCADE,
            source_path TEXT NOT NULL,
            sha256 TEXT NOT NULL,
            PRIMARY KEY (target_id, source_path)
        );
        CREATE TABLE source_fingerprints (
            target_id TEXT NOT NULL REFERENCES targets(id) ON DELETE CASCADE,
            source_path TEXT NOT NULL,
            sha256 TEXT NOT NULL,
            PRIMARY KEY (target_id, source_path)
        );
        CREATE TABLE symbols (
            target_id TEXT NOT NULL REFERENCES targets(id),
            address INTEGER NOT NULL,
            name TEXT NOT NULL,
            kind TEXT NOT NULL,
            PRIMARY KEY (target_id, address),
            UNIQUE (target_id, name)
        );
        CREATE TABLE functions (
            id TEXT PRIMARY KEY,
            target_id TEXT NOT NULL REFERENCES targets(id),
            address INTEGER NOT NULL,
            size INTEGER NOT NULL,
            name TEXT NOT NULL,
            compiled_symbol TEXT,
            analyzer_sha256 TEXT NOT NULL,
            reviewed_sha256 TEXT,
            reviewed_size INTEGER,
            reviewed INTEGER NOT NULL,
            lifted INTEGER NOT NULL,
            source TEXT,
            lift_status TEXT NOT NULL DEFAULT 'unlifted',
            instruction_count INTEGER NOT NULL,
            basic_blocks INTEGER,
            cfg_edges INTEGER,
            cyclomatic_complexity INTEGER,
            loops INTEGER,
            stack_frame INTEGER,
            local_count INTEGER,
            argument_count INTEGER,
            trivial_kind TEXT,
            contains_data INTEGER NOT NULL DEFAULT 0
        );
        CREATE INDEX functions_target_address ON functions(target_id, address);
        CREATE INDEX functions_analyzer_hash ON functions(analyzer_sha256);
        CREATE INDEX functions_reviewed_identity
            ON functions(reviewed_sha256, reviewed_size);
        CREATE TABLE calls (
            caller TEXT NOT NULL REFERENCES functions(id),
            callee TEXT NOT NULL REFERENCES functions(id),
            callsite INTEGER NOT NULL,
            PRIMARY KEY(caller, callee, callsite)
        );
        CREATE TABLE xrefs (
            target_id TEXT NOT NULL REFERENCES targets(id),
            source INTEGER NOT NULL,
            destination INTEGER NOT NULL,
            kind TEXT NOT NULL,
            PRIMARY KEY(target_id, source, destination, kind)
        );
        CREATE TABLE unresolved_calls (
            caller TEXT NOT NULL REFERENCES functions(id),
            target_address INTEGER NOT NULL,
            callsite INTEGER NOT NULL,
            kind TEXT NOT NULL,
            PRIMARY KEY(caller, target_address, callsite, kind)
        );
        CREATE TABLE data_references (
            target_id TEXT NOT NULL REFERENCES targets(id),
            function_id TEXT REFERENCES functions(id),
            source INTEGER NOT NULL,
            address INTEGER NOT NULL,
            symbol TEXT,
            access_kind TEXT NOT NULL,
            opcode TEXT NOT NULL,
            PRIMARY KEY(target_id, function_id, source, address)
        );
        CREATE TABLE function_candidates (
            target_id TEXT NOT NULL REFERENCES targets(id),
            address INTEGER NOT NULL,
            end INTEGER,
            name TEXT,
            provenance TEXT NOT NULL,
            confidence TEXT NOT NULL,
            payload_contained INTEGER NOT NULL,
            PRIMARY KEY(target_id, address, provenance)
        );
        CREATE INDEX function_candidates_range
            ON function_candidates(address, end);
        CREATE TABLE duplicate_groups (
            reviewed_sha256 TEXT NOT NULL,
            reviewed_size INTEGER NOT NULL,
            members INTEGER NOT NULL,
            unlifted_members INTEGER NOT NULL,
            targets INTEGER NOT NULL,
            representative TEXT NOT NULL,
            representative_kind TEXT NOT NULL,
            effort_saved_instructions INTEGER NOT NULL,
            promotion_blockers TEXT NOT NULL DEFAULT '[]',
            trivial_group INTEGER NOT NULL DEFAULT 0,
            PRIMARY KEY (reviewed_sha256, reviewed_size)
        );
        CREATE TABLE duplicate_members (
            reviewed_sha256 TEXT NOT NULL,
            reviewed_size INTEGER NOT NULL,
            function_id TEXT NOT NULL REFERENCES functions(id),
            lift_status TEXT NOT NULL DEFAULT 'unlifted',
            source_path TEXT,
            compiled_symbol TEXT,
            agrees_with_analyzer INTEGER NOT NULL DEFAULT 0,
            PRIMARY KEY (reviewed_sha256, reviewed_size, function_id),
            FOREIGN KEY (reviewed_sha256, reviewed_size)
                REFERENCES duplicate_groups(reviewed_sha256, reviewed_size)
        );
        CREATE TABLE unconfirmed_candidates (
            analyzer_sha256 TEXT NOT NULL,
            size INTEGER NOT NULL,
            members INTEGER NOT NULL,
            function_ids TEXT NOT NULL,
            PRIMARY KEY (analyzer_sha256, size)
        );
        CREATE TABLE psyq_evidence (
            target_id TEXT NOT NULL REFERENCES targets(id),
            address INTEGER NOT NULL,
            name TEXT NOT NULL,
            confidence TEXT NOT NULL,
            evidence TEXT NOT NULL,
            PRIMARY KEY(target_id, address, name)
        );
        """
        + TYPE_SCHEMA
        + MACRO_SCHEMA
    )
    prefix = "PRAGMA foreign_keys = ON;\n"
    if not atomic:
        connection.executescript(prefix + script)
        return
    try:
        connection.executescript(prefix + "BEGIN;\n" + script + "\nCOMMIT;")
    except BaseException:
        if connection.in_transaction:
            connection.execute("ROLLBACK")
        raise
