from __future__ import annotations

import tarfile
import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.toolchain import releases as MODULE


class ReleaseHelpersTests(unittest.TestCase):
    def test_extract_tar_gz_extracts_archive_contents(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            archive_path = root / "compiler.tar.gz"
            dest = root / "dest"
            source = root / "gcc"
            source.write_text("toolchain", encoding="utf-8")

            with tarfile.open(archive_path, "w:gz") as archive:
                archive.add(source, arcname="gcc")

            dest.mkdir(parents=True, exist_ok=True)
            MODULE.extract_tar_gz(archive_path, dest)

            self.assertEqual((dest / "gcc").read_text(encoding="utf-8"), "toolchain")

    def test_extract_tar_gz_rejects_path_traversal(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            archive_path = root / "compiler.tar.gz"
            dest = root / "dest"
            payload = root / "gcc"
            payload.write_text("toolchain", encoding="utf-8")

            with tarfile.open(archive_path, "w:gz") as archive:
                archive.add(payload, arcname="../gcc")

            dest.mkdir(parents=True, exist_ok=True)
            with self.assertRaises(RuntimeError):
                MODULE.extract_tar_gz(archive_path, dest)


if __name__ == "__main__":
    unittest.main()
