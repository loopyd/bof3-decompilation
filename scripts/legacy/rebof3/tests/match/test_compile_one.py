from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.match import compile_one as MODULE


class MatchCompileOneTests(unittest.TestCase):
    def test_plan_compile_one_uses_compile_commands_output(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            build_root = root / "build" / "bof3-psyq40"
            build_root.mkdir(parents=True, exist_ok=True)
            source_file = root / "bof3" / "src" / "modules" / "battle" / "03" / "func.c"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text("void func(void) {}\n", encoding="utf-8")
            compile_commands = build_root / "compile_commands.json"
            compile_commands.write_text(
                json.dumps(
                    [
                        {
                            "directory": str(root),
                            "file": str(source_file),
                            "output": "build/current/func.o",
                            "command": f"cc -Iinclude -o build/current/func.o -c {source_file}",
                        }
                    ]
                ),
                encoding="utf-8",
            )

            workspace_payload = {
                "workspace_dir": "tmp/matching/foo",
                "source_mapping": {"source_file": str(source_file)},
            }

            plan = MODULE.plan_compile_one(workspace_payload, build_root=build_root)

        self.assertEqual(plan["source_file"], source_file)
        self.assertEqual(plan["compile_commands_path"], compile_commands)
        self.assertEqual(plan["cwd"], root.resolve())
        self.assertEqual(
            plan["object_path"], (root / "build" / "current" / "func.o").resolve()
        )
        self.assertEqual(plan["command"][-1], str(source_file.resolve()))

    def test_resolve_workspace_passes_program_rows_to_lookup(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("", encoding="utf-8")
            workspace_json = root / "workspace.json"
            workspace_json.write_text("{}", encoding="utf-8")
            workspace_root = root / "tmp" / "matching"
            program_rows = [
                {
                    "program_name": "0.bin",
                    "program_path": "/bins/BIN/ETC/GAME/0.bin",
                    "program_slug": "bins_bin_etc_game_0_bin",
                    "folder": "/bins/BIN/ETC/GAME",
                    "source_hint": "build/extracted/BIN/ETC/GAME.EMI#0",
                }
            ]
            args = MODULE.argparse.Namespace(
                program="/bins/BIN/ETC/GAME/0.bin",
                entry="0x80196f78",
                inventory_db=inventory_db,
                workspace_root=workspace_root,
            )
            logger = type("Logger", (), {"error": lambda self, message: None})()

            with (
                patch.object(
                    MODULE.workspace_lib, "load_function_rows", return_value=[]
                ),
                patch.object(
                    MODULE.workspace_lib,
                    "load_program_rows",
                    return_value=program_rows,
                ),
                patch.object(
                    MODULE.workspace_lib,
                    "find_workspace_json",
                    return_value=workspace_json,
                ) as find_workspace_json,
                patch.object(
                    MODULE.workspace_lib,
                    "load_workspace_payload",
                    return_value={"workspace_dir": "tmp/matching/foo"},
                ),
            ):
                result = MODULE.resolve_workspace(args, logger)

        self.assertEqual(
            result,
            (workspace_json, {"workspace_dir": "tmp/matching/foo"}),
        )
        find_workspace_json.assert_called_once_with(
            [],
            program="/bins/BIN/ETC/GAME/0.bin",
            entry="0x80196f78",
            workspace_root=workspace_root,
            program_rows=program_rows,
            artifact_root=MODULE.workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
        )


if __name__ == "__main__":
    unittest.main()
