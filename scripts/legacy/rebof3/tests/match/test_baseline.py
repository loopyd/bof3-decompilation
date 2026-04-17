from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import baseline as MODULE


class BaselineTests(unittest.TestCase):
    def test_baseline_from_bundle_json_extracts_expected_metadata(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            previous_root = MODULE.ROOT
            try:
                MODULE.ROOT = root
                asm_path = root / "tmp" / "func.s"
                asm_path.parent.mkdir(parents=True, exist_ok=True)
                asm_path.write_text(".text\n", encoding="utf-8")
                bundle_path = root / "tmp" / "func.json"
                bundle_path.write_text(
                    json.dumps(
                        {
                            "files": {"asm": "tmp/func.s"},
                            "program_name": "SLUS_004.22",
                            "requested_address": "0x80162d00",
                            "function": {
                                "entry": "80162d00",
                                "name": "FUN_80162d00",
                                "signature": "undefined FUN_80162d00(void)",
                                "status": "ok",
                                "decompile_status": "ok",
                                "body_min": "80162d00",
                                "body_max": "80162d17",
                            },
                        }
                    ),
                    encoding="utf-8",
                )

                baseline = MODULE.baseline_from_bundle_json(bundle_path)

                assert baseline is not None
                self.assertEqual(baseline["kind"], "ghidra_decomp_function")
                self.assertEqual(baseline["asm_source"], "tmp/func.s")
                self.assertEqual(baseline["symbol_name"], "FUN_80162d00")
                self.assertEqual(baseline["entry_hex"], "0x80162d00")
                self.assertEqual(baseline["body_max_hex"], "0x80162d17")
            finally:
                MODULE.ROOT = previous_root


if __name__ == "__main__":
    unittest.main()
