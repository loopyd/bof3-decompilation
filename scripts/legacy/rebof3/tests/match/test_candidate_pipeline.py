from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.tasks import candidate as MODULE
from scripts.rebof3.tasks.candidate import common as candidate_common


class CandidatePipelineTests(unittest.TestCase):
    def test_candidate_include_directive_prefers_promoted_internal_header(self) -> None:
        stub_source = (
            candidate_common.ROOT / "bof3/stubs/modules/battle/15/func_80097eb8.c"
        ).resolve()

        include_line = MODULE.candidate_include_directive(
            "/bins/BIN/BATTLE/BATTLE/15.bin",
            "0x80097eb8",
            stub_source_path=stub_source,
        )

        self.assertIn("src/modules/battle/15/internal.h", include_line)

    def test_build_stub_configure_command_enables_stubs(self) -> None:
        command = MODULE.build_stub_configure_command(Path("/tmp/bof3-stubs-build"))

        self.assertIn("-DBOF3_ENABLE_STUBS=ON", command)
        self.assertIn("-G", command)
        self.assertIn("Ninja", command)

    def test_select_candidate_source_task_uses_m2c_bundle_artifact(self) -> None:
        task = MODULE.SelectCandidateSourceTask()
        with tempfile.TemporaryDirectory() as tmp_dir:
            m2c_path = Path(tmp_dir) / "func.m2c.c"
            m2c_path.write_text("s32 func_80097eb8(void) { return 1; }\n", encoding="utf-8")

            result = task.run(
                {
                    "bundle_payload": {
                        "files": {
                            "m2c_c": str(m2c_path),
                            "ghidra_c": None,
                        }
                    }
                },
                options={"candidate_variant": "m2c"},
            )

        self.assertEqual(result["candidate_source_variant"], "m2c")
        self.assertEqual(result["candidate_source_path"], m2c_path)
        self.assertIn("return 1;", result["candidate_source_text"])

    def test_build_candidate_workspace_payload_records_candidate_source(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp_path = Path(tmp_dir)
            asm_path = tmp_path / "func.s"
            asm_path.write_text(".text\n", encoding="utf-8")
            bundle_json = tmp_path / "func.json"
            bundle_json.write_text(
                json.dumps(
                    {
                        "files": {"asm": str(asm_path)},
                        "function": {
                            "entry": "0x80097eb8",
                            "name": "FUN_80097EB8",
                            "signature": "s32 FUN_80097EB8(void)",
                        },
                        "requested_address": "0x80097eb8",
                        "program_name": "BATTLE_e15_80096800.bin",
                    }
                ),
                encoding="utf-8",
            )
            source_file = tmp_path / "func_80097eb8.c"
            source_file.write_text("s32 func_80097eb8(void) { return 1; }\n", encoding="utf-8")

            row = {
                "program_name": "BATTLE_e15_80096800.bin",
                "program_path": "/bins/BIN/BATTLE/BATTLE/15.bin",
                "program_slug": "battle_e15_80096800_bin",
                "folder": "/bins/BIN/BATTLE/BATTLE",
                "entry": "80097eb8",
                "entry_hex": "0x80097eb8",
                "signature": "s32 FUN_80097EB8(void)",
                "namespace": None,
                "comment": None,
                "repeatable_comment": None,
                "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#15",
            }
            bundle_payload = {
                "artifacts_dir": str(tmp_path),
                "files": {"json": str(bundle_json)},
                "function": {"signature": "s32 FUN_80097EB8(void)"},
            }

            with patch.object(
                candidate_common.compile_one,
                "plan_compile_one",
                return_value={
                    "object_path": tmp_path / "obj" / "func_80097eb8.c.obj",
                },
            ):
                workspace_json, payload = MODULE.build_candidate_workspace_payload(
                    row,
                    inventory_db=tmp_path / "inventory.sqlite",
                    workspace_root=tmp_path / "workspaces",
                    build_root=tmp_path / "build",
                    source_file=source_file,
                    bundle_payload=bundle_payload,
                )

        self.assertTrue(str(workspace_json).endswith("workspace.json"))
        self.assertTrue(payload["source_mapping_ready"])
        self.assertTrue(payload["expected_baseline_ready"])
        self.assertEqual(
            payload["source_mapping"]["source_function"],
            "func_80097eb8",
        )
        self.assertEqual(
            payload["source_mapping"]["object_candidates"],
            [str((tmp_path / "obj" / "func_80097eb8.c.obj").resolve())],
        )


if __name__ == "__main__":
    unittest.main()
