from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import report_refresh as refresh_module
from scripts.rebof3.match import status as MODULE


class MatchStatusTests(unittest.TestCase):
    def test_parse_args_accepts_short_foundation_flags(self) -> None:
        args = MODULE.parse_args(
            [
                "-i",
                "tmp/inventory.sqlite",
                "-m",
                "tmp/matching",
                "-s",
                "bof3",
                "-a",
                "tmp/ghidra_decomp",
                "-P",
                "capcom97-bof3",
                "-t",
            ]
        )

        self.assertEqual(args.inventory_db, Path("tmp/inventory.sqlite"))
        self.assertEqual(args.match_root, Path("tmp/matching"))
        self.assertEqual(args.source_root, Path("bof3"))
        self.assertEqual(args.artifact_root, Path("tmp/ghidra_decomp"))
        self.assertEqual(args.profile, "capcom97-bof3")
        self.assertTrue(args.tracked_output)

    def test_write_status_snapshot_emits_expected_files(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            stale_scoreboard = root / "status" / "scoreboard.json"
            stale_scoreboard.parent.mkdir(parents=True, exist_ok=True)
            stale_scoreboard.write_text("{}", encoding="utf-8")
            payload = {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "summary": {
                    "campaign_ready": False,
                    "code_candidate_entries": 2,
                    "code_entries_missing_programs": 1,
                    "code_entries_missing_functions": 0,
                    "reviewed_non_code_entries": 0,
                    "programs": 1,
                    "bin_programs": 1,
                    "boot_programs": 0,
                    "logo_programs": 0,
                    "other_programs": 0,
                    "imported_overlay_programs": 1,
                    "inventory_functions": 1,
                    "bin_functions": 1,
                    "boot_functions": 0,
                    "logo_functions": 0,
                    "other_functions": 0,
                    "build_ok_functions": 1,
                    "build_failed_functions": 0,
                    "lifted_c_functions": 1,
                    "functions_without_source": 0,
                    "source_coverage_percent": 100.0,
                    "diffed_functions": 1,
                    "exact_match_functions": 1,
                    "asm_exact_functions": 1,
                    "matched_function_count": 1,
                    "attempted_functions": 1,
                    "stalled_functions": 0,
                    "highest_objdiff_match_percent": 100.0,
                    "lowest_objdiff_match_percent": 100.0,
                    "average_objdiff_match_percent": 100.0,
                    "median_objdiff_match_percent": 100.0,
                    "duplicate_groups": 1,
                    "multi_entry_duplicate_groups": 1,
                    "entries_in_multi_groups": 2,
                    "largest_duplicate_group": 2,
                    "unresolved_source_mappings": 0,
                    "blocking_issues": [
                        "1 code-candidate EMI entries are missing program rows"
                    ],
                },
                "functions": [
                    {
                        "family": "ETC",
                        "program_path": "/bins/BIN/ETC/GAME/0.bin",
                        "entry_hex": "0x80196f78",
                        "name": "func_80196f78",
                        "archive_id": "BIN/ETC/GAME",
                        "entry_index": 0,
                        "duplicate_group_key": "BIN/ETC/GAME#0",
                        "duplicate_group_size": 2,
                        "function_state": "exact_match",
                        "source_file": "bof3/src/modules/game/00/func_80196f78.c",
                        "source_function": "func_80196f78",
                        "build_succeeded": True,
                        "diff_status": "ready_for_backend_diff",
                        "match_bucket": "excellent",
                        "objdiff_match_percent": 100.0,
                        "asm_score": 0.0,
                        "asm_exact": True,
                        "semantic_status": "exact",
                        "attempt_count": 1,
                        "diff_attempt_count": 1,
                        "permuter_attempt_count": 0,
                        "non_improving_scored_attempts": 0,
                        "best_objdiff_match_percent": 100.0,
                        "best_asm_score": 0.0,
                        "stalled": False,
                    }
                ],
                "programs": [
                    {
                        "family": "ETC",
                        "program_path": "/bins/BIN/ETC/GAME/0.bin",
                        "program_name": "0.bin",
                        "archive_id": "BIN/ETC/GAME",
                        "entry_index": 0,
                        "duplicate_group_key": "BIN/ETC/GAME#0",
                        "duplicate_group_size": 2,
                        "function_count": 1,
                        "lifted_c_functions": 1,
                        "functions_without_source": 0,
                        "source_coverage_percent": 100.0,
                        "attempted_functions": 1,
                        "stalled_functions": 0,
                        "build_ok_functions": 1,
                        "build_failed_functions": 0,
                        "diffed_functions": 1,
                        "exact_match_functions": 1,
                        "asm_exact_functions": 1,
                        "matched_function_count": 1,
                        "highest_objdiff_match_percent": 100.0,
                        "lowest_objdiff_match_percent": 100.0,
                        "average_objdiff_match_percent": 100.0,
                        "median_objdiff_match_percent": 100.0,
                        "program_state": "match_mature",
                    }
                ],
                "families": [
                    {
                        "family": "ETC",
                        "code_candidate_entries": 2,
                        "entries_missing_programs": 1,
                        "entries_missing_functions": 0,
                        "reviewed_non_code_entries": 0,
                        "imported_programs": 1,
                        "programs": 1,
                        "inventory_functions": 1,
                        "lifted_c_functions": 1,
                        "attempted_functions": 1,
                        "stalled_functions": 0,
                        "build_ok_functions": 1,
                        "build_failed_functions": 0,
                        "diffed_functions": 1,
                        "exact_match_functions": 1,
                        "asm_exact_functions": 1,
                        "duplicate_groups": 1,
                        "multi_entry_duplicate_groups": 1,
                        "matched_function_count": 1,
                        "highest_objdiff_match_percent": 100.0,
                        "lowest_objdiff_match_percent": 100.0,
                        "average_objdiff_match_percent": 100.0,
                        "median_objdiff_match_percent": 100.0,
                    }
                ],
                "entries": [
                    {
                        "family": "ETC",
                        "archive_id": "BIN/ETC/GAME",
                        "entry_index": 0,
                        "payload_path": "build/extracted/BIN/ETC/GAME.EMI#0",
                        "duplicate_group_key": "BIN/ETC/GAME#0",
                        "duplicate_group_size": 2,
                        "imported_program_count": 1,
                        "function_count": 1,
                        "entry_state": "candidate_imported",
                        "entry_table_confidence": "high",
                        "review_reason": None,
                        "program_paths": ["/bins/BIN/ETC/GAME/0.bin"],
                    }
                ],
                "artifacts": {
                    "manifest_path": "build/bof3-psyq40/artifacts/metadata/artifacts.json",
                    "declared_artifacts": 3,
                    "placeholder_artifacts": 1,
                    "raw_stage_artifacts": 1,
                    "archive_stage_artifacts": 2,
                    "kinds": {"boot": 1, "module": 2},
                },
            }

            outputs = MODULE.write_status_snapshot(
                payload,
                output_root=root / "status",
                profile="capcom97-bof3",
            )

            manifest = json.loads(outputs["manifest_json"].read_text(encoding="utf-8"))
            summary = json.loads(outputs["status_json"].read_text(encoding="utf-8"))
            summary_md_text = outputs["status_md"].read_text(encoding="utf-8")
            functions_tsv_text = outputs["functions_tsv"].read_text(encoding="utf-8")

        self.assertEqual(summary["profile"], "capcom97-bof3")
        self.assertEqual(summary["coverage"]["programs"]["bin"], 1)
        self.assertEqual(summary["coverage"]["programs"]["boot"], 0)
        self.assertEqual(summary["coverage"]["programs"]["logo"], 0)
        self.assertEqual(summary["coverage"]["functions"]["matched"], 1)
        self.assertEqual(summary["coverage"]["functions"]["asm_exact"], 1)
        self.assertEqual(summary["coverage"]["functions"]["attempted"], 1)
        self.assertEqual(summary["coverage"]["functions"]["stalled"], 0)
        self.assertEqual(
            summary["coverage"]["functions"]["source_coverage_percent"], 100.0
        )
        self.assertEqual(
            summary["coverage"]["functions"]["average_match_percent"], 100.0
        )
        self.assertEqual(summary["artifacts"]["declared_artifacts"], 3)
        self.assertTrue(manifest["output_root"].endswith("/status"))
        self.assertEqual(
            sorted(item["name"] for item in manifest["files"]),
            [
                "entries.tsv",
                "families.tsv",
                "functions.tsv",
                "programs.tsv",
                "status.json",
                "status.md",
            ],
        )
        self.assertFalse(stale_scoreboard.exists())
        self.assertIn("# Decomp Status", summary_md_text)
        self.assertIn("Attempted functions: 1", summary_md_text)
        self.assertIn("Asm-differ exact: 1", summary_md_text)
        self.assertIn("Stalled functions: 0", summary_md_text)
        self.assertIn("func_80196f78", functions_tsv_text)

    def test_resolve_status_output_root_switches_to_tracked_root(self) -> None:
        local_root = refresh_module.resolve_status_output_root(
            profile="capcom97-bof3",
            tracked_output=False,
        )
        tracked_root = refresh_module.resolve_status_output_root(
            profile="capcom97-bof3",
            tracked_output=True,
        )

        self.assertEqual(
            local_root.as_posix().split("/")[-4:],
            ["tmp", "status", "capcom97-bof3", "current"],
        )
        self.assertEqual(
            tracked_root.as_posix().split("/")[-3:],
            ["reports", "decomp-status", "current"],
        )

    def test_load_artifact_manifest_summary_reads_stage_counts(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            manifest_path = Path(tmp_dir) / "artifacts.json"
            manifest_path.write_text(
                json.dumps(
                    {
                        "artifacts": [
                            {
                                "kind": "boot",
                                "build_stage": "raw",
                                "placeholder": False,
                            },
                            {
                                "kind": "module",
                                "build_stage": "archive",
                                "placeholder": True,
                            },
                            {
                                "kind": "module",
                                "build_stage": "archive",
                                "placeholder": False,
                            },
                        ]
                    }
                ),
                encoding="utf-8",
            )

            summary = MODULE.load_artifact_manifest_summary(manifest_path)

        self.assertIsNotNone(summary)
        self.assertEqual(summary["declared_artifacts"], 3)
        self.assertEqual(summary["raw_stage_artifacts"], 1)
        self.assertEqual(summary["archive_stage_artifacts"], 2)
        self.assertEqual(summary["placeholder_artifacts"], 1)


if __name__ == "__main__":
    unittest.main()
