from __future__ import annotations

import sys
from pathlib import Path

from ....common import format_hex
from ....config import GHIDRA_MAIN_MODULE
from ..bootstrap import DEFAULT_PROJECT_NAME, default_project_dir


def build_function_action_command(
    *,
    action: str,
    program_selector: str,
    address: int,
    output_path: Path,
    create_missing: bool,
    metadata_only: bool = False,
) -> list[str]:
    command = [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "function",
        action,
        "--project-dir",
        str(default_project_dir()),
        "--project-name",
        DEFAULT_PROJECT_NAME,
        "--program",
        program_selector,
        "--output",
        str(output_path),
    ]
    if create_missing:
        command.append("--create-missing")
    if metadata_only:
        command.append("--metadata-only")
    command.append(format_hex(address))
    return command


def build_capture_command(
    *, db_path: Path, program_selector: str, kind: str
) -> list[str]:
    return [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "metadata",
        "capture",
        "--db",
        str(db_path),
        "--project-dir",
        "tmp/bof3_ghidra/main",
        "--project-name",
        "bof3_main",
        "--program",
        program_selector,
        "--kind",
        kind,
    ]
