from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.inventory.db.connection import connect_inventory_database
from scripts.rebof3.inventory.db.migrations import ensure_inventory_schema
from scripts.rebof3.inventory.repositories.programs import ProgramRepository
from scripts.rebof3.models.inventory import InventoryFunctionRow, InventoryProgramRow
from scripts.rebof3.re.services.resolver import (
    build_query_plan,
    resolve_address_context,
)


class AddressResolutionTests(unittest.TestCase):
    def seed_db(self, db_path: Path) -> None:
        connection = connect_inventory_database(db_path)
        ensure_inventory_schema(connection)
        programs = ProgramRepository(connection)
        programs.upsert_program(
            InventoryProgramRow(
                program_slug="bins_bin_battle_battle_3_bin",
                program_name="3.bin",
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                folder="/bins/BIN/BATTLE/BATTLE",
                source_hint="build/extracted/BIN/BATTLE/BATTLE.EMI#3",
            )
        )
        programs.upsert_program(
            InventoryProgramRow(
                program_slug="bins_bin_etc_game_1_bin",
                program_name="1.bin",
                program_path="/bins/BIN/ETC/GAME/1.bin",
                folder="/bins/BIN/ETC/GAME",
                source_hint="build/extracted/BIN/ETC/GAME.EMI#1",
            )
        )
        programs.upsert_program(
            InventoryProgramRow(
                program_slug="boot_slus_004_22",
                program_name="SLUS_004.22",
                program_path="/boot/SLUS_004.22",
                folder="/boot",
                source_hint="build/extracted/SLUS_004.22",
            )
        )
        programs.upsert_function(
            InventoryFunctionRow(
                program_slug="bins_bin_battle_battle_3_bin",
                entry_address=0x801D0C04,
                entry_hex="0x801d0c04",
                name="battle_main",
                body_min=0x801D0C04,
                body_max=0x801D0D00,
            )
        )
        connection.execute(
            "INSERT INTO archives(archive_id, archive_name, family, emi_path) VALUES (?, ?, ?, ?)",
            (
                "BIN/BATTLE/BATTLE",
                "BATTLE",
                "BATTLE",
                "build/extracted/BIN/BATTLE/BATTLE.EMI",
            ),
        )
        connection.execute(
            "INSERT INTO archives(archive_id, archive_name, family, emi_path) VALUES (?, ?, ?, ?)",
            ("BIN/ETC/GAME", "GAME", "ETC", "build/extracted/BIN/ETC/GAME.EMI"),
        )
        connection.execute(
            "INSERT INTO emi_entries(archive_id, entry_index, entry_name, type_id, load_arg, size, family, payload_path, code_candidate) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
            (
                "BIN/BATTLE/BATTLE",
                3,
                "3.bin",
                0,
                0x801D0C00,
                4096,
                "BATTLE",
                "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                1,
            ),
        )
        connection.execute(
            "INSERT INTO emi_entries(archive_id, entry_index, entry_name, type_id, load_arg, size, family, payload_path, code_candidate) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
            (
                "BIN/ETC/GAME",
                1,
                "1.bin",
                0,
                0x801D0C00,
                4096,
                "ETC",
                "build/extracted/BIN/ETC/GAME.EMI#1",
                1,
            ),
        )
        connection.execute(
            "INSERT INTO overlay_aliases(archive_id, entry_index, representative_archive_id, representative_entry_index) VALUES (?, ?, ?, ?)",
            ("BIN/BATTLE/BATTLE", 3, "BIN/BATTLE/BATTLE", 3),
        )
        connection.execute(
            "INSERT INTO overlay_aliases(archive_id, entry_index, representative_archive_id, representative_entry_index) VALUES (?, ?, ?, ?)",
            ("BIN/ETC/GAME", 1, "BIN/ETC/GAME", 1),
        )
        connection.commit()
        connection.close()

    def test_resolve_in_program_function(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            resolution = resolve_address_context(
                db_path=db_path,
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                address=0x801D0C04,
                kind="function",
            )

        self.assertEqual(resolution.resolved_kind, "in_program")
        self.assertEqual(resolution.xref_strategy, "direct_program_xrefs")

    def test_resolve_internal_label_inside_function(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            resolution = resolve_address_context(
                db_path=db_path,
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                address=0x801D0C2C,
                kind="label",
            )

        self.assertEqual(resolution.resolved_kind, "internal_label")
        self.assertEqual(
            resolution.xref_strategy, "containing_function_then_exact_refs"
        )

    def test_resolve_shared_region_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            resolution = resolve_address_context(
                db_path=db_path,
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                address=0x801D8000,
                kind="data",
            )

        self.assertEqual(resolution.resolved_kind, "inventory_shared_region_candidate")
        self.assertEqual(resolution.xref_strategy, "ranked_overlay_candidates")
        self.assertEqual(
            resolution.candidate_program_selectors[0],
            "/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin",
        )
        self.assertLessEqual(len(resolution.candidate_program_selectors), 8)
        self.assertTrue(any("same-family" in note for note in resolution.notes))

    def test_shared_region_query_plan_includes_decomp_and_xrefs_per_candidate(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            connection = connect_inventory_database(db_path)
            programs = ProgramRepository(connection)
            programs.upsert_program(
                InventoryProgramRow(
                    program_slug="bins_bin_battle_battle2_3_bin",
                    program_name="3.bin",
                    program_path="/bins/BIN/BATTLE/BATTLE2/3.bin",
                    folder="/bins/BIN/BATTLE/BATTLE2",
                    source_hint="build/extracted/BIN/BATTLE/BATTLE2.EMI#3",
                )
            )
            connection.execute(
                "INSERT INTO archives(archive_id, archive_name, family, emi_path) VALUES (?, ?, ?, ?)",
                (
                    "BIN/BATTLE/BATTLE2",
                    "BATTLE2",
                    "BATTLE",
                    "build/extracted/BIN/BATTLE/BATTLE2.EMI",
                ),
            )
            connection.execute(
                "INSERT INTO emi_entries(archive_id, entry_index, entry_name, type_id, load_arg, size, family, payload_path, code_candidate) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
                (
                    "BIN/BATTLE/BATTLE2",
                    3,
                    "3.bin",
                    0,
                    0x801D0C00,
                    4096,
                    "BATTLE",
                    "build/extracted/BIN/BATTLE/BATTLE2.EMI#3",
                    1,
                ),
            )
            connection.execute(
                "INSERT INTO overlay_aliases(archive_id, entry_index, representative_archive_id, representative_entry_index) VALUES (?, ?, ?, ?)",
                ("BIN/BATTLE/BATTLE2", 3, "BIN/BATTLE/BATTLE2", 3),
            )
            connection.commit()
            connection.close()
            resolution = resolve_address_context(
                db_path=db_path,
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                address=0x801D8000,
                kind="data",
            )

        plan = build_query_plan(resolution)

        self.assertGreaterEqual(len(plan), 14)
        self.assertEqual(plan[0]["tool"], "get-memory-blocks")
        self.assertEqual(plan[1]["tool"], "read-memory")
        self.assertEqual(plan[4]["tool"], "get-data")
        self.assertEqual(plan[-2]["tool"], "find-cross-references")
        self.assertEqual(plan[-1]["tool"], "get-referencers-decompiled")
        self.assertTrue(
            any(item.get("tool") == "get-decompilation" for item in plan),
        )

    def test_resolve_runtime_only_candidate_without_special_address_table(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            resolution = resolve_address_context(
                db_path=db_path,
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                address=0x20000000,
                kind="function",
            )

        self.assertEqual(resolution.resolved_kind, "runtime_only_candidate")
        self.assertEqual(resolution.xref_strategy, "runtime_neighborhood")
        plan = build_query_plan(resolution)
        self.assertEqual(plan[0]["tool"], "get-memory-blocks")
        self.assertEqual(plan[1]["tool"], "read-memory")
        self.assertEqual(plan[4]["tool"], "get-data")
        self.assertEqual(plan[-2]["tool"], "find-cross-references")
        self.assertEqual(plan[-1]["tool"], "get-referencers-decompiled")


if __name__ == "__main__":
    unittest.main()
