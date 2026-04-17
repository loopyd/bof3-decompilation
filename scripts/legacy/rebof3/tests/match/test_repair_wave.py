from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import repair_wave as MODULE


class MatchRepairWaveTests(unittest.TestCase):
    def test_select_items_filters_by_seed_strategy(self) -> None:
        backlog_payload = {
            "items": [
                {
                    "queue_rank": 1,
                    "family": "ETC",
                    "lane": "system_script",
                    "seed_strategy": "duplicate_peer_offsets",
                    "source_hint": "build/extracted/BIN/ETC/RTEST.EMI#1",
                },
                {
                    "queue_rank": 2,
                    "family": "ETC",
                    "lane": "system_script",
                    "seed_strategy": "load_base_only",
                    "source_hint": "build/extracted/BIN/ETC/BATE.EMI#3",
                },
            ]
        }
        selected = MODULE.select_items(
            backlog_payload,
            families=None,
            lanes=None,
            seed_strategies=["duplicate_peer_offsets"],
            limit=8,
            rank_min=None,
            rank_max=None,
        )
        self.assertEqual(len(selected), 1)
        self.assertEqual(selected[0]["seed_strategy"], "duplicate_peer_offsets")

    def test_build_import_command_targets_binary_import(self) -> None:
        command = MODULE.build_import_command(
            ghidra_home=Path("/opt/ghidra"),
            project_dir=Path("tmp/bof3_ghidra/main"),
            project_name="bof3_main",
            config_mode="isolated",
            noanalysis=False,
            source_hint="build/extracted/BIN/ETC/RTEST.EMI#1",
            folder="bins/BIN/ETC/RTEST",
        )
        self.assertIn("binary", command)
        self.assertIn("import", command)
        self.assertIn("build/extracted/BIN/ETC/RTEST.EMI#1", command)
        self.assertIn("bins/BIN/ETC/RTEST", command)

    def test_main_writes_dry_run_report(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "repair.json"
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
                noanalysis=False,
                output_json=output_json,
                log_path=root / "repair.log",
                refresh_reports=False,
                refresh_status=False,
                tracked_output=False,
                dry_run=True,
                quiet=False,
                verbose=False,
            )
            frontier_payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "items": [
                    {
                        "queue_rank": 1,
                        "program_path": "/bins/BIN/ETC/RTEST/1.bin",
                        "family": "ETC",
                        "lane": "system_script",
                        "seed_strategy": "duplicate_peer_offsets",
                        "source_hint": "build/extracted/BIN/ETC/RTEST.EMI#1",
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
            ):
                result = MODULE.main()
            self.assertEqual(result, 0)
            report = json.loads(output_json.read_text(encoding="utf-8"))
            self.assertEqual(report["status"], "planned")
            self.assertEqual(report["selected_count"], 1)

    def test_main_completes_with_partial_failures(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "repair.json"
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
                noanalysis=False,
                output_json=output_json,
                log_path=root / "repair.log",
                refresh_reports=True,
                refresh_status=False,
                tracked_output=False,
                dry_run=False,
                quiet=False,
                verbose=False,
            )
            frontier_payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "items": [
                    {
                        "queue_rank": 1,
                        "program_path": "/bins/BIN/ETC/RTEST/1.bin",
                        "family": "ETC",
                        "lane": "system_script",
                        "seed_strategy": "duplicate_peer_offsets",
                        "source_hint": "build/extracted/BIN/ETC/RTEST.EMI#1",
                    },
                    {
                        "queue_rank": 2,
                        "program_path": "/bins/BIN/ETC/SHOP/8.bin",
                        "family": "ETC",
                        "lane": "system_script",
                        "seed_strategy": "duplicate_peer_offsets",
                        "source_hint": "build/extracted/BIN/ETC/SHOP.EMI#8",
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
            run_results = [
                subprocess.CompletedProcess(["cmd"], 1, stdout="", stderr="missing"),
                subprocess.CompletedProcess(["cmd"], 0, stdout="ok", stderr=""),
            ]
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
                mock.patch.object(MODULE, "run_command", side_effect=run_results),
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
            self.assertEqual(report["imported_count"], 1)


if __name__ == "__main__":
    unittest.main()
