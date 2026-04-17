from __future__ import annotations

import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.models.ghidra import GhidraBootstrapRequest
from scripts.rebof3.re.services.bootstrap import service as bootstrap_service


class _Logger:
    def __init__(self) -> None:
        self.errors: list[str] = []
        self.infos: list[str] = []
        self.summaries: list[str] = []

    def error(self, message: str) -> None:
        self.errors.append(message)

    def info(self, message: str) -> None:
        self.infos.append(message)

    def summary(self, message: str) -> None:
        self.summaries.append(message)


class BootstrapServiceTests(unittest.TestCase):
    def test_run_captures_function_inventory_after_bootstrap(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            (root / "processed" / "emi_raw" / "BIN").mkdir(parents=True)
            inventory_db = root / "processed" / "inventory" / "inventory.sqlite"
            inventory_db.parent.mkdir(parents=True)
            inventory_db.touch()
            project_gpr = root / "tmp" / "bof3_ghidra" / "main" / "bof3_main.gpr"
            project_gpr.parent.mkdir(parents=True, exist_ok=True)
            project_gpr.touch()

            logger = _Logger()
            service = bootstrap_service.GhidraBootstrapService()

            with (
                patch.object(bootstrap_service.constants, "ROOT", root),
                patch.object(bootstrap_service, "default_inventory_db", return_value=inventory_db),
                patch.object(bootstrap_service, "default_project_dir", return_value=project_gpr.parent),
                patch.object(bootstrap_service, "project_busy_message", return_value=None),
                patch.object(bootstrap_service, "ensure_project_marker", return_value=project_gpr),
                patch.object(
                    bootstrap_service,
                    "run_command",
                    return_value=subprocess.CompletedProcess(args=["bootstrap"], returncode=0),
                ),
                patch.object(bootstrap_service, "capture_into_inventory") as capture_mock,
            ):
                status = service.run(
                    GhidraBootstrapRequest(),
                    logger=logger,
                )

        self.assertEqual(status, 0)
        capture_mock.assert_called_once_with(
            db_path=inventory_db,
            kind="function",
            project_dir=project_gpr.parent,
            project_name="bof3_main",
        )


if __name__ == "__main__":
    unittest.main()
