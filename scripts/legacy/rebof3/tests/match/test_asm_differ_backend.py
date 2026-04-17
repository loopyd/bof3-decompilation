from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import asm_differ_backend as MODULE


class AsmDifferBackendTests(unittest.TestCase):
    def test_choose_object_path_picks_existing_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            object_path = (
                root
                / "build"
                / "bof3-psyq40"
                / "bof3"
                / "CMakeFiles"
                / "bof3.dir"
                / "src"
                / "loader.c.obj"
            )
            object_path.parent.mkdir(parents=True, exist_ok=True)
            object_path.write_bytes(b"obj")
            workspace_payload = {
                "source_mapping": {"object_candidates": [str(object_path)]}
            }

            chosen = MODULE.choose_object_path(workspace_payload)

            self.assertEqual(chosen, object_path.resolve())

    def test_prepare_backend_writes_slice_inputs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            workspace_dir = root / "workspace"
            object_path = root / "loader.c.obj"
            baseline_asm = root / "func.s"
            object_path.write_bytes(b"obj")
            baseline_asm.write_text(
                ".text\n.globl FUN_80162d00\nFUN_80162d00:\n.word 0x3c028014\n",
                encoding="utf-8",
            )
            workspace_payload = {
                "workspace_dir": "tmp/matching/foo",
                "source_mapping": {
                    "object_candidates": [str(object_path)],
                    "source_function": "emi_ready",
                },
                "expected_baseline": {
                    "kind": "ghidra_decomp_function",
                    "asm_source": str(baseline_asm),
                    "symbol_name": "FUN_80162d00",
                },
            }

            previous_slice_from_object = MODULE.object_slices.slice_from_object
            previous_write_current = MODULE.object_slices.write_current_slice_asm
            previous_write_expected = MODULE.object_slices.write_expected_slice_asm
            previous_assemble = MODULE.object_slices.assemble_text
            try:
                MODULE.object_slices.slice_from_object = lambda path, symbol: (
                    MODULE.object_slices.FunctionSlice(  # type: ignore[assignment]
                        symbol_name=symbol,
                        start_offset=0x94,
                        size=0x40,
                        asm_text="00000094 <emi_ready>:\n  94: 3c028014 lui v0,0x8014\n",
                    )
                )
                MODULE.object_slices.write_current_slice_asm = (
                    lambda slice_data, output_path: (  # type: ignore[assignment]
                        output_path.parent.mkdir(parents=True, exist_ok=True),
                        output_path.write_text(
                            ".set noreorder\n.globl emi_ready\nemi_ready:\n.word 0x3c028014\n",
                            encoding="utf-8",
                        ),
                    )[-1]
                )
                MODULE.object_slices.write_expected_slice_asm = (
                    lambda baseline_asm_path, original_symbol_name, target_symbol_name, output_path, resolver=None: (
                        (  # type: ignore[assignment]
                            output_path.parent.mkdir(parents=True, exist_ok=True),
                            output_path.write_text(
                                ".set noreorder\n.globl emi_ready\nemi_ready:\n.word 0x3c028014\n",
                                encoding="utf-8",
                            ),
                        )[-1]
                    )
                )
                MODULE.object_slices.assemble_text = lambda asm_path, output_path: (  # type: ignore[assignment]
                    output_path.parent.mkdir(parents=True, exist_ok=True),
                    output_path.write_bytes(b"obj"),
                )

                prepared = MODULE.prepare_backend(workspace_dir, workspace_payload)
            finally:
                MODULE.object_slices.slice_from_object = previous_slice_from_object  # type: ignore[assignment]
                MODULE.object_slices.write_current_slice_asm = previous_write_current  # type: ignore[assignment]
                MODULE.object_slices.write_expected_slice_asm = previous_write_expected  # type: ignore[assignment]
                MODULE.object_slices.assemble_text = previous_assemble  # type: ignore[assignment]

            self.assertEqual(prepared["backend"], "asm-differ")
            self.assertEqual(prepared["baseline_kind"], "ghidra_decomp_function")
            self.assertEqual(prepared["baseline_symbol_name"], "FUN_80162d00")
            self.assertEqual(prepared["current_slice"]["start_offset"], 0x94)
            self.assertEqual(prepared["current_slice"]["size"], 0x40)
            self.assertTrue(
                (workspace_dir / "asm_differ" / "objects" / "current.o").exists()
            )
            self.assertTrue(
                (
                    workspace_dir / "asm_differ" / "expected" / "objects" / "current.o"
                ).exists()
            )

    def test_backend_command_uses_direct_tool_entry(self) -> None:
        prepared = {
            "backend_dir": "tmp/matching/foo/asm_differ",
            "current_object": "tmp/matching/foo/asm_differ/objects/current.o",
            "symbol_name": "emi_ready",
        }

        command = MODULE.backend_command(prepared)

        self.assertTrue(command[1].endswith("third_party/tools/asm-differ/diff.py"))
        self.assertIn("objects/current.o", command)

    def test_viewer_command_uses_interactive_flags(self) -> None:
        prepared = {
            "backend_dir": "tmp/matching/foo/asm_differ",
            "current_object": "tmp/matching/foo/asm_differ/objects/current.o",
            "symbol_name": "emi_ready",
        }

        command = MODULE.viewer_command(prepared)

        self.assertNotIn("-d", command)
        self.assertNotIn("--format", command)
        self.assertIn("objects/current.o", command)

    def test_strip_local_line_labels_invokes_objcopy_when_needed(self) -> None:
        commands: list[list[str]] = []
        previous_run_command = MODULE.run_command
        previous_objcopy = MODULE.OBJCOPY
        try:
            MODULE.OBJCOPY = Path("/tmp/fake-objcopy")

            def fake_run(command, **_kwargs):  # type: ignore[override]
                commands.append(list(command))
                if "-t" in command:
                    return type(
                        "Result",
                        (),
                        {
                            "returncode": 0,
                            "stdout": "00000004 l       .text\t00000000 LM3\n",
                            "stderr": "",
                        },
                    )()
                return type(
                    "Result",
                    (),
                    {
                        "returncode": 0,
                        "stdout": "",
                        "stderr": "",
                    },
                )()

            MODULE.run_command = fake_run  # type: ignore[assignment]
            MODULE.strip_local_line_labels(Path("dummy.o"))
        finally:
            MODULE.run_command = previous_run_command  # type: ignore[assignment]
            MODULE.OBJCOPY = previous_objcopy

        self.assertEqual(len(commands), 2)
        self.assertIn("--strip-symbol", commands[1])
        self.assertIn("LM3", commands[1])

    def test_write_backend_outputs_records_json_summary(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            previous_root = MODULE.ROOT
            try:
                MODULE.ROOT = root
                prepared = {
                    "backend": "asm-differ",
                    "backend_dir": "tmp/matching/foo/asm_differ",
                    "current_asm": "tmp/matching/foo/asm_differ/current/current.s",
                    "current_object": "tmp/matching/foo/asm_differ/objects/current.o",
                    "current_object_source": "build/current/loader.c.obj",
                    "expected_asm": "tmp/matching/foo/asm_differ/expected/expected.s",
                    "expected_object": "tmp/matching/foo/asm_differ/expected/objects/current.o",
                    "expected_asm_source": "tmp/ghidra/func.s",
                    "baseline_kind": "ghidra_decomp_function",
                    "baseline_symbol_name": "FUN_80162d00",
                    "diff_settings": "tmp/matching/foo/asm_differ/diff_settings.py",
                    "stdout_path": "tmp/matching/foo/asm_differ/diff.stdout.json",
                    "stderr_path": "tmp/matching/foo/asm_differ/diff.stderr.log",
                    "report_path": "tmp/matching/foo/asm_differ/backend.json",
                    "symbol_name": "emi_ready",
                    "current_slice": {"start_offset": 0x94, "size": 0x40},
                    "workspace_dir": "tmp/matching/foo",
                }
                result = type(
                    "Result",
                    (),
                    {
                        "returncode": 0,
                        "stdout": json.dumps(
                            {
                                "arch_str": "mips",
                                "current_score": 10,
                                "max_score": 0,
                                "rows": [{"key": "jr\tra"}],
                            }
                        ),
                        "stderr": "",
                    },
                )()

                report = MODULE.write_backend_outputs(prepared, result)

                self.assertTrue(report["succeeded"])
                self.assertEqual(report["diff_summary"]["arch_str"], "mips")
                self.assertEqual(report["diff_summary"]["row_count"], 1)
            finally:
                MODULE.ROOT = previous_root


if __name__ == "__main__":
    unittest.main()
