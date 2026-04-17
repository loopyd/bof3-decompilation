from __future__ import annotations

import os
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import permuter as MODULE


class PermuterTests(unittest.TestCase):
    def test_sanitize_repo_source_text_strips_gnu_attributes(self) -> None:
        text = (
            'void __attribute__((optimize("no-optimize-sibling-calls"))) '
            "func_8014b8b0(void) {\n"
            "  return;\n"
            "}\n"
        )

        sanitized = MODULE.sanitize_repo_source_text(text)

        self.assertNotIn("__attribute__", sanitized)
        self.assertIn("void  func_8014b8b0(void)", sanitized)

    def test_default_threads_divides_cpu_by_active_agents(self) -> None:
        with (
            mock.patch.object(MODULE.os, "cpu_count", return_value=12),
            mock.patch.object(MODULE, "active_agent_count", return_value=3),
        ):
            self.assertEqual(MODULE.default_threads(), 4)

    def test_sanitize_unknown_function_pointer_casts_rewrites_m2c_placeholders(
        self,
    ) -> None:
        text = (
            "(? (*)(void *))0x8017EE0C(temp_a0);\n"
            "((s32 (*)(?, ?, ?))0x80175640)(1, ptr, 0xFF);\n"
            "((? (*)())0x80162178)();\n"
        )

        sanitized = MODULE.sanitize_unknown_function_pointer_casts(text)

        self.assertIn("(void* (*)(void *))0x8017EE0C", sanitized)
        self.assertIn("(s32 (*)(void*, void*, void*))0x80175640", sanitized)
        self.assertIn("(void* (*)())0x80162178", sanitized)

    def test_sanitize_variant_source_adds_m2c_support_prelude(self) -> None:
        text = (
            "extern M2C_UNK D_80143B40;\n"
            "extern M2C_UNK *D_80143D40;\n"
            "void FUN_8014B73C(void) {\n"
            "    M2C_FIELD(D_80143D40, u16 *, 0) = 0x7F;\n"
            "}\n"
        )

        sanitized = MODULE.sanitize_variant_source(text, func_name="func_8014b73c")

        self.assertIn("typedef void* M2C_UNK;", sanitized)
        self.assertIn("#define M2C_FIELD(ptr, type, offset)", sanitized)
        self.assertIn("extern M2C_UNK DAT_80143b40;", sanitized)
        self.assertIn("extern M2C_UNK *DAT_80143d40;", sanitized)
        self.assertIn("void func_8014b73c(void)", sanitized)

    def test_sanitize_variant_source_drops_duplicate_function_prototype(self) -> None:
        text = "void func_8014b73c(void);\nvoid FUN_8014B73C(void) {\n}\n"

        sanitized = MODULE.sanitize_variant_source(text, func_name="func_8014b73c")

        self.assertNotIn("void func_8014b73c(void);\n", sanitized)
        self.assertIn("void func_8014b73c(void)", sanitized)

    def test_parse_args_defaults_timeout_to_standard_task_run(self) -> None:
        args = MODULE.parse_args(
            ["--program", "/boot/SLUS_004.22", "--entry", "0x80162d00"]
        )

        self.assertEqual(args.timeout_seconds, 60)

    def test_parse_args_accepts_auto_variant(self) -> None:
        args = MODULE.parse_args(
            [
                "--program",
                "/boot/SLUS_004.22",
                "--entry",
                "0x80162d00",
                "--variant",
                "auto",
            ]
        )

        self.assertEqual(args.variant, "auto")
        self.assertFalse(args.stdout)

    def test_parse_args_accepts_stdout_streaming(self) -> None:
        args = MODULE.parse_args(
            [
                "--program",
                "/boot/SLUS_004.22",
                "--entry",
                "0x80162d00",
                "--stdout",
            ]
        )

        self.assertTrue(args.stdout)

    def test_active_agent_count_prefers_env_override(self) -> None:
        with mock.patch.dict(os.environ, {"REBOF3_ACTIVE_AGENTS": "5"}, clear=False):
            self.assertEqual(MODULE.active_agent_count(), 5)

    def test_detect_active_opencode_agents_counts_unique_pairs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            proc_root = Path(tmp_dir)
            self._write_environ(
                proc_root / "100" / "environ",
                b"OPENCODE=1\0OPENCODE_PID=10\0AGENT=1\0",
            )
            self._write_environ(
                proc_root / "101" / "environ",
                b"OPENCODE=1\0OPENCODE_PID=10\0AGENT=1\0",
            )
            self._write_environ(
                proc_root / "200" / "environ",
                b"OPENCODE=1\0OPENCODE_PID=10\0AGENT=2\0",
            )
            self._write_environ(
                proc_root / "300" / "environ",
                b"OPENCODE=1\0OPENCODE_PID=20\0AGENT=1\0",
            )

            self.assertEqual(MODULE.detect_active_opencode_agents(proc_root), 3)

    def test_run_permuter_returns_timeout_result(self) -> None:
        with mock.patch.object(
            MODULE.subprocess,
            "run",
            side_effect=MODULE.subprocess.TimeoutExpired(
                cmd=["python3", "permuter.py"],
                timeout=15,
                output="partial stdout",
                stderr="partial stderr",
            ),
        ):
            result, timed_out = MODULE.run_permuter(
                ["python3", "permuter.py"], timeout_seconds=15
            )

        self.assertTrue(timed_out)
        self.assertEqual(result.returncode, 124)
        self.assertEqual(result.stdout, "partial stdout")
        self.assertEqual(result.stderr, "partial stderr")

    def test_run_permuter_streams_output_to_log_file(self) -> None:
        class FakePopen:
            def __init__(self, command, *, stdout=None, stderr=None, text=None):
                _ = (command, stderr, text)
                self._stdout = stdout
                self._stdout.write("permuter output\n")
                self._stdout.flush()

            def wait(self, timeout=None):
                _ = timeout
                return 0

            def kill(self):
                return None

        with tempfile.TemporaryDirectory() as tmp_dir:
            log_path = Path(tmp_dir) / "permuter.log"
            with mock.patch.object(MODULE.subprocess, "Popen", FakePopen):
                result, timed_out = MODULE.run_permuter(
                    ["python3", "permuter.py"],
                    timeout_seconds=15,
                    log_path=log_path,
                )

            self.assertFalse(timed_out)
            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, "")
            self.assertEqual(result.stderr, "")
            self.assertEqual(log_path.read_text(encoding="utf-8"), "permuter output\n")

    def test_run_permuter_can_stream_directly_to_stdout(self) -> None:
        completed = MODULE.subprocess.CompletedProcess(
            ["python3", "permuter.py"], 0, stdout=None, stderr=None
        )

        with mock.patch.object(
            MODULE.subprocess,
            "run",
            return_value=completed,
        ) as run_mock:
            result, timed_out = MODULE.run_permuter(
                ["python3", "permuter.py"],
                timeout_seconds=15,
                stream_stdout=True,
            )

        self.assertFalse(timed_out)
        self.assertIs(result, completed)
        run_mock.assert_called_once_with(
            ["python3", "permuter.py"],
            check=False,
            timeout=15,
        )

    def test_resolve_variant_source_prefers_repo_mapping(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_file = root / "bof3" / "src" / "modules" / "battle" / "03"
            source_file.mkdir(parents=True, exist_ok=True)
            variant_file = source_file / "func_801d9304.c"
            variant_file.write_text(
                "void func_801d9304(void) {}\n",
                encoding="utf-8",
            )

            original_root = MODULE.ROOT
            original_seed_root = MODULE.seed_sources.config.ROOT
            try:
                MODULE.ROOT = root
                MODULE.seed_sources.config.ROOT = root
                path, variant = MODULE.resolve_variant_source(
                    {
                        "source_mapping": {
                            "source_file": "bof3/src/modules/battle/03/func_801d9304.c",
                            "source_function": "func_801d9304",
                        }
                    },
                    variant="repo",
                )
            finally:
                MODULE.ROOT = original_root
                MODULE.seed_sources.config.ROOT = original_seed_root

        self.assertEqual(variant, "repo")
        self.assertEqual(path, variant_file)

    def test_resolve_variant_source_auto_prefers_meaningful_repo(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_file = root / "bof3" / "src" / "core" / "emi" / "func_80162d00.c"
            artifacts_dir = root / "tmp" / "ghidra_decomp" / "boot" / "0x80162d00"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            artifacts_dir.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "int func_80162d00(void) { return 1; }\n",
                encoding="utf-8",
            )
            (artifacts_dir / "func.m2c.c").write_text(
                "int func_80162d00(void) { return 2; }\n",
                encoding="utf-8",
            )
            (artifacts_dir / "func.ghidra.c").write_text(
                "int FUN_80162d00(void) { return 3; }\n",
                encoding="utf-8",
            )

            original_root = MODULE.ROOT
            original_seed_root = MODULE.seed_sources.config.ROOT
            try:
                MODULE.ROOT = root
                MODULE.seed_sources.config.ROOT = root
                path, variant = MODULE.resolve_variant_source(
                    {
                        "name": "func_80162d00",
                        "source_mapping": {
                            "source_file": "bof3/src/core/emi/func_80162d00.c",
                            "source_function": "func_80162d00",
                        },
                        "ghidra_decomp_artifacts_dir": "tmp/ghidra_decomp/boot/0x80162d00",
                    },
                    variant="auto",
                )
            finally:
                MODULE.ROOT = original_root
                MODULE.seed_sources.config.ROOT = original_seed_root

        self.assertEqual(variant, "repo")
        self.assertEqual(path, source_file)

    def test_resolve_variant_source_auto_falls_back_to_m2c_then_ghidra(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_file = root / "bof3" / "src" / "core" / "emi" / "func_80162d00.c"
            artifacts_dir = root / "tmp" / "ghidra_decomp" / "boot" / "0x80162d00"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            artifacts_dir.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "int unrelated(void) { return 0; }\n", encoding="utf-8"
            )
            m2c_file = artifacts_dir / "func.m2c.c"
            ghidra_file = artifacts_dir / "func.ghidra.c"
            m2c_file.write_text(
                "int func_80162d00(void) { return 2; }\n",
                encoding="utf-8",
            )
            ghidra_file.write_text(
                "int FUN_80162d00(void) { return 3; }\n",
                encoding="utf-8",
            )

            original_root = MODULE.ROOT
            original_seed_root = MODULE.seed_sources.config.ROOT
            try:
                MODULE.ROOT = root
                MODULE.seed_sources.config.ROOT = root
                payload = {
                    "name": "func_80162d00",
                    "source_mapping": {
                        "source_file": "bof3/src/core/emi/func_80162d00.c",
                        "source_function": "func_80162d00",
                    },
                    "ghidra_decomp_artifacts_dir": "tmp/ghidra_decomp/boot/0x80162d00",
                }
                path, variant = MODULE.resolve_variant_source(payload, variant="auto")
                m2c_file.unlink()
                fallback_path, fallback_variant = MODULE.resolve_variant_source(
                    payload, variant="auto"
                )
            finally:
                MODULE.ROOT = original_root
                MODULE.seed_sources.config.ROOT = original_seed_root

        self.assertEqual(variant, "m2c")
        self.assertEqual(path, m2c_file)
        self.assertEqual(fallback_variant, "ghidra")
        self.assertEqual(fallback_path, ghidra_file)

    def test_prepare_permuter_dir_copies_expected_assets(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            workspace_dir = root / "tmp" / "matching" / "foo" / "0x1"
            expected_dir = workspace_dir / "asm_differ" / "expected"
            expected_obj_dir = expected_dir / "objects"
            ghidra_dir = root / "tmp" / "ghidra_decomp" / "foo" / "0x1"
            compile_commands = root / "build" / "compile_commands.json"
            ghidra_dir.mkdir(parents=True, exist_ok=True)
            compile_commands.parent.mkdir(parents=True, exist_ok=True)
            compile_commands.write_text("[]\n", encoding="utf-8")
            (ghidra_dir / "func.ghidra.c").write_text(
                "void FUN_00000001(void) {}\n", encoding="utf-8"
            )
            expected_dir.mkdir(parents=True, exist_ok=True)
            expected_obj_dir.mkdir(parents=True, exist_ok=True)
            (expected_dir / "expected.s").write_text(".text\n", encoding="utf-8")
            (expected_obj_dir / "current.o").write_bytes(b"OBJ")

            payload = {
                "workspace_dir": "tmp/matching/foo/0x1",
                "program_path": "/bins/BIN/BATTLE/BATTLE/3.bin",
                "entry_hex": "0x00000001",
                "source_mapping": {
                    "source_function": "func_00000001",
                    "source_file": "bof3/src/modules/battle/03/func_801d9304.c",
                },
                "ghidra_decomp_artifacts_dir": "tmp/ghidra_decomp/foo/0x1",
            }

            original_root = MODULE.ROOT
            original_seed_root = MODULE.seed_sources.config.ROOT
            try:
                MODULE.ROOT = root
                MODULE.seed_sources.config.ROOT = root
                original_prepare = MODULE.asm_differ_backend.prepare_backend
                MODULE.asm_differ_backend.prepare_backend = lambda _a, _b: {
                    "backend_dir": "tmp/matching/foo/0x1/asm_differ"
                }
                prepared = MODULE.prepare_permuter_dir(
                    workspace_dir / "workspace.json",
                    payload,
                    compile_commands=compile_commands,
                    variant="ghidra",
                )
            finally:
                MODULE.ROOT = original_root
                MODULE.seed_sources.config.ROOT = original_seed_root
                MODULE.asm_differ_backend.prepare_backend = original_prepare

            perm_dir = Path(str(prepared["permuter_dir"]))
            if not perm_dir.is_absolute():
                perm_dir = root / perm_dir
            self.assertTrue((perm_dir / "base.c").exists())
            self.assertTrue((perm_dir / "target.s").exists())
            self.assertTrue((perm_dir / "target.o").exists())

    def test_prepare_permuter_dir_reuses_existing_setup_when_inputs_match(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            workspace_dir = root / "tmp" / "matching" / "foo" / "0x1"
            expected_dir = workspace_dir / "asm_differ" / "expected"
            expected_obj_dir = expected_dir / "objects"
            ghidra_dir = root / "tmp" / "ghidra_decomp" / "foo" / "0x1"
            compile_commands = root / "build" / "compile_commands.json"
            ghidra_dir.mkdir(parents=True, exist_ok=True)
            compile_commands.parent.mkdir(parents=True, exist_ok=True)
            compile_commands.write_text("[]\n", encoding="utf-8")
            (ghidra_dir / "func.ghidra.c").write_text(
                "void FUN_00000001(void) {}\n",
                encoding="utf-8",
            )
            expected_dir.mkdir(parents=True, exist_ok=True)
            expected_obj_dir.mkdir(parents=True, exist_ok=True)
            (expected_dir / "expected.s").write_text(".text\n", encoding="utf-8")
            (expected_obj_dir / "current.o").write_bytes(b"OBJ")

            payload = {
                "workspace_dir": "tmp/matching/foo/0x1",
                "program_path": "/bins/BIN/BATTLE/BATTLE/3.bin",
                "entry_hex": "0x00000001",
                "source_mapping": {
                    "source_function": "func_00000001",
                    "source_file": "bof3/src/modules/battle/03/func_801d9304.c",
                },
                "ghidra_decomp_artifacts_dir": "tmp/ghidra_decomp/foo/0x1",
            }

            original_root = MODULE.ROOT
            original_seed_root = MODULE.seed_sources.config.ROOT
            original_prepare = MODULE.asm_differ_backend.prepare_backend
            try:
                MODULE.ROOT = root
                MODULE.seed_sources.config.ROOT = root
                MODULE.asm_differ_backend.prepare_backend = lambda _a, _b: {
                    "backend_dir": "tmp/matching/foo/0x1/asm_differ"
                }
                first = MODULE.prepare_permuter_dir(
                    workspace_dir / "workspace.json",
                    payload,
                    compile_commands=compile_commands,
                    variant="ghidra",
                )
                perm_dir = root / str(first["permuter_dir"])
                (perm_dir / "target.s").write_text("kept\n", encoding="utf-8")
                second = MODULE.prepare_permuter_dir(
                    workspace_dir / "workspace.json",
                    payload,
                    compile_commands=compile_commands,
                    variant="ghidra",
                )
            finally:
                MODULE.ROOT = original_root
                MODULE.seed_sources.config.ROOT = original_seed_root
                MODULE.asm_differ_backend.prepare_backend = original_prepare

            kept_target = (perm_dir / "target.s").read_text(encoding="utf-8")

        self.assertEqual(first["permuter_dir"], second["permuter_dir"])
        self.assertEqual(kept_target, "kept\n")

    def _write_environ(self, path: Path, data: bytes) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)


if __name__ == "__main__":
    unittest.main()
