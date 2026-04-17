from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.inventory import (
    emi_catalog,
    inventory,
    overlay_entry_tables,
    slot_map,
    unique_overlay_map,
)


class InventoryCliCleanupTests(unittest.TestCase):
    def test_inventory_build_main_initializes_sqlite_without_reports(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_dir = root / "processed" / "inventory"
            inventory_db = inventory_dir / "inventory.sqlite"

            with (
                patch.object(inventory, "INVENTORY_DIR", inventory_dir),
                patch.object(inventory, "INVENTORY_SQLITE", inventory_db),
            ):
                result = inventory.build_main()

            self.assertEqual(result, 0)
            self.assertTrue(inventory_db.exists())

    def test_slot_map_parse_args_supports_explicit_paths(self) -> None:
        args = slot_map.parse_args(
            [
                "--slus",
                "build/extracted/SLUS_004.22",
                "--disc-lba",
                "processed/inventory/disc_lba.json",
                "--slot-count",
                "4",
            ]
        )

        self.assertEqual(args.slot_count, 4)
        self.assertEqual(args.slus, Path("build/extracted/SLUS_004.22"))
        self.assertIsNone(args.json_out)

    def test_slot_map_parse_args_defaults_to_no_reports(self) -> None:
        args = slot_map.parse_args([])

        self.assertIsNone(args.json_out)
        self.assertIsNone(args.md_out)

    def test_emi_catalog_parse_args_defaults_to_no_reports(self) -> None:
        args = emi_catalog.parse_args([])

        self.assertIsNone(args.json_out)
        self.assertIsNone(args.md_out)

    def test_unique_overlay_map_parse_args_defaults_to_no_reports(self) -> None:
        args = unique_overlay_map.parse_args([])

        self.assertIsNone(args.json_out)
        self.assertIsNone(args.md_out)

    def test_overlay_entry_tables_parse_args_defaults_to_no_reports(self) -> None:
        args = overlay_entry_tables.parse_args([])

        self.assertIsNone(args.json_out)
        self.assertIsNone(args.md_out)

    def test_slot_map_main_writes_outputs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            slus_path = root / "SLUS_004.22"
            disc_lba_path = root / "disc_lba.json"
            json_out = root / "slot_map.json"
            md_out = root / "slot_map.md"

            slus_data = bytearray(slot_map.PSX_EXE_HEADER_SIZE + 8)
            slus_data[0x18:0x1C] = (slot_map.SLOT_TABLE_VADDR).to_bytes(4, "little")
            slus_data[0x1C:0x20] = (8).to_bytes(4, "little")
            slus_data[
                slot_map.PSX_EXE_HEADER_SIZE : slot_map.PSX_EXE_HEADER_SIZE + 8
            ] = (1).to_bytes(4, "little") + (2).to_bytes(4, "little")
            slus_path.write_bytes(slus_data)
            disc_lba_path.write_text(
                '{"entries": [{"lba": 1, "source_path": "build/extracted/A"}, {"lba": 2, "source_path": "build/extracted/B"}]}',
                encoding="utf-8",
            )

            result = slot_map.main(
                [
                    "--slus",
                    str(slus_path),
                    "--disc-lba",
                    str(disc_lba_path),
                    "--db",
                    str(root / "inventory.sqlite"),
                    "--json-out",
                    str(json_out),
                    "--md-out",
                    str(md_out),
                    "--slot-count",
                    "2",
                ]
            )

            self.assertEqual(result, 0)
            self.assertTrue(json_out.exists())
            self.assertTrue(md_out.exists())
            self.assertTrue((root / "inventory.sqlite").exists())

    def test_inventory_build_command_parser_uses_package_prog(self) -> None:
        parser = inventory.build_command_parser()

        self.assertEqual(parser.prog, "python3 -m scripts.rebof3 inventory build")


if __name__ == "__main__":
    unittest.main()
