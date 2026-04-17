from __future__ import annotations

import sqlite3

from ...models.inventory import InventoryFunctionRow, InventoryProgramRow


class ProgramRepository:
    def __init__(self, connection: sqlite3.Connection):
        self.connection = connection

    def upsert_program(self, row: InventoryProgramRow) -> int:
        with self.connection:
            self.connection.execute(
                """
                INSERT INTO programs(program_slug, program_name, program_path, folder, source_hint)
                VALUES(?, ?, ?, ?, ?)
                ON CONFLICT(program_path) DO UPDATE SET
                    program_slug=excluded.program_slug,
                    program_name=excluded.program_name,
                    folder=excluded.folder,
                    source_hint=excluded.source_hint,
                    updated_at=CURRENT_TIMESTAMP
                """,
                (
                    row.program_slug,
                    row.program_name,
                    row.program_path,
                    row.folder,
                    row.source_hint,
                ),
            )
        result = self.connection.execute(
            "SELECT id FROM programs WHERE program_path = ?",
            (row.program_path,),
        ).fetchone()
        if result is None:
            raise RuntimeError(f"failed to persist program: {row.program_path}")
        return int(result["id"])

    def get_program_by_path(self, program_path: str) -> sqlite3.Row | None:
        return self.connection.execute(
            "SELECT * FROM programs WHERE program_path = ?",
            (program_path,),
        ).fetchone()

    def resolve_program_selector(self, *, program_path: str) -> str:
        program_row = self.get_program_by_path(program_path)
        if program_row is None:
            return program_path

        source_hint = (
            None
            if program_row["source_hint"] is None
            else str(program_row["source_hint"])
        )
        if not source_hint or "#" not in source_hint:
            return self._program_folder_selector(
                program_row["folder"],
                program_row["program_name"],
            )

        payload_row = self.connection.execute(
            "SELECT entry_index, load_arg FROM emi_entries WHERE payload_path = ?",
            (source_hint,),
        ).fetchone()
        if payload_row is None or payload_row["load_arg"] is None:
            return self._program_folder_selector(
                program_row["folder"],
                program_row["program_name"],
            )

        folder = str(program_row["folder"] or "").strip("/")
        archive_stem = (
            str(source_hint).rsplit("#", 1)[0].split("/")[-1].rsplit(".", 1)[0]
        )
        entry_index = int(payload_row["entry_index"])
        load_arg = int(payload_row["load_arg"])
        ghidra_program_name = f"{archive_stem}_e{entry_index:02d}_{load_arg:08x}.bin"
        return (
            f"/{folder}/{ghidra_program_name}" if folder else f"/{ghidra_program_name}"
        )

    @staticmethod
    def _program_folder_selector(folder: object, program_name: object) -> str:
        normalized_folder = str(folder or "").strip("/")
        normalized_program_name = str(program_name or "")
        return (
            f"/{normalized_folder}/{normalized_program_name}"
            if normalized_folder
            else f"/{normalized_program_name}"
        )

    def upsert_function(self, row: InventoryFunctionRow) -> int:
        program_id_row = self.connection.execute(
            "SELECT id FROM programs WHERE program_slug = ?",
            (row.program_slug,),
        ).fetchone()
        if program_id_row is None:
            raise KeyError(f"unknown program slug: {row.program_slug}")
        program_id = int(program_id_row["id"])
        with self.connection:
            self.connection.execute(
                """
                INSERT INTO functions(
                    program_id,
                    entry_address,
                    entry_hex,
                    name,
                    signature,
                    body_min,
                    body_max,
                    comment,
                    repeatable_comment,
                    namespace,
                    name_source,
                    is_thunk,
                    source_hint
                )
                VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT(program_id, entry_address) DO UPDATE SET
                    entry_hex=excluded.entry_hex,
                    name=excluded.name,
                    signature=excluded.signature,
                    body_min=excluded.body_min,
                    body_max=excluded.body_max,
                    comment=excluded.comment,
                    repeatable_comment=excluded.repeatable_comment,
                    namespace=excluded.namespace,
                    name_source=excluded.name_source,
                    is_thunk=excluded.is_thunk,
                    source_hint=excluded.source_hint,
                    updated_at=CURRENT_TIMESTAMP
                """,
                (
                    program_id,
                    row.entry_address,
                    row.entry_hex,
                    row.name,
                    row.signature,
                    row.body_min,
                    row.body_max,
                    row.comment,
                    row.repeatable_comment,
                    row.namespace,
                    row.name_source,
                    int(row.is_thunk),
                    row.source_hint,
                ),
            )
        result = self.connection.execute(
            "SELECT id FROM functions WHERE program_id = ? AND entry_address = ?",
            (program_id, row.entry_address),
        ).fetchone()
        if result is None:
            raise RuntimeError(
                f"failed to persist function: {row.program_slug} {row.entry_hex}"
            )
        return int(result["id"])
