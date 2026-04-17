from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.toolchain import old_gcc as MODULE


class OldGccToolchainTests(unittest.TestCase):
    def test_requested_compiler_ids_defaults_to_tested_matrix(self) -> None:
        result = MODULE.requested_compiler_ids(None, None)

        self.assertIn("gcc-2.7.0-mipsel", result)
        self.assertIn("gcc-2.95.2-psx", result)

    def test_requested_compiler_ids_dedupes_explicit_selection(self) -> None:
        result = MODULE.requested_compiler_ids(
            ["gcc-2.8.0-psx"],
            ["tested-matrix"],
        )

        self.assertEqual(result.count("gcc-2.8.0-psx"), 1)

    def test_installer_extracts_requested_compiler(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            download_dir = root / "downloads"
            dest_root = root / "toolchains"

            archive_path = download_dir / "gcc-2.7.0.tar.gz"
            archive_path.parent.mkdir(parents=True, exist_ok=True)
            archive_path.write_text("", encoding="utf-8")

            logger = type(
                "Logger",
                (),
                {
                    "summary": lambda self, message: None,
                    "error": lambda self, message: None,
                },
            )()

            with (
                patch.object(MODULE, "DEPS_DOWNLOAD_DIR", download_dir),
                patch.object(MODULE, "extract_tar_gz") as extract_tar_gz,
                patch.object(MODULE.shutil, "which", return_value="/usr/bin/gh"),
            ):
                extract_tar_gz.side_effect = (
                    lambda archive, dest: (dest / "gcc").write_text("", encoding="utf-8")
                )
                result = MODULE.DEFAULT_OLD_GCC_INSTALLER.install(
                    MODULE.OldGccInstallRequest(
                        dest_root=dest_root,
                        compiler_ids=("gcc-2.7.0-mipsel",),
                    ),
                    logger=logger,
                )

        self.assertEqual(result, 0)


if __name__ == "__main__":
    unittest.main()
