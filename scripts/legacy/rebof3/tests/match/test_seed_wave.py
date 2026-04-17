from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import seed_wave as MODULE


class MatchSeedWaveTests(unittest.TestCase):
    def test_select_items_uses_safe_default_seed_strategies(self) -> None:
        backlog_payload = {
            "items": [
                {
                    "queue_rank": 1,
                    "frontier_state": "manual_frontier",
                    "seed_strategy": "duplicate_peer_offsets",
                    "family": "ETC",
                    "lane": "system_script",
                    "ghidra_program_selector": "/bins/BIN/ETC/MTEST/MTEST_e00_801d0c00.bin",
                    "seed_candidates": [{"address_hex": "0x801d1250"}],
                },
                {
                    "queue_rank": 2,
                    "frontier_state": "manual_frontier",
                    "seed_strategy": "load_base_only",
                    "family": "ETC",
                    "lane": "system_script",
                    "ghidra_program_selector": "/bins/BIN/ETC/BATE/BATE_e03_80033a00.bin",
                    "seed_candidates": [{"address_hex": "0x80033a00"}],
                },
            ]
        }
        selected = MODULE.select_items(
            backlog_payload,
            families=None,
            lanes=None,
            seed_strategies=None,
            limit=8,
            rank_min=None,
            rank_max=None,
            candidate_index=0,
        )
        self.assertEqual(len(selected), 1)
        self.assertEqual(selected[0]["seed_strategy"], "duplicate_peer_offsets")
        self.assertEqual(selected[0]["selected_seed"]["address_hex"], "0x801d1250")

    def test_build_promote_command_targets_function_promote(self) -> None:
        command = MODULE.build_promote_command(
            ghidra_home=Path("/opt/ghidra"),
            project_dir=Path("tmp/bof3_ghidra/main"),
            project_name="bof3_main",
            config_mode="isolated",
            noanalysis=False,
            selector="/bins/BIN/ETC/MTEST/MTEST_e00_801d0c00.bin",
            address_hex="0x801d1250",
        )
        self.assertIn("function", command)
        self.assertIn("promote", command)
        self.assertIn("0x801d1250", command)
        self.assertIn("/bins/BIN/ETC/MTEST/MTEST_e00_801d0c00.bin", command)

    def test_main_writes_dry_run_report(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "seed.json"
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
                seed_strategy=None,
                limit=4,
                rank_min=None,
                rank_max=None,
                candidate_index=0,
                noanalysis=False,
                refresh_reports=True,
                refresh_status=False,
                tracked_output=False,
                output_json=output_json,
                log_path=root / "seed.log",
                dry_run=True,
                quiet=False,
                verbose=False,
            )
            frontier_payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "items": [
                    {
                        "queue_rank": 1,
                        "frontier_state": "manual_frontier",
                        "seed_strategy": "duplicate_peer_offsets",
                        "family": "ETC",
                        "lane": "system_script",
                        "ghidra_program_selector": "/bins/BIN/ETC/MTEST/MTEST_e00_801d0c00.bin",
                        "seed_candidates": [{"address_hex": "0x801d1250"}],
                    }
                ],
            }
            logger = type("Logger", (), {"summary": lambda self, message: None})()
            with (
                mock.patch.object(MODULE, "parse_args", return_value=args),
                mock.patch.object(MODULE, "logger_from_args", return_value=logger),
                mock.patch.object(
                    MODULE.frontier_backlog_lib,
                    "build_frontier_backlog_payload",
                    return_value=frontier_payload,
                ),
                mock.patch.object(
                    MODULE.refresh_lib,
                    "refresh_outputs",
                    return_value={"scoreboard_json": root / "tmp" / "scoreboard.json"},
                ),
            ):
                result = MODULE.main()
            self.assertEqual(result, 0)
            report = json.loads(output_json.read_text(encoding="utf-8"))
            self.assertEqual(report["status"], "planned")
            self.assertEqual(report["selected_count"], 1)

    def test_main_completes_with_capture_failures(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "seed.json"
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
                seed_strategy=None,
                limit=4,
                rank_min=None,
                rank_max=None,
                candidate_index=0,
                noanalysis=False,
                refresh_reports=True,
                refresh_status=False,
                tracked_output=False,
                output_json=output_json,
                log_path=root / "seed.log",
                dry_run=False,
                quiet=False,
                verbose=False,
            )
            frontier_payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "items": [
                    {
                        "program_path": "/bins/BIN/ETC/MTEST/0.bin",
                        "archive_id": "BIN/ETC/MTEST",
                        "entry_index": 0,
                        "queue_rank": 1,
                        "frontier_state": "manual_frontier",
                        "seed_strategy": "duplicate_peer_offsets",
                        "family": "ETC",
                        "lane": "system_script",
                        "ghidra_program_selector": "/bins/BIN/ETC/MTEST/MTEST_e00_801d0c00.bin",
                        "seed_candidates": [{"address_hex": "0x801d1250"}],
                    }
                ],
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
                    MODULE.frontier_backlog_lib,
                    "build_frontier_backlog_payload",
                    return_value=frontier_payload,
                ),
                mock.patch.object(MODULE, "project_busy_message", return_value=None),
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
                    "capture_into_inventory",
                    side_effect=RuntimeError("capture failed"),
                ),
                mock.patch.object(
                    MODULE.refresh_lib,
                    "refresh_outputs",
                    return_value={"scoreboard_json": root / "tmp" / "scoreboard.json"},
                ),
            ):
                result = MODULE.main()
            self.assertEqual(result, 0)
            report = json.loads(output_json.read_text(encoding="utf-8"))
            self.assertEqual(report["status"], "completed_with_failures")
            self.assertEqual(report["promoted_count"], 1)
            self.assertEqual(report["captured_count"], 0)

    def test_main_refreshes_outputs_when_no_items_selected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "seed.json"
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
                seed_strategy=None,
                limit=4,
                rank_min=None,
                rank_max=None,
                candidate_index=0,
                noanalysis=False,
                refresh_reports=True,
                refresh_status=True,
                tracked_output=True,
                output_json=output_json,
                log_path=root / "seed.log",
                dry_run=False,
                quiet=False,
                verbose=False,
            )
            frontier_payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "items": [],
            }
            logger = type("Logger", (), {"summary": lambda self, message: None})()
            with (
                mock.patch.object(MODULE, "parse_args", return_value=args),
                mock.patch.object(MODULE, "logger_from_args", return_value=logger),
                mock.patch.object(
                    MODULE.frontier_backlog_lib,
                    "build_frontier_backlog_payload",
                    return_value=frontier_payload,
                ),
                mock.patch.object(
                    MODULE.refresh_lib,
                    "refresh_outputs",
                    return_value={"status_root": root / "reports" / "decomp-status"},
                ) as refresh_mock,
            ):
                result = MODULE.main()
            self.assertEqual(result, 0)
            refresh_mock.assert_called_once()
            report = json.loads(output_json.read_text(encoding="utf-8"))
            self.assertEqual(report["status"], "no_items_selected")
            self.assertEqual(
                report["refreshed_reports"]["status_root"],
                str(root / "reports" / "decomp-status"),
            )

    def test_main_batches_capture_on_success(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "seed.json"
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
                seed_strategy=None,
                limit=4,
                rank_min=None,
                rank_max=None,
                candidate_index=0,
                noanalysis=False,
                refresh_reports=False,
                refresh_status=False,
                tracked_output=False,
                output_json=output_json,
                log_path=root / "seed.log",
                dry_run=False,
                quiet=False,
                verbose=False,
            )
            frontier_payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "items": [
                    {
                        "program_path": "/bins/BIN/ETC/MTEST/0.bin",
                        "archive_id": "BIN/ETC/MTEST",
                        "entry_index": 0,
                        "queue_rank": 1,
                        "frontier_state": "manual_frontier",
                        "seed_strategy": "duplicate_peer_offsets",
                        "family": "ETC",
                        "lane": "system_script",
                        "ghidra_program_selector": "/bins/BIN/ETC/MTEST/MTEST_e00_801d0c00.bin",
                        "seed_candidates": [{"address_hex": "0x801d1250"}],
                    },
                    {
                        "program_path": "/bins/BIN/ETC/RTEST/1.bin",
                        "archive_id": "BIN/ETC/RTEST",
                        "entry_index": 1,
                        "queue_rank": 2,
                        "frontier_state": "manual_frontier",
                        "seed_strategy": "duplicate_peer_offsets",
                        "family": "ETC",
                        "lane": "system_script",
                        "ghidra_program_selector": "/bins/BIN/ETC/RTEST/RTEST_e01_801d0c00.bin",
                        "seed_candidates": [{"address_hex": "0x801d1250"}],
                    },
                ],
            }
            logger = type(
                "Logger",
                (),
                {
                    "summary": lambda self, message: None,
                    "error": lambda self, message: None,
                },
            )()
            capture_report = {
                "canonical_program_count": 2,
                "row_count": 2,
                "persisted": {"function_rows": 2},
                "rows": [
                    {
                        "program_path": "/bins/BIN/ETC/MTEST/0.bin",
                        "address": "801d1250",
                    },
                    {
                        "program_path": "/bins/BIN/ETC/RTEST/1.bin",
                        "address": "801d1250",
                    },
                ],
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
                    MODULE.frontier_backlog_lib,
                    "build_frontier_backlog_payload",
                    return_value=frontier_payload,
                ),
                mock.patch.object(MODULE, "project_busy_message", return_value=None),
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
                    "capture_into_inventory",
                    return_value=capture_report,
                ) as capture_mock,
            ):
                result = MODULE.main()
            self.assertEqual(result, 0)
            report = json.loads(output_json.read_text(encoding="utf-8"))
            self.assertEqual(report["status"], "completed")
            self.assertEqual(report["captured_count"], 2)
            self.assertEqual(capture_mock.call_count, 1)
            self.assertIsNone(report["refreshed_reports"])


if __name__ == "__main__":
    unittest.main()
