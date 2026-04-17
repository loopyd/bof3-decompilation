from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.inventory import overlay_catalog, overlay_clusters


class InventoryMergeTests(unittest.TestCase):
    def test_overlay_catalog_parse_args_supports_outputs(self) -> None:
        args = overlay_catalog.parse_args(
            [
                "--emi-root",
                "build/extracted/BIN",
                "--json-out",
                "tmp/candidates.json",
                "--md-out",
                "tmp/candidates.md",
            ]
        )

        self.assertEqual(args.emi_root, Path("build/extracted/BIN"))
        self.assertEqual(args.json_out, Path("tmp/candidates.json"))
        self.assertEqual(args.md_out, Path("tmp/candidates.md"))

    def test_overlay_catalog_parse_args_defaults_to_no_reports(self) -> None:
        args = overlay_catalog.parse_args([])

        self.assertIsNone(args.json_out)
        self.assertIsNone(args.md_out)

    def test_overlay_clusters_parse_args_supports_outputs(self) -> None:
        args = overlay_clusters.parse_args(
            ["--json-out", "tmp/clusters.json", "--md-out", "tmp/clusters.md"]
        )

        self.assertEqual(args.json_out, Path("tmp/clusters.json"))
        self.assertEqual(args.md_out, Path("tmp/clusters.md"))

    def test_overlay_clusters_parse_args_defaults_to_no_reports(self) -> None:
        args = overlay_clusters.parse_args([])

        self.assertIsNone(args.json_out)
        self.assertIsNone(args.md_out)

    def test_overlay_catalog_main_writes_outputs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            emi_root = root / "build" / "extracted" / "BIN"
            json_out = root / "candidates.json"
            md_out = root / "candidates.md"
            catalog = {
                "generated_from": "build/extracted/BIN",
                "candidate_count": 1,
                "family_counts": {"ETC": 1},
                "load_address_counts": {"0x801d0c00": 1},
                "unique_payload_hashes": 1,
                "candidates": [
                    {
                        "family": "ETC",
                        "candidate_name": "ovl_etc_game_e01_801d0c00",
                        "payload_path": "build/extracted/BIN/ETC/GAME.EMI#1",
                        "ram_ptr_hex": "0x801d0c00",
                        "size": 4096,
                        "duplicate_group_size": 1,
                    }
                ],
            }

            with patch.object(overlay_catalog, "build_catalog", return_value=catalog):
                result = overlay_catalog.main(
                    [
                        "--emi-root",
                        str(emi_root),
                        "--json-out",
                        str(json_out),
                        "--md-out",
                        str(md_out),
                    ]
                )

            self.assertEqual(result, 0)
            self.assertEqual(
                json.loads(json_out.read_text(encoding="utf-8")),
                catalog,
            )
            markdown = md_out.read_text(encoding="utf-8")
            self.assertIn("# Overlay Candidates", markdown)
            self.assertIn("ovl_etc_game_e01_801d0c00", markdown)


if __name__ == "__main__":
    unittest.main()
