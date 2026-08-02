"""Compiler configuration helpers for the BOF3 build system."""

from __future__ import annotations

import re
from pathlib import Path

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
