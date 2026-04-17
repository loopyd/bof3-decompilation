from __future__ import annotations

import subprocess
from pathlib import Path
from typing import Any

from ..jsonio import write_json
from .workspace import build_command_argv, load_workspace


def build_status_payload(
    workspace_payload: dict[str, Any],
    *,
    command_text: str,
    cwd: Path | None,
    result: subprocess.CompletedProcess[str],
) -> dict[str, Any]:
    return {
        "schema": "rebof3-simple.match-build/v1",
        "workspace_dir": workspace_payload["workspace"]["workspace_dir"],
        "program_path": workspace_payload["function"]["program_path"],
        "entry_hex": workspace_payload["function"]["entry_hex"],
        "command": command_text,
        "cwd": None if cwd is None else str(cwd),
        "returncode": int(result.returncode),
        "succeeded": result.returncode == 0,
    }


def resolve_build_inputs(
    workspace_payload: dict[str, Any],
    *,
    build_command: str | None,
    build_cwd: Path | None,
) -> tuple[str, Path | None]:
    effective_command = build_command or workspace_payload["inputs"].get(
        "build_command"
    )
    if not effective_command:
        raise ValueError("no build command configured for this workspace")

    effective_cwd = build_cwd
    if effective_cwd is None:
        configured_cwd = workspace_payload["inputs"].get("build_cwd")
        if configured_cwd:
            effective_cwd = Path(str(configured_cwd))
    if effective_cwd is not None:
        effective_cwd = effective_cwd.expanduser().resolve()
    return str(effective_command), effective_cwd


def run_match_build(
    workspace_path: Path,
    *,
    build_command: str | None,
    build_cwd: Path | None,
) -> tuple[int, Path, Path]:
    workspace_payload = load_workspace(workspace_path)
    command_text, effective_cwd = resolve_build_inputs(
        workspace_payload,
        build_command=build_command,
        build_cwd=build_cwd,
    )

    if effective_cwd is not None and not effective_cwd.is_dir():
        raise FileNotFoundError(f"build cwd not found: {effective_cwd}")

    argv = build_command_argv(command_text)
    result = subprocess.run(
        argv,
        cwd=effective_cwd,
        check=False,
        capture_output=True,
        text=True,
    )

    outputs = workspace_payload["outputs"]
    log_path = Path(str(outputs["build_log"]))
    status_path = Path(str(outputs["build_status"]))
    log_path.parent.mkdir(parents=True, exist_ok=True)
    log_text = result.stdout
    if result.stderr:
        if log_text:
            log_text += "\n"
        log_text += result.stderr
    log_path.write_text(log_text, encoding="utf-8")
    write_json(
        status_path,
        build_status_payload(
            workspace_payload,
            command_text=command_text,
            cwd=effective_cwd,
            result=result,
        ),
    )
    return int(result.returncode), log_path, status_path
