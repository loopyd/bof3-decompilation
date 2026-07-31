"""Compiler variant configuration helpers for the BOF3 build system."""

from __future__ import annotations

import os
import re
from pathlib import Path

from .io import RepoLayout, repo_layout
from .toolchain.gcc_variants import (
    CompilerVariant,
    EmptyCatalog,
    lookup_variant,
)

# ── shared source-key and object-parsing helpers ──────────────────────

OBJECT_FLAGS_RE = re.compile(
    r"^\s*set\(\s*BOF3_OBJFLAGS_(\S+)\s+(.*?)\)\s*$"
)
OBJCOMPILER_RE = re.compile(
    r"^\s*set\(\s*BOF3_OBJCOMPILER_(\S+)\s+(\S+)\s*\)\s*$"
)


def sanitize_identifier(relative: str) -> str:
    """Mirror CMake's string(MAKE_C_IDENTIFIER ...)."""
    return re.sub(r"[^A-Za-z0-9]", "_", relative)


def load_object_flags(root: Path) -> dict[str, list[str]]:
    """Parse config/compiler/object-flags.cmake -> {sanitized_key: flags}.

    Mirrors the include() in CMakeLists.txt so the compile database
    matches the actual per-object build flags.
    """
    path = root / "config" / "compiler" / "object-flags.cmake"
    overrides: dict[str, list[str]] = {}
    if not path.is_file():
        return overrides
    for line in path.read_text(encoding="utf-8").splitlines():
        m = OBJECT_FLAGS_RE.match(line)
        if m is not None:
            overrides[m.group(1)] = m.group(2).split()
    return overrides


def load_object_compilers(root: Path) -> dict[str, str]:
    """Parse BOF3_OBJCOMPILER_<key> <catalog-id> entries.

    Returns {sanitized_key: catalog_id}. Validates that compiler IDs
    are non-empty and contain only safe characters.
    Raises ValueError on duplicate key or malformed ID.
    """
    path = root / "config" / "compiler" / "object-flags.cmake"
    compilers: dict[str, str] = {}
    if not path.is_file():
        return compilers
    for line in path.read_text(encoding="utf-8").splitlines():
        m = OBJCOMPILER_RE.match(line)
        if m is None:
            # Raise on any active BOF3_OBJCOMPILER_ assignment that is malformed,
            # so compile_commands.py parity with CMake is explicit.
            if "BOF3_OBJCOMPILER_" in line and "#" not in line.split("BOF3_OBJCOMPILER_")[0]:
                stripped = line.strip()
                if stripped.startswith("set(BOF3_OBJCOMPILER_"):
                    raise ValueError(
                        f"malformed BOF3_OBJCOMPILER_ assignment: {stripped!r}"
                    )
            continue
        key, cid = m.group(1), m.group(2)
        if not re.match(r"^[A-Za-z0-9._-]+$", cid):
            raise ValueError(f"malformed compiler ID {cid!r} for key {key}")
        if key in compilers:
            raise ValueError(f"duplicate BOF3_OBJCOMPILER key: {key}")
        compilers[key] = cid
    return compilers


# ── variant resolution and environment ────────────────────────────────

def resolve_compiler_variant(
    layout: RepoLayout | None = None,
    compiler_id: str | None = None,
) -> CompilerVariant:
    """Resolve a specific compiler variant from the catalog.

    When compiler_id is given, looks up exactly that ID.
    When None, returns EmptyCatalog (no active variant).
    Raises ValueError on schema violation, missing ID, or validation failure.
    """
    if layout is None:
        layout = repo_layout()

    if compiler_id is None:
        return EmptyCatalog()

    return lookup_variant(layout, compiler_id)


def set_environment_for_variant(
    layout: RepoLayout,
    variant: CompilerVariant | None = None,
    compiler_id: str | None = None,
) -> dict[str, str]:
    """Return environment overrides for a resolved variant.

    When the catalog is empty (EmptyCatalog), returns empty env dict
    to preserve canonical toolchain behavior with no modifications.
    """
    if variant is None:
        variant = resolve_compiler_variant(layout, compiler_id=compiler_id)

    if isinstance(variant, EmptyCatalog):
        return {}

    env: dict[str, str] = {}
    install_dir = variant.install_path(layout)
    env["BOF3_OBJCOMPILER"] = str(install_dir)
    env["BOF3_OBJCOMPILER_LABEL"] = variant.label
    return env


def get_objcompiler_env() -> dict[str, str]:
    """Read BOF3_OBJCOMPILER_* environment variables for the current process.

    Returns empty dict when no variant is selected.
    """
    env: dict[str, str] = {}
    for key in ("BOF3_OBJCOMPILER", "BOF3_OBJCOMPILER_LABEL"):
        value = os.environ.get(key)
        if value is not None and value:
            env[key.lower()] = value
    return env
