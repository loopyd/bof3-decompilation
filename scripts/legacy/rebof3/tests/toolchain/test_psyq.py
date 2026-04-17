from __future__ import annotations

import os
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest import mock

from rebof3.toolchain import psyq as MODULE


def _make_fake_psyq_tree(root: Path, *, crlf: bool = False) -> None:
    include_dir = root / "PSYQ" / "INCLUDE"
    lib_dir = root / "PSYQ" / "LIB"
    include_dir.mkdir(parents=True, exist_ok=True)
    lib_dir.mkdir(parents=True, exist_ok=True)
    libgpu_text = "#define setVector(v, _x, _y, _z) \\\n(v)->vx = _x\n"
    if crlf:
        libgpu_text = libgpu_text.replace("\n", "\r\n")
    (include_dir / "LIBGPU.H").write_text(libgpu_text, encoding="utf-8", newline="")
    (include_dir / "GPUCORE.H").write_text(
        ("#define GPU_FLAG 1\n" if not crlf else "#define GPU_FLAG 1\r\n"),
        encoding="utf-8",
        newline="",
    )
    (lib_dir / "LIBGPU.LIB").write_text("lib", encoding="utf-8")


class PsyqToolchainTests(unittest.TestCase):
    def test_discover_source_root_uses_explicit_source(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir) / "psyq-4.7"
            _make_fake_psyq_tree(root)

            with self.assertRaisesRegex(
                ValueError, "must stay inside the repo workspace"
            ):
                MODULE.discover_source_root(root)

    def test_discover_source_root_uses_repo_local_environment_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            repo_root = Path(tmp_dir) / "repo"
            env_root = repo_root / "inputs" / "psyq-4.7-converted-full"
            _make_fake_psyq_tree(env_root)

            with (
                mock.patch.object(MODULE, "ROOT", repo_root),
                mock.patch.dict(
                    os.environ, {"PSYQ_SOURCE": str(env_root)}, clear=False
                ),
            ):
                discovered = MODULE.discover_source_root()

        self.assertEqual(discovered, env_root)

    def test_source_root_looks_valid_false_for_missing_tree(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            self.assertFalse(MODULE.source_root_looks_valid(Path(tmp_dir) / "missing"))

    def test_discover_source_archive_uses_repo_local_environment_candidate(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            repo_root = Path(tmp_dir) / "repo"
            archive = repo_root / "inputs" / "psyq-4.7-converted-full.zip"
            archive.parent.mkdir(parents=True, exist_ok=True)
            archive.write_bytes(b"zip")

            with (
                mock.patch.object(MODULE, "ROOT", repo_root),
                mock.patch.dict(
                    os.environ, {"PSYQ_ARCHIVE": str(archive)}, clear=False
                ),
            ):
                discovered = MODULE.discover_source_archive()

        self.assertEqual(discovered, archive)

    def test_materialized_source_root_extracts_zip_archive(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            archive = root / "psyq.zip"
            with zipfile.ZipFile(archive, "w") as zf:
                zf.writestr("bundle/INCLUDE/LIBGPU.H", "// gpu")
                zf.writestr("bundle/LIB/LIBGPU.LIB", "lib")

            with MODULE.materialized_source_root(
                MODULE.PsyqSourceInput(kind="archive", path=archive)
            ) as materialized:
                self.assertTrue(MODULE.source_root_looks_valid(materialized))

    def test_original_sdk_is_ready_rejects_crlf_anywhere_in_staged_tree(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            include_dir = root / "include"
            lib_dir = root / "lib"
            include_dir.mkdir(parents=True, exist_ok=True)
            lib_dir.mkdir(parents=True, exist_ok=True)
            (include_dir / "libgpu.h").write_text("// gpu\n", encoding="utf-8")
            (include_dir / "gpucore.h").write_text(
                "#define GPU_FLAG 1\r\n",
                encoding="utf-8",
                newline="",
            )

            self.assertFalse(MODULE.original_sdk_is_ready(root))

    def test_stage_normalizes_header_newlines(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_root = root / "psyq-source"
            dest_root = root / "psyq-staged"
            _make_fake_psyq_tree(source_root, crlf=True)
            logger = type(
                "Logger",
                (),
                {
                    "summary": lambda self, message: None,
                    "item": lambda self, message: None,
                    "error": lambda self, message: None,
                },
            )()

            result = MODULE.DEFAULT_PSYQ_ORIGINAL_STAGER.stage(
                MODULE.PsyqOriginalStageRequest(
                    source_root=source_root,
                    dest=dest_root,
                ),
                logger=logger,
            )

            self.assertEqual(result, 0)
            self.assertTrue(MODULE.original_sdk_is_ready(dest_root))
            self.assertFalse(MODULE.file_uses_crlf(dest_root / "include" / "LIBGPU.H"))
            self.assertFalse(MODULE.file_uses_crlf(dest_root / "include" / "GPUCORE.H"))

    def test_repair_staged_sdk_roots_repairs_all_versions_under_parent(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            first_root = root / "psyq-original" / "4.0"
            second_root = root / "psyq-original" / "4.1"
            _make_fake_psyq_tree(root / "source-40", crlf=True)
            _make_fake_psyq_tree(root / "source-41", crlf=True)
            logger = type(
                "Logger",
                (),
                {
                    "summary": lambda self, message: None,
                    "item": lambda self, message: None,
                    "error": lambda self, message: None,
                },
            )()

            MODULE.DEFAULT_PSYQ_ORIGINAL_STAGER.stage(
                MODULE.PsyqOriginalStageRequest(
                    source_root=root / "source-40",
                    dest=first_root,
                ),
                logger=logger,
            )
            MODULE.DEFAULT_PSYQ_ORIGINAL_STAGER.stage(
                MODULE.PsyqOriginalStageRequest(
                    source_root=root / "source-41",
                    dest=second_root,
                ),
                logger=logger,
            )
            (first_root / "include" / "GPUCORE.H").write_text(
                "#define GPU_FLAG 1\r\n",
                encoding="utf-8",
                newline="",
            )
            (second_root / "include" / "GPUCORE.H").write_text(
                "#define GPU_FLAG 1\r\n",
                encoding="utf-8",
                newline="",
            )

            result = MODULE.repair_staged_sdk_roots(
                root / "psyq-original", logger=logger
            )

            self.assertEqual(result, 0)
            self.assertFalse(
                MODULE.file_uses_crlf(first_root / "include" / "GPUCORE.H")
            )
            self.assertFalse(
                MODULE.file_uses_crlf(second_root / "include" / "GPUCORE.H")
            )


if __name__ == "__main__":
    unittest.main()
