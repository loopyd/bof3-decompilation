from __future__ import annotations

import io
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock, patch

from scripts.rebof3.match import candidate_build as candidate_build_module
from scripts.rebof3.match import candidate_full as candidate_full_module
from scripts.rebof3.match import candidate_prepare as candidate_prepare_module
from scripts.rebof3.logger import make_logger


class CandidateCliTests(unittest.TestCase):
    def test_candidate_prepare_main_uses_shared_logger(self) -> None:
        args = SimpleNamespace(
            program="/bins/BIN/BATTLE/BATTLE/15.bin",
            entry="0x80097eb8",
            source_text="build/extracted/BIN/BATTLE/BATTLE.EMI#15",
            inventory_db=Path("inventory.sqlite"),
            workspace_root=Path("tmp/matching/candidates"),
            artifacts_dir=None,
            build_root=Path("build/bof3-psyq40-stubs"),
            asm_backend="spimdisasm",
            no_spimdisasm=False,
            no_m2c=False,
            candidate_variant="m2c",
            force_decomp=False,
            force_rewrite_source=False,
            force_reconfigure=False,
            quiet=False,
            verbose=True,
        )
        logger = SimpleNamespace(
            summary=Mock(),
            item=Mock(),
            detail=Mock(),
            error=Mock(),
        )
        fake_pipeline = SimpleNamespace(
            run=Mock(
                return_value={
                    "workspace_json": Path(
                        "tmp/matching/candidates/foo/workspace.json"
                    ),
                    "candidate_source_file": Path(
                        "bof3/stubs/modules/battle/15/func_80097eb8.c"
                    ),
                    "candidate_source_variant": "m2c",
                    "bundle_payload": {
                        "artifacts_dir": "tmp/ghidra_decomp/foo",
                        "asm_backend": "spimdisasm",
                        "files": {
                            "json": "tmp/ghidra_decomp/foo/func.json",
                            "ghidra_asm": "tmp/ghidra_decomp/foo/func.ghidra.s",
                            "spim_asm": "tmp/ghidra_decomp/foo/func.spim.s",
                            "m2c_c": "tmp/ghidra_decomp/foo/func.m2c.c",
                        },
                    },
                }
            )
        )

        with (
            patch.object(candidate_prepare_module, "parse_args", return_value=args),
            patch.object(
                candidate_prepare_module, "logger_from_args", return_value=logger
            ),
            patch.object(
                candidate_prepare_module,
                "pipeline_candidate_prepare",
                return_value=fake_pipeline,
            ),
        ):
            result = candidate_prepare_module.main()

        self.assertEqual(result, 0)
        logger.summary.assert_called_once()
        logger.item.assert_called()
        logger.detail.assert_called()

    def test_candidate_build_main_uses_shared_logger(self) -> None:
        args = SimpleNamespace(
            program=None,
            entry=None,
            workspace_json=Path("tmp/matching/candidates/foo/workspace.json"),
            inventory_db=Path("inventory.sqlite"),
            workspace_root=Path("tmp/matching/candidates"),
            build_root=Path("build/bof3-psyq40-stubs"),
            profile="capcom97-bof3",
            quiet=False,
            verbose=True,
        )
        logger = SimpleNamespace(
            summary=Mock(),
            item=Mock(),
            detail=Mock(),
            error=Mock(),
        )
        fake_context = {
            "workspace_json": Path("tmp/matching/candidates/foo/workspace.json"),
            "workspace_payload": {},
            "build_root": Path("build/bof3-psyq40-stubs"),
        }
        fake_pipeline = SimpleNamespace(
            run=Mock(
                return_value={
                    **fake_context,
                    "build_status": {
                        "log_path": "tmp/matching/candidates/foo/build.log",
                        "object_path": "build/foo.o",
                    },
                    "diff_report_path": Path("tmp/matching/candidates/foo/diff.json"),
                    "diff_report": {
                        "match_metrics": {
                            "semantic_status": "exact",
                            "asm_score": 0,
                            "asm_max_score": 200,
                            "objdiff_match_percent": 100.0,
                            "asm_row_count": 2,
                        }
                    },
                }
            )
        )

        with (
            patch.object(candidate_build_module, "parse_args", return_value=args),
            patch.object(
                candidate_build_module, "logger_from_args", return_value=logger
            ),
            patch.object(
                candidate_build_module,
                "resolve_existing_workspace_context",
                return_value=fake_context,
            ),
            patch.object(
                candidate_build_module,
                "pipeline_candidate_build",
                return_value=fake_pipeline,
            ),
        ):
            result = candidate_build_module.main()

        self.assertEqual(result, 0)
        logger.summary.assert_called_once()
        logger.item.assert_called()
        logger.detail.assert_called()

    def test_candidate_build_quiet_suppresses_output(self) -> None:
        args = SimpleNamespace(
            program=None,
            entry=None,
            workspace_json=Path("tmp/matching/candidates/foo/workspace.json"),
            inventory_db=Path("inventory.sqlite"),
            workspace_root=Path("tmp/matching/candidates"),
            build_root=Path("build/bof3-psyq40-stubs"),
            profile="capcom97-bof3",
            quiet=True,
            verbose=False,
        )
        fake_context = {
            "workspace_json": Path("tmp/matching/candidates/foo/workspace.json"),
            "workspace_payload": {},
            "build_root": Path("build/bof3-psyq40-stubs"),
        }
        fake_pipeline = SimpleNamespace(
            run=Mock(
                return_value={
                    **fake_context,
                    "build_status": {},
                    "diff_report_path": Path("tmp/matching/candidates/foo/diff.json"),
                    "diff_report": {
                        "match_metrics": {
                            "semantic_status": "exact",
                            "asm_score": 0,
                            "asm_max_score": 200,
                            "objdiff_match_percent": 100.0,
                            "asm_row_count": 2,
                        }
                    },
                }
            )
        )
        stdout = io.StringIO()

        with (
            patch.object(candidate_build_module, "parse_args", return_value=args),
            patch.object(
                candidate_build_module,
                "logger_from_args",
                return_value=make_logger("candidate_build", quiet=True, verbose=False),
            ),
            patch.object(
                candidate_build_module,
                "resolve_existing_workspace_context",
                return_value=fake_context,
            ),
            patch.object(
                candidate_build_module,
                "pipeline_candidate_build",
                return_value=fake_pipeline,
            ),
            redirect_stdout(stdout),
        ):
            result = candidate_build_module.main()

        self.assertEqual(result, 0)
        self.assertEqual(stdout.getvalue(), "")

    def test_candidate_build_verbose_emits_detail_output(self) -> None:
        args = SimpleNamespace(
            program=None,
            entry=None,
            workspace_json=Path("tmp/matching/candidates/foo/workspace.json"),
            inventory_db=Path("inventory.sqlite"),
            workspace_root=Path("tmp/matching/candidates"),
            build_root=Path("build/bof3-psyq40-stubs"),
            profile="capcom97-bof3",
            quiet=False,
            verbose=True,
        )
        fake_context = {
            "workspace_json": Path("tmp/matching/candidates/foo/workspace.json"),
            "workspace_payload": {},
            "build_root": Path("build/bof3-psyq40-stubs"),
        }
        fake_pipeline = SimpleNamespace(
            run=Mock(
                return_value={
                    **fake_context,
                    "build_status": {
                        "log_path": "tmp/matching/candidates/foo/build.log",
                    },
                    "diff_report_path": Path("tmp/matching/candidates/foo/diff.json"),
                    "diff_report": {
                        "match_metrics": {
                            "semantic_status": "exact",
                            "asm_score": 0,
                            "asm_max_score": 200,
                            "objdiff_match_percent": 100.0,
                            "asm_row_count": 2,
                        }
                    },
                }
            )
        )
        stdout = io.StringIO()

        with (
            patch.object(candidate_build_module, "parse_args", return_value=args),
            patch.object(
                candidate_build_module,
                "logger_from_args",
                return_value=make_logger("candidate_build", quiet=False, verbose=True),
            ),
            patch.object(
                candidate_build_module,
                "resolve_existing_workspace_context",
                return_value=fake_context,
            ),
            patch.object(
                candidate_build_module,
                "pipeline_candidate_build",
                return_value=fake_pipeline,
            ),
            redirect_stdout(stdout),
        ):
            result = candidate_build_module.main()

        self.assertEqual(result, 0)
        rendered = stdout.getvalue()
        self.assertIn("workspace=tmp/matching/candidates/foo/workspace.json", rendered)
        self.assertIn("[candidate_build] objdiff match 100.0%", rendered)

    def test_candidate_full_main_uses_shared_logger(self) -> None:
        args = SimpleNamespace(
            program="/bins/BIN/BATTLE/BATTLE/15.bin",
            entry="0x80097eb8",
            source_text="build/extracted/BIN/BATTLE/BATTLE.EMI#15",
            inventory_db=Path("inventory.sqlite"),
            workspace_root=Path("tmp/matching/candidates"),
            artifacts_dir=None,
            build_root=Path("build/bof3-psyq40-stubs"),
            asm_backend="spimdisasm",
            no_spimdisasm=False,
            no_m2c=False,
            candidate_variant="m2c",
            force_decomp=False,
            force_rewrite_source=False,
            force_reconfigure=False,
            profile="capcom97-bof3",
            permuter_variant="repo",
            permuter_timeout_seconds=1,
            permuter_threads=1,
            permuter_arg=[],
            quiet=False,
            verbose=True,
        )
        logger = SimpleNamespace(
            summary=Mock(),
            item=Mock(),
            detail=Mock(),
            error=Mock(),
        )
        fake_pipeline = SimpleNamespace(
            run=Mock(
                return_value={
                    "workspace_json": Path(
                        "tmp/matching/candidates/foo/workspace.json"
                    ),
                    "candidate_source_file": Path(
                        "bof3/stubs/modules/battle/15/func_80097eb8.c"
                    ),
                    "candidate_source_variant": "m2c",
                    "bundle_payload": {
                        "artifacts_dir": "tmp/ghidra_decomp/foo",
                        "asm_backend": "spimdisasm",
                        "files": {
                            "json": "tmp/ghidra_decomp/foo/func.json",
                            "ghidra_asm": "tmp/ghidra_decomp/foo/func.ghidra.s",
                            "spim_asm": "tmp/ghidra_decomp/foo/func.spim.s",
                            "m2c_c": "tmp/ghidra_decomp/foo/func.m2c.c",
                        },
                    },
                    "build_status": {
                        "log_path": "tmp/matching/candidates/foo/build.log",
                        "object_path": "build/foo.o",
                    },
                    "diff_report_path": Path("tmp/matching/candidates/foo/diff.json"),
                    "diff_report": {
                        "match_metrics": {
                            "semantic_status": "exact",
                            "asm_score": 0,
                            "asm_max_score": 200,
                            "objdiff_match_percent": 100.0,
                            "asm_row_count": 2,
                        }
                    },
                    "permuter": {
                        "log_path": "tmp/matching/candidates/foo/permuter.log",
                        "returncode": 124,
                        "timed_out": True,
                    },
                }
            )
        )

        with (
            patch.object(candidate_full_module, "parse_args", return_value=args),
            patch.object(
                candidate_full_module, "logger_from_args", return_value=logger
            ),
            patch.object(
                candidate_full_module,
                "pipeline_candidate_full",
                return_value=fake_pipeline,
            ),
        ):
            result = candidate_full_module.main()

        self.assertEqual(result, 0)
        self.assertGreaterEqual(logger.summary.call_count, 2)
        logger.item.assert_called()
        logger.detail.assert_called()


if __name__ == "__main__":
    unittest.main()
