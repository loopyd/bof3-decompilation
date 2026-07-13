from __future__ import annotations

import json
from pathlib import Path
import shutil
import subprocess
from typing import Any

from ..domain import TargetManifest, load_target_manifests, normalize_target_id
from ..jsonio import write_json


_QUERY_COMMANDS = {
    "functions": "aflj",
    "strings": "izzj",
    "xrefs": "axlj",
    "types": "tj",
}


def _engine(requested: str | None = None) -> tuple[str, Path]:
    candidates = (requested,) if requested else ("rizin", "r2")
    for name in candidates:
        if name and (executable := shutil.which(name)):
            return name, Path(executable)
    expected = requested or "rizin or r2"
    raise FileNotFoundError(f"analysis engine not found: {expected}")


def _target(root: Path, value: str) -> TargetManifest:
    target_id = normalize_target_id(value).value
    manifest = load_target_manifests(root).get(target_id)
    if manifest is None:
        raise ValueError(f"unknown promoted target: {value}")
    binary = root / manifest.binary
    if not binary.is_file():
        raise FileNotFoundError(f"target binary missing: {manifest.binary}")
    return manifest


def _slug(manifest: TargetManifest) -> str:
    return manifest.id.value.replace("/", "__")


def _paths(root: Path, engine: str, manifest: TargetManifest) -> tuple[Path, Path]:
    project = root / "out" / "analysis" / "projects" / engine / _slug(manifest)
    export = root / "out" / "analysis" / "exports" / _slug(manifest)
    return project, export


def _run(
    executable: Path,
    manifest: TargetManifest,
    root: Path,
    commands: list[str],
    *,
    project_dir: Path,
) -> str:
    arguments = [
        str(executable),
        "-q0",
        "-a",
        "mips",
        "-b",
        "32",
        "-e",
        "cfg.bigendian=false",
        "-e",
        f"dir.projects={project_dir}",
        "-e",
        "prj.vc=false",
        "-m",
        f"0x{manifest.load_address:08x}",
    ]
    for command in commands:
        arguments.extend(("-c", command))
    arguments.append(str(root / manifest.binary))
    result = subprocess.run(arguments, check=True, capture_output=True, text=True)
    return result.stdout.rstrip("\x00\n")


def doctor() -> dict[str, Any]:
    tools: dict[str, dict[str, Any]] = {}
    for name in ("rizin", "r2"):
        path = shutil.which(name)
        tools[name] = {"available": path is not None, "path": path}
    for name in ("rz-ghidra", "r2ghidra"):
        tools[name] = {"available": shutil.which(name) is not None}
    return {
        "preferred": "rizin" if tools["rizin"]["available"] else "r2",
        "tools": tools,
    }


def initialize_project(
    root: Path, target: str, requested: str | None = None
) -> dict[str, Any]:
    engine, executable = _engine(requested)
    manifest = _target(root, target)
    project_dir, _ = _paths(root, engine, manifest)
    project_dir.mkdir(parents=True, exist_ok=True)
    commands = ["aaa"]
    types = root / "config" / "analysis" / "bof3_objects.h"
    if types.is_file():
        commands.append(f"to {types}")
    replay = root / "config" / "analysis" / f"{manifest.id.value}.r2"
    if replay.is_file():
        commands.append(f". {replay}")
    # Generated projects are disposable snapshots; replace the named snapshot.
    commands.extend((f"P-{_slug(manifest)}", f"Ps {_slug(manifest)}"))
    _run(executable, manifest, root, commands, project_dir=project_dir)
    return {
        "engine": engine,
        "target": manifest.id.value,
        "project": str(project_dir.relative_to(root)),
        "replay": str(replay.relative_to(root)) if replay.is_file() else None,
    }


def _json_output(output: str) -> Any:
    value = json.loads(output or "[]")
    if isinstance(value, list):
        return sorted(
            value, key=lambda row: (row.get("offset", 0), row.get("name", ""))
        )
    return value


def query_project(
    root: Path, target: str, query: str, requested: str | None = None
) -> Any:
    engine, executable = _engine(requested)
    manifest = _target(root, target)
    project_dir, _ = _paths(root, engine, manifest)
    command = _QUERY_COMMANDS.get(query, query)
    return _json_output(
        _run(executable, manifest, root, ["aaa", command], project_dir=project_dir)
    )


def export_project(
    root: Path, target: str, requested: str | None = None
) -> dict[str, Any]:
    engine, _ = _engine(requested)
    manifest = _target(root, target)
    _, export_dir = _paths(root, engine, manifest)
    payload = {
        "schema": "bof3.analysis/v1",
        "engine": engine,
        "target": manifest.id.value,
        "binary": manifest.binary,
        "load_address": manifest.load_address,
        "functions": query_project(root, target, "functions", engine),
        "strings": query_project(root, target, "strings", engine),
        "xrefs": query_project(root, target, "xrefs", engine),
    }
    output = export_dir / "analysis.json"
    write_json(output, payload)
    return {"output": str(output.relative_to(root)), **payload}
