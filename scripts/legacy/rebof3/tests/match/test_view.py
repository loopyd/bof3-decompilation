from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.match import view as MODULE


class MatchViewTests(unittest.TestCase):
    def test_view_launches_asm_differ_when_workspace_ready(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("", encoding="utf-8")
            workspace_json = root / "workspace.json"
            workspace_json.write_text("{}", encoding="utf-8")
            args = MODULE.parse_args.__globals__["argparse"].Namespace(
                program="/boot/SLUS_004.22",
                entry="0x80162d00",
                inventory_db=inventory_db,
                workspace_root=Path("tmp/matching"),
                refresh_ghidra_bundle=False,
                dry_run=False,
                quiet=False,
                verbose=False,
            )

            with (
                patch.object(MODULE, "parse_args", return_value=args),
                patch.object(
                    MODULE.workspace_lib, "load_function_rows", return_value=[]
                ),
                patch.object(
                    MODULE.workspace_lib,
                    "find_workspace_json",
                    return_value=workspace_json,
                ),
                patch.object(
                    MODULE.workspace_lib,
                    "load_workspace_payload",
                    return_value={
                        "workspace_dir": "tmp/matching/foo",
                        "program_path": "/boot/SLUS_004.22",
                        "entry_hex": "0x80162d00",
                        "ghidra_decomp_bundle_json": "tmp/bundle.json",
                        "source_mapping_ready": True,
                        "expected_baseline_ready": True,
                        "expected_baseline": {"asm_source": "tmp/func.s"},
                        "source_mapping": {"source_function": "emi_ready"},
                    },
                ),
                patch.object(
                    MODULE.diff_lib,
                    "load_build_status",
                    return_value={"succeeded": True},
                ),
                patch.object(
                    MODULE, "normalize_repo_path", return_value=root / "bundle.json"
                ),
                patch.object(
                    MODULE.diff_lib, "refresh_expected_baseline"
                ) as refresh_baseline,
                patch.object(
                    MODULE.diff_lib,
                    "diff_status",
                    return_value=("ready_for_backend_diff", []),
                ),
                patch.object(
                    MODULE.asm_differ_backend,
                    "prepare_backend",
                    return_value={"backend_dir": "tmp/matching/foo/asm_differ"},
                ),
                patch.object(
                    MODULE.asm_differ_backend,
                    "run_viewer",
                    return_value=type("Result", (), {"returncode": 0})(),
                ) as run_viewer,
            ):
                refresh_baseline.side_effect = lambda workspace_json, payload: payload
                result = MODULE.main()

        self.assertEqual(result, 0)
        run_viewer.assert_called_once()

    def test_view_fails_when_workspace_not_ready(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("", encoding="utf-8")
            workspace_json = root / "workspace.json"
            workspace_json.write_text("{}", encoding="utf-8")
            args = MODULE.parse_args.__globals__["argparse"].Namespace(
                program="/boot/SLUS_004.22",
                entry="0x80162d00",
                inventory_db=inventory_db,
                workspace_root=Path("tmp/matching"),
                refresh_ghidra_bundle=False,
                dry_run=False,
                quiet=False,
                verbose=False,
            )

            with (
                patch.object(MODULE, "parse_args", return_value=args),
                patch.object(
                    MODULE.workspace_lib, "load_function_rows", return_value=[]
                ),
                patch.object(
                    MODULE.workspace_lib,
                    "find_workspace_json",
                    return_value=workspace_json,
                ),
                patch.object(
                    MODULE.workspace_lib,
                    "load_workspace_payload",
                    return_value={
                        "workspace_dir": "tmp/matching/foo",
                        "program_path": "/boot/SLUS_004.22",
                        "entry_hex": "0x80162d00",
                        "ghidra_decomp_bundle_json": "tmp/bundle.json",
                    },
                ),
                patch.object(MODULE.diff_lib, "load_build_status", return_value=None),
                patch.object(
                    MODULE, "normalize_repo_path", return_value=root / "bundle.json"
                ),
                patch.object(
                    MODULE.diff_lib, "refresh_expected_baseline"
                ) as refresh_baseline,
                patch.object(
                    MODULE.diff_lib,
                    "diff_status",
                    return_value=("needs_build_status", ["run match_build"]),
                ),
                patch.object(
                    MODULE.asm_differ_backend, "prepare_backend"
                ) as prepare_backend,
            ):
                refresh_baseline.side_effect = lambda workspace_json, payload: payload
                result = MODULE.main()

        self.assertEqual(result, 1)
        prepare_backend.assert_not_called()


if __name__ == "__main__":
    unittest.main()
