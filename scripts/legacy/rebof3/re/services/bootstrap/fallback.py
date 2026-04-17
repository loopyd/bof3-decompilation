from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ....common import prepend_pythonpath, run_command
from . import constants
from .project import default_project_dir, overlay_import_rows


def ghidra_env(ghidra_home: Path) -> dict[str, str]:
    env = prepend_pythonpath(constants.GHIDRA_SRC_DIR)
    env.setdefault("GHIDRA_HOME", str(ghidra_home))
    env.setdefault("GHIDRA_INSTALL_DIR", str(ghidra_home))
    env.setdefault("PYTHONUNBUFFERED", "1")
    return env


def fallback_overlay_import_commands(ghidra_home: Path) -> list[list[str]]:
    project_dir = default_project_dir()
    return [
        [
            sys.executable,
            "-m",
            constants.GHIDRA_MAIN_MODULE,
            "binary",
            "import",
            str(payload_path),
            "--ghidra-home",
            str(ghidra_home),
            "--project-dir",
            str(project_dir),
            "--project-name",
            constants.DEFAULT_PROJECT_NAME,
            "--folder",
            f"bins/{archive_id}",
        ]
        for archive_id, payload_path in overlay_import_rows()
    ]


def fallback_commands(args: argparse.Namespace, ghidra_home: Path) -> list[list[str]]:
    project_dir = default_project_dir()
    commands = [
        [
            sys.executable,
            "-m",
            constants.GHIDRA_MAIN_MODULE,
            "binary",
            "import",
            "build/extracted/SLUS_004.22",
            "--ghidra-home",
            str(ghidra_home),
            "--project-dir",
            str(project_dir),
            "--project-name",
            constants.DEFAULT_PROJECT_NAME,
            "--folder",
            "boot",
            "--program-name",
            "SLUS_004.22",
        ],
        [
            sys.executable,
            "-m",
            constants.GHIDRA_MAIN_MODULE,
            "binary",
            "import",
            "build/extracted/LOGO/LOGO.EXE",
            "--ghidra-home",
            str(ghidra_home),
            "--project-dir",
            str(project_dir),
            "--project-name",
            constants.DEFAULT_PROJECT_NAME,
            "--folder",
            "boot/LOGO",
            "--program-name",
            "LOGO.EXE",
        ],
    ]
    commands.extend(fallback_overlay_import_commands(ghidra_home))
    if args.noanalysis:
        for command in commands:
            command.append("--noanalysis")
    return commands


def run_fallback_bootstrap(args: argparse.Namespace, logger, ghidra_home: Path) -> int:
    logger.info("primary ghidra bootstrap failed; falling back to direct imports")
    if args.no_restore_metadata or args.restore_metadata_from is not None:
        logger.info("fallback bootstrap skips metadata restore controls")
    env = ghidra_env(ghidra_home)
    for command in fallback_commands(args, ghidra_home):
        result = run_command(command, env=env, stream_output=True)
        if result.returncode != 0:
            if result.stderr:
                sys.stderr.write(result.stderr)
            elif result.stdout:
                sys.stderr.write(result.stdout)
            return result.returncode
    return 0


__all__ = [
    "fallback_commands",
    "fallback_overlay_import_commands",
    "ghidra_env",
    "run_fallback_bootstrap",
    "run_command",
]
