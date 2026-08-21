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
        CREATE TABLE type_declarations (
            id TEXT PRIMARY KEY,
            target_id TEXT NOT NULL,
            name TEXT NOT NULL,
            kind TEXT NOT NULL,
            tag_name TEXT,
            source_path TEXT NOT NULL,
            provenance TEXT NOT NULL,
            canonical TEXT NOT NULL,
            review_status TEXT NOT NULL,
            byte_size INTEGER,
            byte_alignment INTEGER,
            diagnostic TEXT,
            UNIQUE(target_id, name, kind, source_path)
        );
        CREATE INDEX type_declarations_name
            ON type_declarations(target_id, name);
        CREATE TABLE type_fields (
            declaration_id TEXT NOT NULL REFERENCES type_declarations(id),
            target_id TEXT NOT NULL,
            ordinal INTEGER NOT NULL,
            name TEXT NOT NULL,
            type_name TEXT NOT NULL,
            byte_offset INTEGER,
            byte_width INTEGER,
            array_extent TEXT,
            qualifiers TEXT NOT NULL,
            semantic_status TEXT NOT NULL,
            provenance TEXT NOT NULL,
            PRIMARY KEY(declaration_id, ordinal)
        );
        CREATE TABLE type_usages (
            target_id TEXT NOT NULL REFERENCES targets(id),
            source_path TEXT NOT NULL,
            subject TEXT NOT NULL,
            function_id TEXT,
            type_name TEXT NOT NULL,
            use_kind TEXT NOT NULL,
            storage_kind TEXT,
            provenance TEXT NOT NULL,
            evidence TEXT NOT NULL,
            PRIMARY KEY(target_id, source_path, subject, type_name, use_kind)
        );
        CREATE TABLE type_constraints (
            target_id TEXT NOT NULL REFERENCES targets(id),
            type_name TEXT NOT NULL,
            source_path TEXT NOT NULL,
            field_name TEXT,
            constraint_kind TEXT NOT NULL,
            value TEXT NOT NULL,
            expression TEXT NOT NULL,
            provenance TEXT NOT NULL,
            evidence_class TEXT NOT NULL,
            PRIMARY KEY(target_id, type_name, source_path, constraint_kind, field_name)
        );
        CREATE TABLE type_conflicts (
            target_id TEXT NOT NULL REFERENCES targets(id),
            subject TEXT NOT NULL,
            left_value TEXT NOT NULL,
            right_value TEXT NOT NULL,
            source_path TEXT NOT NULL,
            conflict_kind TEXT NOT NULL,
            PRIMARY KEY(target_id, subject, left_value, right_value, source_path)
        );
        CREATE TABLE type_candidates (
            id TEXT PRIMARY KEY,
            target_id TEXT NOT NULL REFERENCES targets(id),
            address INTEGER NOT NULL,
            end INTEGER,
            kind TEXT NOT NULL,
            evidence_class TEXT NOT NULL,
            width INTEGER,
            signedness TEXT NOT NULL,
            status TEXT NOT NULL,
            representation_status TEXT NOT NULL,
            semantic_status TEXT NOT NULL,
            evidence TEXT NOT NULL,
            blocker TEXT
        );
        CREATE INDEX type_candidates_target_address
            ON type_candidates(target_id, address);
        CREATE TABLE type_input_fingerprints (
            target_id TEXT NOT NULL REFERENCES targets(id),
            source_path TEXT NOT NULL,
            sha256 TEXT NOT NULL,
            input_kind TEXT NOT NULL,
            PRIMARY KEY(target_id, source_path)
        );
        CREATE TABLE macro_definitions (
            id TEXT PRIMARY KEY,
            owner_target TEXT NOT NULL,
            name TEXT NOT NULL,
            source_path TEXT NOT NULL,
            source_line INTEGER NOT NULL,
            parameters TEXT NOT NULL,
            body TEXT NOT NULL,
            conditional_context TEXT NOT NULL,
            classification TEXT NOT NULL,
            provenance TEXT NOT NULL,
            restrictions TEXT NOT NULL,
            generated INTEGER NOT NULL,
            candidate_status TEXT NOT NULL,
            source_sha256 TEXT NOT NULL,
            diagnostic TEXT,
            UNIQUE(owner_target, source_path, source_line, name)
        );
        CREATE INDEX macro_definitions_name
            ON macro_definitions(name, owner_target);
        CREATE TABLE macro_uses (
            target_id TEXT NOT NULL REFERENCES targets(id),
            definition_id TEXT NOT NULL REFERENCES macro_definitions(id),
            name TEXT NOT NULL,
            source_path TEXT NOT NULL,
            source_line INTEGER NOT NULL,
            source_column INTEGER NOT NULL,
            arguments TEXT,
            conditional_context TEXT NOT NULL,
            use_context TEXT NOT NULL,
            function_id TEXT REFERENCES functions(id),
            generated INTEGER NOT NULL,
            candidate_status TEXT NOT NULL,
            restrictions TEXT NOT NULL,
            PRIMARY KEY(target_id, definition_id, source_path, source_line, source_column)
        );
        CREATE INDEX macro_uses_name ON macro_uses(name, target_id);
        CREATE TABLE macro_templates (
            definition_id TEXT PRIMARY KEY REFERENCES macro_definitions(id),
            owner_target TEXT NOT NULL,
            source_path TEXT NOT NULL,
            name TEXT NOT NULL,
            template_kind TEXT NOT NULL,
            wrapper_contract TEXT NOT NULL,
            source_sha256 TEXT NOT NULL
        );
        CREATE TABLE macro_input_fingerprints (
            target_id TEXT NOT NULL REFERENCES targets(id),
            source_path TEXT NOT NULL,
            sha256 TEXT NOT NULL,
            input_kind TEXT NOT NULL,
            owner_target TEXT NOT NULL,
            PRIMARY KEY(target_id, source_path, owner_target)
        );
        """
    )
