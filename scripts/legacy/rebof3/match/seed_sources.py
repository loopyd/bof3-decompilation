from __future__ import annotations

from pathlib import Path

from .. import config

EXPLICIT_VARIANTS = ("repo", "ghidra", "m2c")
VARIANT_CHOICES = EXPLICIT_VARIANTS + ("auto",)


def source_function_name(workspace_payload: dict[str, object]) -> str:
    source_mapping = workspace_payload.get("source_mapping") or {}
    name = source_mapping.get("source_function") or workspace_payload.get("name")
    if not name:
        raise LookupError("workspace is missing a source function name")
    return str(name)


def repo_variant_is_meaningful(workspace_payload: dict[str, object]) -> bool:
    source_mapping = workspace_payload.get("source_mapping") or {}
    source_file = source_mapping.get("source_file")
    if not source_file:
        return False
    path = config.ROOT / str(source_file)
    if not path.exists():
        return False
    try:
        text = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return False
    if not text.strip():
        return False
    return source_function_name(workspace_payload) in text


def ghidra_artifacts_dir(workspace_payload: dict[str, object]) -> Path:
    artifacts_dir = workspace_payload.get("ghidra_decomp_artifacts_dir")
    if not artifacts_dir:
        raise LookupError("workspace is missing ghidra_decomp_artifacts_dir")
    return config.ROOT / str(artifacts_dir)


def resolve_variant_source(
    workspace_payload: dict[str, object], *, variant: str
) -> tuple[Path, str]:
    if variant == "auto":
        if repo_variant_is_meaningful(workspace_payload):
            return resolve_variant_source(workspace_payload, variant="repo")
        base_dir = ghidra_artifacts_dir(workspace_payload)
        m2c_path = base_dir / "func.m2c.c"
        if m2c_path.exists():
            return m2c_path, "m2c"
        ghidra_path = base_dir / "func.ghidra.c"
        if ghidra_path.exists():
            return ghidra_path, "ghidra"
        raise FileNotFoundError(m2c_path)

    if variant == "repo":
        source_mapping = workspace_payload.get("source_mapping") or {}
        source_file = source_mapping.get("source_file")
        if not source_file:
            raise LookupError("workspace is missing source_mapping.source_file")
        return config.ROOT / str(source_file), "repo"

    base_dir = ghidra_artifacts_dir(workspace_payload)
    if variant == "ghidra":
        path = base_dir / "func.ghidra.c"
        if not path.exists():
            raise FileNotFoundError(path)
        return path, "ghidra"
    path = base_dir / "func.m2c.c"
    if not path.exists():
        raise FileNotFoundError(path)
    return path, "m2c"
