from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import scaffold as MODULE


class MatchScaffoldTests(unittest.TestCase):
    def test_parse_args_accepts_batch_filters(self) -> None:
        args = MODULE.parse_args(
            [
                "-i",
                "tmp/inventory.sqlite",
                "-m",
                "tmp/matching",
                "-s",
                "bof3",
                "-a",
                "tmp/ghidra_decomp",
                "-f",
                "ETC",
                "-k",
                "bin",
                "--path-glob",
                "bof3/stubs/modules/game/*",
                "--exclude-glob",
                "bof3/stubs/modules/game/00/*",
                "--limit",
                "5",
                "--asm-root",
                "tmp/asm-stage",
                "--no-stubs",
                "--refresh-bundles",
                "-n",
            ]
        )

        self.assertEqual(args.inventory_db, Path("tmp/inventory.sqlite"))
        self.assertEqual(args.match_root, Path("tmp/matching"))
        self.assertEqual(args.source_root, Path("bof3"))
        self.assertEqual(args.artifact_root, Path("tmp/ghidra_decomp"))
        self.assertEqual(args.family, ["ETC"])
        self.assertEqual(args.program_kind, ["bin"])
        self.assertEqual(args.path_glob, "bof3/stubs/modules/game/*")
        self.assertEqual(args.exclude_glob, ["bof3/stubs/modules/game/00/*"])
        self.assertEqual(args.limit, 5)
        self.assertEqual(args.asm_root, Path("tmp/asm-stage"))
        self.assertTrue(args.no_stubs)
        self.assertTrue(args.refresh_bundles)
        self.assertTrue(args.dry_run)

    def test_preferred_repo_function_path_uses_source_or_stub_layout(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            original_root = MODULE.ROOT
            try:
                MODULE.ROOT = root
                source_row = {
                    "source_file": "bof3/src/core/emi/func_80162d00.c",
                    "program_path": "/boot/SLUS_004.22",
                    "entry_hex": "0x80162d00",
                }
                bin_row = {
                    "source_file": None,
                    "program_path": "/bins/BIN/ETC/GAME/0.bin",
                    "entry_hex": "0x80196f78",
                }

                source_path = MODULE.preferred_repo_function_path(source_row)
                bin_path = MODULE.preferred_repo_function_path(bin_row)
                asm_targets = MODULE.asm_target_paths(
                    Path("bof3/src/core/emi/func_80162d00.c"),
                    asm_root=Path("tmp/asm-stage"),
                )
            finally:
                MODULE.ROOT = original_root

        self.assertEqual(source_path, Path("bof3/src/core/emi/func_80162d00.c"))
        self.assertEqual(
            bin_path,
            Path("bof3/stubs/modules/game/00/func_80196f78.c"),
        )
        assert asm_targets is not None
        self.assertEqual(
            asm_targets["asm"], Path("tmp/asm-stage/core/emi/func_80162d00.s")
        )
        self.assertEqual(
            asm_targets["m2c"], Path("tmp/asm-stage/core/emi/func_80162d00.m2c.c")
        )

    def test_select_items_filters_by_family_kind_and_path(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            payload = {
                "functions": [
                    {
                        "family": "ETC",
                        "program_kind": "bin",
                        "program_path": "/bins/BIN/ETC/GAME/0.bin",
                        "entry_hex": "0x80196f78",
                        "source_hint": "build/extracted/BIN/ETC/GAME.EMI#0",
                        "source_file": None,
                    },
                    {
                        "family": "ETC",
                        "program_kind": "boot",
                        "program_path": "/boot/SLUS_004.22",
                        "entry_hex": "0x80162d00",
                        "source_hint": "build/extracted/SLUS_004.22",
                        "source_file": "bof3/src/core/emi/func_80162d00.c",
                    },
                ]
            }

            original_root = MODULE.ROOT
            try:
                MODULE.ROOT = root
                items = MODULE.select_items(
                    payload,
                    families={"ETC"},
                    program_kinds={"bin"},
                    path_glob="bof3/stubs/modules/game/*",
                    exclude_globs=["bof3/stubs/modules/game/01/*"],
                    limit=None,
                )
            finally:
                MODULE.ROOT = original_root

        self.assertEqual(len(items), 1)
        self.assertEqual(items[0]["program_path"], "/bins/BIN/ETC/GAME/0.bin")
        self.assertEqual(
            items[0]["repo_path"], "bof3/stubs/modules/game/00/func_80196f78.c"
        )

    def test_select_items_skips_excluded_paths(self) -> None:
        payload = {
            "functions": [
                {
                    "family": "ETC",
                    "program_kind": "bin",
                    "program_path": "/bins/BIN/ETC/GAME/0.bin",
                    "entry_hex": "0x80196f78",
                    "source_hint": "build/extracted/BIN/ETC/GAME.EMI#0",
                    "source_file": None,
                }
            ]
        }

        items = MODULE.select_items(
            payload,
            families={"ETC"},
            program_kinds={"bin"},
            path_glob="bof3/stubs/modules/game/*",
            exclude_globs=["bof3/stubs/modules/game/00/*"],
            limit=None,
        )

        self.assertEqual(items, [])

    def test_main_creates_stub_and_mirrors_bundle_artifacts(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("", encoding="utf-8")
            output_json = root / "scaffold.json"
            args = MODULE.argparse.Namespace(
                inventory_db=inventory_db,
                match_root=root / "tmp" / "matching",
                source_root=root / "bof3",
                artifact_root=root / "tmp" / "ghidra_decomp",
                family=None,
                program_kind=None,
                path_glob=None,
                exclude_glob=None,
                limit=None,
                asm_root=root / "tmp" / "asm-stage",
                no_stubs=False,
                no_asm=False,
                no_m2c=False,
                refresh_bundles=False,
                dry_run=False,
                output_json=output_json,
                refresh_status=False,
                tracked_output=False,
                quiet=False,
                verbose=False,
            )
            logger = type(
                "Logger",
                (),
                {
                    "summary": lambda self, message: None,
                    "item": lambda self, message: None,
                    "error": lambda self, message: None,
                },
            )()
            payload = {
                "functions": [
                    {
                        "family": "WORLD00",
                        "program_kind": "bin",
                        "program_path": "/bins/BIN/WORLD00/AREA000/10.bin",
                        "entry_hex": "0x800e4000",
                        "source_hint": "build/extracted/BIN/WORLD00/AREA000.EMI#10",
                        "source_file": None,
                    }
                ]
            }

            def fake_run_decomp_bundle(**kwargs):
                artifacts_dir = Path(str(kwargs["artifacts_dir"]))
                artifacts_dir.mkdir(parents=True, exist_ok=True)
                (artifacts_dir / "func.json").write_text(
                    json.dumps(
                        {
                            "files": {
                                "json": str(artifacts_dir / "func.json"),
                                "asm": str(artifacts_dir / "func.s"),
                                "m2c_c": str(artifacts_dir / "func.m2c.c"),
                            },
                            "m2c": {
                                "attempted": True,
                                "path": "func.m2c.c",
                                "status": "ok",
                            },
                        }
                    ),
                    encoding="utf-8",
                )
                (artifacts_dir / "func.s").write_text(".text\n", encoding="utf-8")
                (artifacts_dir / "func.m2c.c").write_text(
                    "void func_800e4000(void) {}\n",
                    encoding="utf-8",
                )
                return (
                    0,
                    {
                        "files": {
                            "json": str(artifacts_dir / "func.json"),
                            "asm": str(artifacts_dir / "func.s"),
                            "m2c_c": str(artifacts_dir / "func.m2c.c"),
                        },
                        "m2c": {
                            "attempted": True,
                            "path": str(artifacts_dir / "func.m2c.c"),
                            "status": "ok",
                        },
                    },
                )

            original_root = MODULE.ROOT
            try:
                MODULE.ROOT = root
                with (
                    mock.patch.object(MODULE, "parse_args", return_value=args),
                    mock.patch.object(MODULE, "logger_from_args", return_value=logger),
                    mock.patch.object(
                        MODULE.scoreboard_lib,
                        "build_scoreboard_payload",
                        return_value=payload,
                    ),
                    mock.patch.object(
                        MODULE,
                        "run_decomp_bundle",
                        side_effect=fake_run_decomp_bundle,
                    ),
                ):
                    result = MODULE.main()
            finally:
                MODULE.ROOT = original_root

            report = json.loads(output_json.read_text(encoding="utf-8"))
            stub_path = root / "bof3/stubs/modules/world00/area000/10/func_800e4000.c"
            asm_path = root / "tmp/asm-stage/modules/world00/area000/10/func_800e4000.s"
            m2c_path = (
                root / "tmp/asm-stage/modules/world00/area000/10/func_800e4000.m2c.c"
            )
            self.assertEqual(result, 0)
            self.assertTrue(stub_path.exists())
            self.assertTrue(asm_path.exists())
            self.assertTrue(m2c_path.exists())
            self.assertEqual(report["item_count"], 1)
            self.assertEqual(report["asm_root"], str(root / "tmp" / "asm-stage"))
            self.assertEqual(report["stub_created"], 1)
            self.assertEqual(report["bundle_exported"], 1)
            self.assertEqual(report["asm_copied"], 1)
            self.assertEqual(report["m2c_copied"], 1)


if __name__ == "__main__":
    unittest.main()
