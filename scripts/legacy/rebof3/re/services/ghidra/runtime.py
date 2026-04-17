from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from ....common import run_command
from ....config import ROOT
from .commands import build_function_action_command
from .env import ghidra_cli_env


def run_function_action(
    *,
    action: str,
    program_selector: str,
    address: int,
    output_path: Path,
    create_missing: bool,
    metadata_only: bool = False,
) -> tuple[list[str], Any, int, str, str]:
    command = build_function_action_command(
        action=action,
        program_selector=program_selector,
        address=address,
        output_path=output_path,
        create_missing=create_missing,
        metadata_only=metadata_only,
    )
    result = run_command(command, cwd=ROOT, env=ghidra_cli_env())
    payload = None
    if result.returncode == 0 and output_path.exists():
        payload = json.loads(output_path.read_text(encoding="utf-8"))
    return (
        command,
        payload,
        int(result.returncode),
        result.stdout or "",
        result.stderr or "",
    )
