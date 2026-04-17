from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import workspace as MODULE
from scripts.rebof3.inventory.db.connection import connect_inventory_database
from scripts.rebof3.inventory.db.migrations import ensure_inventory_schema
from scripts.rebof3.inventory.repositories.programs import ProgramRepository
from scripts.rebof3.models.inventory import InventoryFunctionRow, InventoryProgramRow


class MatchWorkspaceTests(unittest.TestCase):
    def test_parse_args_accepts_short_init_flags(self) -> None:
        args = MODULE.parse_args(
            [
                "-p",
                "/boot/SLUS_004.22",
                "-e",
                "0x80162d00",
                "-i",
                "tmp/inventory.sqlite",
                "-w",
                "tmp/matching",
                "-a",
                "tmp/ghidra_decomp",
                "-s",
                "bof3/src/core/emi/func_80162d00.c",
                "-n",
            ]
        )

        self.assertEqual(args.program, "/boot/SLUS_004.22")
        self.assertEqual(args.entry, "0x80162d00")
        self.assertEqual(args.inventory_db, Path("tmp/inventory.sqlite"))
        self.assertEqual(args.workspace_root, Path("tmp/matching"))
        self.assertEqual(args.artifact_root, Path("tmp/ghidra_decomp"))
        self.assertEqual(args.source, "bof3/src/core/emi/func_80162d00.c")
        self.assertTrue(args.dry_run)

    def test_parse_args_accepts_legacy_workspace_init_sentinel(self) -> None:
        args = MODULE.parse_args(
            [
                MODULE.LEGACY_WORKSPACE_INIT_SENTINEL,
                "-p",
                "/boot/SLUS_004.22",
                "-e",
                "0x80162d00",
            ]
        )

        self.assertEqual(args.program, "/boot/SLUS_004.22")
        self.assertEqual(args.entry, "0x80162d00")

    def test_find_function_row_matches_program_path_and_entry(self) -> None:
        rows = [
            {
                "program_path": "/boot/SLUS_004.22",
                "program_name": "SLUS_004.22",
                "program_slug": "boot_slus_004_22",
                "entry": "80162d00",
            }
        ]

        row = MODULE.find_function_row(
            rows, program="/boot/SLUS_004.22", entry="0x80162d00"
        )

        self.assertEqual(row["program_slug"], "boot_slus_004_22")

    def test_suggested_artifacts_dir_uses_source_hint(self) -> None:
        row = {
            "entry": "801d0c04",
            "source_hint": "build/extracted/BIN/ETC/GAME.EMI#1",
        }

        result = MODULE.suggested_artifacts_dir(
            row,
            MODULE.ROOT / "tmp" / "ghidra_decomp",
            source_override=None,
        )

        self.assertEqual(
            result,
            MODULE.ROOT
            / "tmp"
            / "ghidra_decomp"
            / "build"
            / "extracted"
            / "BIN"
            / "ETC"
            / "GAME.EMI"
            / "entry_1"
            / "0x801d0c04",
        )

    def test_build_workspace_payload_records_bundle_paths(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            row = {
                "program_name": "SLUS_004.22",
                "program_path": "/boot/SLUS_004.22",
                "program_slug": "boot_slus_004_22",
                "folder": "/boot",
                "entry": "80162d00",
                "entry_hex": "0x80162d00",
                "name": "emi_ready",
                "signature": "bool emi_ready(void)",
                "namespace": "Global",
                "comment": "loader ready helper",
                "repeatable_comment": None,
                "name_source": "USER_DEFINED",
                "source_hint": "build/extracted/SLUS_004.22",
            }
            artifacts_dir = (
                root
                / "tmp"
                / "ghidra_decomp"
                / "build"
                / "extracted"
                / "SLUS_004.22"
                / "0x80162d00"
            )
            artifacts_dir.mkdir(parents=True, exist_ok=True)
            (artifacts_dir / "func.s").write_text(".text\n", encoding="utf-8")
            (artifacts_dir / "func.json").write_text("{}", encoding="utf-8")
            (artifacts_dir / "func.json").write_text(
                '{"files": {"asm": "'
                + str(artifacts_dir / "func.s")
                + '"}, "function": {"entry": "80162d00", "name": "FUN_80162d00"}}',
                encoding="utf-8",
            )

            workspace_dir, payload = MODULE.build_workspace_payload(
                row,
                inventory_db=root / "processed" / "inventory" / "inventory.sqlite",
                workspace_root=root / "tmp" / "matching",
                artifact_root=root / "tmp" / "ghidra_decomp",
                source_override=None,
            )

            self.assertEqual(
                workspace_dir,
                root / "tmp" / "matching" / "boot_slus_004_22" / "0x80162d00",
            )
            self.assertTrue(payload["source_mapping_ready"])
            self.assertTrue(payload["expected_baseline_ready"])
            self.assertEqual(
                payload["source_mapping"]["source_file"],
                "bof3/src/core/emi/func_80162d00.c",
            )
            self.assertEqual(
                payload["source_mapping"]["source_function"], "func_80162d00"
            )
            self.assertEqual(
                payload["expected_baseline"]["asm_source"],
                str(artifacts_dir / "func.s"),
            )
            self.assertTrue(payload["ghidra_decomp_bundle_exists"])
            self.assertEqual(
                payload["ghidra_decomp_bundle_json"],
                str(artifacts_dir / "func.json"),
            )

    def test_build_workspace_payload_keeps_binary_bundle_when_source_overridden(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            row = {
                "program_name": "3.bin",
                "program_path": "/bins/BIN/ETC/BATE/3.bin",
                "program_slug": "bins_bin_etc_bate_3_bin",
                "folder": "/bins/BIN/ETC/BATE",
                "entry": "80033a00",
                "entry_hex": "0x80033a00",
                "name": "FUN_80033a00",
                "signature": "undefined FUN_80033a00(void)",
                "namespace": "Global",
                "comment": None,
                "repeatable_comment": None,
                "name_source": "DEFAULT",
                "source_hint": "build/extracted/BIN/ETC/BATE.EMI#3",
            }
            artifacts_dir = (
                root
                / "tmp"
                / "ghidra_decomp"
                / "build"
                / "extracted"
                / "BIN"
                / "ETC"
                / "BATE.EMI"
                / "entry_3"
                / "0x80033a00"
            )
            artifacts_dir.mkdir(parents=True, exist_ok=True)
            (artifacts_dir / "func.s").write_text(".text\n", encoding="utf-8")
            (artifacts_dir / "func.json").write_text(
                '{"files": {"asm": "'
                + str(artifacts_dir / "func.s")
                + '"}, "function": {"entry": "80033a00", "name": "FUN_80033a00"}}',
                encoding="utf-8",
            )
            source_override = "bof3/src/modules/bate/03/func_80033a00.c"
            source_mapping = {
                "source_file": source_override,
                "source_function": "func_80033a00",
            }

            with mock.patch.object(
                MODULE.source_map,
                "find_source_mapping",
                return_value=source_mapping,
            ):
                _, payload = MODULE.build_workspace_payload(
                    row,
                    inventory_db=root / "processed" / "inventory" / "inventory.sqlite",
                    workspace_root=root / "tmp" / "matching",
                    artifact_root=root / "tmp" / "ghidra_decomp",
                    source_override=source_override,
                )

            self.assertEqual(
                payload["source_hint"], "build/extracted/BIN/ETC/BATE.EMI#3"
            )
            self.assertEqual(payload["source_override"], source_override)
            self.assertEqual(
                payload["ghidra_decomp_artifacts_dir"],
                str(artifacts_dir),
            )
            self.assertTrue(payload["ghidra_decomp_bundle_exists"])
            self.assertEqual(
                payload["commands"]["ghidra_decomp"],
                "python3 -m scripts.rebof3 re ghidra-decomp build/extracted/BIN/ETC/BATE.EMI#3 0x80033a00 --artifacts-dir "
                + str(artifacts_dir),
            )
            self.assertEqual(payload["source_mapping"]["source_file"], source_override)

    def test_load_function_rows_supports_inventory_sqlite(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="boot_slus_004_22",
                    program_name="SLUS_004.22",
                    program_path="/boot/SLUS_004.22",
                    folder="/boot",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            repo.upsert_function(
                InventoryFunctionRow(
                    program_slug="boot_slus_004_22",
                    entry_address=0x80162D00,
                    entry_hex="0x80162d00",
                    name="emi_ready",
                    signature="bool emi_ready(void)",
                    namespace="Global",
                    name_source="USER_DEFINED",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            connection.close()

            rows = MODULE.load_function_rows(db_path)

        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["program_slug"], "boot_slus_004_22")
        self.assertEqual(rows[0]["entry_hex"], "0x80162d00")

    def test_load_program_rows_supports_inventory_sqlite(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            repo = ProgramRepository(connection)
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug="boot_slus_004_22",
                    program_name="SLUS_004.22",
                    program_path="/boot/SLUS_004.22",
                    folder="/boot",
                    source_hint="build/extracted/SLUS_004.22",
                )
            )
            connection.close()

            rows = MODULE.load_program_rows(db_path)

        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["program_slug"], "boot_slus_004_22")
        self.assertEqual(rows[0]["source_hint"], "build/extracted/SLUS_004.22")

    def test_find_function_row_can_fall_back_to_bundle_backed_program(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_root = root / "bof3"
            source_file = source_root / "src" / "core" / "emi" / "func_8016728c.c"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "unsigned int func_8016728c(void)\n{\n}\n",
                encoding="utf-8",
            )

            program_rows = [
                {
                    "program_name": "SLUS_004.22",
                    "program_path": "/boot/SLUS_004.22",
                    "program_slug": "boot_slus_004_22",
                    "folder": "/boot",
                    "source_hint": "build/extracted/SLUS_004.22",
                }
            ]

            artifacts_dir = (
                root
                / "tmp"
                / "ghidra_decomp"
                / "build"
                / "extracted"
                / "SLUS_004.22"
                / "0x8016728c"
            )
            artifacts_dir.mkdir(parents=True, exist_ok=True)
            (artifacts_dir / "func.json").write_text(
                '{"requested_address":"0x8016728c","load_address":null,'
                '"function":{"entry":"8016728c","requested_address":"0x8016728c"}}',
                encoding="utf-8",
            )

            row = MODULE.find_function_row(
                [],
                program="/boot/SLUS_004.22",
                entry="0x8016728c",
                program_rows=program_rows,
                artifact_root=root / "tmp" / "ghidra_decomp",
                source_root=source_root,
            )

        self.assertEqual(row["program_slug"], "boot_slus_004_22")
        self.assertEqual(row["name"], "func_8016728c")
        self.assertEqual(row["entry_hex"], "0x8016728c")


if __name__ == "__main__":
    unittest.main()
