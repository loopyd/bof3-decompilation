from __future__ import annotations

import io
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from subprocess import CompletedProcess
from types import SimpleNamespace
from unittest.mock import patch

from scripts.rebof3.logger import make_logger
from scripts.rebof3.re.services import m2c_runner
from scripts.rebof3.re.services.bootstrap import default_project_dir
from scripts.rebof3.re.services.ghidra import (
    bundle_export,
    decomp_parser,
    decomp_service,
    decomp_runtime,
)


class GhidraDecompRuntimeTests(unittest.TestCase):
    def test_parser_defaults_to_shared_project_name(self) -> None:
        parser = decomp_parser._build_ghidra_decomp_parser()

        args = parser.parse_args(["build/extracted/SLUS_004.22", "0x80100000"])

        self.assertEqual(args.project_name, "bof3_main")
        self.assertIsNone(args.project_dir)
        self.assertEqual(args.asm_backend, "ghidra")
        self.assertFalse(args.no_spimdisasm)
        self.assertFalse(getattr(args, "quiet", False))
        self.assertFalse(getattr(args, "verbose", False))

    def test_run_decomp_bundle_dry_run_uses_shared_project_by_default(self) -> None:
        returncode, payload = decomp_runtime.run_decomp_bundle(
            source_text="build/extracted/SLUS_004.22",
            address_text="0x80100000",
            dry_run=True,
        )

        assert payload is not None
        self.assertEqual(returncode, 0)
        self.assertEqual(payload["project_dir"], "tmp/bof3_ghidra/main")
        self.assertEqual(payload["commands"][0][7], str(default_project_dir()))
        self.assertIn("bof3_main", payload["commands"][0])

    def test_run_decomp_bundle_preserves_explicit_project_override(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            project_dir = Path(tmp_dir) / "isolated"
            returncode, payload = decomp_runtime.run_decomp_bundle(
                source_text="build/extracted/SLUS_004.22",
                address_text="0x80100000",
                project_dir=project_dir,
                project_name="bof3_decomp",
                dry_run=True,
            )

        assert payload is not None
        self.assertEqual(returncode, 0)
        self.assertEqual(payload["project_dir"], str(project_dir.resolve()))
        self.assertIn(str(project_dir), payload["commands"][0])
        self.assertIn("bof3_decomp", payload["commands"][0])

    def test_run_decomp_bundle_resolves_boot_program_path(self) -> None:
        returncode, payload = decomp_runtime.run_decomp_bundle(
            source_text="/boot/SLUS_004.22",
            address_text="0x80150098",
            dry_run=True,
        )

        assert payload is not None
        self.assertEqual(returncode, 0)
        self.assertEqual(
            payload["artifacts_dir"],
            "tmp/ghidra_decomp/build/extracted/SLUS_004.22/0x80150098",
        )
        self.assertIn("build/extracted/SLUS_004.22", payload["commands"][0])

    def test_legacy_project_name_maps_to_shared_default(self) -> None:
        self.assertEqual(bundle_export.resolve_project_name("bof3_decomp"), "bof3_main")

    def test_run_m2c_sidecar_writes_rewritten_asm_and_output(self) -> None:
        asm_text = (
            ".text\n\n"
            ".globl FUN_801ef27c\n"
            "FUN_801ef27c:\n"
            "/* 801ef2c4 */ andi v0, v1, 0xff\n"
            "/* 801ef2d8 */ bne v0, zero, 0x801ef2c4\n"
        )

        with tempfile.TemporaryDirectory() as tmp_dir:
            rewritten_asm_path = Path(tmp_dir) / "func.m2c.s"
            output_path = Path(tmp_dir) / "func.m2c.c"
            with patch.object(
                m2c_runner,
                "run_command",
                return_value=CompletedProcess(
                    args=["m2c"],
                    returncode=0,
                    stdout="void func(void) {}\n",
                    stderr="",
                ),
            ):
                metadata = m2c_runner.run_m2c_sidecar(
                    asm_text=asm_text,
                    rewritten_asm_path=rewritten_asm_path,
                    output_path=output_path,
                )
            rewritten_asm = rewritten_asm_path.read_text(encoding="utf-8")
            output_text = output_path.read_text(encoding="utf-8")

        self.assertEqual(metadata["status"], "ok")
        self.assertIsNotNone(metadata["path"])
        self.assertIn(".L801ef2c4:", rewritten_asm)
        self.assertEqual(output_text, "void func(void) {}\n")

    def test_run_decomp_bundle_dry_run_lists_m2c_asm_artifact(self) -> None:
        returncode, payload = decomp_runtime.run_decomp_bundle(
            source_text="build/extracted/SLUS_004.22",
            address_text="0x80100000",
            dry_run=True,
        )

        assert payload is not None
        self.assertEqual(returncode, 0)
        self.assertTrue(str(payload["commands"][-1][-1]).endswith("func.m2c.s"))
        self.assertIn("--context", payload["commands"][-1])
        self.assertTrue(
            any(
                str(command[-1]).endswith("func.m2c.ctx.c")
                for command in payload["commands"]
            )
        )
        self.assertEqual(payload["asm_backend"], "ghidra")
        self.assertTrue(payload["emit_spimdisasm"])

    def test_run_decomp_bundle_spim_backend_preserves_both_asm_artifacts(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            artifacts_dir = Path(tmp_dir) / "artifacts"

            def fake_bundle_export(**kwargs):
                return 0, {
                    "asm_text": ".text\njal CdSync\n",
                    "exported": [{}],
                    "function_payload": {
                        "name": "FUN_80161fdc",
                        "entry": "80161fdc",
                        "body_min": "80161fdc",
                        "body_max": "8016215c",
                    },
                    "ghidra_c": "void FUN_80161fdc(void) {}\n",
                    "project_dir": "tmp/project",
                    "project_name": "bof3_main",
                    "commands": [],
                }

            def fake_spimdisasm(**kwargs):
                output_path = kwargs["output_path"]
                output_path.parent.mkdir(parents=True, exist_ok=True)
                output_path.write_text(".text\njal func_80162160\n", encoding="utf-8")
                return {
                    "status": "ok",
                    "command": ["spimdisasm"],
                    "slice_path": "tmp/fake.bin",
                    "symbol_addrs_path": None,
                    "output_path": "tmp/fake.s",
                    "source_kind": "psx-exe",
                    "slice_start": "0x80161fdc",
                    "slice_end": "0x8016215c",
                }

            with (
                patch(
                    "scripts.rebof3.tasks.decomp.ghidra_bundle_export.run_bundle_export",
                    side_effect=fake_bundle_export,
                ),
                patch(
                    "scripts.rebof3.tasks.decomp.spimdisasm_asm.run_spimdisasm_function_asm",
                    side_effect=fake_spimdisasm,
                ),
            ):
                returncode, payload = decomp_runtime.run_decomp_bundle(
                    source_text="build/extracted/SLUS_004.22",
                    address_text="0x80161fdc",
                    artifacts_dir=artifacts_dir,
                    asm_backend="spimdisasm",
                    no_m2c=True,
                )

            assert payload is not None
            self.assertEqual(returncode, 0)
            self.assertEqual(payload["asm_backend"], "spimdisasm")
            self.assertIsNotNone(payload["files"]["ghidra_asm"])
            self.assertIsNotNone(payload["files"]["spim_asm"])
            canonical_asm = (artifacts_dir / "func.s").read_text(encoding="utf-8")
            ghidra_asm = (artifacts_dir / "func.ghidra.s").read_text(encoding="utf-8")
            spim_asm = (artifacts_dir / "func.spim.s").read_text(encoding="utf-8")
            self.assertEqual(canonical_asm, spim_asm)
            self.assertNotEqual(canonical_asm, ghidra_asm)

    def test_execute_args_uses_logger_for_bundle_output(self) -> None:
        args = SimpleNamespace(
            input="build/extracted/SLUS_004.22",
            address="0x80161fdc",
            project_dir=None,
            project_name="bof3_main",
            program_name=None,
            artifacts_dir=None,
            base_addr=None,
            loader_mode="auto",
            asm_backend="spimdisasm",
            no_spimdisasm=False,
            no_m2c=False,
            noanalysis=False,
            dry_run=False,
            quiet=False,
            verbose=True,
        )
        logger = SimpleNamespace(
            summary=unittest.mock.Mock(),
            item=unittest.mock.Mock(),
            detail=unittest.mock.Mock(),
            error=unittest.mock.Mock(),
        )
        payload = {
            "artifacts_dir": "tmp/ghidra_decomp/foo",
            "program_name": "SLUS_004.22",
            "asm_backend": "spimdisasm",
            "files": {
                "json": "tmp/ghidra_decomp/foo/func.json",
                "ghidra_c": "tmp/ghidra_decomp/foo/func.ghidra.c",
                "ghidra_asm": "tmp/ghidra_decomp/foo/func.ghidra.s",
                "spim_asm": "tmp/ghidra_decomp/foo/func.spim.s",
                "asm": "tmp/ghidra_decomp/foo/func.s",
                "m2c_context_source": "tmp/ghidra_decomp/foo/func.m2c.ctx.c",
                "m2c_context": "tmp/ghidra_decomp/foo/func.m2c.ctx.i",
                "m2c_asm": "tmp/ghidra_decomp/foo/func.m2c.s",
                "m2c_c": "tmp/ghidra_decomp/foo/func.m2c.c",
            },
            "m2c": {
                "attempted": True,
                "path": "tmp/ghidra_decomp/foo/func.m2c.c",
                "status": "ok",
                "stderr": None,
            },
        }

        with (
            patch.object(decomp_service, "logger_from_args", return_value=logger),
            patch.object(
                decomp_service.DEFAULT_GHIDRA_DECOMP_SERVICE,
                "run",
                return_value=(0, payload),
            ),
        ):
            result = decomp_service._execute_args(args)

        self.assertEqual(result, 0)
        logger.summary.assert_called_once()
        logger.item.assert_called()
        logger.detail.assert_called()

    def test_execute_args_quiet_suppresses_output(self) -> None:
        args = SimpleNamespace(
            input="build/extracted/SLUS_004.22",
            address="0x80161fdc",
            project_dir=None,
            project_name="bof3_main",
            program_name=None,
            artifacts_dir=None,
            base_addr=None,
            loader_mode="auto",
            asm_backend="ghidra",
            no_spimdisasm=False,
            no_m2c=True,
            noanalysis=False,
            dry_run=True,
            quiet=True,
            verbose=False,
        )
        payload = {
            "artifacts_dir": "tmp/ghidra_decomp/foo",
            "asm_backend": "ghidra",
            "commands": [["python3", "-m", "bof3_ghidra", "noop"]],
        }
        stdout = io.StringIO()

        with (
            patch.object(
                decomp_service,
                "logger_from_args",
                return_value=make_logger("ghidra_decomp", quiet=True, verbose=False),
            ),
            patch.object(
                decomp_service.DEFAULT_GHIDRA_DECOMP_SERVICE,
                "run",
                return_value=(0, payload),
            ),
            redirect_stdout(stdout),
        ):
            result = decomp_service._execute_args(args)

        self.assertEqual(result, 0)
        self.assertEqual(stdout.getvalue(), "")


if __name__ == "__main__":
    unittest.main()
