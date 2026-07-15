"""Analysis operations wired to the new Rizin-only pipeline."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path
from typing import Any

from ..domain import TargetManifest, load_target_manifests, normalize_target_id
from ..domain.registry import ResolvedTarget, resolve_target
from ..jsonio import write_json
from ..layout import parse_splat_layout
from ..source_inventory import build_source_inventory
from .graph import build_graph, write_graph
from .replay import (
    build_replay_plan,
    replay_output_path,
    write_generated_replay,
)
from .rizin import find_engine, run_plan, run_project_init, run_query
from .snapshot import SNAPSHOT_SCHEMA, build_snapshot, read_snapshot, write_snapshot


_PROJECT_STATE_SCHEMA = "bof3.analysis-project/v3"
_PROJECT_STATE_FILE = "state.json"

_QUERY_COMMANDS = {
    "functions": "aflj",
    "strings": "izzj",
    "xrefs": "axlj",
    "types": "tj",
}


def _target(root: Path, value: str) -> ResolvedTarget:
    return resolve_target(root, value)


def _slug(manifest: TargetManifest) -> str:
    return manifest.id.value.replace("/", "__")


def _paths(root: Path, manifest: TargetManifest) -> tuple[Path, Path]:
    project = root / "out" / "analysis" / "projects" / "rizin" / _slug(manifest)
    export = root / "out" / "analysis" / "exports" / "rizin" / _slug(manifest)
    return project, export


def _sha256(path: Path) -> str | None:
    if not path.is_file():
        return None
    return hashlib.sha256(path.read_bytes()).hexdigest()


def doctor() -> dict[str, Any]:
    """Check Rizin availability and capabilities."""

    try:
        identity = find_engine("rizin")
        return {
            "engine": "rizin",
            "available": True,
            "path": str(identity.executable),
            "version": identity.version,
            "capabilities": identity.capabilities,
        }
    except (FileNotFoundError, RuntimeError):
        pass
    try:
        identity = find_engine("r2")
        return {
            "engine": "r2",
            "available": True,
            "path": str(identity.executable),
            "version": identity.version,
            "capabilities": identity.capabilities,
        }
    except (FileNotFoundError, RuntimeError) as error:
        return {
            "engine": None,
            "available": False,
            "error": str(error),
        }


def generate_replay(root: Path, target: str) -> dict[str, Any]:
    """Render the generated replay under ``out/analysis/replay``."""

    resolved = _target(root, target)
    layout = parse_splat_layout(resolved.splat_path, resolved.load_address)
    inventory = build_source_inventory(resolved.source_dir, resolved.id.value)
    output = write_generated_replay(root, resolved, layout, inventory)
    text = output.read_text(encoding="utf-8")
    return {
        "target": resolved.id.value,
        "output": str(output.relative_to(root)),
        "splat_funcs": len(re.findall(r"^afn func_", text, flags=re.M)),
        "psyq_funcs": len(re.findall(r"^f psyq\.", text, flags=re.M)),
        "data_flags": len(re.findall(r"^f data\.", text, flags=re.M)),
    }


def initialize_project(
    root: Path, target: str, requested: str | None = None
) -> dict[str, Any]:
    """Initialize a per-target Rizin project with verified replay."""

    identity = find_engine("rizin")
    resolved = _target(root, target)
    layout = parse_splat_layout(resolved.splat_path, resolved.load_address)
    inventory = build_source_inventory(resolved.source_dir, resolved.id.value)

    # Write generated replay first so inputs capture its hash.
    write_generated_replay(root, resolved, layout, inventory)
    plan = build_replay_plan(root, resolved, layout, inventory)

    project_dir, _ = _paths(root, resolved.id)
    project_dir.mkdir(parents=True, exist_ok=True)
    project_path = project_dir / f"{_slug(resolved.id)}.rzdb"
    sentinel = f"harness.sentinel_{plan.inputs.composite_hash()}"

    verified = run_project_init(
        identity, resolved, plan, project_path, sentinel
    )

    state = {
        "schema": _PROJECT_STATE_SCHEMA,
        "engine": "rizin",
        "target": resolved.id.value,
        "project": str(project_path),
        "sentinel": sentinel,
        "inputs": {
            "manifest_sha256": plan.inputs.manifest_sha256,
            "binary_sha256": plan.inputs.binary_sha256,
            "splat_sha256": plan.inputs.splat_sha256,
            "source_inventory_sha256": plan.inputs.source_inventory_sha256,
            "generated_replay_sha256": plan.inputs.generated_replay_sha256,
            "reviewed_replay_sha256": plan.inputs.reviewed_replay_sha256,
        },
    }
    write_json(project_dir / _PROJECT_STATE_FILE, state)

    return {
        "engine": "rizin",
        "target": resolved.id.value,
        "project": str(project_dir.relative_to(root)),
        "generated": str(replay_output_path(root, resolved.id).relative_to(root)),
        "reviewed": str(resolved.reviewed_replay_path.relative_to(root))
        if resolved.reviewed_replay_path.is_file()
        else None,
        "verified_reopen": verified,
    }


def query_project(
    root: Path, target: str, query: str, requested: str | None = None
) -> Any:
    """Run a named read-only query against an initialized project."""

    if query not in _QUERY_COMMANDS:
        raise ValueError(
            f"unsupported query {query!r}; allowed: {sorted(_QUERY_COMMANDS)}"
        )
    identity = find_engine("rizin")
    resolved = _target(root, target)
    layout = parse_splat_layout(resolved.splat_path, resolved.load_address)
    inventory = build_source_inventory(resolved.source_dir, resolved.id.value)
    plan = build_replay_plan(root, resolved, layout, inventory)
    return run_query(identity, resolved, plan, query)


def export_project(
    root: Path, target: str, requested: str | None = None
) -> dict[str, Any]:
    """Export a normalized snapshot for a single target."""

    resolved = _target(root, target)
    layout = parse_splat_layout(resolved.splat_path, resolved.load_address)
    inventory = build_source_inventory(resolved.source_dir, resolved.id.value)
    write_generated_replay(root, resolved, layout, inventory)
    plan = build_replay_plan(root, resolved, layout, inventory)
    inputs = plan.inputs

    identity = find_engine("rizin")
    dump = run_plan(identity, resolved, plan)
    snapshot = build_snapshot(
        resolved=resolved,
        layout=layout,
        inventory=inventory,
        dump=dump,
        inputs=inputs,
        root=root,
    )

    snapshot_path = (
        root / "out" / "analysis" / "snapshots" / "rizin" / f"{resolved.id.value}.json"
    )
    write_snapshot(snapshot, snapshot_path)

    return {
        "schema": SNAPSHOT_SCHEMA,
        "target": resolved.id.value,
        "snapshot": str(snapshot_path.relative_to(root)),
        "functions": len(snapshot.functions),
        "calls": len(snapshot.calls),
        "unresolved_calls": len(snapshot.unresolved_calls),
    }


def graph_analysis(
    root: Path, target: str | None = None, requested: str | None = None
) -> dict[str, Any]:
    """Build the canonical analysis graph from normalized snapshots."""

    manifests = load_target_manifests(root)
    selected: list[TargetManifest]
    if target:
        tid = normalize_target_id(target).value
        if tid not in manifests:
            raise ValueError(f"unknown target: {target}")
        selected = [manifests[tid]]
    else:
        selected = list(manifests.values())
    selected.sort(key=lambda m: m.id.value)

    snapshots: dict[str, Any] = {}
    skipped: list[dict[str, str]] = []
    for manifest in selected:
        snapshot_path = (
            root / "out" / "analysis" / "snapshots" / "rizin" / f"{manifest.id.value}.json"
        )
        if not snapshot_path.is_file():
            binary = root / manifest.binary
            if binary.is_file():
                skipped.append(
                    {"target": manifest.id.value, "reason": "snapshot missing"}
                )
            continue
        try:
            snapshot = read_snapshot(snapshot_path)
            snapshots[manifest.id.value] = snapshot
        except (ValueError, KeyError) as error:
            skipped.append(
                {"target": manifest.id.value, "reason": f"invalid snapshot: {error}"}
            )

    graph = build_graph(
        snapshots,
        engine_name="rizin",
        skipped=skipped if skipped else None,
    )
    output = root / "out" / "analysis" / "graph.json"
    write_graph(graph, output)

    return {
        "output": str(output.relative_to(root)),
        "targets": len(graph.targets_analyzed),
        "skipped": graph.targets_skipped,
        "functions": len(graph.functions),
        "calls": len(graph.calls),
        "unresolved_calls": len(graph.unresolved_calls),
        "exact_duplicates": len(graph.duplicate_groups),
    }


def _classify_analyzer_string(value: Any) -> str:
    """Classify analyzer string guesses without treating them as decoded text."""

    if not isinstance(value, str) or not value:
        return "data_pattern"

    codepoints = [ord(character) for character in value]
    printable_ascii = all(0x20 <= point <= 0x7E for point in codepoints)
    repeated = len(set(codepoints)) == 1
    if repeated and (len(codepoints) >= 8 or not printable_ascii):
        return "repeated_fill"

    if len(codepoints) >= 4:
        deltas = [right - left for left, right in zip(codepoints, codepoints[1:])]
        if len(set(deltas)) == 1 and abs(deltas[0]) == 1 and not printable_ascii:
            return "sequential_table"

    if any(
        point < 0x20 or point == 0x7F or 0x80 <= point <= 0x9F for point in codepoints
    ):
        return "control_bytes"

    if (
        printable_ascii
        and len(value) >= 3
        and any(character.isalnum() for character in value)
    ):
        return "text_candidate"
    return "data_pattern"


__all__ = [
    "doctor",
    "export_project",
    "generate_replay",
    "graph_analysis",
    "initialize_project",
    "query_project",
]
