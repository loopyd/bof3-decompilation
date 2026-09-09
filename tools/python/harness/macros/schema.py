"""Reverse-index macros tables and constraints."""

from __future__ import annotations

SCHEMA = """
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
