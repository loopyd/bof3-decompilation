from __future__ import annotations

import json
import os
import shutil
import subprocess
from pathlib import Path
from typing import Any

from ..common import ROOT as COMMON_ROOT
from ..config import OBJDIFF_BINARY


ROOT = COMMON_ROOT


def relative_to_root(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return str(path.resolve())


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def write_text(path: Path, text: str) -> None:
    ensure_parent(path)
    path.write_text(text, encoding="utf-8")


def relative_to_dir(path: Path, base_dir: Path) -> str:
    return os.path.relpath(path.resolve(), base_dir.resolve())


def maybe_load_json(text: str) -> dict[str, Any] | None:
    payload = text.strip()
    if not payload:
        return None
    try:
        loaded = json.loads(payload)
    except json.JSONDecodeError:
        return None
    if not isinstance(loaded, dict):
        return None
    return loaded


def summarize_objdiff_result(
    stdout_json: dict[str, Any], symbol_name: str
) -> dict[str, Any]:
    symbol_payload = None
    for side_name in ("left", "right"):
        side = stdout_json.get(side_name) or {}
        symbols = side.get("symbols") or []
        candidate = next(
            (item for item in symbols if item.get("name") == symbol_name),
            None,
        )
        if candidate is None:
            continue
        symbol_payload = candidate
        if (
            candidate.get("instructions") is not None
            or candidate.get("match_percent") is not None
        ):
            break
    instructions = symbol_payload.get("instructions") if symbol_payload else None
    mismatches = 0
    if isinstance(instructions, list):
        for instruction in instructions:
            diff_kind = instruction.get("diff_kind")
            if diff_kind and diff_kind != "DIFF_NONE":
                mismatches += 1
    return {
        "has_json": True,
        "top_level_keys": sorted(stdout_json.keys()),
        "text_match_percent": None
        if symbol_payload is None
        else symbol_payload.get("match_percent"),
        "instruction_count": None
        if not isinstance(instructions, list)
        else len(instructions),
        "mismatch_count": None if not isinstance(instructions, list) else mismatches,
    }


def workspace_backend_dir(workspace_dir: Path) -> Path:
    return workspace_dir / "objdiff"


def backend_layout(workspace_dir: Path) -> dict[str, Path]:
    backend_dir = workspace_backend_dir(workspace_dir)
    return {
        "backend_dir": backend_dir,
        "config": backend_dir / "objdiff.json",
        "stdout": backend_dir / "diff.stdout.json",
        "stderr": backend_dir / "diff.stderr.log",
        "report": backend_dir / "backend.json",
    }


def prepare_backend(
    workspace_dir: Path,
    workspace_payload: dict[str, Any],
    *,
    asm_backend_report: dict[str, Any],
) -> dict[str, Any]:
    source_mapping = workspace_payload.get("source_mapping") or {}
    symbol_name = source_mapping.get("source_function") or workspace_payload.get("name")
    if not symbol_name:
        raise ValueError("workspace is missing a source function name for objdiff")

    current_object = ROOT / str(asm_backend_report["current_object"])
    expected_object = ROOT / str(asm_backend_report["expected_object"])
    if not current_object.exists() or not expected_object.exists():
        raise FileNotFoundError(
            "objdiff backend requires prepared asm-differ object slices"
        )

    layout = backend_layout(workspace_dir)
    layout["backend_dir"].mkdir(parents=True, exist_ok=True)
    config = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "min_version": "3.7.1",
        "build_base": False,
        "build_target": False,
        "units": [
            {
                "name": str(symbol_name),
                "target_path": relative_to_dir(expected_object, layout["backend_dir"]),
                "base_path": relative_to_dir(current_object, layout["backend_dir"]),
                "metadata": {
                    "complete": False,
                    "auto_generated": True,
                },
            }
        ],
    }
    write_text(layout["config"], json.dumps(config, indent=2, sort_keys=True) + "\n")
    return {
        "backend": "objdiff",
        "backend_dir": relative_to_root(layout["backend_dir"]),
        "config_path": relative_to_root(layout["config"]),
        "current_object": relative_to_root(current_object),
        "expected_object": relative_to_root(expected_object),
        "stdout_path": relative_to_root(layout["stdout"]),
        "stderr_path": relative_to_root(layout["stderr"]),
        "report_path": relative_to_root(layout["report"]),
        "symbol_name": str(symbol_name),
        "workspace_dir": workspace_payload.get("workspace_dir"),
    }


def backend_command(prepared: dict[str, Any]) -> list[str]:
    objdiff = str(OBJDIFF_BINARY)
    if not OBJDIFF_BINARY.exists():
        objdiff = shutil.which("objdiff-cli") or objdiff
    return [
        objdiff,
        "diff",
        "-u",
        str(prepared["symbol_name"]),
        "-o",
        "-",
        "--format",
        "json-pretty",
    ]


def run_backend(prepared: dict[str, Any]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        backend_command(prepared),
        cwd=ROOT / str(prepared["backend_dir"]),
        capture_output=True,
        text=True,
        check=False,
    )


def write_backend_outputs(
    prepared: dict[str, Any], result: subprocess.CompletedProcess[str]
) -> dict[str, Any]:
    stdout_path = ROOT / str(prepared["stdout_path"])
    stderr_path = ROOT / str(prepared["stderr_path"])
    report_path = ROOT / str(prepared["report_path"])
    write_text(stdout_path, result.stdout)
    write_text(stderr_path, result.stderr)
    stdout_json = maybe_load_json(result.stdout)
    report = {
        **prepared,
        "command": backend_command(prepared),
        "returncode": int(result.returncode),
        "succeeded": result.returncode == 0,
        "diff_summary": None
        if stdout_json is None
        else summarize_objdiff_result(stdout_json, str(prepared["symbol_name"])),
    }
    write_text(report_path, json.dumps(report, indent=2, sort_keys=True) + "\n")
    return report
