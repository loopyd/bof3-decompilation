from __future__ import annotations

import json
import sqlite3
from typing import Any


class MetadataRepository:
    def __init__(self, connection: sqlite3.Connection):
        self.connection = connection

    def upsert_row(
        self,
        *,
        row_key: str,
        program_path: str | None,
        kind: str,
        address_key: str | None,
        address: int | None,
        entry_text: str | None,
        path: str | None,
        name: str | None,
        comment: str | None,
        repeatable_comment: str | None,
        type_spec: str | None,
        source: str | None,
        confidence: str | None,
        tags: list[str] | None = None,
        extra: dict[str, Any] | None = None,
        updated_at: str | None = None,
    ) -> int:
        with self.connection:
            cursor = self.connection.execute(
                """
                INSERT INTO metadata_rows(
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
                )
                VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, COALESCE(?, CURRENT_TIMESTAMP))
                ON CONFLICT(row_key) DO UPDATE SET
                    program_path=excluded.program_path,
                    kind=excluded.kind,
                    address_key=excluded.address_key,
                    address=excluded.address,
                    entry_text=excluded.entry_text,
                    path=excluded.path,
                    name=excluded.name,
                    comment=excluded.comment,
                    repeatable_comment=excluded.repeatable_comment,
                    type_spec=excluded.type_spec,
                    source=excluded.source,
                    confidence=excluded.confidence,
                    extra_json=excluded.extra_json,
                    updated_at=COALESCE(excluded.updated_at, metadata_rows.updated_at)
                RETURNING id
                """,
                (
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
                    json.dumps(extra or {}, sort_keys=True),
                    updated_at,
                ),
            )
            row_id = int(cursor.fetchone()["id"])
            self.connection.execute(
                "DELETE FROM metadata_tags WHERE metadata_row_id = ?",
                (row_id,),
            )
            for tag in tags or []:
                self.connection.execute(
                    "INSERT OR IGNORE INTO metadata_tags(metadata_row_id, tag) VALUES(?, ?)",
                    (row_id, tag),
                )
        return row_id

    def update_defined_row(
        self,
        *,
        row_id: int,
        name: str | None = None,
        comment: str | None = None,
        type_spec: str | None = None,
        source: str | None = None,
        confidence: str | None = None,
        extra: dict[str, Any] | None = None,
    ) -> None:
        row = self.connection.execute(
            "SELECT extra_json FROM metadata_rows WHERE id = ?",
            (row_id,),
        ).fetchone()
        if row is None:
            raise KeyError(f"unknown metadata row id: {row_id}")
        existing_extra: dict[str, Any] = {}
        raw_extra = row["extra_json"]
        if raw_extra:
            try:
                decoded = json.loads(str(raw_extra))
                if isinstance(decoded, dict):
                    existing_extra = decoded
            except json.JSONDecodeError:
                existing_extra = {}
        merged_extra = {**existing_extra, **(extra or {})}
        with self.connection:
            self.connection.execute(
                """
                UPDATE metadata_rows
                SET name = COALESCE(?, name),
                    comment = COALESCE(?, comment),
                    type_spec = COALESCE(?, type_spec),
                    source = COALESCE(?, source),
                    confidence = COALESCE(?, confidence),
                    extra_json = ?,
                    updated_at = CURRENT_TIMESTAMP
                WHERE id = ?
                """,
                (
                    name,
                    comment,
                    type_spec,
                    source,
                    confidence,
                    json.dumps(merged_extra, sort_keys=True),
                    row_id,
                ),
            )
