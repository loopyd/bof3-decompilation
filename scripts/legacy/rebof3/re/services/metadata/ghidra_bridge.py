from __future__ import annotations

import json
import sys
from pathlib import Path

from ....common import prepend_pythonpath
from ....config import GHIDRA_MAIN_MODULE, GHIDRA_SRC_DIR, ROOT
from ....inventory.layout import INVENTORY_SQLITE
from ....models.metadata import MetadataSyncPlan
from .planning import selected_program_selectors


def ghidra_cli_env() -> dict[str, str]:
    return prepend_pythonpath(GHIDRA_SRC_DIR)


def ghidra_metadata_payload(plan: MetadataSyncPlan) -> dict[str, object]:
    rows: list[dict[str, object]] = []
    for batch in plan.batches:
        for row in batch.rows:
            payload = dict(row.row)
            program_path = str(payload.get("program_path") or "").strip()
            if program_path:
                payload["program_path"] = plan.program_selectors.get(
                    program_path, program_path
                )
            rows.append(payload)
    return {
        "schema": "bof3.metadata.apply_request/v1",
        "mode": "project",
        "replace_data": False,
        "allow_code_overwrite": False,
        "validate_only": plan.mode == "preflight",
        "rows": rows,
    }


def run_ghidra_metadata(
    *,
    plan: MetadataSyncPlan,
    project_dir: Path,
    project_name: str,
    output_path: Path,
    log_path: Path | None = None,
) -> tuple[int, dict[str, object] | None, str, str]:
    from . import run_command

    command = [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "metadata",
        "validate" if plan.mode == "preflight" else "apply",
        "--project-dir",
        str(project_dir),
        "--project-name",
        project_name,
        "--output",
        str(output_path),
    ]
    if log_path is not None:
        command.extend(["--log-path", str(log_path)])
    command.append("--allow-partial")
    request_path = output_path.with_suffix(".request.json")
    request_path.write_text(
        json.dumps(ghidra_metadata_payload(plan), indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    command.extend(["--input", str(request_path)])
    result = run_command(command, cwd=ROOT, env=ghidra_cli_env())
    payload = None
    if output_path.exists():
        payload = json.loads(output_path.read_text(encoding="utf-8"))
    return int(result.returncode), payload, result.stdout or "", result.stderr or ""


def load_known_type_names(
    *,
    db_path: Path = INVENTORY_SQLITE,
    owner: str | None = None,
    selectors: tuple[str, ...] = (),
    project_dir: Path,
    project_name: str,
    output_path: Path,
    log_path: Path | None = None,
) -> tuple[str, ...]:
    from . import run_command

    selected_programs = selected_program_selectors(
        db_path=db_path,
        owner=owner,
        selectors=selectors,
    )
    if not selected_programs:
        return ()
    command = [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "metadata",
        "known-types",
        "--project-dir",
        str(project_dir),
        "--project-name",
        project_name,
        "--output",
        str(output_path),
    ]
    if log_path is not None:
        command.extend(["--log-path", str(log_path)])
    for program_path in selected_programs:
        command.extend(["--program", program_path])
    result = run_command(command, cwd=ROOT, env=ghidra_cli_env())
    if result.returncode != 0:
        raise RuntimeError("ghidra metadata known-types command failed")
    payload = json.loads(output_path.read_text(encoding="utf-8"))
    names: list[str] = []
    for program in payload.get("programs", []):
        if not isinstance(program, dict):
            continue
        for name in program.get("type_names", []):
            normalized = str(name or "").strip()
            if normalized and normalized not in names:
                names.append(normalized)
    return tuple(names)
