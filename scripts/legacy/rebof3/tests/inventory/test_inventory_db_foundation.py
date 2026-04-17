from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.inventory.db.connection import connect_inventory_database
from scripts.rebof3.inventory.db.migrations import ensure_inventory_schema
from scripts.rebof3.inventory.repositories.archives import ArchiveRepository
from scripts.rebof3.inventory.repositories.programs import ProgramRepository
from scripts.rebof3.models.inventory import (
    InventoryArchiveRow,
    InventoryEmiEntryRow,
    InventoryFunctionRow,
    InventoryProgramRow,
)


class InventoryDbFoundationTests(unittest.TestCase):
    def test_schema_bootstrap_creates_core_tables_and_views(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            connection = connect_inventory_database(db_path)

            result = ensure_inventory_schema(connection)

            self.assertEqual(result.current_version, 5)
            table_names = {
                row["name"]
                for row in connection.execute(
                    "SELECT name FROM sqlite_master WHERE type IN ('table', 'view')"
                )
            }
            self.assertIn("programs", table_names)
            self.assertIn("functions", table_names)
            self.assertIn("emi_entries", table_names)
            self.assertIn("v_function_index", table_names)
            connection.close()

    def test_program_and_archive_repositories_upsert_rows(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)

            programs = ProgramRepository(connection)
            archives = ArchiveRepository(connection)

            program_id = programs.upsert_program(
                InventoryProgramRow(
                    program_slug="boot_slus_004_22",
                    program_name="SLUS_004.22",
                    program_path="/boot/SLUS_004.22",
                    folder="/boot",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            function_id = programs.upsert_function(
                InventoryFunctionRow(
                    program_slug="boot_slus_004_22",
                    entry_address=0x80162D00,
                    entry_hex="0x80162d00",
                    name="emi_ready",
                    signature="bool emi_ready(void)",
                    namespace="Global",
                    name_source="USER_DEFINED",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            archives.upsert_archive(
                InventoryArchiveRow(
                    archive_id="BIN/ETC/GAME",
                    archive_name="GAME",
                    family="ETC",
                    emi_path="build/extracted/BIN/ETC/GAME.EMI",
                )
            )
            archives.upsert_entry(
                InventoryEmiEntryRow(
                    archive_id="BIN/ETC/GAME",
                    entry_index=1,
                    size=4096,
                    family="ETC",
                    type_id=0,
                    load_arg=0x801D0C00,
                    sha256="abc123",
                    payload_path="build/extracted/BIN/ETC/GAME.EMI#1",
                    code_candidate=True,
                )
            )

            self.assertGreater(program_id, 0)
            self.assertGreater(function_id, 0)
            function_row = connection.execute(
                "SELECT name FROM v_function_index WHERE program_slug = ?",
                ("boot_slus_004_22",),
            ).fetchone()
            self.assertIsNotNone(function_row)
            self.assertEqual(function_row["name"], "emi_ready")

            overlay_row = connection.execute(
                "SELECT archive_id, entry_index FROM v_overlay_candidates"
            ).fetchone()
            self.assertIsNotNone(overlay_row)
            self.assertEqual(overlay_row["archive_id"], "BIN/ETC/GAME")
            self.assertEqual(int(overlay_row["entry_index"]), 1)
            connection.close()


if __name__ == "__main__":
    unittest.main()
