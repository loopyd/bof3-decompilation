"""Compiler configuration helpers for the BOF3 build system."""

from __future__ import annotations

import re
from pathlib import Path

from harness.common.lexicon import iter_c_lexemes

# ── shared source-key and object-parsing helpers ──────────────────────

OBJECT_FLAGS_RE = re.compile(r"^\s*set\(\s*BOF3_OBJFLAGS_(\S+)\s+(.*?)\)\s*$")
OBJCOMPILER_RE = re.compile(r"^\s*set\(\s*BOF3_OBJCOMPILER_(\S+)\s+(\S+)\s*\)\s*$")


def sanitize_identifier(relative: str) -> str:
    """Mirror CMake's string(MAKE_C_IDENTIFIER ...)."""
    return re.sub(r"[^A-Za-z0-9]", "_", relative)


def build_compiler_arguments(
    root: Path, source: Path, overrides: dict[str, list[str]]
) -> list[str]:
    """Construct the configured driver arguments shared by tooling readers."""
    common = [
        str(root / "bin" / "cc"),
        "-DHARNESS_TARGET_PSX=1",
        f"-I{root / 'src'}",
        f"-I{root / 'include'}",
        f"-I{root / 'toolchains' / 'psyq' / '4.7' / 'include'}",
        "-O2",
        "-G0",
        "-funsigned-char",
        "-msoft-float",
        "-gcoff",
        "-Wa,--aspsx-version=2.56",
        "-Wa,-G0,-EL,-mips1",
    ]
    key = sanitize_identifier(source.relative_to(root / "src").as_posix())
    override = overrides.get(key)
    if override is None:
        return common
    base = [flag for flag in common if not flag.startswith(("-Wa", "-O"))]
    return [*base, *override, *(flag for flag in common if flag.startswith("-Wa"))]


def validate_compiler_annotations(text: str) -> None:
    """Reject comment-level compiler settings that no producer implements."""
    if any(
        lexeme.group().startswith(("/*", "//"))
        and re.search(r"@(compiler|gcc|cflags|flags)\b", lexeme.group())
        for lexeme in iter_c_lexemes(text)
    ):
        raise ValueError(
            "per-function compiler annotations are not implemented; "
            "use reviewed compatible object profiles or defer consolidation"
        )


def load_object_flags(root: Path) -> dict[str, list[str]]:
    """Parse config/compiler/object-flags.cmake -> {sanitized_key: flags}.

    Mirrors the include() in CMakeLists.txt so the compile database
    matches the actual per-object build flags.
    """
    path = root / "config" / "compiler" / "object-flags.cmake"
    if not path.is_file():
        return {}
    return parse_object_flags(path.read_text(encoding="utf-8"))


def parse_object_flags(text: str) -> dict[str, list[str]]:
    """Parse ordered object flags from supplied text without reading a file."""
    overrides: dict[str, list[str]] = {}
    for line in text.splitlines():
        match = OBJECT_FLAGS_RE.match(line)
        if match is not None:
            overrides[match.group(1)] = match.group(2).split()
    return overrides


def load_object_compilers(root: Path) -> dict[str, str]:
    """Parse BOF3_OBJCOMPILER_<key> <catalog-id> entries.

    Returns {sanitized_key: catalog_id}. Validates that compiler IDs
    are non-empty and contain only safe characters.
    Raises ValueError on duplicate key or malformed ID.
    """
    path = root / "config" / "compiler" / "object-flags.cmake"
    if not path.is_file():
        return {}
    return parse_object_compilers(path.read_text(encoding="utf-8"))


def parse_object_compilers(text: str) -> dict[str, str]:
    """Parse and validate object compiler IDs without reading configuration."""
    compilers: dict[str, str] = {}
    for line in text.splitlines():
        match = OBJCOMPILER_RE.match(line)
        if match is None:
            # Raise on any active BOF3_OBJCOMPILER_ assignment that is malformed,
            # so compile_commands.py parity with CMake is explicit.
            if (
                "BOF3_OBJCOMPILER_" in line
                and "#" not in line.split("BOF3_OBJCOMPILER_")[0]
            ):
                stripped = line.strip()
                if stripped.startswith("set(BOF3_OBJCOMPILER_"):
                    raise ValueError(
                        f"malformed BOF3_OBJCOMPILER_ assignment: {stripped!r}"
                    )
            continue
        key, compiler_id = match.group(1), match.group(2)
        if not re.match(r"^[A-Za-z0-9._-]+$", compiler_id):
            raise ValueError(f"malformed compiler ID {compiler_id!r} for key {key}")
        if key in compilers:
            raise ValueError(f"duplicate BOF3_OBJCOMPILER key: {key}")
        compilers[key] = compiler_id
    return compilers
