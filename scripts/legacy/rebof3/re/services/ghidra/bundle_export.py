from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

from ....common import format_hex as format_address
from ....common import relative_to_root, run_command
from ....config import GHIDRA_MAIN_MODULE
from ..bootstrap import DEFAULT_PROJECT_NAME as SHARED_PROJECT_NAME
from ..bootstrap import default_project_dir
from .decomp_helpers import extract_decompiled_c
from .env import ghidra_cli_env

LEGACY_PROJECT_NAME = "bof3_decomp"


def resolve_project_path(project_dir: Path | None) -> Path:
    return project_dir if project_dir is not None else default_project_dir()


def resolve_project_name(project_name: str, project_dir: Path | None = None) -> str:
    if project_dir is None and project_name == LEGACY_PROJECT_NAME:
        return SHARED_PROJECT_NAME
    return project_name


def build_import_command(
    *,
    source_text: str,
    project_dir: Path,
    project_name: str,
    program_name: str,
    loader_mode: str,
    base_addr: int | None,
    noanalysis: bool,
) -> list[str]:
    command = [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "binary",
        "import",
        source_text,
        "--project-dir",
        str(project_dir),
        "--project-name",
        project_name,
        "--program-name",
        program_name,
        "--loader-mode",
        loader_mode,
    ]
    if base_addr is not None:
        command.extend(["--base-addr", format_address(base_addr)])
    command.append("--noanalysis" if noanalysis else "--with-analysis")
    return command


def build_function_export_command(
    *,
    project_dir: Path,
    project_name: str,
    program_name: str,
    requested_address: int,
    output_path: Path,
    noanalysis: bool,
) -> list[str]:
    command = [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "function",
        "export",
        "--project-dir",
        str(project_dir),
        "--project-name",
        project_name,
        "--program",
        program_name,
        "--create-missing",
        "--output",
        str(output_path),
        format_address(requested_address),
    ]
    if noanalysis:
        command.append("--noanalysis")
    return command


def build_function_asm_command(
    *,
    project_dir: Path,
    project_name: str,
    program_name: str,
    requested_address: int,
    output_path: Path,
) -> list[str]:
    return [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "function",
        "asm",
        format_address(requested_address),
        "--project-dir",
        str(project_dir),
        "--project-name",
        project_name,
        "--program",
        program_name,
        "--output",
        str(output_path),
    ]


def build_bundle_export_commands(
    *,
    source_text: str,
    project_dir: Path,
    project_name: str,
    program_name: str,
    requested_address: int,
    exported_json_path: Path,
    asm_path: Path,
    loader_mode: str,
    base_addr: int | None,
    noanalysis: bool,
) -> list[list[str]]:
    return [
        build_import_command(
            source_text=source_text,
            project_dir=project_dir,
            project_name=project_name,
            program_name=program_name,
            loader_mode=loader_mode,
            base_addr=base_addr,
            noanalysis=noanalysis,
        ),
        build_function_export_command(
            project_dir=project_dir,
            project_name=project_name,
            program_name=program_name,
            requested_address=requested_address,
            output_path=exported_json_path,
            noanalysis=noanalysis,
        ),
        build_function_asm_command(
            project_dir=project_dir,
            project_name=project_name,
            program_name=program_name,
            requested_address=requested_address,
            output_path=asm_path,
        ),
    ]


def run_bundle_export(
    *,
    source_text: str,
    project_dir: Path,
    project_name: str,
    program_name: str,
    requested_address: int,
    exported_json_path: Path,
    asm_path: Path,
    loader_mode: str,
    base_addr: int | None,
    noanalysis: bool,
) -> tuple[int, dict[str, Any] | None]:
    commands = build_bundle_export_commands(
        source_text=source_text,
        project_dir=project_dir,
        project_name=project_name,
        program_name=program_name,
        requested_address=requested_address,
        exported_json_path=exported_json_path,
        asm_path=asm_path,
        loader_mode=loader_mode,
        base_addr=base_addr,
        noanalysis=noanalysis,
    )
    ghidra_env = ghidra_cli_env()
    for command in commands:
        result = run_command(command, env=ghidra_env)
        if result.returncode != 0:
            return result.returncode, None

    try:
        exported = json.loads(exported_json_path.read_text(encoding="utf-8"))
    finally:
        exported_json_path.unlink(missing_ok=True)

    return 0, {
        "asm_text": asm_path.read_text(encoding="utf-8"),
        "exported": exported,
        "function_payload": exported[0] if exported else None,
        "ghidra_c": extract_decompiled_c(exported),
        "project_dir": relative_to_root(project_dir),
        "project_name": project_name,
        "commands": commands,
    }


__all__ = [
    "LEGACY_PROJECT_NAME",
    "SHARED_PROJECT_NAME",
    "build_bundle_export_commands",
    "build_function_asm_command",
    "build_function_export_command",
    "build_import_command",
    "resolve_project_name",
    "resolve_project_path",
    "run_bundle_export",
]
