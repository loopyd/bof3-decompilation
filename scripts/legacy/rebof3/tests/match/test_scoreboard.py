from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.inventory.db.connection import connect_inventory_database
from scripts.rebof3.inventory.db.migrations import ensure_inventory_schema
from scripts.rebof3.inventory.repositories.programs import ProgramRepository
from scripts.rebof3.match import scoreboard as MODULE
from scripts.rebof3.models.inventory import InventoryFunctionRow, InventoryProgramRow


class MatchScoreboardTests(unittest.TestCase):
    def seed_overlay_entry(
        self,
        connection,
        *,
        archive_id: str,
        archive_name: str,
        family: str,
        emi_path: str,
        entry_index: int,
        payload_path: str,
        representative_archive_id: str,
        representative_entry_index: int,
        duplicate_group_size: int,
    ) -> None:
        with connection:
            connection.execute(
                "INSERT INTO archives(archive_id, archive_name, family, emi_path) VALUES(?, ?, ?, ?)",
                (archive_id, archive_name, family, emi_path),
            )
            connection.execute(
                """
                INSERT INTO emi_entries(
                    archive_id,
                    entry_index,
                    family,
                    size,
                    payload_path,
                    sha256,
                    code_candidate
                ) VALUES(?, ?, ?, ?, ?, ?, 1)
                """,
                (
                    archive_id,
                    entry_index,
                    family,
                    0x100,
                    payload_path,
                    f"sha-{archive_id}-{entry_index}",
                ),
            )
            connection.execute(
                """
                INSERT INTO overlay_aliases(
                    archive_id,
                    entry_index,
                    representative_archive_id,
                    representative_entry_index
                ) VALUES(?, ?, ?, ?)
                """,
                (
                    archive_id,
                    entry_index,
                    representative_archive_id,
                    representative_entry_index,
                ),
            )
            if duplicate_group_size > 0:
                connection.execute(
                    """
                    INSERT OR REPLACE INTO overlay_entry_tables(
                        archive_id,
                        entry_index,
                        entry_count,
                        entry_in_range_count,
                        confidence,
                        payload_path
                    ) VALUES(?, ?, ?, ?, ?, ?)
                    """,
                    (
                        archive_id,
                        entry_index,
                        duplicate_group_size,
                        duplicate_group_size,
                        "high",
                        payload_path,
                    ),
                )

    def test_build_scoreboard_payload_reports_missing_program_rows(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            db_path = root / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            self.seed_overlay_entry(
                connection,
                archive_id="BIN/ETC/GAME",
                archive_name="GAME",
                family="ETC",
                emi_path="build/extracted/BIN/ETC/GAME.EMI",
                entry_index=0,
                payload_path="build/extracted/BIN/ETC/GAME.EMI#0",
                representative_archive_id="BIN/ETC/GAME",
                representative_entry_index=0,
                duplicate_group_size=2,
            )
            self.seed_overlay_entry(
                connection,
                archive_id="BIN/ETC/COMMU00",
                archive_name="COMMU00",
                family="ETC",
                emi_path="build/extracted/BIN/ETC/COMMU00.EMI",
                entry_index=0,
                payload_path="build/extracted/BIN/ETC/COMMU00.EMI#0",
                representative_archive_id="BIN/ETC/GAME",
                representative_entry_index=0,
                duplicate_group_size=2,
            )
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="bins_bin_etc_game_0_bin",
                    program_name="0.bin",
                    program_path="/bins/BIN/ETC/GAME/0.bin",
                    folder="/bins/BIN/ETC/GAME",
                    source_hint="build/extracted/BIN/ETC/GAME.EMI#0",
                )
            )
            repo.upsert_function(
                InventoryFunctionRow(
                    program_slug="bins_bin_etc_game_0_bin",
                    entry_address=0x80196F78,
                    entry_hex="0x80196f78",
                    name="func_80196f78",
                    signature="void func_80196f78(void)",
                    source_hint="build/extracted/BIN/ETC/GAME.EMI#0",
                )
            )
            connection.close()

            source_root = root / "bof3"
            source_file = (
                source_root / "src" / "modules" / "game" / "00" / "func_80196f78.c"
            )
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "void func_80196f78(void)\n{\n}\n",
                encoding="utf-8",
            )

            match_root = root / "tmp" / "matching"
            workspace_dir = match_root / "bins_bin_etc_game_0_bin" / "0x80196f78"
            workspace_dir.mkdir(parents=True, exist_ok=True)
            (workspace_dir / "build.json").write_text(
                json.dumps(
                    {
                        "program_path": "/bins/BIN/ETC/GAME/0.bin",
                        "entry_hex": "0x80196f78",
                        "workspace_dir": str(workspace_dir),
                        "succeeded": True,
                    }
                ),
                encoding="utf-8",
            )
            (workspace_dir / "diff.json").write_text(
                json.dumps(
                    {
                        "program_path": "/bins/BIN/ETC/GAME/0.bin",
                        "entry_hex": "0x80196f78",
                        "workspace_dir": str(workspace_dir),
                        "status": "ready_for_backend_diff",
                        "match_metrics": {"objdiff_match_percent": 100.0},
                    }
                ),
                encoding="utf-8",
            )
            (workspace_dir / "history.jsonl").write_text(
                "\n".join(
                    [
                        json.dumps(
                            {
                                "event": "build",
                                "program_path": "/bins/BIN/ETC/GAME/0.bin",
                                "entry_hex": "0x80196f78",
                            },
                            sort_keys=True,
                        ),
                        json.dumps(
                            {
                                "event": "diff",
                                "program_path": "/bins/BIN/ETC/GAME/0.bin",
                                "entry_hex": "0x80196f78",
                                "match_metrics": {
                                    "objdiff_match_percent": 100.0,
                                    "asm_score": 0.0,
                                },
                            },
                            sort_keys=True,
                        ),
                    ]
                )
                + "\n",
                encoding="utf-8",
            )

            payload = MODULE.build_scoreboard_payload(
                inventory_db=db_path,
                match_root=match_root,
                source_root=source_root,
                artifact_root=root / "tmp" / "ghidra_decomp",
            )

        self.assertEqual(payload["summary"]["code_candidate_entries"], 2)
        self.assertEqual(payload["summary"]["code_entries_missing_programs"], 1)
        self.assertEqual(payload["summary"]["exact_match_functions"], 1)
        self.assertEqual(payload["summary"]["asm_exact_functions"], 1)
        self.assertEqual(payload["summary"]["matched_function_count"], 1)
        self.assertEqual(payload["summary"]["highest_objdiff_match_percent"], 100.0)
        self.assertEqual(payload["summary"]["lowest_objdiff_match_percent"], 100.0)
        self.assertEqual(payload["summary"]["average_objdiff_match_percent"], 100.0)
        self.assertEqual(payload["summary"]["median_objdiff_match_percent"], 100.0)
        self.assertEqual(payload["summary"]["functions_without_source"], 0)
        self.assertEqual(payload["summary"]["source_coverage_percent"], 100.0)
        self.assertEqual(payload["summary"]["attempted_functions"], 1)
        self.assertEqual(payload["summary"]["stalled_functions"], 0)
        self.assertEqual(payload["summary"]["multi_entry_duplicate_groups"], 1)
        self.assertFalse(payload["summary"]["campaign_ready"])
        self.assertTrue(
            any(
                "missing program rows" in message
                for message in payload["summary"]["blocking_issues"]
            )
        )
        self.assertEqual(payload["families"][0]["family"], "ETC")
        self.assertEqual(payload["families"][0]["code_candidate_entries"], 2)
        self.assertEqual(payload["families"][0]["imported_programs"], 1)
        self.assertEqual(payload["families"][0]["exact_match_functions"], 1)
        self.assertEqual(payload["families"][0]["asm_exact_functions"], 1)
        self.assertEqual(payload["families"][0]["matched_function_count"], 1)
        self.assertEqual(payload["families"][0]["average_objdiff_match_percent"], 100.0)
        self.assertEqual(payload["functions"][0]["function_state"], "exact_match")
        self.assertEqual(payload["functions"][0]["asm_score"], 0.0)
        self.assertTrue(payload["functions"][0]["asm_exact"])
        self.assertEqual(payload["functions"][0]["attempt_count"], 2)
        self.assertFalse(payload["functions"][0]["stalled"])
        self.assertEqual(payload["programs"][0]["program_state"], "match_mature")
        self.assertEqual(payload["programs"][0]["asm_exact_functions"], 1)
        self.assertEqual(payload["programs"][0]["matched_function_count"], 1)
        self.assertEqual(payload["programs"][0]["attempted_functions"], 1)
        self.assertEqual(payload["programs"][0]["stalled_functions"], 0)
        self.assertEqual(payload["programs"][0]["functions_without_source"], 0)
        self.assertEqual(payload["programs"][0]["source_coverage_percent"], 100.0)
        self.assertEqual(payload["programs"][0]["average_objdiff_match_percent"], 100.0)

    def test_build_scoreboard_payload_prefers_canonical_slus_rows_over_shadow_aliases(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            db_path = root / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="boot_slus_004_22",
                    program_name="SLUS_004.22",
                    program_path="/boot/SLUS_004.22",
                    folder="/boot",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="boot_slus_004_22_0",
                    program_name="SLUS_004.22.0",
                    program_path="/boot/SLUS_004.22.0",
                    folder="/boot",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            repo.upsert_function(
                InventoryFunctionRow(
                    program_slug="boot_slus_004_22",
                    entry_address=0x8014ECAC,
                    entry_hex="0x8014ecac",
                    name="func_8014ecac",
                    signature="void func_8014ecac(void)",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            repo.upsert_function(
                InventoryFunctionRow(
                    program_slug="boot_slus_004_22_0",
                    entry_address=0x8014ECAC,
                    entry_hex="0x8014ecac",
                    name="func_8014ecac",
                    signature="void func_8014ecac(void)",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            connection.close()

            source_root = root / "bof3"
            source_file = (
                source_root / "src" / "core" / "game_front" / "func_8014ecac.c"
            )
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "void func_8014ecac(void)\n{\n}\n",
                encoding="utf-8",
            )

            match_root = root / "tmp" / "matching"
            workspace_dir = match_root / "boot_slus_004_22_0" / "0x8014ecac"
            workspace_dir.mkdir(parents=True, exist_ok=True)
            (workspace_dir / "build.json").write_text(
                json.dumps(
                    {
                        "program_path": "/boot/SLUS_004.22.0",
                        "entry_hex": "0x8014ecac",
                        "workspace_dir": str(workspace_dir),
                        "succeeded": True,
                    }
                ),
                encoding="utf-8",
            )
            (workspace_dir / "diff.json").write_text(
                json.dumps(
                    {
                        "program_path": "/boot/SLUS_004.22.0",
                        "entry_hex": "0x8014ecac",
                        "workspace_dir": str(workspace_dir),
                        "status": "ready_for_backend_diff",
                        "match_metrics": {
                            "objdiff_match_percent": 100.0,
                            "asm_score": 0.0,
                        },
                    }
                ),
                encoding="utf-8",
            )
            (workspace_dir / "history.jsonl").write_text(
                json.dumps(
                    {
                        "event": "diff",
                        "program_path": "/boot/SLUS_004.22.0",
                        "entry_hex": "0x8014ecac",
                        "match_metrics": {
                            "objdiff_match_percent": 100.0,
                            "asm_score": 0.0,
                        },
                    },
                    sort_keys=True,
                )
                + "\n",
                encoding="utf-8",
            )

            payload = MODULE.build_scoreboard_payload(
                inventory_db=db_path,
                match_root=match_root,
                source_root=source_root,
                artifact_root=root / "tmp" / "ghidra_decomp",
            )

        matches = [
            row for row in payload["functions"] if row.get("entry_hex") == "0x8014ecac"
        ]
        self.assertEqual(len(matches), 1)
        self.assertEqual(matches[0]["program_path"], "/boot/SLUS_004.22")
        self.assertEqual(matches[0]["family"], "SLUS")
        self.assertEqual(matches[0]["function_state"], "exact_match")
        self.assertTrue(matches[0]["build_succeeded"])
        self.assertEqual(matches[0]["diff_status"], "ready_for_backend_diff")

    def test_build_scoreboard_payload_adds_synthetic_slus_function_rows(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            db_path = root / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="boot_slus_004_22",
                    program_name="SLUS_004.22",
                    program_path="/boot/SLUS_004.22",
                    folder="/boot",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            connection.close()

            source_root = root / "bof3"
            source_file = source_root / "src" / "core" / "emi" / "func_8016728c.c"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "void func_8016728c(void)\n{\n}\n",
                encoding="utf-8",
            )

            artifact_root = root / "tmp" / "ghidra_decomp"
            bundle_dir = (
                artifact_root / "build" / "extracted" / "SLUS_004.22" / "0x8016728c"
            )
            bundle_dir.mkdir(parents=True, exist_ok=True)
            (bundle_dir / "func.json").write_text(
                json.dumps(
                    {
                        "requested_address": "0x8016728c",
                        "function": {"entry": "8016728c"},
                    }
                ),
                encoding="utf-8",
            )

            match_root = root / "tmp" / "matching"
            workspace_dir = match_root / "boot_slus_004_22" / "0x8016728c"
            workspace_dir.mkdir(parents=True, exist_ok=True)
            (workspace_dir / "build.json").write_text(
                json.dumps(
                    {
                        "program_path": "/boot/SLUS_004.22",
                        "entry_hex": "0x8016728c",
                        "workspace_dir": str(workspace_dir),
                        "succeeded": True,
                    }
                ),
                encoding="utf-8",
            )
            (workspace_dir / "diff.json").write_text(
                json.dumps(
                    {
                        "program_path": "/boot/SLUS_004.22",
                        "entry_hex": "0x8016728c",
                        "workspace_dir": str(workspace_dir),
                        "status": "ready_for_backend_diff",
                        "match_metrics": {
                            "objdiff_match_percent": 81.91304,
                            "asm_score": 925.0,
                        },
                    }
                ),
                encoding="utf-8",
            )
            (workspace_dir / "history.jsonl").write_text(
                json.dumps(
                    {
                        "event": "diff",
                        "program_path": "/boot/SLUS_004.22",
                        "entry_hex": "0x8016728c",
                        "match_metrics": {
                            "objdiff_match_percent": 81.91304,
                            "asm_score": 925.0,
                        },
                    },
                    sort_keys=True,
                )
                + "\n",
                encoding="utf-8",
            )

            payload = MODULE.build_scoreboard_payload(
                inventory_db=db_path,
                match_root=match_root,
                source_root=source_root,
                artifact_root=artifact_root,
            )

        matches = [
            row for row in payload["functions"] if row.get("entry_hex") == "0x8016728c"
        ]
        self.assertEqual(len(matches), 1)
        self.assertEqual(matches[0]["program_path"], "/boot/SLUS_004.22")
        self.assertEqual(matches[0]["family"], "SLUS")
        self.assertEqual(matches[0]["function_state"], "ready_for_backend_diff")
        self.assertEqual(matches[0]["source_function"], "func_8016728c")
        self.assertTrue(matches[0]["build_succeeded"])
        self.assertEqual(matches[0]["objdiff_match_percent"], 81.91304)

    def test_build_scoreboard_payload_marks_build_failed_functions(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            db_path = root / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            self.seed_overlay_entry(
                connection,
                archive_id="BIN/ETC/GAME",
                archive_name="GAME",
                family="ETC",
                emi_path="build/extracted/BIN/ETC/GAME.EMI",
                entry_index=1,
                payload_path="build/extracted/BIN/ETC/GAME.EMI#1",
                representative_archive_id="BIN/ETC/GAME",
                representative_entry_index=1,
                duplicate_group_size=1,
            )
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="bins_bin_etc_game_1_bin",
                    program_name="1.bin",
                    program_path="/bins/BIN/ETC/GAME/1.bin",
                    folder="/bins/BIN/ETC/GAME",
                    source_hint="build/extracted/BIN/ETC/GAME.EMI#1",
                )
            )
            repo.upsert_function(
                InventoryFunctionRow(
                    program_slug="bins_bin_etc_game_1_bin",
                    entry_address=0x801D104C,
                    entry_hex="0x801d104c",
                    name="func_801d104c",
                    signature="void func_801d104c(void)",
                    source_hint="build/extracted/BIN/ETC/GAME.EMI#1",
                )
            )
            connection.close()

            source_root = root / "bof3"
            source_file = (
                source_root / "src" / "modules" / "game" / "01" / "func_801d104c.c"
            )
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "void func_801d104c(void)\n{\n}\n",
                encoding="utf-8",
            )

            match_root = root / "tmp" / "matching"
            workspace_dir = match_root / "bins_bin_etc_game_1_bin" / "0x801d104c"
            workspace_dir.mkdir(parents=True, exist_ok=True)
            (workspace_dir / "build.json").write_text(
                json.dumps(
                    {
                        "program_path": "/bins/BIN/ETC/GAME/1.bin",
                        "entry_hex": "0x801d104c",
                        "workspace_dir": str(workspace_dir),
                        "succeeded": False,
                    }
                ),
                encoding="utf-8",
            )

            payload = MODULE.build_scoreboard_payload(
                inventory_db=db_path,
                match_root=match_root,
                source_root=source_root,
                artifact_root=root / "tmp" / "ghidra_decomp",
            )

        self.assertEqual(payload["summary"]["build_failed_functions"], 1)
        self.assertEqual(payload["functions"][0]["function_state"], "build_failed")
        self.assertEqual(payload["programs"][0]["program_state"], "partial_coverage")

    def test_build_scoreboard_payload_excludes_reviewed_noncode_entries(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            db_path = root / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            self.seed_overlay_entry(
                connection,
                archive_id="BIN/ETC/FIRST",
                archive_name="FIRST",
                family="ETC",
                emi_path="build/extracted/BIN/ETC/FIRST.EMI",
                entry_index=13,
                payload_path="build/extracted/BIN/ETC/FIRST.EMI#13",
                representative_archive_id="BIN/ETC/FIRST",
                representative_entry_index=13,
                duplicate_group_size=1,
            )
            connection.close()

            payload = MODULE.build_scoreboard_payload(
                inventory_db=db_path,
                match_root=root / "tmp" / "matching",
                source_root=root / "bof3",
                artifact_root=root / "tmp" / "ghidra_decomp",
            )

        self.assertEqual(payload["summary"]["code_entries_missing_programs"], 0)
        self.assertEqual(payload["summary"]["reviewed_non_code_entries"], 1)
        self.assertEqual(payload["entries"][0]["entry_state"], "reviewed_non_code")
        self.assertEqual(
            payload["entries"][0]["review_reason"],
            "title/menu CLUT block, not a code overlay",
        )

    def test_main_refreshes_status_when_requested(self) -> None:
        args = MODULE.argparse.Namespace(
            inventory_db=Path("inventory.sqlite"),
            match_root=Path("tmp/matching"),
            source_root=Path("bof3"),
            artifact_root=Path("tmp/ghidra_decomp"),
            output_json=Path("tmp/scoreboard.json"),
            output_tsv=Path("tmp/scoreboard.tsv"),
            refresh_status=True,
            tracked_output=True,
            quiet=False,
            verbose=False,
        )
        logger = type(
            "Logger",
            (),
            {
                "summary": lambda self, message: None,
                "item": lambda self, message: None,
                "error": lambda self, message: None,
            },
        )()
        payload = {
            "summary": {"campaign_ready": False},
            "entries": [],
            "functions": [],
        }
        with (
            mock.patch.object(MODULE, "parse_args", return_value=args),
            mock.patch.object(MODULE, "logger_from_args", return_value=logger),
            mock.patch.object(Path, "exists", return_value=True),
            mock.patch.object(MODULE, "build_scoreboard_payload", return_value=payload),
            mock.patch.object(MODULE, "write_json_output"),
            mock.patch.object(MODULE, "write_text_output"),
            mock.patch.object(
                MODULE.report_refresh,
                "refresh_status_snapshot",
                return_value=Path("reports/decomp-status/current"),
            ) as refresh_mock,
        ):
            result = MODULE.main()

        self.assertEqual(result, 0)
        refresh_mock.assert_called_once_with(
            profile=MODULE.DEFAULT_PSX_PROFILE,
            tracked_output=True,
            inventory_db=Path("inventory.sqlite"),
            match_root=Path("tmp/matching"),
            source_root=Path("bof3"),
            artifact_root=Path("tmp/ghidra_decomp"),
        )

    def test_build_scoreboard_payload_separates_bin_boot_and_logo_coverage(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            db_path = root / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="boot_slus_004_22",
                    program_name="SLUS_004.22",
                    program_path="/boot/SLUS_004.22",
                    folder="/boot",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="boot_logo_logo_exe",
                    program_name="LOGO.EXE",
                    program_path="/boot/LOGO/LOGO.EXE",
                    folder="/boot/LOGO",
                    source_hint="build/extracted/LOGO/LOGO.EXE",
                )
            )
            repo.upsert_function(
                InventoryFunctionRow(
                    program_slug="boot_slus_004_22",
                    entry_address=0x80010000,
                    entry_hex="0x80010000",
                    name="func_80010000",
                    signature="void func_80010000(void)",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            repo.upsert_function(
                InventoryFunctionRow(
                    program_slug="boot_logo_logo_exe",
                    entry_address=0x80035800,
                    entry_hex="0x80035800",
                    name="func_80035800",
                    signature="void func_80035800(void)",
                    source_hint="build/extracted/LOGO/LOGO.EXE",
                )
            )
            connection.close()

            payload = MODULE.build_scoreboard_payload(
                inventory_db=db_path,
                match_root=root / "tmp" / "matching",
                source_root=root / "bof3",
                artifact_root=root / "tmp" / "ghidra_decomp",
            )

        self.assertEqual(payload["summary"]["programs"], 2)
        self.assertEqual(payload["summary"]["bin_programs"], 0)
        self.assertEqual(payload["summary"]["boot_programs"], 1)
        self.assertEqual(payload["summary"]["logo_programs"], 1)
        self.assertEqual(payload["summary"]["inventory_functions"], 2)
        self.assertEqual(payload["summary"]["boot_functions"], 1)
        self.assertEqual(payload["summary"]["logo_functions"], 1)

    def test_build_scoreboard_payload_tracks_attempts_and_excludes_zero_matches(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            db_path = root / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="boot_slus_004_22",
                    program_name="SLUS_004.22",
                    program_path="/boot/SLUS_004.22",
                    folder="/boot",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            repo.upsert_function(
                InventoryFunctionRow(
                    program_slug="boot_slus_004_22",
                    entry_address=0x80162D00,
                    entry_hex="0x80162d00",
                    name="func_80162d00",
                    signature="void func_80162d00(void)",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            connection.close()

            source_root = root / "bof3"
            source_file = source_root / "src" / "core" / "emi" / "func_80162d00.c"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text("void func_80162d00(void) {}\n", encoding="utf-8")

            workspace_dir = (
                root / "tmp" / "matching" / "boot_slus_004_22" / "0x80162d00"
            )
            workspace_dir.mkdir(parents=True, exist_ok=True)
            (workspace_dir / "build.json").write_text(
                json.dumps(
                    {
                        "program_path": "/boot/SLUS_004.22",
                        "entry_hex": "0x80162d00",
                        "workspace_dir": str(workspace_dir),
                        "succeeded": True,
                    }
                ),
                encoding="utf-8",
            )
            (workspace_dir / "diff.json").write_text(
                json.dumps(
                    {
                        "program_path": "/boot/SLUS_004.22",
                        "entry_hex": "0x80162d00",
                        "workspace_dir": str(workspace_dir),
                        "status": "ready_for_backend_diff",
                        "match_metrics": {"objdiff_match_percent": 0.0, "asm_score": 9},
                    }
                ),
                encoding="utf-8",
            )
            (workspace_dir / "history.jsonl").write_text(
                "\n".join(
                    [
                        json.dumps(
                            {
                                "event": "diff",
                                "program_path": "/boot/SLUS_004.22",
                                "entry_hex": "0x80162d00",
                                "match_metrics": {
                                    "objdiff_match_percent": 0.0,
                                    "asm_score": 12.0,
                                },
                            },
                            sort_keys=True,
                        ),
                        json.dumps(
                            {
                                "event": "diff",
                                "program_path": "/boot/SLUS_004.22",
                                "entry_hex": "0x80162d00",
                                "match_metrics": {
                                    "objdiff_match_percent": 0.0,
                                    "asm_score": 12.0,
                                },
                            },
                            sort_keys=True,
                        ),
                        json.dumps(
                            {
                                "event": "diff",
                                "program_path": "/boot/SLUS_004.22",
                                "entry_hex": "0x80162d00",
                                "match_metrics": {
                                    "objdiff_match_percent": 0.0,
                                    "asm_score": 12.0,
                                },
                            },
                            sort_keys=True,
                        ),
                        json.dumps(
                            {
                                "event": "diff",
                                "program_path": "/boot/SLUS_004.22",
                                "entry_hex": "0x80162d00",
                                "match_metrics": {
                                    "objdiff_match_percent": 0.0,
                                    "asm_score": 12.0,
                                },
                            },
                            sort_keys=True,
                        ),
                    ]
                )
                + "\n",
                encoding="utf-8",
            )

            payload = MODULE.build_scoreboard_payload(
                inventory_db=db_path,
                match_root=root / "tmp" / "matching",
                source_root=source_root,
                artifact_root=root / "tmp" / "ghidra_decomp",
            )

        self.assertEqual(payload["summary"]["diffed_functions"], 1)
        self.assertEqual(payload["summary"]["matched_function_count"], 0)
        self.assertEqual(payload["summary"]["attempted_functions"], 1)
        self.assertEqual(payload["summary"]["stalled_functions"], 1)
        self.assertEqual(payload["functions"][0]["attempt_count"], 4)
        self.assertTrue(payload["functions"][0]["stalled"])
        self.assertEqual(payload["programs"][0]["matched_function_count"], 0)
        self.assertEqual(payload["programs"][0]["attempted_functions"], 1)
        self.assertEqual(payload["programs"][0]["stalled_functions"], 1)


if __name__ == "__main__":
    unittest.main()
