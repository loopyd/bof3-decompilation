from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import workspace_store as MODULE


class WorkspaceStoreTests(unittest.TestCase):
    def test_workspace_ref_uses_program_slug_and_entry_hex(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            row = {
                "program_slug": "boot_slus_004_22",
                "entry": "80162d00",
            }

            ref = MODULE.workspace_ref(root, row)

        self.assertEqual(ref.dir_path, root / "boot_slus_004_22" / "0x80162d00")
        self.assertEqual(
            ref.json_path, root / "boot_slus_004_22" / "0x80162d00" / "workspace.json"
        )


if __name__ == "__main__":
    unittest.main()
