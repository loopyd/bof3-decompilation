from __future__ import annotations

from dataclasses import dataclass


SCHEMA_VERSION = 6


@dataclass(frozen=True, slots=True)
class SchemaMigration:
    version: int
    name: str
    statements: tuple[str, ...]


FOUNDATION_STATEMENTS = (
    """
    CREATE TABLE IF NOT EXISTS schema_migrations (
        version INTEGER PRIMARY KEY,
        name TEXT NOT NULL,
        applied_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS programs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        program_slug TEXT NOT NULL UNIQUE,
        program_name TEXT NOT NULL,
        program_path TEXT NOT NULL UNIQUE,
        folder TEXT,
        source_hint TEXT,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS functions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        program_id INTEGER NOT NULL,
        entry_address INTEGER NOT NULL,
        entry_hex TEXT NOT NULL,
        name TEXT NOT NULL,
        signature TEXT,
        body_min INTEGER,
        body_max INTEGER,
        comment TEXT,
        repeatable_comment TEXT,
        namespace TEXT,
        name_source TEXT,
        is_thunk INTEGER NOT NULL DEFAULT 0,
        source_hint TEXT,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(program_id) REFERENCES programs(id) ON DELETE CASCADE,
        UNIQUE(program_id, entry_address)
    )
    """,
    "CREATE INDEX IF NOT EXISTS idx_functions_program ON functions(program_id)",
    "CREATE INDEX IF NOT EXISTS idx_functions_name ON functions(name)",
    """
    CREATE TABLE IF NOT EXISTS metadata_rows (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        row_key TEXT UNIQUE,
        program_id INTEGER,
        program_path TEXT,
        kind TEXT NOT NULL,
        address_key TEXT,
        address INTEGER,
        entry_text TEXT,
        path TEXT,
        name TEXT,
        comment TEXT,
        repeatable_comment TEXT,
        type_spec TEXT,
        source TEXT,
        confidence TEXT,
        extra_json TEXT,
        updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(program_id) REFERENCES programs(id) ON DELETE SET NULL
    )
    """,
    "CREATE INDEX IF NOT EXISTS idx_metadata_rows_kind ON metadata_rows(kind)",
    "CREATE INDEX IF NOT EXISTS idx_metadata_rows_program ON metadata_rows(program_id)",
    "CREATE INDEX IF NOT EXISTS idx_metadata_rows_program_path ON metadata_rows(program_path)",
    "CREATE INDEX IF NOT EXISTS idx_metadata_rows_program_address ON metadata_rows(program_path, address_key)",
    """
    CREATE TABLE IF NOT EXISTS metadata_tags (
        metadata_row_id INTEGER NOT NULL,
        tag TEXT NOT NULL,
        PRIMARY KEY(metadata_row_id, tag),
        FOREIGN KEY(metadata_row_id) REFERENCES metadata_rows(id) ON DELETE CASCADE
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS archives (
        archive_id TEXT PRIMARY KEY,
        archive_name TEXT NOT NULL,
        family TEXT NOT NULL,
        emi_path TEXT NOT NULL UNIQUE,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
    )
    """,
    "CREATE INDEX IF NOT EXISTS idx_archives_family ON archives(family)",
    """
    CREATE TABLE IF NOT EXISTS emi_entries (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        archive_id TEXT NOT NULL,
        entry_index INTEGER NOT NULL,
        entry_name TEXT,
        type_id INTEGER,
        load_arg INTEGER,
        size INTEGER NOT NULL,
        first_word INTEGER,
        sha256 TEXT,
        family TEXT NOT NULL,
        payload_path TEXT,
        code_candidate INTEGER NOT NULL DEFAULT 0,
        palette_candidate INTEGER NOT NULL DEFAULT 0,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(archive_id) REFERENCES archives(archive_id) ON DELETE CASCADE,
        UNIQUE(archive_id, entry_index)
    )
    """,
    "CREATE INDEX IF NOT EXISTS idx_emi_entries_family ON emi_entries(family)",
    "CREATE INDEX IF NOT EXISTS idx_emi_entries_sha256 ON emi_entries(sha256)",
    "CREATE INDEX IF NOT EXISTS idx_emi_entries_load_arg ON emi_entries(load_arg)",
    """
    CREATE TABLE IF NOT EXISTS disc_lba_entries (
        lba INTEGER PRIMARY KEY,
        source_path TEXT,
        size INTEGER,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS slot_map (
        slot_index INTEGER PRIMARY KEY,
        lba INTEGER,
        source_path TEXT,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(lba) REFERENCES disc_lba_entries(lba) ON DELETE SET NULL
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS overlay_aliases (
        archive_id TEXT NOT NULL,
        entry_index INTEGER NOT NULL,
        representative_archive_id TEXT NOT NULL,
        representative_entry_index INTEGER NOT NULL,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        PRIMARY KEY(archive_id, entry_index),
        FOREIGN KEY(archive_id, entry_index) REFERENCES emi_entries(archive_id, entry_index) ON DELETE CASCADE,
        FOREIGN KEY(representative_archive_id, representative_entry_index) REFERENCES emi_entries(archive_id, entry_index) ON DELETE CASCADE
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS overlay_entry_tables (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        archive_id TEXT NOT NULL,
        entry_index INTEGER NOT NULL,
        entry_count INTEGER,
        entry_in_range_count INTEGER,
        confidence TEXT,
        payload_path TEXT,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(archive_id, entry_index) REFERENCES emi_entries(archive_id, entry_index) ON DELETE CASCADE,
        UNIQUE(archive_id, entry_index)
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS overlay_entry_points (
        table_id INTEGER NOT NULL,
        table_index INTEGER NOT NULL,
        address INTEGER NOT NULL,
        address_hex TEXT NOT NULL,
        label_name TEXT,
        label_comment TEXT,
        PRIMARY KEY(table_id, table_index),
        FOREIGN KEY(table_id) REFERENCES overlay_entry_tables(id) ON DELETE CASCADE
    )
    """,
    "CREATE INDEX IF NOT EXISTS idx_overlay_entry_points_address ON overlay_entry_points(address)",
    """
    CREATE VIEW IF NOT EXISTS v_function_index AS
    SELECT
        functions.id AS function_id,
        programs.program_slug,
        programs.program_name,
        programs.program_path,
        programs.source_hint AS program_source_hint,
        functions.entry_hex,
        functions.entry_address,
        functions.name,
        functions.signature,
        functions.namespace,
        functions.name_source,
        functions.is_thunk,
        functions.source_hint AS function_source_hint
    FROM functions
    JOIN programs ON programs.id = functions.program_id
    """,
    """
    CREATE VIEW IF NOT EXISTS v_program_summary AS
    SELECT
        programs.id AS program_id,
        programs.program_slug,
        programs.program_name,
        programs.program_path,
        COUNT(functions.id) AS function_count
    FROM programs
    LEFT JOIN functions ON functions.program_id = programs.id
    GROUP BY programs.id, programs.program_slug, programs.program_name, programs.program_path
    """,
    """
    CREATE VIEW IF NOT EXISTS v_overlay_candidates AS
    SELECT
        archive_id,
        entry_index,
        family,
        load_arg,
        size,
        sha256,
        payload_path
    FROM emi_entries
    WHERE code_candidate = 1
    """,
    """
    CREATE VIEW IF NOT EXISTS v_overlay_representatives AS
    SELECT
        alias.archive_id,
        alias.entry_index,
        alias.representative_archive_id,
        alias.representative_entry_index
    FROM overlay_aliases AS alias
    """,
    """
    CREATE VIEW IF NOT EXISTS v_overlay_project_imports AS
    SELECT
        tables.archive_id,
        tables.entry_index,
        entries.family,
        entries.payload_path,
        tables.entry_count,
        tables.entry_in_range_count,
        tables.confidence,
        alias.representative_archive_id,
        alias.representative_entry_index
    FROM overlay_entry_tables AS tables
    JOIN emi_entries AS entries
        ON entries.archive_id = tables.archive_id
       AND entries.entry_index = tables.entry_index
    LEFT JOIN overlay_aliases AS alias
        ON alias.archive_id = tables.archive_id
       AND alias.entry_index = tables.entry_index
    """,
    """
    CREATE VIEW IF NOT EXISTS v_overlay_project_entry_labels AS
    SELECT
        tables.archive_id,
        tables.entry_index,
        tables.confidence,
        points.table_index,
        points.address,
        points.address_hex,
        points.label_name,
        points.label_comment
    FROM overlay_entry_points AS points
    JOIN overlay_entry_tables AS tables ON tables.id = points.table_id
    """,
)


CLEANUP_STATEMENTS = (
    "DROP VIEW IF EXISTS v_owner_run_progress",
    "DROP VIEW IF EXISTS v_owner_recent_failures",
    "DROP VIEW IF EXISTS v_owner_context_hits",
    "DROP VIEW IF EXISTS v_owner_attempt_summary",
    "DROP VIEW IF EXISTS v_owner_pending_items",
    "DROP VIEW IF EXISTS v_clut_candidates",
    "DROP TABLE IF EXISTS owner_work_item_evidence",
    "DROP TABLE IF EXISTS owner_slice_context",
    "DROP TABLE IF EXISTS owner_context_memory",
    "DROP TABLE IF EXISTS owner_work_attempts",
    "DROP TABLE IF EXISTS owner_work_items",
    "DROP TABLE IF EXISTS owner_workflow_programs",
    "DROP TABLE IF EXISTS owner_workflow_runs",
    "DROP TABLE IF EXISTS search_chunks_fts",
    "DROP TABLE IF EXISTS search_chunks",
    "DROP TABLE IF EXISTS render_bundle_archives",
    "DROP TABLE IF EXISTS render_bundles",
    "DROP TABLE IF EXISTS render_archive_entries",
    "DROP TABLE IF EXISTS render_archives",
    "DROP TABLE IF EXISTS render_family_recommended_archives",
    "DROP TABLE IF EXISTS render_family_notes",
    "DROP TABLE IF EXISTS render_families",
    "DROP TABLE IF EXISTS loader_callsites",
    "DROP TABLE IF EXISTS function_decompilations",
    "DROP TABLE IF EXISTS report_registry",
    "DROP TABLE IF EXISTS inventory_runs",
    "DROP TABLE IF EXISTS disk_files",
)


QUERY_FOCUSED_VIEW_STATEMENTS = (
    """
    CREATE VIEW IF NOT EXISTS v_query_functions AS
    SELECT
        functions.id AS function_id,
        programs.program_slug,
        programs.program_name,
        programs.program_path,
        programs.folder,
        programs.source_hint,
        entries.archive_id,
        entries.entry_index,
        entries.family,
        entries.load_arg,
        functions.entry_address,
        functions.entry_hex,
        functions.name,
        functions.signature,
        functions.body_min,
        functions.body_max,
        functions.comment,
        functions.repeatable_comment,
        functions.namespace,
        functions.name_source,
        functions.is_thunk
    FROM functions
    JOIN programs ON programs.id = functions.program_id
    LEFT JOIN emi_entries AS entries
        ON entries.payload_path = programs.source_hint
    """,
    """
    CREATE VIEW IF NOT EXISTS v_query_meaningful_metadata AS
    SELECT
        row_key,
        program_path,
        kind,
        address_key,
        address,
        entry_text,
        path,
        name,
        comment,
        repeatable_comment,
        type_spec,
        source,
        confidence,
        extra_json,
        updated_at
    FROM metadata_rows
    WHERE kind IN ('structure', 'enum', 'typedef', 'function')
       OR COALESCE(comment, '') != ''
       OR COALESCE(repeatable_comment, '') != ''
       OR (kind = 'data' AND COALESCE(type_spec, '') != '')
    """,
    """
    CREATE VIEW IF NOT EXISTS v_query_programs AS
    SELECT
        programs.id AS program_id,
        programs.program_slug,
        programs.program_name,
        programs.program_path,
        programs.folder,
        programs.source_hint,
        entries.archive_id,
        entries.entry_index,
        entries.family,
        entries.load_arg,
        entries.payload_path,
        alias.representative_archive_id,
        alias.representative_entry_index
    FROM programs
    LEFT JOIN emi_entries AS entries
        ON entries.payload_path = programs.source_hint
    LEFT JOIN overlay_aliases AS alias
        ON alias.archive_id = entries.archive_id
       AND alias.entry_index = entries.entry_index
    """,
)


MIGRATIONS: tuple[SchemaMigration, ...] = (
    SchemaMigration(
        version=1,
        name="inventory_foundation",
        statements=FOUNDATION_STATEMENTS,
    ),
    SchemaMigration(
        version=5,
        name="remove_retired_inventory_subsystems",
        statements=CLEANUP_STATEMENTS,
    ),
    SchemaMigration(
        version=6,
        name="add_query_focused_inventory_views",
        statements=QUERY_FOCUSED_VIEW_STATEMENTS,
    ),
)
