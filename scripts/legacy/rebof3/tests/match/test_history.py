from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import history as MODULE


class MatchHistoryTests(unittest.TestCase):
    def test_summarize_entries_tracks_attempts_and_stall_state(self) -> None:
        summary = MODULE.summarize_entries(
            [
                {
                    "event": "build",
                    "program_path": "/boot/SLUS_004.22",
                    "entry_hex": "0x80162d00",
                },
                {
                    "event": "diff",
                    "program_path": "/boot/SLUS_004.22",
                    "entry_hex": "0x80162d00",
                    "match_metrics": {
                        "objdiff_match_percent": 20.0,
                        "asm_score": 10.0,
                    },
                },
                {
                    "event": "diff",
                    "program_path": "/boot/SLUS_004.22",
                    "entry_hex": "0x80162d00",
                    "match_metrics": {
                        "objdiff_match_percent": 20.0,
                        "asm_score": 10.0,
                    },
                },
                {
                    "event": "diff",
                    "program_path": "/boot/SLUS_004.22",
                    "entry_hex": "0x80162d00",
                    "match_metrics": {
                        "objdiff_match_percent": 20.0,
                        "asm_score": 10.0,
                    },
                },
                {
                    "event": "diff",
                    "program_path": "/boot/SLUS_004.22",
                    "entry_hex": "0x80162d00",
                    "match_metrics": {
                        "objdiff_match_percent": 20.0,
                        "asm_score": 10.0,
                    },
                },
                {
                    "event": "permuter",
                    "program_path": "/boot/SLUS_004.22",
                    "entry_hex": "0x80162d00",
                    "timed_out": True,
                },
            ]
        )

        self.assertEqual(summary["attempt_count"], 6)
        self.assertEqual(summary["build_attempt_count"], 1)
        self.assertEqual(summary["diff_attempt_count"], 4)
        self.assertEqual(summary["permuter_attempt_count"], 1)
        self.assertEqual(summary["timed_out_permuter_attempt_count"], 1)
        self.assertEqual(summary["best_objdiff_match_percent"], 20.0)
        self.assertEqual(summary["best_asm_score"], 10.0)
        self.assertEqual(summary["non_improving_scored_attempts"], 3)
        self.assertTrue(summary["stalled"])

    def test_append_entry_and_summarize_workspace_write_history_file(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            workspace_dir = Path(tmp_dir) / "workspace"
            MODULE.append_entry(
                workspace_dir,
                {
                    "event": "diff",
                    "program_path": "/boot/SLUS_004.22",
                    "entry_hex": "0x80162d00",
                    "match_metrics": {"objdiff_match_percent": 40.0},
                },
            )

            summary = MODULE.summarize_workspace(workspace_dir)

        self.assertEqual(summary["attempt_count"], 1)
        self.assertEqual(summary["history_path"], str(workspace_dir / "history.jsonl"))
        self.assertEqual(summary["latest_objdiff_match_percent"], 40.0)


if __name__ == "__main__":
    unittest.main()
