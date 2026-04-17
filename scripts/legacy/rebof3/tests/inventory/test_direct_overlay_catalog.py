from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.inventory import direct_overlay_catalog as MODULE


class DirectOverlayCatalogTests(unittest.TestCase):
    def test_archive_id_from_emi_path_uses_extracted_relative_layout(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            previous_root = MODULE.ROOT
            try:
                MODULE.ROOT = root
                emi_root = root / "build" / "extracted"
                path = emi_root / "BIN" / "ETC" / "GAME.EMI"

                archive_id = MODULE.archive_id_from_emi_path(path, emi_root)

                self.assertEqual(archive_id, "BIN/ETC/GAME")
            finally:
                MODULE.ROOT = previous_root

    def test_family_from_emi_path_prefers_bin_subdirectory(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            previous_root = MODULE.ROOT
            try:
                MODULE.ROOT = root
                emi_root = root / "build" / "extracted"
                path = emi_root / "BIN" / "SCENARIO" / "SCENA16.EMI"

                family = MODULE.family_from_emi_path(path, emi_root)

                self.assertEqual(family, "SCENARIO")
            finally:
                MODULE.ROOT = previous_root


if __name__ == "__main__":
    unittest.main()
