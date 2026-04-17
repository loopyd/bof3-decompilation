from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import report as MODULE


class MatchReportTests(unittest.TestCase):
    def test_match_bucket_thresholds(self) -> None:
        self.assertEqual(
            MODULE.match_bucket({"objdiff_match_percent": 95.0}), "excellent"
        )
        self.assertEqual(MODULE.match_bucket({"objdiff_match_percent": 65.0}), "strong")
        self.assertEqual(
            MODULE.match_bucket({"objdiff_match_percent": 25.0}), "promising"
        )
        self.assertEqual(MODULE.match_bucket({"objdiff_match_percent": 10.0}), "weak")
        self.assertEqual(
            MODULE.match_bucket({"objdiff_match_percent": 0.0}), "unmatched"
        )

    def test_collect_reports_sorts_by_match_percent(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            first = root / "a" / "diff.json"
            second = root / "b" / "diff.json"
            first.parent.mkdir(parents=True, exist_ok=True)
            second.parent.mkdir(parents=True, exist_ok=True)
            first.write_text(
                json.dumps(
                    {
                        "program_path": "/boot/A",
                        "entry_hex": "0x1",
                        "match_metrics": {"objdiff_match_percent": 10.0},
                    }
                ),
                encoding="utf-8",
            )
            second.write_text(
                json.dumps(
                    {
                        "program_path": "/boot/B",
                        "entry_hex": "0x2",
                        "match_metrics": {"objdiff_match_percent": 20.0},
                    }
                ),
                encoding="utf-8",
            )

            rows = MODULE.collect_reports(root)

            self.assertEqual(rows[0]["program_path"], "/boot/B")
            self.assertEqual(rows[1]["program_path"], "/boot/A")

    def test_render_brief_rows_formats_human_readable_lines(self) -> None:
        lines = MODULE.render_brief_rows(
            [
                {
                    "source_function": "func_801f1204",
                    "status": "ready_for_backend_diff",
                    "match_bucket": "excellent",
                    "match_metrics": {
                        "objdiff_match_percent": 99.0,
                        "asm_score": 12,
                    },
                    "attempt_count": 2,
                    "stalled": False,
                }
            ]
        )

        self.assertEqual(len(lines), 1)
        self.assertIn("func_801f1204", lines[0])
        self.assertIn("excellent", lines[0])
        self.assertIn("99.000", lines[0])
        self.assertIn("attempts 2", lines[0])
        self.assertIn("stalled no", lines[0])

    def test_collect_reports_reads_adjacent_history_summary(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            report_dir = root / "a"
            report_dir.mkdir(parents=True, exist_ok=True)
            (report_dir / "diff.json").write_text(
                json.dumps(
                    {
                        "program_path": "/boot/A",
                        "entry_hex": "0x1",
                        "status": "ready_for_backend_diff",
                        "match_metrics": {
                            "objdiff_match_percent": 50.0,
                            "asm_score": 8.0,
                        },
                    }
                ),
                encoding="utf-8",
            )
            (report_dir / "history.jsonl").write_text(
                json.dumps(
                    {
                        "event": "diff",
                        "program_path": "/boot/A",
                        "entry_hex": "0x1",
                        "match_metrics": {
                            "objdiff_match_percent": 50.0,
                            "asm_score": 8.0,
                        },
                    },
                    sort_keys=True,
                )
                + "\n",
                encoding="utf-8",
            )

            rows = MODULE.collect_reports(root)

        self.assertEqual(rows[0]["attempt_count"], 1)
        self.assertEqual(rows[0]["best_objdiff_match_percent"], 50.0)
        self.assertFalse(rows[0]["stalled"])


if __name__ == "__main__":
    unittest.main()
