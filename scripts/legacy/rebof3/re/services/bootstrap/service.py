from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path

from ....cli import context_from_args
from ....models.ghidra import GhidraBootstrapRequest
from ..service import Service
from ..metadata.capture import capture_into_inventory
from . import constants
from .fallback import ghidra_env, run_fallback_bootstrap, run_command
from .project import (
    default_project_dir,
    default_inventory_db,
    ensure_project_marker,
    project_busy_message,
)


class GhidraBootstrapService(Service):
    service_name = "ghidra_bootstrap"

    def run(self, request: GhidraBootstrapRequest, *, logger) -> int:
        if not (constants.ROOT / "processed" / "emi_raw" / "BIN").exists():
            logger.error("processed/emi_raw/BIN not found. Run 'make unpack' first.")
            return 1
        ghidra_home = Path(
            os.environ.get("GHIDRA_HOME") or constants.DEFAULT_GHIDRA_HOME
        )
        project_dir = default_project_dir()
        busy_message = project_busy_message(project_dir)
        if busy_message is not None:
            logger.error(busy_message)
            return 1
        command = [
            sys.executable,
            "-m",
            constants.GHIDRA_MAIN_MODULE,
            "bootstrap",
            "project",
        ]
        if request.noanalysis:
            command.append("--noanalysis")
        if request.no_restore_metadata:
            command.append("--no-restore-metadata")
        if request.restore_metadata_from is not None:
            command.extend(
                ["--restore-metadata-from", str(request.restore_metadata_from)]
            )
        if request.strict_restore:
            command.append("--strict-restore")
        logger.info(
            f"bootstrapping Ghidra project under {project_dir} with GHIDRA_HOME={ghidra_home}"
        )
        result = run_command(command, env=ghidra_env(ghidra_home), stream_output=True)
        if result.returncode != 0:
            busy_message = project_busy_message(project_dir)
            if busy_message is not None:
                logger.error(busy_message)
                return result.returncode
            fallback_args = argparse.Namespace(
                noanalysis=request.noanalysis,
                no_restore_metadata=request.no_restore_metadata,
                restore_metadata_from=request.restore_metadata_from,
                strict_restore=request.strict_restore,
            )
            fallback_status = run_fallback_bootstrap(fallback_args, logger, ghidra_home)
            if fallback_status != 0:
                if result.stderr:
                    sys.stderr.write(result.stderr)
                return fallback_status
        project_gpr = ensure_project_marker(project_dir, constants.DEFAULT_PROJECT_NAME)
        if project_gpr is None:
            logger.error("Main Ghidra project was not created")
            return 1
        logger.info("capturing function inventory from the refreshed Ghidra project")
        try:
            capture_into_inventory(
                db_path=default_inventory_db(),
                kind="function",
                project_dir=project_dir,
                project_name=constants.DEFAULT_PROJECT_NAME,
            )
        except Exception as exc:  # noqa: BLE001
            logger.error(f"failed to capture function inventory from Ghidra: {exc}")
            return 1
        logger.summary("ghidra workspace bootstrapped successfully")
        return 0


DEFAULT_GHIDRA_BOOTSTRAP_SERVICE = GhidraBootstrapService()


def _execute_args(args: argparse.Namespace) -> int:
    context = context_from_args(args, "re_bootstrap_ghidra")
    return DEFAULT_GHIDRA_BOOTSTRAP_SERVICE.run(
        GhidraBootstrapRequest(
            noanalysis=args.noanalysis,
            no_restore_metadata=args.no_restore_metadata,
            restore_metadata_from=args.restore_metadata_from,
            strict_restore=args.strict_restore,
        ),
        logger=context.logger,
    )


__all__ = [
    "DEFAULT_GHIDRA_BOOTSTRAP_SERVICE",
    "GhidraBootstrapService",
    "_execute_args",
    "run_command",
]
