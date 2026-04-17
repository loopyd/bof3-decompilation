from __future__ import annotations

import argparse
import json
import shlex
import subprocess
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Any

from ..common import (
    normalize_repo_path,
    run_command,
    write_json_output,
    write_text_output,
)
from . import baseline, workspace as workspace_lib


@dataclass(frozen=True, slots=True)
class WorkspaceState:
    workspace_json: Path
    workspace_dir: Path
    workspace_payload: dict[str, Any]
    build_status: dict[str, Any] | None
    ghidra_bundle_path: Path | None
    ghidra_bundle_exists: bool
    refresh_log_path: Path


def add_workspace_resolver_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("-p", "--program", help="program path, name, or slug")
    parser.add_argument("-e", "--entry", help="function entry address")
    parser.add_argument(
        "-w",
        "--workspace-json",
        type=Path,
        default=None,
        help="Use an existing workspace.json instead of resolving from --program/--entry.",
    )
    parser.add_argument(
        "--inventory-db",
        type=Path,
        default=workspace_lib.DEFAULT_INVENTORY_DB,
    )
    parser.add_argument(
        "--workspace-root",
        type=Path,
        default=workspace_lib.DEFAULT_WORKSPACE_ROOT,
    )


def resolve_workspace(
    args: argparse.Namespace, logger: Any
) -> tuple[Path, dict[str, Any]] | None:
    workspace_json = getattr(args, "workspace_json", None)
    if workspace_json is not None:
        if not workspace_json.exists():
            logger.error(f"workspace not found: {workspace_json}")
            return None
        return workspace_json, workspace_lib.load_workspace_payload(workspace_json)

    program = getattr(args, "program", None)
    entry = getattr(args, "entry", None)
    if not program or not entry:
        logger.error("pass --workspace-json or both --program and --entry")
        return None

    inventory_db = getattr(args, "inventory_db")
    if not inventory_db.exists():
        logger.error(f"inventory db not found: {inventory_db}")
        return None

    rows = workspace_lib.load_function_rows(inventory_db)
    program_rows = workspace_lib.load_program_rows(inventory_db)
    try:
        workspace_json = workspace_lib.find_workspace_json(
            rows,
            program=program,
            entry=entry,
            workspace_root=getattr(args, "workspace_root"),
            program_rows=program_rows,
            artifact_root=workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
        )
    except LookupError as exc:
        logger.error(str(exc))
        return None

    if not workspace_json.exists():
        logger.error(f"workspace not found: {workspace_json}")
        return None
    return workspace_json, workspace_lib.load_workspace_payload(workspace_json)


def load_build_status(path: Path) -> dict[str, Any] | None:
    if not path.exists():
        return None
    return dict(json.loads(path.read_text(encoding="utf-8")))


def build_workspace_state(
    workspace_json: Path, workspace_payload: dict[str, Any]
) -> WorkspaceState:
    workspace_dir = workspace_json.parent
    ghidra_bundle_path = normalize_repo_path(
        workspace_payload.get("ghidra_decomp_bundle_json")
    )
    return WorkspaceState(
        workspace_json=workspace_json,
        workspace_dir=workspace_dir,
        workspace_payload=workspace_payload,
        build_status=load_build_status(workspace_dir / "build.json"),
        ghidra_bundle_path=ghidra_bundle_path,
        ghidra_bundle_exists=bool(ghidra_bundle_path and ghidra_bundle_path.exists()),
        refresh_log_path=workspace_dir / "ghidra_decomp.log",
    )


def maybe_refresh_ghidra_bundle(
    state: WorkspaceState,
    *,
    refresh: bool,
) -> tuple[WorkspaceState, subprocess.CompletedProcess[str] | None]:
    command_text = (state.workspace_payload.get("commands") or {}).get("ghidra_decomp")
    if not refresh or not command_text or state.ghidra_bundle_path is None:
        return state, None
    command = shlex.split(str(command_text))
    result = run_command(command)
    write_text_output(
        state.refresh_log_path,
        result.stdout + ("" if not result.stderr else "\n" + result.stderr),
    )
    return (
        replace(state, ghidra_bundle_exists=bool(state.ghidra_bundle_path.exists())),
        result,
    )


def refresh_expected_baseline(state: WorkspaceState) -> WorkspaceState:
    if state.workspace_payload.get("expected_baseline_ready"):
        return state
    if state.ghidra_bundle_path is None or not state.ghidra_bundle_path.exists():
        return state
    baseline_info = baseline.baseline_from_bundle_json(state.ghidra_bundle_path)
    if baseline_info is None:
        return state
    refreshed = dict(state.workspace_payload)
    refreshed["expected_baseline"] = baseline_info
    refreshed["expected_baseline_ready"] = True
    refreshed["ghidra_decomp_bundle_exists"] = True
    write_json_output(state.workspace_json, refreshed)
    return replace(state, workspace_payload=refreshed, ghidra_bundle_exists=True)


def diff_status(state: WorkspaceState) -> tuple[str, list[str]]:
    next_steps: list[str] = []
    if not state.ghidra_bundle_exists:
        next_steps.append(
            "run the recorded ghidra_decomp command to generate func.json evidence"
        )
        return "blocked_missing_ghidra_bundle", next_steps
    if state.build_status is None:
        next_steps.append(
            "run match_build to capture a fresh PSX build status for this workspace"
        )
        return "needs_build_status", next_steps
    if not bool(state.build_status.get("succeeded")):
        next_steps.append("fix the PSX build failure before diffing")
        return "blocked_build_failed", next_steps

    if not bool(state.workspace_payload.get("source_mapping_ready")):
        next_steps.append(
            "add a lifted C function with an address-stable name such as func_80162d00 so the workspace can resolve a source/object target"
        )
        return "blocked_missing_source_mapping", next_steps

    if not bool(state.workspace_payload.get("expected_baseline_ready")):
        next_steps.append(
            "refresh the workspace after generating the ghidra_decomp bundle so it can record an expected baseline asm"
        )
        return "blocked_missing_expected_baseline", next_steps

    source_mapping = state.workspace_payload.get("source_mapping") or {}
    object_candidates = source_mapping.get("object_candidates") or []
    if object_candidates:
        next_steps.append(
            "inspect the asm-differ and objdiff reports for the function-sliced expected/current objects and tighten the expected baseline model if needed"
        )
    else:
        next_steps.append(
            "wire asm-differ or objdiff against the resolved object candidates for this workspace"
        )
    return "ready_for_backend_diff", next_steps
