from __future__ import annotations

import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import import_wave as MODULE
from scripts.rebof3.inventory.db.connection import connect_inventory_database
from scripts.rebof3.inventory.db.migrations import ensure_inventory_schema


class MatchImportWaveTests(unittest.TestCase):
    def test_select_items_filters_by_family_lane_and_limit(self) -> None:
        backlog_payload = {
            "items": [
                {
                    "queue_rank": 1,
                    "family": "BATTLE",
                    "lane": "battle_runtime",
                    "recommended_action": "import_representative",
                    "archive_id": "BIN/BATTLE/BATL_RE2",
                    "entry_index": 1,
                },
                {
                    "queue_rank": 2,
                    "family": "ETC",
                    "lane": "system_script",
                    "recommended_action": "import_member",
                    "archive_id": "BIN/ETC/BATE",
                    "entry_index": 3,
                },
                {
                    "queue_rank": 3,
                    "family": "SCENARIO",
                    "lane": "system_script",
                    "recommended_action": "import_representative",
                    "archive_id": "BIN/SCENARIO/SCENA17",
                    "entry_index": 3,
                },
            ]
        }

        selected = MODULE.select_items(
            backlog_payload,
            families=["ETC", "SCENARIO"],
            lanes=["system_script"],
            recommended_actions=["import_member"],
            limit=1,
            rank_min=2,
            rank_max=None,
        )

        self.assertEqual(len(selected), 1)
        self.assertEqual(selected[0]["archive_id"], "BIN/ETC/BATE")

    def test_canonical_program_path_uses_archive_and_entry_index(self) -> None:
        self.assertEqual(
            MODULE.canonical_program_path(
                {"archive_id": "BIN/BATTLE/BATL_RE2", "entry_index": 1}
            ),
            "/bins/BIN/BATTLE/BATL_RE2/1.bin",
        )

    def test_persist_imported_program_rows_upserts_canonical_programs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            connection.close()

            persisted = MODULE.persist_imported_program_rows(
                db_path=db_path,
                items=[
                    {
                        "archive_id": "BIN/ETC/BATE",
                        "entry_index": 3,
                        "payload_path": "build/extracted/BIN/ETC/BATE.EMI#3",
                    }
                ],
            )

            connection = connect_inventory_database(db_path)
            row = connection.execute(
                "SELECT program_path, program_name, folder, source_hint FROM programs"
            ).fetchone()
            connection.close()

        self.assertEqual(persisted, ["/bins/BIN/ETC/BATE/3.bin"])
        self.assertEqual(row[0], "/bins/BIN/ETC/BATE/3.bin")
        self.assertEqual(row[1], "3.bin")
        self.assertEqual(row[2], "/bins/BIN/ETC/BATE")
        self.assertEqual(row[3], "build/extracted/BIN/ETC/BATE.EMI#3")

    def test_main_writes_dry_run_report_for_selected_items(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "wave.json"
            manifest_out = root / "manifest.json"
            args = MODULE.argparse.Namespace(
                inventory_db=inventory_db,
                match_root=root / "tmp" / "matching",
                source_root=root / "bof3",
                artifact_root=root / "tmp" / "ghidra_decomp",
                project_dir=root / "tmp" / "bof3_ghidra" / "main",
                project_name="bof3_main",
                ghidra_home=root / "ghidra",
                config_mode="isolated",
                family=["ETC"],
                lane=None,
                recommended_action=None,
                limit=4,
                rank_min=None,
                rank_max=None,
                max_cpu=None,
                noanalysis=False,
                restore_metadata=False,
                strict_restore=False,
                manifest_out=manifest_out,
                output_json=output_json,
                log_path=root / "wave.log",
                refresh_reports=False,
                refresh_status=False,
                tracked_output=False,
                dry_run=True,
                quiet=False,
                verbose=False,
            )
            scoreboard_payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
            }
            backlog_payload = {
                "items": [
                    {
                        "queue_rank": 1,
                        "family": "ETC",
                        "lane": "system_script",
                        "recommended_action": "import_representative",
                        "archive_id": "BIN/ETC/BATE",
                        "entry_index": 3,
                        "payload_path": "build/extracted/BIN/ETC/BATE.EMI#3",
                        "suggested_folder": "bins/BIN/ETC/BATE",
                    }
                ]
            }
            logger = type("Logger", (), {"summary": lambda self, message: None})()

            with (
                mock.patch.object(MODULE, "parse_args", return_value=args),
                mock.patch.object(MODULE, "logger_from_args", return_value=logger),
                mock.patch.object(
                    MODULE.scoreboard_lib,
                    "build_scoreboard_payload",
                    return_value=scoreboard_payload,
                ),
                mock.patch.object(
                    MODULE.backlog_lib,
                    "build_import_backlog_payload",
                    return_value=backlog_payload,
                ),
                mock.patch.object(
                    MODULE,
                    "build_manifest_payload",
                    return_value={"imports": [{"payload_path": "x"}], "analyze": True},
                ),
            ):
                result = MODULE.main()

            self.assertEqual(result, 0)
            self.assertTrue(output_json.exists())
            self.assertTrue(manifest_out.exists())
            report = MODULE.json.loads(output_json.read_text(encoding="utf-8"))
            self.assertEqual(report["status"], "planned")
            self.assertEqual(report["selected_count"], 1)

    def test_main_tolerates_metadata_capture_failure_after_import(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "wave.json"
            manifest_out = root / "manifest.json"
            args = MODULE.argparse.Namespace(
                inventory_db=inventory_db,
                match_root=root / "tmp" / "matching",
                source_root=root / "bof3",
                artifact_root=root / "tmp" / "ghidra_decomp",
                project_dir=root / "tmp" / "bof3_ghidra" / "main",
                project_name="bof3_main",
                ghidra_home=root / "ghidra",
                config_mode="isolated",
                family=None,
                lane=None,
                recommended_action=None,
                limit=4,
                rank_min=None,
                rank_max=None,
                max_cpu=None,
                noanalysis=False,
                restore_metadata=False,
                strict_restore=False,
                manifest_out=manifest_out,
                output_json=output_json,
                log_path=root / "wave.log",
                refresh_reports=True,
                refresh_status=False,
                tracked_output=False,
                dry_run=False,
                quiet=False,
                verbose=False,
            )
            scoreboard_payload = {"generated_at": "2026-01-01T00:00:00+00:00"}
            backlog_payload = {
                "items": [
                    {
                        "queue_rank": 1,
                        "family": "ETC",
                        "lane": "system_script",
                        "recommended_action": "import_representative",
                        "archive_id": "BIN/ETC/BATE",
                        "entry_index": 3,
                        "payload_path": "build/extracted/BIN/ETC/BATE.EMI#3",
                        "suggested_folder": "bins/BIN/ETC/BATE",
                    }
                ]
            }
            logger = type(
                "Logger",
                (),
                {
                    "summary": lambda self, message: None,
                    "error": lambda self, message: None,
                },
            )()

            with (
                mock.patch.object(MODULE, "parse_args", return_value=args),
                mock.patch.object(MODULE, "logger_from_args", return_value=logger),
                mock.patch.object(
                    MODULE.scoreboard_lib,
                    "build_scoreboard_payload",
                    return_value=scoreboard_payload,
                ),
                mock.patch.object(
                    MODULE.backlog_lib,
                    "build_import_backlog_payload",
                    return_value=backlog_payload,
                ),
                mock.patch.object(
                    MODULE,
                    "build_manifest_payload",
                    return_value={
                        "imports": [
                            {
                                "project_folder_path": "/bins/BIN/ETC/BATE",
                                "program_name": "BATE_e03_80033a00.bin",
                            }
                        ],
                        "analyze": True,
                    },
                ),
                mock.patch.object(
                    MODULE,
                    "project_busy_message",
                    return_value=None,
                ),
                mock.patch.object(
                    MODULE,
                    "ensure_project_marker",
                    return_value=root
                    / "tmp"
                    / "bof3_ghidra"
                    / "main"
                    / "bof3_main.gpr",
                ),
                mock.patch.object(
                    MODULE,
                    "run_command",
                    return_value=subprocess.CompletedProcess(
                        ["cmd"], 0, stdout="ok", stderr=""
                    ),
                ),
                mock.patch.object(
                    MODULE,
                    "persist_imported_program_rows",
                    return_value=["/bins/BIN/ETC/BATE/3.bin"],
                ),
                mock.patch.object(
                    MODULE,
                    "capture_into_inventory",
                    side_effect=RuntimeError("ghidra metadata capture failed"),
                ),
                mock.patch.object(
                    MODULE,
                    "refresh_reports",
                    return_value={"scoreboard_json": "tmp/scoreboard.json"},
                ),
            ):
                result = MODULE.main()

            self.assertEqual(result, 0)
            report = MODULE.json.loads(output_json.read_text(encoding="utf-8"))
            self.assertEqual(report["status"], "imported_capture_failed")
            self.assertEqual(
                report["metadata_capture"]["error"], "ghidra metadata capture failed"
            )

    def test_main_skips_refresh_when_not_requested(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "wave.json"
            manifest_out = root / "manifest.json"
            args = MODULE.argparse.Namespace(
                inventory_db=inventory_db,
                match_root=root / "tmp" / "matching",
                source_root=root / "bof3",
                artifact_root=root / "tmp" / "ghidra_decomp",
                project_dir=root / "tmp" / "bof3_ghidra" / "main",
                project_name="bof3_main",
                ghidra_home=root / "ghidra",
                config_mode="isolated",
                family=None,
                lane=None,
                recommended_action=None,
                limit=4,
                rank_min=None,
                rank_max=None,
                max_cpu=None,
                noanalysis=False,
                restore_metadata=False,
                strict_restore=False,
                manifest_out=manifest_out,
                output_json=output_json,
                log_path=root / "wave.log",
                refresh_reports=False,
                refresh_status=False,
                tracked_output=False,
                dry_run=False,
                quiet=False,
                verbose=False,
            )
            scoreboard_payload = {"generated_at": "2026-01-01T00:00:00+00:00"}
            backlog_payload = {
                "items": [
                    {
                        "queue_rank": 1,
                        "family": "ETC",
                        "lane": "system_script",
                        "recommended_action": "import_representative",
                        "archive_id": "BIN/ETC/BATE",
                        "entry_index": 3,
                        "payload_path": "build/extracted/BIN/ETC/BATE.EMI#3",
                        "suggested_folder": "bins/BIN/ETC/BATE",
                    }
                ]
            }
            logger = type(
                "Logger",
                (),
                {
                    "summary": lambda self, message: None,
                    "error": lambda self, message: None,
                },
            )()

            with (
                mock.patch.object(MODULE, "parse_args", return_value=args),
                mock.patch.object(MODULE, "logger_from_args", return_value=logger),
                mock.patch.object(
                    MODULE.scoreboard_lib,
                    "build_scoreboard_payload",
                    return_value=scoreboard_payload,
                ),
                mock.patch.object(
                    MODULE.backlog_lib,
                    "build_import_backlog_payload",
                    return_value=backlog_payload,
                ),
                mock.patch.object(
                    MODULE,
                    "build_manifest_payload",
                    return_value={
                        "imports": [
                            {
                                "project_folder_path": "/bins/BIN/ETC/BATE",
                                "program_name": "BATE_e03_80033a00.bin",
                            }
                        ],
                        "analyze": True,
                    },
                ),
                mock.patch.object(
                    MODULE,
                    "project_busy_message",
                    return_value=None,
                ),
                mock.patch.object(
                    MODULE,
                    "ensure_project_marker",
                    return_value=root
                    / "tmp"
                    / "bof3_ghidra"
                    / "main"
                    / "bof3_main.gpr",
                ),
                mock.patch.object(
                    MODULE,
                    "run_command",
                    return_value=subprocess.CompletedProcess(
                        ["cmd"], 0, stdout="ok", stderr=""
                    ),
                ),
                mock.patch.object(
                    MODULE,
                    "persist_imported_program_rows",
                    return_value=["/bins/BIN/ETC/BATE/3.bin"],
                ),
                mock.patch.object(
                    MODULE,
                    "capture_into_inventory",
                    return_value={
                        "canonical_program_count": 1,
                        "row_count": 1,
                        "persisted": {"program_rows": 1},
                    },
                ),
                mock.patch.object(MODULE, "refresh_reports") as refresh_mock,
            ):
                result = MODULE.main()

            self.assertEqual(result, 0)
            refresh_mock.assert_not_called()
            report = MODULE.json.loads(output_json.read_text(encoding="utf-8"))
            self.assertIsNone(report["refreshed_reports"])


if __name__ == "__main__":
    unittest.main()
