from __future__ import annotations

import io
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from subprocess import CompletedProcess
from unittest.mock import patch

from scripts.rebof3.lib import (
    PipelineOptions,
    PipelineTask,
    options_with_logger,
    pipeline as build_pipeline,
)
from scripts.rebof3.logger import make_logger
from scripts.rebof3.tasks import decomp as decomp_tasks
from scripts.rebof3.tasks.decomp import generate_m2c_context as generate_m2c_context_module
from scripts.rebof3.tasks.decomp import run_m2c as run_m2c_module


class _AppendTask(PipelineTask):
    def __init__(self, marker: str):
        self.marker = marker

    def run(
        self,
        context: dict[str, object],
        *,
        options: PipelineOptions | None = None,
    ) -> dict[str, object]:
        context.setdefault("steps", []).append(self.marker)
        return context


class _OptionTask(PipelineTask):
    def run(
        self,
        context: dict[str, object],
        *,
        options: PipelineOptions | None = None,
    ) -> dict[str, object]:
        context["backend"] = None if options is None else options.get("asm_backend")
        return context


class PipelineTests(unittest.TestCase):
    def test_pipeline_runs_tasks_in_order(self) -> None:
        composed = build_pipeline(_AppendTask("a"), _AppendTask("b"))

        result = composed.run({})

        self.assertEqual(result["steps"], ["a", "b"])

    def test_pipeline_passes_shared_options_to_each_task(self) -> None:
        composed = build_pipeline(_OptionTask())

        result = composed.run({}, options={"asm_backend": "spimdisasm"})

        self.assertEqual(result["backend"], "spimdisasm")

    def test_pipeline_is_composable_as_a_task(self) -> None:
        nested = build_pipeline(_AppendTask("b"), _AppendTask("c"), task_name="nested")
        composed = build_pipeline(_AppendTask("a"), nested)

        result = composed.run({})

        self.assertEqual(result["steps"], ["a", "b", "c"])

    def test_pipeline_emits_task_debug_output_when_verbose(self) -> None:
        composed = build_pipeline(_AppendTask("a"), task_name="demo")
        stdout = io.StringIO()

        with redirect_stdout(stdout):
            composed.run(
                {},
                options=options_with_logger(
                    {"asm_backend": "ghidra"},
                    make_logger("demo", quiet=False, verbose=True),
                ),
            )

        output = stdout.getvalue()
        self.assertIn("[demo] start demo", output)
        self.assertIn("[demo.task] start", output)
        self.assertIn("[demo.task] done in", output)
        self.assertIn("[demo] done demo in", output)

    def test_normalize_asm_task_writes_rewritten_asm(self) -> None:
        task = decomp_tasks.NormalizeAsmForM2CTask()
        asm_text = (
            ".text\n\n"
            ".globl FUN_801ef27c\n"
            "FUN_801ef27c:\n"
            "/* 801ef2c4 */ andi v0, v1, 0xff\n"
            "/* 801ef2d8 */ bne v0, zero, 0x801ef2c4\n"
        )
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_path = Path(tmp_dir) / "func.m2c.s"
            result = task.run(
                {
                    "source_text": "build/extracted/SLUS_004.22",
                    "selected_asm_text": asm_text,
                    "m2c_asm_path": out_path,
                    "resolver": None,
                    "returncode": 0,
                },
                options={"asm_backend": "ghidra"},
            )

            rewritten = out_path.read_text(encoding="utf-8")

        self.assertIn(".L801ef2c4:", rewritten)
        self.assertIn("bne v0, zero, .L801ef2c4", rewritten)
        self.assertEqual(result["rewritten_asm"], rewritten)

    def test_run_m2c_task_writes_output(self) -> None:
        task = decomp_tasks.RunM2CTask()
        with tempfile.TemporaryDirectory() as tmp_dir:
            asm_path = Path(tmp_dir) / "func.m2c.s"
            c_path = Path(tmp_dir) / "func.m2c.c"
            ctx_path = Path(tmp_dir) / "func.m2c.ctx.i"
            asm_path.write_text(".text\n", encoding="utf-8")
            ctx_path.write_text("typedef int s32;\n", encoding="utf-8")
            with patch.object(
                run_m2c_module,
                "run_command",
                return_value=CompletedProcess(
                    args=["m2c"],
                    returncode=0,
                    stdout="void func(void) {}\n",
                    stderr="",
                ),
            ):
                result = task.run(
                    {
                        "no_m2c": False,
                        "m2c_asm_path": asm_path,
                        "m2c_c_path": c_path,
                        "m2c_context_paths": [ctx_path],
                        "selected_asm_backend": "ghidra",
                    }
                )

            self.assertEqual(c_path.read_text(encoding="utf-8"), "void func(void) {}\n")
            self.assertEqual(result["m2c_metadata"]["status"], "ok")
            self.assertEqual(
                result["m2c_metadata"]["context_paths"],
                [str(ctx_path.resolve())],
            )

    def test_generate_m2c_context_task_sets_context_paths(self) -> None:
        task = decomp_tasks.GenerateM2CContextTask()
        with tempfile.TemporaryDirectory() as tmp_dir:
            ctx_source = Path(tmp_dir) / "func.m2c.ctx.c"
            ctx_preprocessed = Path(tmp_dir) / "func.m2c.ctx.i"
            with patch.object(
                generate_m2c_context_module,
                "generate_m2c_context_artifacts",
                return_value={
                    "attempted": True,
                    "status": "ok",
                    "path": "tmp/func.m2c.ctx.i",
                    "stderr": None,
                },
            ):
                result = task.run(
                    {
                        "no_m2c": False,
                        "returncode": 0,
                        "source_text": "build/extracted/SLUS_004.22",
                        "requested_address": 0x80161FDC,
                        "selected_asm_text": ".text\n",
                        "program_name": "SLUS_004.22",
                        "m2c_context_source_path": ctx_source,
                        "m2c_context_preprocessed_path": ctx_preprocessed,
                    }
                )

            self.assertEqual(result["m2c_context_metadata"]["status"], "ok")
            self.assertEqual(result["m2c_context_paths"], [ctx_preprocessed])

    def test_select_asm_artifact_task_uses_spim_backend_when_requested(self) -> None:
        task = decomp_tasks.SelectAsmArtifactTask()
        with tempfile.TemporaryDirectory() as tmp_dir:
            asm_path = Path(tmp_dir) / "func.s"
            result = task.run(
                {
                    "asm_path": asm_path,
                    "ghidra_asm_text": "ghidra\n",
                    "spim_asm_text": "spim\n",
                },
                options={"asm_backend": "spimdisasm"},
            )

            selected = asm_path.read_text(encoding="utf-8")

        self.assertEqual(selected, "spim\n")
        self.assertEqual(result["selected_asm_backend"], "spimdisasm")


if __name__ == "__main__":
    unittest.main()
