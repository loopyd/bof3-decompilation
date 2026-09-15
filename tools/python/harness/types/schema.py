"""Reverse-index types tables and constraints."""

from __future__ import annotations

SCHEMA = """
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
            function_id TEXT REFERENCES functions(id),
            type_name TEXT NOT NULL,
            use_kind TEXT NOT NULL,
            storage_kind TEXT,
            provenance TEXT NOT NULL,
            evidence TEXT NOT NULL,
            PRIMARY KEY(target_id, source_path, subject, type_name, use_kind, function_id)
        );
        CREATE UNIQUE INDEX type_usages_context
            ON type_usages(target_id, source_path, subject, type_name, use_kind)
            WHERE function_id IS NULL;
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
"""
