from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import objdiff_backend as MODULE


class ObjdiffBackendTests(unittest.TestCase):
    def test_prepare_backend_writes_objdiff_config(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            previous_root = MODULE.ROOT
            try:
                MODULE.ROOT = root
                workspace_dir = root / "workspace"
                current_object = root / "tmp" / "current.o"
                expected_object = root / "tmp" / "expected.o"
                current_object.parent.mkdir(parents=True, exist_ok=True)
                current_object.write_bytes(b"obj")
                expected_object.write_bytes(b"obj")

                prepared = MODULE.prepare_backend(
                    workspace_dir,
                    {
                        "workspace_dir": "tmp/matching/foo",
                        "source_mapping": {"source_function": "emi_ready"},
                    },
                    asm_backend_report={
                        "current_object": "tmp/current.o",
                        "expected_object": "tmp/expected.o",
                    },
                )

                config_path = workspace_dir / "objdiff" / "objdiff.json"
                config = json.loads(config_path.read_text(encoding="utf-8"))
                self.assertEqual(prepared["backend"], "objdiff")
                self.assertEqual(config["units"][0]["name"], "emi_ready")
            finally:
                MODULE.ROOT = previous_root

    def test_backend_command_uses_objdiff_cli(self) -> None:
        prepared = {
            "backend_dir": "tmp/matching/foo/objdiff",
            "symbol_name": "emi_ready",
        }

        command = MODULE.backend_command(prepared)

        self.assertTrue(
            command[0].endswith("objdiff-cli") or command[0] == "objdiff-cli"
        )
        self.assertIn("diff", command)

    def test_summarize_objdiff_result_extracts_symbol_metrics(self) -> None:
        summary = MODULE.summarize_objdiff_result(
            {
                "left": {
                    "symbols": [
                        {
                            "name": "emi_ready",
                            "match_percent": 25.0,
                            "instructions": [
                                {"diff_kind": "DIFF_NONE"},
                                {"diff_kind": "DIFF_INSERT"},
                            ],
                        }
                    ]
                },
                "right": {},
            },
            "emi_ready",
        )

        self.assertEqual(summary["text_match_percent"], 25.0)
        self.assertEqual(summary["instruction_count"], 2)
        self.assertEqual(summary["mismatch_count"], 1)

    def test_summarize_objdiff_result_falls_back_to_right_symbol_payload(self) -> None:
        summary = MODULE.summarize_objdiff_result(
            {
                "left": {
                    "symbols": [
                        {
                            "name": "emi_ready",
                        }
                    ]
                },
                "right": {
                    "symbols": [
                        {
                            "name": "emi_ready",
                            "match_percent": 100.0,
                            "instructions": [
                                {"diff_kind": "DIFF_NONE"},
                                {"diff_kind": "DIFF_NONE"},
                            ],
                        }
                    ]
                },
            },
            "emi_ready",
        )

        self.assertEqual(summary["text_match_percent"], 100.0)
        self.assertEqual(summary["instruction_count"], 2)
        self.assertEqual(summary["mismatch_count"], 0)


if __name__ == "__main__":
    unittest.main()
