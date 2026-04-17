from __future__ import annotations

import os
import subprocess
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from scripts.rebof3.match import build as MODULE


class MatchBuildTests(unittest.TestCase):
    def test_parse_args_accepts_short_program_and_entry_flags(self) -> None:
        args = MODULE.parse_args(["-p", "/boot/SLUS_004.22", "-e", "0x80162d00"])

        self.assertEqual(args.program, "/boot/SLUS_004.22")
        self.assertEqual(args.entry, "0x80162d00")

    def test_build_status_payload_marks_success(self) -> None:
        result = subprocess.CompletedProcess(
            ["make", "build"], 0, stdout="ok", stderr=""
        )
        payload = MODULE.build_status_payload(
            {
                "workspace_dir": "tmp/matching/foo",
                "program_path": "/boot/SLUS_004.22",
                "entry_hex": "0x80162d00",
            },
            profile="capcom97-bof3",
            command=["make", "build"],
            log_path=MODULE.ROOT / "tmp" / "matching" / "foo" / "build.log",
            build_root=MODULE.ROOT / "build" / "bof3-psyq40",
            result=result,
        )

        self.assertTrue(payload["succeeded"])
        self.assertEqual(payload["command_text"], "make build")
        self.assertEqual(payload["entry_hex"], "0x80162d00")
        self.assertEqual(payload["psx_profile"], "capcom97-bof3")
        self.assertEqual(payload["build_mode"], "full-build")
        self.assertEqual(payload["aspsx_version"], "2.56")
        self.assertTrue(payload["compiler_root"].endswith("deps/gcc-2.7.2-psx"))
        self.assertTrue(payload["compiler_gcc"].endswith("deps/gcc-2.7.2-psx/gcc"))
        self.assertTrue(payload["toolchain_bin"].endswith("deps/psn00b_toolchain/bin"))
        self.assertTrue(payload["psyq_root"].endswith("deps/psyq-original/4.0"))

    def test_build_env_prepends_local_toolchain_bin(self) -> None:
        env = MODULE.build_env("capcom97-bof3")

        self.assertTrue(
            env["PATH"].split(os.pathsep)[0].endswith("deps/psn00b_toolchain/bin")
        )
        self.assertEqual(env["BOF3_PROFILE"], "capcom97-bof3")
        self.assertTrue(env["BOF3_PSX_GCC_ROOT"].endswith("deps/gcc-2.7.2-psx"))
        self.assertTrue(env["BOF3_PSX_GCC"].endswith("deps/gcc-2.7.2-psx/gcc"))

    def test_main_uses_compile_one_by_default(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            workspace_json = root / "tmp" / "matching" / "foo" / "workspace.json"
            workspace_json.parent.mkdir(parents=True, exist_ok=True)
            workspace_json.write_text("{}", encoding="utf-8")
            args = MODULE.argparse.Namespace(
                program="/bins/BIN/ETC/GAME/0.bin",
                entry="0x80196f78",
                inventory_db=root / "inventory.sqlite",
                workspace_root=root / "tmp" / "matching",
                build_root=None,
                build_command=["make", "build"],
                full_build=False,
                dry_run=True,
                quiet=False,
                verbose=False,
            )
            logger = SimpleNamespace(
                summary=lambda message: None, error=lambda message: None
            )
            plan = {
                "source_file": root / "bof3" / "src" / "foo.c",
                "object_path": root / "build" / "foo.o",
                "compile_commands_path": root / "build" / "compile_commands.json",
                "command": ["cc", "-c", str(root / "bof3" / "src" / "foo.c")],
            }

            with (
                patch.object(MODULE, "parse_args", return_value=args),
                patch.object(MODULE, "logger_from_args", return_value=logger),
                patch.object(
                    MODULE.compile_one,
                    "resolve_workspace",
                    return_value=(
                        workspace_json,
                        {"workspace_dir": "tmp/matching/foo"},
                    ),
                ),
                patch.object(MODULE.compile_one, "plan_compile_one", return_value=plan),
            ):
                result = MODULE.main()

        self.assertEqual(result, 0)

    def test_main_uses_full_build_when_requested(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            workspace_json = root / "tmp" / "matching" / "foo" / "workspace.json"
            workspace_json.parent.mkdir(parents=True, exist_ok=True)
            workspace_json.write_text("{}", encoding="utf-8")
            args = MODULE.argparse.Namespace(
                program="/bins/BIN/ETC/GAME/0.bin",
                entry="0x80196f78",
                inventory_db=root / "inventory.sqlite",
                workspace_root=root / "tmp" / "matching",
                build_root=None,
                build_command=["make", "build"],
                full_build=True,
                dry_run=False,
                quiet=False,
                verbose=False,
            )
            logger = SimpleNamespace(
                summary=lambda message: None, error=lambda message: None
            )
            result = subprocess.CompletedProcess(
                ["make", "build"], 0, stdout="ok", stderr=""
            )

            with (
                patch.object(MODULE, "parse_args", return_value=args),
                patch.object(MODULE, "logger_from_args", return_value=logger),
                patch.object(
                    MODULE.compile_one,
                    "resolve_workspace",
                    return_value=(
                        workspace_json,
                        {"workspace_dir": "tmp/matching/foo"},
                    ),
                ),
                patch.object(
                    MODULE.compile_one, "run_command", return_value=result
                ) as run_command,
            ):
                exit_code = MODULE.main()

        self.assertEqual(exit_code, 0)
        run_command.assert_called_once_with(
            ["make", "build"], env=MODULE.build_env("capcom97-bof3")
        )


if __name__ == "__main__":
    unittest.main()
