from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import enhanced_report as MODULE


class MatchEnhancedReportTests(unittest.TestCase):
    def test_build_binary_report_groups_slots_under_same_binary(self) -> None:
        payload = {
            "generated_at": "2026-01-01T00:00:00+00:00",
            "functions": [
                {
                    "family": "LOGO",
                    "program_path": "/boot/LOGO/LOGO.EXE",
                    "source_hint": "build/extracted/LOGO/LOGO.EXE",
                    "entry_hex": "0x801ce758",
                    "source_function": "func_801ce758",
                    "has_source_mapping": True,
                    "objdiff_match_percent": 100.0,
                    "asm_exact": True,
                },
                {
                    "family": "BOOT",
                    "program_path": "/boot/LOGO/LOGO.EXE.0",
                    "source_hint": "build/extracted/LOGO/LOGO.EXE",
                    "entry_hex": "0x801cedfc",
                    "source_function": "func_801cedfc",
                    "has_source_mapping": True,
                    "objdiff_match_percent": None,
                    "asm_exact": False,
                },
                {
                    "family": "LOGO",
                    "program_path": "/boot/LOGO/LOGO.EXE",
                    "source_hint": "build/extracted/LOGO/LOGO.EXE",
                    "entry_hex": "0x801cf000",
                    "name": "FUN_801cf000",
                    "has_source_mapping": False,
                    "objdiff_match_percent": None,
                    "asm_exact": False,
                },
            ],
        }

        report = MODULE.build_binary_report_payload(payload)

        self.assertEqual(report["summary"]["binary_count"], 1)
        binary = report["binaries"][0]
        self.assertEqual(binary["binary_path"], "build/extracted/LOGO/LOGO.EXE")
        self.assertEqual(binary["program_count"], 1)
        self.assertEqual(binary["matching_functions"], 1)
        self.assertEqual(binary["asm_exact_functions"], 1)
        self.assertEqual(binary["lifted_functions"], 1)
        self.assertEqual(binary["missing_functions"], 1)
        self.assertAlmostEqual(binary["completion_percent"], 66.667, places=3)
        self.assertEqual(len(binary["programs"]), 1)
        self.assertEqual(len(report["views"]["most_complete"]), 1)
        self.assertEqual(len(report["views"]["progressed"]), 1)

    def test_render_tsv_includes_binary_and_slot_metrics(self) -> None:
        rows = [
            {
                "binary_path": "build/extracted/BIN/BATTLE/BATTLE.EMI",
                "family": "BATTLE",
                "total_functions": 2,
                "matching_functions": 1,
                "asm_exact_functions": 1,
                "lifted_functions": 0,
                "missing_functions": 1,
                "completion_percent": 50.0,
                "programs": [
                    {
                        "program_path": "/bins/BIN/BATTLE/BATTLE/3.bin",
                        "family": "BATTLE",
                        "total_functions": 2,
                        "matching_functions": 1,
                        "exact_functions": 1,
                        "asm_exact_functions": 1,
                        "lifted_functions": 0,
                        "missing_functions": 1,
                        "completion_percent": 50.0,
                        "matching_percent": 50.0,
                        "matching_function_names": ["func_801ddf28"],
                        "lifted_function_names": [],
                        "missing_function_names": ["func_801ddf50"],
                    }
                ],
            }
        ]

        text = MODULE.render_tsv(rows)

        self.assertIn("binary_path", text)
        self.assertIn("/bins/BIN/BATTLE/BATTLE/3.bin", text)
        self.assertIn("func_801ddf28", text)
        self.assertIn("func_801ddf50", text)

    def test_render_markdown_summary_omits_detailed_binary_sections(self) -> None:
        payload = {
            "summary": {
                "binary_count": 1,
                "progressed_binary_count": 1,
                "matching_functions": 1,
                "asm_exact_functions": 1,
                "lifted_functions": 1,
                "missing_functions": 0,
            },
            "views": {
                "most_complete": [
                    {
                        "family": "BATTLE",
                        "binary_path": "build/extracted/BIN/BATTLE/BATTLE.EMI",
                        "program_count": 1,
                        "total_functions": 2,
                        "matching_functions": 1,
                        "asm_exact_functions": 1,
                        "lifted_functions": 1,
                        "missing_functions": 0,
                        "completion_percent": 100.0,
                        "matching_percent": 50.0,
                    }
                ],
                "biggest_gaps": [],
                "progressed": [
                    {
                        "family": "BATTLE",
                        "binary_path": "build/extracted/BIN/BATTLE/BATTLE.EMI",
                        "program_count": 1,
                        "total_functions": 2,
                        "matching_functions": 1,
                        "asm_exact_functions": 1,
                        "lifted_functions": 1,
                        "missing_functions": 0,
                        "completion_percent": 100.0,
                        "matching_percent": 50.0,
                    }
                ],
            },
            "binaries": [
                {
                    "family": "BATTLE",
                    "binary_path": "build/extracted/BIN/BATTLE/BATTLE.EMI",
                    "program_count": 1,
                    "total_functions": 2,
                    "matching_functions": 1,
                    "asm_exact_functions": 1,
                    "lifted_functions": 1,
                    "missing_functions": 0,
                    "completion_percent": 100.0,
                    "matching_percent": 50.0,
                    "programs": [],
                }
            ],
        }

        text = MODULE.render_markdown(payload, view="summary", table_limit=5)

        self.assertIn("## Most Complete Binaries", text)
        self.assertIn("## Progressed Binaries", text)
        self.assertNotIn("## `build/extracted/BIN/BATTLE/BATTLE.EMI`", text)

    def test_main_writes_default_outputs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            match_root = root / "tmp" / "matching"
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("placeholder", encoding="utf-8")
            payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "functions": [
                    {
                        "family": "BATTLE",
                        "program_path": "/bins/BIN/BATTLE/BATTLE/3.bin",
                        "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                        "source_function": "func_801ddf28",
                        "has_source_mapping": True,
                        "objdiff_match_percent": 100.0,
                        "asm_exact": True,
                    }
                ],
            }

            with mock.patch.object(
                MODULE.scoreboard_lib,
                "build_scoreboard_payload",
                return_value=payload,
            ):
                result = MODULE.main(
                    [
                        "--inventory-db",
                        str(inventory_db),
                        "--match-root",
                        str(match_root),
                        "--source-root",
                        str(root / "bof3"),
                        "--artifact-root",
                        str(root / "tmp" / "ghidra_decomp"),
                    ]
                )

            self.assertEqual(result, 0)
            output_json, output_tsv, output_md = MODULE.default_output_paths(
                match_root,
                MODULE.DEFAULT_PSX_PROFILE,
            )
            self.assertTrue(output_json.exists())
            self.assertTrue(output_tsv.exists())
            self.assertTrue(output_md.exists())

            report = json.loads(output_json.read_text(encoding="utf-8"))
            self.assertEqual(report["summary"]["binary_count"], 1)
            self.assertEqual(
                report["binaries"][0]["binary_path"],
                "build/extracted/BIN/BATTLE/BATTLE.EMI",
            )

    def test_main_summary_view_writes_summary_markdown_path(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            match_root = root / "tmp" / "matching"
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("placeholder", encoding="utf-8")
            payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "functions": [
                    {
                        "family": "BATTLE",
                        "program_path": "/bins/BIN/BATTLE/BATTLE/3.bin",
                        "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                        "source_function": "func_801ddf28",
                        "has_source_mapping": True,
                        "objdiff_match_percent": 100.0,
                        "asm_exact": True,
                    }
                ],
            }

            with mock.patch.object(
                MODULE.scoreboard_lib,
                "build_scoreboard_payload",
                return_value=payload,
            ):
                result = MODULE.main(
                    [
                        "--inventory-db",
                        str(inventory_db),
                        "--match-root",
                        str(match_root),
                        "--source-root",
                        str(root / "bof3"),
                        "--artifact-root",
                        str(root / "tmp" / "ghidra_decomp"),
                        "--view",
                        "summary",
                    ]
                )

            self.assertEqual(result, 0)
            summary_md = MODULE.default_summary_md_path(
                match_root,
                MODULE.DEFAULT_PSX_PROFILE,
            )
            self.assertTrue(summary_md.exists())
            self.assertNotIn(
                "## `build/extracted/BIN/BATTLE/BATTLE.EMI`",
                summary_md.read_text(encoding="utf-8"),
            )


if __name__ == "__main__":
    unittest.main()
