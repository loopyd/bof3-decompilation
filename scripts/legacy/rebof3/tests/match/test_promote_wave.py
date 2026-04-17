from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import promote_wave as MODULE


class MatchPromoteWaveTests(unittest.TestCase):
    def test_select_items_filters_by_confidence_and_limit(self) -> None:
        backlog_payload = {
            "items": [
                {
                    "queue_rank": 1,
                    "frontier_state": "promotable_entry_labels",
                    "entry_table_confidence": "high",
                    "family": "BATTLE",
                    "lane": "battle_runtime",
                    "ghidra_program_selector": "/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin",
                },
                {
                    "queue_rank": 2,
                    "frontier_state": "promotable_entry_labels",
                    "entry_table_confidence": "medium",
                    "family": "ETC",
                    "lane": "system_script",
                    "ghidra_program_selector": "/bins/BIN/ETC/GAME/GAME_e01_801d0c00.bin",
                },
            ]
        }
        selected = MODULE.select_items(
            backlog_payload,
            families=None,
            lanes=None,
            limit=1,
            rank_min=None,
            rank_max=None,
            min_confidence="high",
        )
        self.assertEqual(len(selected), 1)
        self.assertEqual(selected[0]["family"], "BATTLE")

    def test_main_writes_dry_run_report(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            output_json = root / "promote.json"
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
                limit=4,
                rank_min=None,
                rank_max=None,
                min_confidence="medium",
                noanalysis=False,
                output_json=output_json,
                log_path=root / "promote.log",
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
                        "frontier_state": "promotable_entry_labels",
                        "entry_table_confidence": "high",
                        "family": "BATTLE",
                        "lane": "battle_runtime",
                        "ghidra_program_selector": "/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin",
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
            self.assertTrue(output_json.exists())


if __name__ == "__main__":
    unittest.main()
