from __future__ import annotations

import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.match import sweep as MODULE


class MatchSweepTests(unittest.TestCase):
    def test_collect_lift_targets_resolves_matching_inventory_row(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_root = root / "bof3"
            source_file = source_root / "src" / "modules" / "logo" / "func_801cedfc.c"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "void func_801cedfc(void)\n{\n}\n",
                encoding="utf-8",
            )
            rows = [
                {
                    "program_path": "/boot/SLUS_004.22",
                    "program_name": "SLUS_004.22",
                    "entry": "801cedfc",
                    "entry_hex": "0x801cedfc",
                    "source_hint": "build/extracted/SLUS_004.22",
                },
                {
                    "program_path": "/boot/LOGO/LOGO.EXE",
                    "program_name": "LOGO.EXE",
                    "entry": "801cedfc",
                    "entry_hex": "0x801cedfc",
                    "source_hint": "build/extracted/LOGO/LOGO.EXE",
                },
            ]

            targets, unresolved = MODULE.collect_lift_targets(
                rows, source_root=source_root, source_glob=None
            )

            self.assertEqual(unresolved, [])
            self.assertEqual(len(targets), 1)
            self.assertEqual(targets[0]["program_path"], "/boot/LOGO/LOGO.EXE")
            self.assertEqual(targets[0]["source_function"], "func_801cedfc")

    def test_collect_lift_targets_falls_back_to_bundle_backed_program_row(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_root = root / "bof3"
            source_file = source_root / "src" / "core" / "emi" / "func_8016728c.c"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "unsigned int func_8016728c(void)\n{\n}\n",
                encoding="utf-8",
            )

            artifact_root = root / "tmp" / "ghidra_decomp"
            artifacts_dir = (
                artifact_root / "build" / "extracted" / "SLUS_004.22" / "0x8016728c"
            )
            artifacts_dir.mkdir(parents=True, exist_ok=True)
            (artifacts_dir / "func.json").write_text(
                '{"requested_address":"0x8016728c","load_address":null,'
                '"function":{"entry":"8016728c","requested_address":"0x8016728c"}}',
                encoding="utf-8",
            )

            targets, unresolved = MODULE.collect_lift_targets(
                [],
                program_rows=[
                    {
                        "program_name": "SLUS_004.22",
                        "program_path": "/boot/SLUS_004.22",
                        "program_slug": "boot_slus_004_22",
                        "folder": "/boot",
                        "source_hint": "build/extracted/SLUS_004.22",
                    }
                ],
                artifact_root=artifact_root,
                source_root=source_root,
                source_glob=None,
            )

            self.assertEqual(unresolved, [])
            self.assertEqual(len(targets), 1)
            self.assertEqual(targets[0]["program_path"], "/boot/SLUS_004.22")
            self.assertEqual(targets[0]["source_function"], "func_8016728c")

    def test_collect_lift_targets_rejects_bundle_below_overlay_load_address(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_root = root / "bof3"
            source_file = source_root / "src" / "modules" / "game" / "func_8014ecac.c"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "void func_8014ecac(void)\n{\n}\n",
                encoding="utf-8",
            )

            artifact_root = root / "tmp" / "ghidra_decomp"
            artifacts_dir = (
                artifact_root
                / "build"
                / "extracted"
                / "BIN"
                / "ETC"
                / "GAME.EMI"
                / "entry_1"
                / "0x8014ecac"
            )
            artifacts_dir.mkdir(parents=True, exist_ok=True)
            (artifacts_dir / "func.json").write_text(
                '{"requested_address":"0x8014ecac","load_address":"0x801d0c00",'
                '"function":{"requested_address":"0x8014ecac","status":"missing_function"}}',
                encoding="utf-8",
            )

            targets, unresolved = MODULE.collect_lift_targets(
                [],
                program_rows=[
                    {
                        "program_name": "1.bin",
                        "program_path": "/bins/BIN/ETC/GAME/1.bin",
                        "program_slug": "bins_bin_etc_game_1_bin",
                        "folder": "/bins/BIN/ETC/GAME",
                        "source_hint": "build/extracted/BIN/ETC/GAME.EMI#1",
                    }
                ],
                artifact_root=artifact_root,
                source_root=source_root,
                source_glob=None,
            )

            self.assertEqual(targets, [])
            self.assertEqual(len(unresolved), 1)
            self.assertEqual(unresolved[0]["entry_hex"], "0x8014ecac")

    def test_select_seed_program_row_prefers_slus_for_core_sources(self) -> None:
        row = MODULE.select_seed_program_row(
            {
                "entry_hex": "0x8016728c",
                "source_file": "bof3/src/core/emi/func_8016728c.c",
                "source_function": "func_8016728c",
            },
            program_rows=[
                {
                    "program_name": "SLUS_004.22",
                    "program_path": "/boot/SLUS_004.22",
                    "program_slug": "boot_slus_004_22",
                    "folder": "/boot",
                    "source_hint": "build/extracted/SLUS_004.22",
                },
                {
                    "program_name": "LOGO.EXE",
                    "program_path": "/boot/LOGO/LOGO.EXE",
                    "program_slug": "boot_logo_logo_exe",
                    "folder": "/boot/LOGO",
                    "source_hint": "build/extracted/LOGO/LOGO.EXE",
                },
                {
                    "program_name": "0.bin",
                    "program_path": "/bins/BIN/ETC/GAME/0.bin",
                    "program_slug": "bins_bin_etc_game_0_bin",
                    "folder": "/bins/BIN/ETC/GAME",
                    "source_hint": "build/extracted/BIN/ETC/GAME.EMI#0",
                },
            ],
        )

        self.assertIsNotNone(row)
        self.assertEqual(row["program_path"], "/boot/SLUS_004.22")

    def test_seed_ghidra_bundles_promotes_unresolved_mapping(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_root = root / "bof3"
            source_file = (
                source_root / "src" / "modules" / "game" / "00" / "func_80196f78.c"
            )
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "void func_80196f78(void)\n{\n}\n",
                encoding="utf-8",
            )

            artifact_root = root / "tmp" / "ghidra_decomp"
            program_rows = [
                {
                    "program_name": "0.bin",
                    "program_path": "/bins/BIN/ETC/GAME/0.bin",
                    "program_slug": "bins_bin_etc_game_0_bin",
                    "folder": "/bins/BIN/ETC/GAME",
                    "source_hint": "build/extracted/BIN/ETC/GAME.EMI#0",
                },
                {
                    "program_name": "1.bin",
                    "program_path": "/bins/BIN/ETC/GAME/1.bin",
                    "program_slug": "bins_bin_etc_game_1_bin",
                    "folder": "/bins/BIN/ETC/GAME",
                    "source_hint": "build/extracted/BIN/ETC/GAME.EMI#1",
                },
            ]
            unresolved = [
                {
                    "entry_hex": "0x80196f78",
                    "source_file": "bof3/src/modules/game/00/func_80196f78.c",
                    "source_function": "func_80196f78",
                    "source_signature": "void func_80196f78(void)",
                }
            ]
            logger = type("Logger", (), {"detail": lambda self, message: None})()

            def fake_run_command(
                command: list[str],
            ) -> subprocess.CompletedProcess[str]:
                artifacts_dir = Path(command[command.index("--artifacts-dir") + 1])
                artifacts_dir.mkdir(parents=True, exist_ok=True)
                (artifacts_dir / "func.json").write_text(
                    '{"requested_address":"0x80196f78","load_address":"0x80195800",'
                    '"function":{"entry":"80196f78","requested_address":"0x80196f78"}}',
                    encoding="utf-8",
                )
                return subprocess.CompletedProcess(command, 0, stdout="ok", stderr="")

            with patch.object(MODULE, "run_command", side_effect=fake_run_command):
                attempts = MODULE.seed_ghidra_bundles_for_unresolved(
                    unresolved,
                    program_rows=program_rows,
                    artifact_root=artifact_root,
                    logger=logger,
                )

            self.assertEqual(len(attempts), 1)
            self.assertEqual(attempts[0]["status"], "seeded")
            self.assertEqual(attempts[0]["program_path"], "/bins/BIN/ETC/GAME/0.bin")

            targets, unresolved_after = MODULE.collect_lift_targets(
                [],
                program_rows=program_rows,
                artifact_root=artifact_root,
                source_root=source_root,
                source_glob=None,
            )

            self.assertEqual(unresolved_after, [])
            self.assertEqual(len(targets), 1)
            self.assertEqual(targets[0]["program_path"], "/bins/BIN/ETC/GAME/0.bin")
            self.assertEqual(targets[0]["entry_hex"], "0x80196f78")


if __name__ == "__main__":
    unittest.main()
