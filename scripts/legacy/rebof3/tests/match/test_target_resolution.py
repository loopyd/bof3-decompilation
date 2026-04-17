from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import target as MODULE


class TargetResolutionTests(unittest.TestCase):
    def test_find_function_row_matches_program_selector_and_entry(self) -> None:
        rows = [
            {
                "program_path": "/boot/SLUS_004.22",
                "program_name": "SLUS_004.22",
                "program_slug": "boot_slus_004_22",
                "entry": "80162d00",
            }
        ]

        row = MODULE.find_function_row(
            rows, program="/boot/SLUS_004.22", entry="0x80162d00"
        )

        self.assertEqual(row["program_slug"], "boot_slus_004_22")

    def test_find_function_row_can_infer_bundle_backed_program(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_root = root / "bof3"
            source_file = source_root / "src" / "core" / "emi" / "func_8016728c.c"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "unsigned int func_8016728c(void)\n{\n}\n",
                encoding="utf-8",
            )

            program_rows = [
                {
                    "program_name": "SLUS_004.22",
                    "program_path": "/boot/SLUS_004.22",
                    "program_slug": "boot_slus_004_22",
                    "folder": "/boot",
                    "source_hint": "build/extracted/SLUS_004.22",
                }
            ]
            artifacts_dir = (
                root
                / "tmp"
                / "ghidra_decomp"
                / "build"
                / "extracted"
                / "SLUS_004.22"
                / "0x8016728c"
            )
            artifacts_dir.mkdir(parents=True, exist_ok=True)
            (artifacts_dir / "func.json").write_text(
                '{"requested_address":"0x8016728c","load_address":null,'
                '"function":{"entry":"8016728c","requested_address":"0x8016728c"}}',
                encoding="utf-8",
            )

            row = MODULE.find_function_row(
                [],
                program="/boot/SLUS_004.22",
                entry="0x8016728c",
                program_rows=program_rows,
                artifact_root=root / "tmp" / "ghidra_decomp",
                source_root=source_root,
            )

        self.assertEqual(row["program_slug"], "boot_slus_004_22")
        self.assertEqual(row["name"], "func_8016728c")
        self.assertEqual(row["entry_hex"], "0x8016728c")


if __name__ == "__main__":
    unittest.main()
