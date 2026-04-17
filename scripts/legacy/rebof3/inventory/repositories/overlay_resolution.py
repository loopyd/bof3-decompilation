from __future__ import annotations

import sqlite3

from ...models.address_resolution import ResolvedProgramCandidate
from .programs import ProgramRepository


class OverlayResolutionRepository:
    def __init__(self, connection: sqlite3.Connection):
        self.connection = connection
        self.programs = ProgramRepository(connection)

    def get_program_overlay_row(self, program_path: str) -> sqlite3.Row | None:
        program_row = self.programs.get_program_by_path(program_path)
        if program_row is None:
            return None
        source_hint = str(program_row["source_hint"] or "")
        if not source_hint:
            return None
        return self.connection.execute(
            """
            SELECT
                programs.program_path,
                programs.program_name,
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
            JOIN emi_entries AS entries ON entries.payload_path = programs.source_hint
            LEFT JOIN overlay_aliases AS alias
                ON alias.archive_id = entries.archive_id
               AND alias.entry_index = entries.entry_index
            WHERE programs.program_path = ?
            """,
            (program_path,),
        ).fetchone()

    def list_overlay_candidates_by_load_arg(
        self, load_arg: int
    ) -> list[ResolvedProgramCandidate]:
        rows = self.connection.execute(
            """
            SELECT
                programs.program_path,
                entries.archive_id,
                entries.entry_index,
                entries.family,
                entries.load_arg,
                alias.representative_archive_id,
                alias.representative_entry_index
            FROM programs
            JOIN emi_entries AS entries ON entries.payload_path = programs.source_hint
            LEFT JOIN overlay_aliases AS alias
                ON alias.archive_id = entries.archive_id
               AND alias.entry_index = entries.entry_index
            WHERE entries.load_arg = ?
            ORDER BY programs.program_path
            """,
            (load_arg,),
        ).fetchall()
        candidates: list[ResolvedProgramCandidate] = []
        for row in rows:
            program_path = str(row["program_path"])
            candidates.append(
                ResolvedProgramCandidate(
                    program_path=program_path,
                    program_selector=self.programs.resolve_program_selector(
                        program_path=program_path
                    ),
                    archive_id=(
                        None if row["archive_id"] is None else str(row["archive_id"])
                    ),
                    entry_index=(
                        None if row["entry_index"] is None else int(row["entry_index"])
                    ),
                    family=None if row["family"] is None else str(row["family"]),
                    load_arg=(
                        None if row["load_arg"] is None else int(row["load_arg"])
                    ),
                    representative_archive_id=(
                        None
                        if row["representative_archive_id"] is None
                        else str(row["representative_archive_id"])
                    ),
                    representative_entry_index=(
                        None
                        if row["representative_entry_index"] is None
                        else int(row["representative_entry_index"])
                    ),
                )
            )
        return candidates

    def list_entry_label_hits(self, address: int) -> list[sqlite3.Row]:
        return self.connection.execute(
            """
            SELECT *
            FROM v_overlay_project_entry_labels
            WHERE address = ?
            ORDER BY archive_id, entry_index, table_index
            """,
            (address,),
        ).fetchall()

    def find_containing_function(
        self, *, program_path: str, address: int
    ) -> sqlite3.Row | None:
        return self.connection.execute(
            """
            SELECT
                functions.entry_address,
                functions.entry_hex,
                functions.name,
                functions.signature,
                functions.body_min,
                functions.body_max
            FROM functions
            JOIN programs ON programs.id = functions.program_id
            WHERE programs.program_path = ?
              AND functions.body_min IS NOT NULL
              AND functions.body_max IS NOT NULL
              AND functions.body_min <= ?
              AND functions.body_max >= ?
            ORDER BY functions.body_min DESC, functions.entry_address DESC
            LIMIT 1
            """,
            (program_path, address, address),
        ).fetchone()

    def get_exe_candidate(self) -> ResolvedProgramCandidate | None:
        program_path = "/boot/SLUS_004.22"
        row = self.programs.get_program_by_path(program_path)
        if row is None:
            return None
        return ResolvedProgramCandidate(
            program_path=program_path,
            program_selector=self.programs.resolve_program_selector(
                program_path=program_path
            ),
            family="SLUS",
            confidence="high",
            reason="boot executable fallback",
        )
