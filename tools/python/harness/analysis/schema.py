"""SQLite schema for the reverse-engineering index."""

from __future__ import annotations

import sqlite3


def create_schema(connection: sqlite3.Connection) -> None:
    """Create the reverse-index tables and foreign-key enforcement."""
    connection.executescript(
        """
        PRAGMA foreign_keys = ON;
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
            exact_sha256 TEXT NOT NULL,
            reviewed INTEGER NOT NULL,
            lifted INTEGER NOT NULL,
            source TEXT,
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
        CREATE INDEX functions_hash ON functions(exact_sha256);
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
            address INTEGER NOT NULL,
            symbol TEXT,
            PRIMARY KEY(target_id, function_id, address)
        );
        CREATE TABLE duplicate_groups (
            hash TEXT PRIMARY KEY,
            size INTEGER NOT NULL,
            members INTEGER NOT NULL
        );
        CREATE TABLE duplicate_members (
            hash TEXT NOT NULL REFERENCES duplicate_groups(hash),
            function_id TEXT NOT NULL REFERENCES functions(id),
            PRIMARY KEY(hash, function_id)
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
    )
