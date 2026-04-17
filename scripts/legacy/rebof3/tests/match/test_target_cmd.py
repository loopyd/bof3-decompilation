from __future__ import annotations

import io
import json
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import target_cmd as MODULE


class MatchTargetCommandTests(unittest.TestCase):
    def test_build_payload_infers_program_kind(self) -> None:
        payload = MODULE.build_payload(
            Path("tmp/matching/foo/workspace.json"),
            {
                "workspace_dir": "tmp/matching/foo",
                "program_path": "/bins/BIN/ETC/GAME/00.bin",
                "entry_hex": "0x80196f78",
                "source_hint": "build/extracted/BIN/ETC/GAME.EMI#0",
                "source_mapping": {
                    "source_file": "bof3/src/modules/game/00/func_80196f78.c",
                    "source_function": "func_80196f78",
                },
                "ghidra_decomp_bundle_json": "tmp/ghidra_decomp/foo/func.json",
            },
        )

        self.assertEqual(payload["program_kind"], "bin")
        self.assertEqual(payload["source_function"], "func_80196f78")

    def test_main_can_emit_json(self) -> None:
        args = MODULE.parse_args.__globals__["argparse"].Namespace(
            program="/boot/SLUS_004.22",
            entry="0x80162d00",
            workspace_json=None,
            inventory_db=Path("inventory.sqlite"),
            workspace_root=Path("tmp/matching"),
            quiet=False,
            verbose=False,
            json=True,
        )
        with tempfile.TemporaryDirectory() as tmp_dir:
            workspace_json = Path(tmp_dir) / "workspace.json"
            workspace_json.write_text("{}", encoding="utf-8")
            stdout = io.StringIO()
            with (
                mock.patch.object(MODULE, "parse_args", return_value=args),
                mock.patch.object(
                    MODULE.pipeline_ready,
                    "resolve_workspace",
                    return_value=(
                        workspace_json,
                        {
                            "workspace_dir": "tmp/matching/foo",
                            "program_path": "/boot/SLUS_004.22",
                            "entry_hex": "0x80162d00",
                            "source_mapping": {},
                        },
                    ),
                ),
                redirect_stdout(stdout),
            ):
                result = MODULE.main()

        self.assertEqual(result, 0)
        rendered = json.loads(stdout.getvalue())
        self.assertEqual(rendered["program_kind"], "boot")


if __name__ == "__main__":
    unittest.main()
