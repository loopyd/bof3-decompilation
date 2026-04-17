from __future__ import annotations

import json
import os
from pathlib import Path
from typing import Any

from ...common import ROOT, parse_hexish, relative_to_root
from ...config import DEFAULT_MATCH_ROOT, DEFAULT_PSX_PROFILE
from ...match import baseline, compile_one, workspace_store
from ...stubs import sync as stub_sync


DEFAULT_CANDIDATE_WORKSPACE_ROOT = DEFAULT_MATCH_ROOT / "candidates"
DEFAULT_CANDIDATE_BUILD_ROOT = ROOT / "build" / "bof3-psyq40-stubs"
STUB_BUILD_GENERATOR = "Ninja"
STUB_BUILD_TOOLCHAIN_FILE = ROOT / "cmake" / "toolchains" / "psyq.cmake"
STUB_BUILD_CACHE_VARS = (
    ("CMAKE_BUILD_TYPE", "Debug"),
    ("CMAKE_EXPORT_COMPILE_COMMANDS", "ON"),
    ("BOF3_TARGET_PSX", "ON"),
    ("BOF3_PSX_PROFILE", DEFAULT_PSX_PROFILE),
    ("BOF3_ENABLE_STUBS", "ON"),
)


def stable_function_name(entry_hex: str) -> str:
    """Return the repo-owned stable function name for one address."""

    return f"func_{parse_hexish(entry_hex):08x}"


def candidate_stub_source_path(program_path: str, entry_hex: str) -> Path:
    """Map one inventory row to the canonical disabled-stub source path."""

    return ROOT / stub_sync.stub_target_path(program_path, entry_hex)


def candidate_workspace_json_path(workspace_root: Path, row: dict[str, Any]) -> Path:
    """Return the canonical workspace.json path for one function row."""

    return workspace_store.workspace_json_path(workspace_root, row)


def candidate_internal_header_path(program_path: str, entry_hex: str) -> Path | None:
    """Resolve the promoted module-local header when one exists."""

    promoted_path = ROOT / stub_sync.promoted_target_path(program_path, entry_hex)
    header_path = promoted_path.parent / "internal.h"
    if header_path.exists():
        return header_path
    return None


def candidate_include_directive(
    program_path: str,
    entry_hex: str,
    *,
    stub_source_path: Path,
) -> str:
    """Choose the least noisy include for a generated candidate stub."""

    header_path = candidate_internal_header_path(program_path, entry_hex)
    if header_path is None:
        return '#include "bof3/defines.h"'
    rel = os.path.relpath(header_path, stub_source_path.parent)
    return f'#include "{Path(rel).as_posix()}"'


def rewrite_seed_function_name(text: str, *, function_name: str) -> str:
    """Normalize Ghidra-style FUN_* names to the repo-owned function symbol."""

    suffix = function_name.split("func_")[-1]
    return text.replace(f"FUN_{suffix.upper()}", function_name).replace(
        f"FUN_{suffix.lower()}",
        function_name,
    )


def wrap_candidate_source_text(
    seed_text: str,
    *,
    program_path: str,
    entry_hex: str,
    function_name: str,
    original_symbol_name: str | None,
    stub_source_path: Path,
) -> str:
    """Wrap one seed source with the include and provenance header we expect."""

    include_line = candidate_include_directive(
        program_path,
        entry_hex,
        stub_source_path=stub_source_path,
    )
    comment_label = original_symbol_name or function_name
    rewritten = rewrite_seed_function_name(seed_text.strip(), function_name=function_name)
    return (
        f"{include_line}\n\n"
        f"/* @source: {entry_hex} {comment_label} */\n"
        f"{rewritten}\n"
    )


def build_stub_configure_command(build_root: Path) -> list[str]:
    """Build the CMake configure command for the stub build tree."""

    command = [
        "cmake",
        "-S",
        str(ROOT),
        "-B",
        str(build_root),
        "-G",
        STUB_BUILD_GENERATOR,
        f"-DCMAKE_TOOLCHAIN_FILE={STUB_BUILD_TOOLCHAIN_FILE}",
    ]
    command.extend(f"-D{name}={value}" for name, value in STUB_BUILD_CACHE_VARS)
    return command


def compile_commands_has_source(
    compile_commands_path: Path,
    *,
    source_file: Path,
) -> bool:
    """Return whether compile_commands.json already knows about the candidate file."""

    if not compile_commands_path.exists():
        return False
    try:
        payload = json.loads(compile_commands_path.read_text(encoding="utf-8"))
    except (OSError, ValueError, TypeError):
        return False

    resolved = source_file.resolve()
    for entry in payload:
        candidate = Path(str(entry.get("file") or ""))
        try:
            if candidate.resolve() == resolved:
                return True
        except OSError:
            continue
    return False


def resolve_object_candidates(
    workspace_payload: dict[str, Any],
    *,
    build_root: Path,
) -> list[str]:
    """Resolve the object path candidates used by compile/diff tools."""

    try:
        plan = compile_one.plan_compile_one(workspace_payload, build_root=build_root)
    except (FileNotFoundError, LookupError):
        source_mapping = workspace_payload.get("source_mapping") or {}
        return list(source_mapping.get("object_candidates") or [])
    return [relative_to_root(Path(str(plan["object_path"])))]


def build_candidate_workspace_payload(
    row: dict[str, Any],
    *,
    inventory_db: Path,
    workspace_root: Path,
    build_root: Path,
    source_file: Path,
    bundle_payload: dict[str, Any],
) -> tuple[Path, dict[str, Any]]:
    """Create the workspace payload consumed by the compile/diff pipeline."""

    workspace_json = candidate_workspace_json_path(workspace_root, row)
    workspace_dir = workspace_json.parent
    bundle_json = ROOT / str(bundle_payload["files"]["json"])
    baseline_info = baseline.baseline_from_bundle_json(bundle_json)
    function_meta = bundle_payload.get("function") or {}
    source_mapping = {
        "source_file": relative_to_root(source_file),
        "source_function": stable_function_name(str(row["entry_hex"])),
        "source_signature": function_meta.get("signature"),
        "object_candidates": [],
    }
    payload: dict[str, Any] = {
        "program_name": row.get("program_name"),
        "program_path": row.get("program_path"),
        "program_slug": row.get("program_slug"),
        "folder": row.get("folder"),
        "entry": row.get("entry"),
        "entry_hex": row.get("entry_hex"),
        "name": stable_function_name(str(row["entry_hex"])),
        "signature": function_meta.get("signature") or row.get("signature"),
        "namespace": row.get("namespace"),
        "comment": row.get("comment"),
        "repeatable_comment": row.get("repeatable_comment"),
        "name_source": "CANDIDATE_STUB",
        "source_hint": row.get("source_hint"),
        "source_override": row.get("source_hint"),
        "source_mapping": source_mapping,
        "source_mapping_ready": True,
        "expected_baseline": baseline_info,
        "expected_baseline_ready": baseline_info is not None,
        "inventory_db": relative_to_root(inventory_db),
        "workspace_dir": relative_to_root(workspace_dir),
        "ghidra_decomp_artifacts_dir": bundle_payload.get("artifacts_dir"),
        "ghidra_decomp_bundle_json": bundle_payload["files"]["json"],
        "ghidra_decomp_bundle_exists": True,
        "commands": {
            "ghidra_decomp": workspace_store.ghidra_decomp_command(
                row,
                ROOT / str(bundle_payload["artifacts_dir"]),
                row.get("source_hint"),
            )
        },
    }
    source_mapping["object_candidates"] = resolve_object_candidates(
        payload,
        build_root=build_root,
    )
    return workspace_json, payload


__all__ = [
    "DEFAULT_CANDIDATE_BUILD_ROOT",
    "DEFAULT_CANDIDATE_WORKSPACE_ROOT",
    "STUB_BUILD_CACHE_VARS",
    "STUB_BUILD_GENERATOR",
    "STUB_BUILD_TOOLCHAIN_FILE",
    "build_candidate_workspace_payload",
    "build_stub_configure_command",
    "candidate_include_directive",
    "candidate_stub_source_path",
    "compile_commands_has_source",
    "resolve_object_candidates",
    "stable_function_name",
    "wrap_candidate_source_text",
]
