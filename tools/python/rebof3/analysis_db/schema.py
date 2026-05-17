"""SQLite schema for the analysis database.

Tables: programs, functions, symbols, xrefs, call_edges, constants, duplicates.
"""

from __future__ import annotations

import sqlite3

SCHEMA_VERSION = "rebof3-simple.analysis/v1"

CREATE_ALL = """
CREATE TABLE IF NOT EXISTS programs (
    path       TEXT PRIMARY KEY,
    name       TEXT NOT NULL,
    base_addr  INTEGER,
    size       INTEGER,
    sha256     TEXT
);

CREATE TABLE IF NOT EXISTS functions (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    address      TEXT NOT NULL,
    name         TEXT,
    signature    TEXT,
    body_min     TEXT,
    body_max     TEXT,
    program_path TEXT NOT NULL REFERENCES programs(path),
    is_thunk     INTEGER NOT NULL DEFAULT 0,
    name_source  TEXT,
    namespace    TEXT
);

CREATE TABLE IF NOT EXISTS symbols (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    address      TEXT NOT NULL,
    name         TEXT,
    kind         TEXT,
    program_path TEXT NOT NULL REFERENCES programs(path),
    name_source  TEXT
);

CREATE TABLE IF NOT EXISTS xrefs (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    from_address   TEXT NOT NULL,
    to_address     TEXT NOT NULL,
    reference_type TEXT,
    program_path   TEXT NOT NULL REFERENCES programs(path)
);

CREATE TABLE IF NOT EXISTS call_edges (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    from_func    TEXT NOT NULL,
    to_func      TEXT NOT NULL,
    from_program TEXT NOT NULL,
    to_external  INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS constants (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    address      TEXT NOT NULL,
    name         TEXT,
    data_type    TEXT,
    program_path TEXT NOT NULL REFERENCES programs(path),
    xref_count   INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS duplicates (
    sha256        TEXT PRIMARY KEY,
    program_count INTEGER NOT NULL DEFAULT 0,
    entries_json  TEXT NOT NULL DEFAULT '[]'
);

CREATE INDEX IF NOT EXISTS idx_functions_program
    ON functions(program_path, address);
CREATE INDEX IF NOT EXISTS idx_functions_address
    ON functions(address);
CREATE INDEX IF NOT EXISTS idx_symbols_program
    ON symbols(program_path, address);
CREATE INDEX IF NOT EXISTS idx_xrefs_to
    ON xrefs(to_address);
CREATE INDEX IF NOT EXISTS idx_xrefs_from
    ON xrefs(from_address);
CREATE INDEX IF NOT EXISTS idx_xrefs_program
    ON xrefs(program_path, from_address, to_address);
CREATE INDEX IF NOT EXISTS idx_call_edges_from
    ON call_edges(from_func);
CREATE INDEX IF NOT EXISTS idx_call_edges_to
    ON call_edges(to_func);
CREATE INDEX IF NOT EXISTS idx_call_edges_program
    ON call_edges(from_program);
CREATE INDEX IF NOT EXISTS idx_constants_address
    ON constants(address);
CREATE INDEX IF NOT EXISTS idx_constants_program
    ON constants(program_path);
CREATE INDEX IF NOT EXISTS idx_constants_xrefs
    ON constants(xref_count);
"""

def init_db(conn: sqlite3.Connection) -> None:
    conn.executescript(CREATE_ALL)
    conn.execute(
        """
        CREATE TABLE IF NOT EXISTS metadata (
            key   TEXT PRIMARY KEY,
            value TEXT NOT NULL
        )
        """
    )
    conn.execute(
        "INSERT OR REPLACE INTO metadata(key, value) VALUES(?, ?)",
        ("schema", SCHEMA_VERSION),
    )
