from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from ..common import (
    ROOT,
    default_artifacts_dir,
    format_hex,
    parse_hexish,
    parse_source_spec,
    relative_to_root,
    write_json_output,
)
from . import baseline, source_map
from .models import WorkspaceRef
from .target import find_function_row, program_slug_for_row


def workspace_dir(root: Path, row: dict[str, Any]) -> Path:
    entry_value = parse_hexish(str(row.get("entry") or "0"))
    return root / program_slug_for_row(row) / format_hex(entry_value)


def workspace_json_path(root: Path, row: dict[str, Any]) -> Path:
    return workspace_dir(root, row) / "workspace.json"


def workspace_ref(root: Path, row: dict[str, Any]) -> WorkspaceRef:
    dir_path = workspace_dir(root, row)
    return WorkspaceRef(
        root=root, dir_path=dir_path, json_path=dir_path / "workspace.json"
    )


def suggested_artifacts_dir(
    row: dict[str, Any],
    artifact_root: Path,
    *,
    source_override: str | None,
) -> Path | None:
    source_hint = row.get("source_hint") or source_override
    if not source_hint:
        return None
    source_spec = parse_source_spec(str(source_hint))
    source_path_raw, entry_index = source_spec.path, source_spec.entry_index
    source_path = (
        source_path_raw if source_path_raw.is_absolute() else (ROOT / source_path_raw)
    )
    return default_artifacts_dir(
        artifact_root,
        source_path.resolve(),
        parse_hexish(str(row.get("entry") or "0")),
        entry_index,
    )


def ghidra_decomp_command(
    row: dict[str, Any], artifacts_dir: Path | None, source: str | None
) -> str | None:
    source_hint = row.get("source_hint") or source
    if not source_hint or artifacts_dir is None:
        return None
    return " ".join(
        [
            "python3",
            "-m",
            "scripts.rebof3",
            "re",
            "ghidra-decomp",
            str(source_hint),
            str(
                row.get("entry_hex")
                or format_hex(parse_hexish(str(row.get("entry") or "0")))
            ),
            "--artifacts-dir",
            relative_to_root(artifacts_dir),
        ]
    )


def build_workspace_payload(
    row: dict[str, Any],
    *,
    inventory_db: Path,
    workspace_root: Path,
    artifact_root: Path,
    source_override: str | None,
) -> tuple[Path, dict[str, Any]]:
    ref = workspace_ref(workspace_root, row)
    artifacts_dir = suggested_artifacts_dir(
        row, artifact_root, source_override=source_override
    )
    bundle_json = None if artifacts_dir is None else artifacts_dir / "func.json"
    bundle_exists = bool(bundle_json and bundle_json.exists())
    source_mapping = source_map.find_source_mapping(
        str(row.get("entry") or "0"),
        program_path=str(row.get("program_path") or ""),
        program_name=str(row.get("program_name") or ""),
        source_hint=str(source_override or row.get("source_hint") or ""),
    )
    baseline_info = (
        None if bundle_json is None else baseline.baseline_from_bundle_json(bundle_json)
    )
    payload = {
        "program_name": row.get("program_name"),
        "program_path": row.get("program_path"),
        "program_slug": row.get("program_slug"),
        "folder": row.get("folder"),
        "entry": row.get("entry"),
        "entry_hex": row.get("entry_hex")
        or format_hex(parse_hexish(str(row.get("entry") or "0"))),
        "name": row.get("name"),
        "signature": row.get("signature"),
        "namespace": row.get("namespace"),
        "comment": row.get("comment"),
        "repeatable_comment": row.get("repeatable_comment"),
        "name_source": row.get("name_source"),
        "source_hint": row.get("source_hint"),
        "source_override": source_override,
        "source_mapping": None
        if source_mapping is None
        else {
            key: value for key, value in source_mapping.items() if key != "source_text"
        },
        "source_mapping_ready": source_mapping is not None,
        "expected_baseline": baseline_info,
        "expected_baseline_ready": baseline_info is not None,
        "inventory_db": relative_to_root(inventory_db),
        "workspace_dir": relative_to_root(ref.dir_path),
        "ghidra_decomp_artifacts_dir": None
        if artifacts_dir is None
        else relative_to_root(artifacts_dir),
        "ghidra_decomp_bundle_json": None
        if bundle_json is None
        else relative_to_root(bundle_json),
        "ghidra_decomp_bundle_exists": bundle_exists,
        "commands": {
            "ghidra_decomp": ghidra_decomp_command(row, artifacts_dir, source_override)
        },
    }
    return ref.dir_path, payload


def load_workspace_payload(path: Path) -> dict[str, Any]:
    return dict(json.loads(path.read_text(encoding="utf-8")))


def write_workspace_payload(path: Path, payload: dict[str, Any]) -> None:
    write_json_output(path, payload)


def find_workspace_json(
    rows: list[dict[str, Any]],
    *,
    program: str,
    entry: str,
    workspace_root: Path,
    program_rows: list[dict[str, Any]] | None = None,
    artifact_root: Path,
    source_root: Path = source_map.DEFAULT_SOURCE_ROOT,
) -> Path:
    row = find_function_row(
        rows,
        program=program,
        entry=entry,
        program_rows=program_rows,
        artifact_root=artifact_root,
        source_root=source_root,
    )
    return workspace_json_path(workspace_root, row)


def refresh_workspace_json(
    rows: list[dict[str, Any]],
    *,
    program: str,
    entry: str,
    inventory_db: Path,
    workspace_root: Path,
    artifact_root: Path,
    source_override: str | None = None,
    program_rows: list[dict[str, Any]] | None = None,
    source_root: Path = source_map.DEFAULT_SOURCE_ROOT,
) -> tuple[Path, dict[str, Any]]:
    row = find_function_row(
        rows,
        program=program,
        entry=entry,
        program_rows=program_rows,
        artifact_root=artifact_root,
        source_root=source_root,
    )
    ref = workspace_ref(workspace_root, row)
    _, payload = build_workspace_payload(
        row,
        inventory_db=inventory_db,
        workspace_root=workspace_root,
        artifact_root=artifact_root,
        source_override=source_override,
    )
    write_workspace_payload(ref.json_path, payload)
    return ref.json_path, payload
