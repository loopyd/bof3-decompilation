from __future__ import annotations

import sqlite3

from ...models.inventory import InventoryArchiveRow, InventoryEmiEntryRow


class ArchiveRepository:
    def __init__(self, connection: sqlite3.Connection):
        self.connection = connection

    def upsert_archive(self, row: InventoryArchiveRow) -> None:
        with self.connection:
            self.connection.execute(
                """
                INSERT INTO archives(archive_id, archive_name, family, emi_path)
                VALUES(?, ?, ?, ?)
                ON CONFLICT(archive_id) DO UPDATE SET
                    archive_name=excluded.archive_name,
                    family=excluded.family,
                    emi_path=excluded.emi_path,
                    updated_at=CURRENT_TIMESTAMP
                """,
                (row.archive_id, row.archive_name, row.family, row.emi_path),
            )

    def upsert_entry(self, row: InventoryEmiEntryRow) -> None:
        with self.connection:
            self.connection.execute(
                """
                INSERT INTO emi_entries(
                    archive_id,
                    entry_index,
                    entry_name,
                    type_id,
                    load_arg,
                    size,
                    first_word,
                    sha256,
                    family,
                    payload_path,
                    code_candidate,
                    palette_candidate
                )
                VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT(archive_id, entry_index) DO UPDATE SET
                    entry_name=excluded.entry_name,
                    type_id=excluded.type_id,
                    load_arg=excluded.load_arg,
                    size=excluded.size,
                    first_word=excluded.first_word,
                    sha256=excluded.sha256,
                    family=excluded.family,
                    payload_path=excluded.payload_path,
                    code_candidate=excluded.code_candidate,
                    palette_candidate=excluded.palette_candidate,
                    updated_at=CURRENT_TIMESTAMP
                """,
                (
                    row.archive_id,
                    row.entry_index,
                    row.entry_name,
                    row.type_id,
                    row.load_arg,
                    row.size,
                    row.first_word,
                    row.sha256,
                    row.family,
                    row.payload_path,
                    int(row.code_candidate),
                    int(row.palette_candidate),
                ),
            )
