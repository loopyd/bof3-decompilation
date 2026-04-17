from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.rebof3.match import refresh as MODULE


class MatchRefreshTests(unittest.TestCase):
    def test_parse_args_accepts_short_foundation_flags(self) -> None:
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
                "-P",
                "capcom97-bof3",
                "-t",
            ]
        )

        self.assertEqual(args.inventory_db, Path("tmp/inventory.sqlite"))
        self.assertEqual(args.match_root, Path("tmp/matching"))
        self.assertEqual(args.source_root, Path("bof3"))
        self.assertEqual(args.artifact_root, Path("tmp/ghidra_decomp"))
        self.assertEqual(args.profile, "capcom97-bof3")
        self.assertTrue(args.tracked_output)

    def test_refresh_outputs_returns_paths(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            with mock.patch.object(
                MODULE.report_refresh,
                "refresh_report_artifacts",
                return_value={
                    "scoreboard_json": str(root / "scoreboard.json"),
                    "status_root": str(root / "status"),
                },
            ) as refresh_report_artifacts:
                refreshed = MODULE.refresh_outputs(
                    inventory_db=root / "inventory.sqlite",
                    match_root=root / "tmp" / "matching",
                    source_root=root / "bof3",
                    artifact_root=root / "tmp" / "ghidra_decomp",
                )

        refresh_report_artifacts.assert_called_once()
        self.assertEqual(refreshed["scoreboard_json"], root / "scoreboard.json")
        self.assertEqual(refreshed["status_root"], root / "status")

    def test_main_skips_status_when_requested(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            inventory_db = root / "inventory.sqlite"
            inventory_db.write_text("sqlite placeholder", encoding="utf-8")
            logger = type("Logger", (), {"summary": lambda self, message: None})()

            with (
                mock.patch.object(MODULE, "logger_from_args", return_value=logger),
                mock.patch.object(
                    MODULE,
                    "refresh_outputs",
                    return_value={"scoreboard_json": root / "scoreboard.json"},
                ) as refresh_outputs,
            ):
                result = MODULE.main(
                    [
                        "--inventory-db",
                        str(inventory_db),
                        "--match-root",
                        str(root / "tmp" / "matching"),
                        "--source-root",
                        str(root / "bof3"),
                        "--artifact-root",
                        str(root / "tmp" / "ghidra_decomp"),
                        "--no-status",
                    ]
                )

        self.assertEqual(result, 0)
        refresh_outputs.assert_called_once_with(
            inventory_db=inventory_db,
            match_root=root / "tmp" / "matching",
            source_root=root / "bof3",
            artifact_root=root / "tmp" / "ghidra_decomp",
            profile=MODULE.DEFAULT_PSX_PROFILE,
            tracked_output=False,
            refresh_reports=True,
            refresh_status=False,
            build_artifact_manifest=MODULE.status_lib.DEFAULT_BUILD_ARTIFACT_MANIFEST,
        )


if __name__ == "__main__":
    unittest.main()
