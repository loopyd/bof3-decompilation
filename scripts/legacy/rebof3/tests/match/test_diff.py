from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from scripts.rebof3.match import diff as MODULE


class MatchDiffTests(unittest.TestCase):
    def _ready_workspace(
        self,
        root: Path,
        *,
        bundle_exists: bool = True,
    ) -> tuple[Path, dict[str, object]]:
        workspace_dir = root / "tmp" / "matching" / "boot_slus_004_22" / "0x80162d00"
        workspace_dir.mkdir(parents=True, exist_ok=True)
        bundle_path = root / "bundle.json"
        if bundle_exists:
            bundle_path.write_text("{}", encoding="utf-8")
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
        workspace_json = workspace_dir / "workspace.json"
        workspace_payload = {
            "workspace_dir": str(workspace_dir),
            "program_path": "/boot/SLUS_004.22",
            "entry_hex": "0x80162d00",
            "ghidra_decomp_bundle_json": str(bundle_path),
            "source_mapping_ready": True,
            "expected_baseline_ready": True,
            "expected_baseline": {"asm_source": "tmp/func.s"},
            "source_mapping": {
                "source_function": "func_80162d00",
                "object_candidates": ["build/foo.obj"],
            },
        }
        workspace_json.write_text(
            json.dumps(workspace_payload, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        return workspace_json, workspace_payload

    def test_diff_status_blocks_without_ghidra_bundle(self) -> None:
        status, next_steps = MODULE.diff_status(
            {}, build_status=None, ghidra_bundle_exists=False
        )

        self.assertEqual(status, "blocked_missing_ghidra_bundle")
        self.assertTrue(next_steps)

    def test_diff_status_blocks_without_source_mapping_even_after_build(self) -> None:
        status, next_steps = MODULE.diff_status(
            {"source_mapping_ready": False},
            build_status={"succeeded": True},
            ghidra_bundle_exists=True,
        )

        self.assertEqual(status, "blocked_missing_source_mapping")
        self.assertIn("func_80162d00", next_steps[0])

    def test_diff_status_can_be_ready_for_backend_diff(self) -> None:
        status, next_steps = MODULE.diff_status(
            {
                "source_mapping_ready": True,
                "expected_baseline_ready": True,
                "source_mapping": {"object_candidates": ["build/foo.obj"]},
            },
            build_status={"succeeded": True},
            ghidra_bundle_exists=True,
        )

        self.assertEqual(status, "ready_for_backend_diff")
        self.assertIn("function-sliced expected/current objects", next_steps[0])

    def test_render_markdown_includes_status(self) -> None:
        report = {
            "program_path": "/boot/SLUS_004.22",
            "entry_hex": "0x80162d00",
            "status": "blocked_missing_source_mapping",
            "ghidra_bundle_exists": True,
            "build_status_present": True,
            "source_mapping_ready": False,
            "expected_baseline_ready": False,
            "backend_ready": False,
            "backends": {},
            "next_steps": ["map this workspace"],
        }

        markdown = MODULE.render_markdown(report)

        self.assertIn("blocked_missing_source_mapping", markdown)
        self.assertIn("/boot/SLUS_004.22", markdown)

    def test_diff_status_mentions_backend_wiring_without_object_candidates(
        self,
    ) -> None:
        status, next_steps = MODULE.diff_status(
            {
                "source_mapping_ready": True,
                "expected_baseline_ready": True,
                "source_mapping": {},
            },
            build_status={"succeeded": True},
            ghidra_bundle_exists=True,
        )

        self.assertEqual(status, "ready_for_backend_diff")
        self.assertIn("asm-differ or objdiff", next_steps[0])

    def test_diff_status_blocks_without_expected_baseline(self) -> None:
        status, next_steps = MODULE.diff_status(
            {
                "source_mapping_ready": True,
                "expected_baseline_ready": False,
                "source_mapping": {"object_candidates": ["build/foo.obj"]},
            },
            build_status={"succeeded": True},
            ghidra_bundle_exists=True,
        )

        self.assertEqual(status, "blocked_missing_expected_baseline")
        self.assertIn("expected baseline asm", next_steps[0])

    def test_run_backend_flag_writes_diff_report_and_history_when_ready(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            workspace_json, workspace_payload = self._ready_workspace(root)
            args = MODULE.parse_args.__globals__["argparse"].Namespace(
                workspace_json=workspace_json,
                program=None,
                entry=None,
                inventory_db=root / "inventory.sqlite",
                workspace_root=root / "tmp" / "matching",
                refresh_ghidra_bundle=False,
                run_backend=True,
                dry_run=False,
                quiet=False,
                verbose=False,
            )
            logger = SimpleNamespace(
                summary=lambda message: None,
                error=lambda message: None,
                item=lambda message: None,
            )
            backend_reports = {
                "asm-differ": {
                    "diff_summary": {
                        "current_score": 12,
                        "max_score": 100,
                        "row_count": 4,
                    },
                    "current_slice": {"size": 16},
                },
                "objdiff": {
                    "diff_summary": {
                        "text_match_percent": 75.0,
                        "instruction_count": 10,
                        "mismatch_count": 2,
                    }
                },
                "semantic-diff": {
                    "diff_summary": {
                        "semantic_status": "relocation_only",
                        "classified_mismatch_count": 2,
                        "unclassified_mismatch_count": 0,
                        "asm_view_only_noise": False,
                        "category_counts": {
                            "move_zero_sugar": 0,
                            "li_zero_sugar": 0,
                            "branch_zero_sugar": 0,
                            "commutative_swap": 0,
                            "call_target_reloc": 0,
                            "address_materialization": 2,
                        },
                    }
                },
            }

            with (
                patch.object(MODULE, "parse_args", return_value=args),
                patch.object(MODULE, "logger_from_args", return_value=logger),
                patch.object(
                    MODULE.pipeline_ready,
                    "resolve_workspace",
                    return_value=(workspace_json, workspace_payload),
                ),
                patch.object(MODULE, "run_diff_backends", return_value=backend_reports),
            ):
                result = MODULE.main()
            report = json.loads(
                (workspace_json.parent / "diff.json").read_text(encoding="utf-8")
            )
            history_entries = MODULE.history_lib.load_entries(workspace_json.parent)

        self.assertEqual(result, 0)
        self.assertEqual(report["status"], "ready_for_backend_diff")
        self.assertEqual(report["match_metrics"]["objdiff_match_percent"], 75.0)
        self.assertEqual(report["match_metrics"]["semantic_status"], "relocation_only")
        self.assertEqual(report["history_summary"]["attempt_count"], 1)
        self.assertFalse(report["history_summary"]["stalled"])
        self.assertEqual(len(history_entries), 1)
        self.assertEqual(history_entries[0]["event"], "diff")

    def test_backend_failure_writes_error_report_and_history(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            workspace_json, workspace_payload = self._ready_workspace(root)
            args = MODULE.parse_args.__globals__["argparse"].Namespace(
                workspace_json=workspace_json,
                program=None,
                entry=None,
                inventory_db=root / "inventory.sqlite",
                workspace_root=root / "tmp" / "matching",
                refresh_ghidra_bundle=False,
                run_backend=True,
                dry_run=False,
                quiet=False,
                verbose=False,
            )
            logger = SimpleNamespace(
                summary=lambda message: None,
                error=lambda message: None,
                item=lambda message: None,
            )

            with (
                patch.object(MODULE, "parse_args", return_value=args),
                patch.object(MODULE, "logger_from_args", return_value=logger),
                patch.object(
                    MODULE.pipeline_ready,
                    "resolve_workspace",
                    return_value=(workspace_json, workspace_payload),
                ),
                patch.object(
                    MODULE,
                    "run_diff_backends",
                    side_effect=MODULE.pipeline_backend.BackendFailure(
                        "no built object matched the workspace object candidates"
                    ),
                ),
            ):
                result = MODULE.main()
            report = json.loads(
                (workspace_json.parent / "diff.json").read_text(encoding="utf-8")
            )
            history_entries = MODULE.history_lib.load_entries(workspace_json.parent)

        self.assertEqual(result, 1)
        self.assertEqual(report["status"], "backend_failed")
        self.assertEqual(
            report["next_steps"],
            ["no built object matched the workspace object candidates"],
        )
        self.assertEqual(report["history_summary"]["attempt_count"], 1)
        self.assertEqual(report["history_summary"]["diff_attempt_count"], 1)
        self.assertEqual(len(history_entries), 1)
        self.assertEqual(history_entries[0]["status"], "backend_failed")


if __name__ == "__main__":
    unittest.main()
