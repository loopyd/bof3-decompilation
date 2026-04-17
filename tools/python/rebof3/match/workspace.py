from __future__ import annotations

import shlex
from pathlib import Path
from typing import Any

from ..jsonio import read_json, write_json


def parse_hexish(value: str) -> int:
    return int(value, 16)


def format_hex(value: int) -> str:
    return f"0x{value:08x}"


def normalize_program_selector(value: str) -> str:
    return str(value or "").strip("/")


def load_function_rows(path: Path) -> list[dict[str, Any]]:
    payload = read_json(path)
    rows = payload.get("rows", payload)
    if not isinstance(rows, list):
        raise ValueError(f"function index rows must be a list: {path}")
    normalized_rows: list[dict[str, Any]] = []
    for row in rows:
        if not isinstance(row, dict):
            continue
        normalized_rows.append(dict(row))
    return normalized_rows


def row_matches_program(row: dict[str, Any], selector: str) -> bool:
    normalized_selector = normalize_program_selector(selector)
    for key in ("program_path", "program_name", "program_slug"):
        value = row.get(key)
        if value is None:
            continue
        if str(value) == selector:
            return True
        if normalize_program_selector(str(value)) == normalized_selector:
            return True
    return False


def find_function_row(
    rows: list[dict[str, Any]],
    *,
    program: str,
    entry: str,
) -> dict[str, Any]:
    target_entry = parse_hexish(entry.removeprefix("0x"))
    matches = [
        row
        for row in rows
        if row_matches_program(row, program)
        and parse_hexish(str(row.get("entry") or "0").removeprefix("0x"))
        == target_entry
    ]
    if not matches:
        raise LookupError(f"no function found for program={program} entry={entry}")
    if len(matches) > 1:
        raise LookupError(f"multiple functions matched program={program} entry={entry}")
    return matches[0]


def workspace_dir_name(row: dict[str, Any]) -> str:
    return format_hex(parse_hexish(str(row.get("entry") or "0").removeprefix("0x")))


def workspace_path_for_row(workspace_root: Path, row: dict[str, Any]) -> Path:
    program_slug = str(row.get("program_slug") or row.get("program_name") or "program")
    return workspace_root / program_slug / workspace_dir_name(row) / "workspace.json"


def normalize_optional_path(path: Path | None) -> str | None:
    if path is None:
        return None
    return str(path.expanduser().resolve())


def build_workspace_payload(
    row: dict[str, Any],
    *,
    function_index_path: Path,
    workspace_root: Path,
    build_command: str | None,
    build_cwd: Path | None,
    expected_artifact: Path | None,
    actual_artifact: Path | None,
    source_file: Path | None,
) -> dict[str, Any]:
    workspace_path = workspace_path_for_row(workspace_root, row)
    workspace_dir = workspace_path.parent
    entry_value = parse_hexish(str(row.get("entry") or "0").removeprefix("0x"))
    effective_build_cwd = build_cwd.expanduser().resolve() if build_cwd else None
    payload = {
        "schema": "rebof3-simple.match-workspace/v1",
        "workspace": {
            "workspace_json": str(workspace_path.resolve()),
            "workspace_dir": str(workspace_dir.resolve()),
            "function_index": str(function_index_path.expanduser().resolve()),
        },
        "function": {
            "program_path": row.get("program_path"),
            "program_name": row.get("program_name"),
            "program_slug": row.get("program_slug"),
            "folder": row.get("folder"),
            "entry": str(row.get("entry") or "0"),
            "entry_hex": str(row.get("entry_hex") or format_hex(entry_value)),
            "name": row.get("name"),
            "signature": row.get("signature"),
            "namespace": row.get("namespace"),
            "name_source": row.get("name_source"),
            "source_hint": row.get("source_hint"),
            "symbol_file": row.get("symbol_file"),
        },
        "inputs": {
            "build_command": build_command,
            "build_cwd": None
            if effective_build_cwd is None
            else str(effective_build_cwd),
            "expected_artifact": normalize_optional_path(expected_artifact),
            "actual_artifact": normalize_optional_path(actual_artifact),
            "source_file": normalize_optional_path(source_file),
        },
        "outputs": {
            "build_log": str((workspace_dir / "build.log").resolve()),
            "build_status": str((workspace_dir / "build.json").resolve()),
            "diff_json": str((workspace_dir / "diff.json").resolve()),
            "diff_markdown": str((workspace_dir / "diff.md").resolve()),
        },
    }
    return payload


def initialize_workspace(
    row: dict[str, Any],
    *,
    function_index_path: Path,
    workspace_root: Path,
    build_command: str | None,
    build_cwd: Path | None,
    expected_artifact: Path | None,
    actual_artifact: Path | None,
    source_file: Path | None,
) -> Path:
    payload = build_workspace_payload(
        row,
        function_index_path=function_index_path,
        workspace_root=workspace_root,
        build_command=build_command,
        build_cwd=build_cwd,
        expected_artifact=expected_artifact,
        actual_artifact=actual_artifact,
        source_file=source_file,
    )
    workspace_path = Path(str(payload["workspace"]["workspace_json"]))
    workspace_path.parent.mkdir(parents=True, exist_ok=True)
    write_json(workspace_path, payload)
    return workspace_path


def load_workspace(path: Path) -> dict[str, Any]:
    payload = read_json(path)
    if not isinstance(payload, dict):
        raise ValueError(f"workspace payload must be an object: {path}")
    return payload


def resolve_workspace_path(
    *,
    workspace: Path | None,
    function_index: Path | None,
    workspace_root: Path | None,
    program: str | None,
    entry: str | None,
) -> Path:
    if workspace is not None:
        return workspace.expanduser().resolve()
    if (
        function_index is None
        or workspace_root is None
        or program is None
        or entry is None
    ):
        raise ValueError(
            "provide --workspace or the full --function-index/--workspace-root/--program/--entry selector"
        )
    row = find_function_row(
        load_function_rows(function_index), program=program, entry=entry
    )
    return workspace_path_for_row(workspace_root, row).resolve()


def build_command_argv(command_text: str) -> list[str]:
    argv = shlex.split(command_text)
    if not argv:
        raise ValueError("build command must not be empty")
    return argv
