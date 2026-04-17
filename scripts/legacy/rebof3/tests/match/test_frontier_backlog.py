from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.inventory.db.connection import connect_inventory_database
from scripts.rebof3.inventory.db.migrations import ensure_inventory_schema
from scripts.rebof3.inventory.repositories.programs import ProgramRepository
from scripts.rebof3.match import frontier_backlog as MODULE
from scripts.rebof3.models.inventory import InventoryFunctionRow, InventoryProgramRow


class MatchFrontierBacklogTests(unittest.TestCase):
    def test_build_frontier_backlog_payload_marks_promotable_programs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            db_path = root / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            with connection:
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
                    """
                    INSERT INTO emi_entries(archive_id, entry_index, family, size, load_arg, payload_path, sha256, code_candidate)
                    VALUES (?, ?, ?, ?, ?, ?, ?, 1)
                    """,
                    (
                        "BIN/BATTLE/BATTLE",
                        3,
                        "BATTLE",
                        0x100,
                        0x801D0C00,
                        "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                        "sha-battle-3",
                    ),
                )
                connection.execute(
                    """
                    INSERT INTO overlay_aliases(archive_id, entry_index, representative_archive_id, representative_entry_index)
                    VALUES (?, ?, ?, ?)
                    """,
                    ("BIN/BATTLE/BATTLE", 3, "BIN/BATTLE/BATTLE", 3),
                )
                connection.execute(
                    """
                    INSERT INTO overlay_entry_tables(archive_id, entry_index, entry_count, entry_in_range_count, confidence, payload_path)
                    VALUES (?, ?, ?, ?, ?, ?)
                    """,
                    (
                        "BIN/BATTLE/BATTLE",
                        3,
                        8,
                        8,
                        "high",
                        "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                    ),
                )
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="bins_bin_battle_battle_3_bin",
                    program_name="3.bin",
                    program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                    folder="/bins/BIN/BATTLE/BATTLE",
                    source_hint="build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                )
            )
            connection.close()

            payload = MODULE.build_frontier_backlog_payload(
                inventory_db=db_path,
                match_root=root / "tmp" / "matching",
                source_root=root / "bof3",
                artifact_root=root / "tmp" / "ghidra_decomp",
            )

        self.assertEqual(payload["summary"]["zero_function_programs"], 1)
        self.assertEqual(payload["summary"]["promotable_programs"], 1)
        self.assertEqual(payload["summary"]["load_base_only_programs"], 1)
        self.assertEqual(
            payload["items"][0]["frontier_state"], "promotable_entry_labels"
        )
        self.assertEqual(payload["items"][0]["seed_strategy"], "load_base_only")
        self.assertEqual(
            payload["items"][0]["seed_candidates"][0]["address_hex"], "0x801d0c00"
        )
        self.assertEqual(
            payload["items"][0]["ghidra_program_selector"],
            "/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin",
        )

    def test_build_frontier_backlog_payload_uses_family_load_peer_offsets(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            db_path = root / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            with connection:
                connection.execute(
                    "INSERT INTO archives(archive_id, archive_name, family, emi_path) VALUES (?, ?, ?, ?)",
                    (
                        "BIN/ETC/TESTA",
                        "TESTA",
                        "ETC",
                        "build/extracted/BIN/ETC/TESTA.EMI",
                    ),
                )
                connection.execute(
                    "INSERT INTO archives(archive_id, archive_name, family, emi_path) VALUES (?, ?, ?, ?)",
                    (
                        "BIN/ETC/TESTB",
                        "TESTB",
                        "ETC",
                        "build/extracted/BIN/ETC/TESTB.EMI",
                    ),
                )
                connection.execute(
                    """
                    INSERT INTO emi_entries(archive_id, entry_index, family, size, load_arg, payload_path, sha256, code_candidate)
                    VALUES (?, ?, ?, ?, ?, ?, ?, 1)
                    """,
                    (
                        "BIN/ETC/TESTA",
                        0,
                        "ETC",
                        0x200,
                        0x801D0C00,
                        "build/extracted/BIN/ETC/TESTA.EMI#0",
                        "sha-testa-0",
                    ),
                )
                connection.execute(
                    """
                    INSERT INTO emi_entries(archive_id, entry_index, family, size, load_arg, payload_path, sha256, code_candidate)
                    VALUES (?, ?, ?, ?, ?, ?, ?, 1)
                    """,
                    (
                        "BIN/ETC/TESTB",
                        0,
                        "ETC",
                        0x200,
                        0x801D0C00,
                        "build/extracted/BIN/ETC/TESTB.EMI#0",
                        "sha-testb-0",
                    ),
                )
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="bins_bin_etc_testa_0_bin",
                    program_name="0.bin",
                    program_path="/bins/BIN/ETC/TESTA/0.bin",
                    folder="/bins/BIN/ETC/TESTA",
                    source_hint="build/extracted/BIN/ETC/TESTA.EMI#0",
                )
            )
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="bins_bin_etc_testb_0_bin",
                    program_name="0.bin",
                    program_path="/bins/BIN/ETC/TESTB/0.bin",
                    folder="/bins/BIN/ETC/TESTB",
                    source_hint="build/extracted/BIN/ETC/TESTB.EMI#0",
                )
            )
            repo.upsert_function(
                InventoryFunctionRow(
                    program_slug="bins_bin_etc_testb_0_bin",
                    entry_address=0x801D0C20,
                    entry_hex="0x801d0c20",
                    name="func_801d0c20",
                    signature="void func_801d0c20(void)",
                    source_hint="build/extracted/BIN/ETC/TESTB.EMI#0",
                )
            )
            connection.close()

            payload = MODULE.build_frontier_backlog_payload(
                inventory_db=db_path,
                match_root=root / "tmp" / "matching",
                source_root=root / "bof3",
                artifact_root=root / "tmp" / "ghidra_decomp",
            )

        self.assertEqual(payload["summary"]["zero_function_programs"], 1)
        self.assertEqual(payload["summary"]["family_load_seed_programs"], 1)
        self.assertEqual(
            payload["items"][0]["program_path"], "/bins/BIN/ETC/TESTA/0.bin"
        )
        self.assertEqual(
            payload["items"][0]["seed_strategy"], "family_load_peer_offsets"
        )
        self.assertEqual(
            payload["items"][0]["seed_candidates"][0]["address_hex"], "0x801d0c20"
        )

    def test_main_writes_outputs_and_logs_summary(self) -> None:
        class Logger:
            def __init__(self) -> None:
                self.messages: list[str] = []

            def summary(self, message: str) -> None:
                self.messages.append(message)

            def item(self, message: str) -> None:
                self.messages.append(message)

        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            output_json = (
                root
                / "tmp"
                / "matching"
                / "_reports"
                / "frontier_backlog_capcom97_bof3.json"
            )
            output_tsv = (
                root
                / "tmp"
                / "matching"
                / "_reports"
                / "frontier_backlog_capcom97_bof3.tsv"
            )
            inventory_db.write_text("placeholder", encoding="utf-8")
            logger = Logger()
            payload = {
                "summary": {"zero_function_programs": 1, "promotable_programs": 1},
                "items": [
                    {
                        "queue_rank": 1,
                        "family": "ETC",
                        "lane": "system_script",
                        "program_path": "/bins/BIN/ETC/GAME/0.bin",
                        "archive_id": "BIN/ETC/GAME",
                        "entry_index": 0,
                        "frontier_state": "promotable_entry_labels",
                        "seed_strategy": "load_base_only",
                        "seed_count": 1,
                        "seed_candidates": [{"address_hex": "0x801d0c00"}],
                        "entry_table_confidence": "high",
                        "duplicate_group_size": 1,
                        "ghidra_program_selector": "/bins/BIN/ETC/GAME/GAME_e00_801d0c00.bin",
                    }
                ],
            }

            with (
                mock.patch.object(MODULE, "logger_from_args", return_value=logger),
                mock.patch.object(
                    MODULE,
                    "build_frontier_backlog_payload",
                    return_value=payload,
                ) as build_payload,
            ):
                result = MODULE.main(
                    [
                        "--inventory-db",
                        str(inventory_db),
                        "--match-root",
                        str(root / "tmp" / "matching"),
                        "--source-root",
                        str(root / "bof3"),
                        "--artifact-root",
                        str(root / "tmp" / "ghidra_decomp"),
                    ]
                )

            self.assertEqual(result, 0)
            build_payload.assert_called_once_with(
                inventory_db=inventory_db,
                match_root=root / "tmp" / "matching",
                source_root=root / "bof3",
                artifact_root=root / "tmp" / "ghidra_decomp",
            )
            self.assertTrue(
                any(
                    "zero_function_programs=1" in message for message in logger.messages
                )
            )
            self.assertTrue(output_json.exists())
            self.assertTrue(output_tsv.exists())


if __name__ == "__main__":
    unittest.main()
