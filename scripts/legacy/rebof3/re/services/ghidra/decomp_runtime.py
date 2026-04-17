from __future__ import annotations

from pathlib import Path
from typing import Any

from ....common import (
    ROOT,
    default_artifacts_dir,
    format_hex as format_address,
    parse_hexish as parse_address,
    parse_source_spec,
    relative_to_root,
    write_json_output,
    write_text_output,
)
from ....config import DEFAULT_GHIDRA_DECOMP_ROOT
from ....logger import Rebof3Logger
from ....lib import options_with_logger
from ....pipelines.pipeline_decomp import pipeline_decomp
from ....program_identity import infer_source_hint
from ..m2c_runner import build_m2c_command
from ..m2c_context import build_m2c_context_preprocess_command
from .bundle_export import (
    SHARED_PROJECT_NAME,
    build_bundle_export_commands,
    resolve_project_name,
    resolve_project_path,
)
from .decomp_helpers import (
    bundle_artifact_paths,
    bundle_function_metadata,
    default_program_name,
    infer_source_base_addr,
    load_program_symbol_resolver,
)

DEFAULT_ARTIFACT_ROOT = DEFAULT_GHIDRA_DECOMP_ROOT
DEFAULT_PROJECT_NAME = SHARED_PROJECT_NAME


def _resolve_source_text(source_text: str) -> str:
    source_spec = parse_source_spec(source_text)
    source_path = source_spec.path
    if source_path.is_absolute() and not source_path.exists():
        source_hint = infer_source_hint(
            str(source_path),
            str(source_path.parent).replace("//", "/"),
            source_path.name,
        )
        if source_hint:
            suffix = (
                "" if source_spec.entry_index is None else f"#{source_spec.entry_index}"
            )
            return f"{source_hint}{suffix}"
    return source_text


def run_decomp_bundle(
    *,
    source_text: str,
    address_text: str,
    project_dir: Path | None = None,
    project_name: str = DEFAULT_PROJECT_NAME,
    program_name: str | None = None,
    artifacts_dir: Path | None = None,
    base_addr: int | None = None,
    loader_mode: str = "auto",
    asm_backend: str = "ghidra",
    emit_spimdisasm: bool = True,
    no_m2c: bool = False,
    noanalysis: bool = False,
    dry_run: bool = False,
    logger: Rebof3Logger | None = None,
) -> tuple[int, dict[str, Any] | None]:
    source_text = _resolve_source_text(source_text)
    source_spec = parse_source_spec(source_text)
    source_path, entry_index = source_spec.path, source_spec.entry_index
    if not source_path.is_absolute():
        source_path = (ROOT / source_path).resolve()
    requested_address = parse_address(address_text)
    inferred_base_addr = infer_source_base_addr(source_path, entry_index)
    resolved_base_addr = base_addr if base_addr is not None else inferred_base_addr
    resolved_artifacts_dir = artifacts_dir or default_artifacts_dir(
        DEFAULT_ARTIFACT_ROOT, source_path, requested_address, entry_index
    )
    resolved_project_dir = resolve_project_path(project_dir)
    resolved_project_name = resolve_project_name(project_name, project_dir)
    resolved_program_name = program_name or default_program_name(
        source_text, resolved_base_addr
    )

    resolved_artifacts_dir.mkdir(parents=True, exist_ok=True)
    resolved_project_dir.mkdir(parents=True, exist_ok=True)

    artifact_paths = bundle_artifact_paths(resolved_artifacts_dir)
    bundle_json_path = artifact_paths["json"]
    ghidra_c_path = artifact_paths["ghidra_c"]
    ghidra_asm_path = artifact_paths["ghidra_asm"]
    spim_asm_path = artifact_paths["spim_asm"]
    asm_path = artifact_paths["asm"]
    m2c_context_source_path = artifact_paths["m2c_context_source"]
    m2c_context_path = artifact_paths["m2c_context"]
    m2c_asm_path = artifact_paths["m2c_asm"]
    m2c_path = artifact_paths["m2c_c"]
    exported_json_path = resolved_artifacts_dir / ".func.export.json"

    commands = build_bundle_export_commands(
        source_text=source_text,
        project_dir=resolved_project_dir,
        project_name=resolved_project_name,
        program_name=resolved_program_name,
        requested_address=requested_address,
        exported_json_path=exported_json_path,
        asm_path=ghidra_asm_path,
        loader_mode=loader_mode,
        base_addr=resolved_base_addr,
        noanalysis=noanalysis,
    )
    if not no_m2c:
        commands.append(
            build_m2c_context_preprocess_command(
                source_path=m2c_context_source_path,
                output_path=m2c_context_path,
            )
        )
        commands.append(
            build_m2c_command(m2c_asm_path, context_paths=[m2c_context_path])
        )

    if dry_run:
        return 0, {
            "commands": commands,
            "artifacts_dir": relative_to_root(resolved_artifacts_dir),
            "project_dir": relative_to_root(resolved_project_dir),
            "asm_backend": asm_backend,
            "emit_spimdisasm": emit_spimdisasm,
        }

    pipeline_options: dict[str, Any] = {
        "asm_backend": asm_backend,
        "emit_spimdisasm": emit_spimdisasm,
    }
    if logger is not None:
        active_options = options_with_logger(pipeline_options, logger)
    else:
        active_options = pipeline_options

    context = pipeline_decomp(include_m2c=not no_m2c).run(
        {
            "source_text": source_text,
            "project_dir": resolved_project_dir,
            "project_name": resolved_project_name,
            "program_name": resolved_program_name,
            "requested_address": requested_address,
            "exported_json_path": exported_json_path,
            "ghidra_asm_path": ghidra_asm_path,
            "spim_asm_path": spim_asm_path,
            "asm_path": asm_path,
            "loader_mode": loader_mode,
            "base_addr": resolved_base_addr,
            "noanalysis": noanalysis,
            "ghidra_c_path": ghidra_c_path,
            "m2c_context_source_path": m2c_context_source_path,
            "m2c_context_preprocessed_path": m2c_context_path,
            "m2c_asm_path": m2c_asm_path,
            "m2c_c_path": m2c_path,
            "no_m2c": no_m2c,
        },
        options=active_options,
    )
    returncode = int(context.get("returncode") or 0)
    export_payload = context.get("export_payload")
    if returncode != 0 or export_payload is None:
        return returncode, None

    function_payload = export_payload["function_payload"]
    ghidra_c = context.get("ghidra_c") or export_payload["ghidra_c"]

    m2c_metadata: dict[str, Any] = {
        "attempted": not no_m2c,
        "path": None,
        "status": "skipped" if no_m2c else "pending",
        "stderr": None,
    }
    if not no_m2c:
        m2c_metadata = context["m2c_metadata"]
    spimdisasm_metadata = context.get("spimdisasm_metadata") or {
        "attempted": False,
        "status": "skipped",
        "path": None,
        "stderr": None,
    }
    m2c_context_metadata = context.get("m2c_context_metadata") or {
        "attempted": False,
        "status": "skipped",
        "path": None,
        "stderr": None,
    }

    bundle_payload: dict[str, Any] = {
        "input": source_text,
        "source_path": relative_to_root(source_path),
        "entry_index": entry_index,
        "requested_address": format_address(requested_address),
        "load_address": format_address(resolved_base_addr)
        if resolved_base_addr is not None
        else None,
        "project_dir": relative_to_root(resolved_project_dir),
        "project_name": resolved_project_name,
        "program_name": resolved_program_name,
        "artifacts_dir": relative_to_root(resolved_artifacts_dir),
        "asm_backend": context.get("selected_asm_backend", asm_backend),
        "files": {
            "json": relative_to_root(bundle_json_path),
            "ghidra_c": relative_to_root(ghidra_c_path) if ghidra_c else None,
            "ghidra_asm": relative_to_root(ghidra_asm_path),
            "spim_asm": (
                relative_to_root(spim_asm_path) if spim_asm_path.exists() else None
            ),
            "asm": relative_to_root(asm_path),
            "m2c_context_source": (
                relative_to_root(m2c_context_source_path)
                if m2c_context_source_path.exists()
                else None
            ),
            "m2c_context": (
                relative_to_root(m2c_context_path)
                if m2c_context_path.exists()
                else None
            ),
            "m2c_asm": relative_to_root(m2c_asm_path) if not no_m2c else None,
            "m2c_c": m2c_metadata["path"],
        },
        "function": bundle_function_metadata(function_payload),
        "spimdisasm": spimdisasm_metadata,
        "m2c_context": m2c_context_metadata,
        "m2c": m2c_metadata,
    }

    write_json_output(bundle_json_path, bundle_payload)
    return 0, bundle_payload


__all__ = [
    "DEFAULT_ARTIFACT_ROOT",
    "DEFAULT_PROJECT_NAME",
    "ROOT",
    "default_artifacts_dir",
    "run_decomp_bundle",
]
