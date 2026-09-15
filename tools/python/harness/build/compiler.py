"""Compiler configuration helpers for the BOF3 build system."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from harness.common.files import read_file
from harness.common.deadlines import check_deadline
from harness.common.lexicon import iter_c_lexemes
from harness.domain.functions import parse_function_records
from harness.toolchain.gcc import OLD_GCC_IDENTITY

# ── shared source-key and object-parsing helpers ──────────────────────

OBJECT_FLAGS_RE = re.compile(r"^\s*set\(\s*BOF3_OBJFLAGS_(\S+)\s+(.*?)\)\s*$")
OBJCOMPILER_RE = re.compile(r"^\s*set\(\s*BOF3_OBJCOMPILER_(\S+)\s+(\S+)\s*\)\s*$")
COMPILER_TAG_RE = re.compile(r"@(compiler|gcc|cflags|flags)\b")
DEFAULT_COMPILER_ID = f"gcc-{OLD_GCC_IDENTITY}-psx"
CONFIGURATION_PATH = "config/compiler/object-flags.cmake"
_INVOCATION_FLAGS = {
    "-c",
    "-E",
    "-S",
    "-fsyntax-only",
    "-fhelp",
    "-specs",
    "--help",
    "--version",
    "--syntax-only",
    "-dumpspecs",
    "-dumpversion",
    "-dumpfullversion",
    "-dumpmachine",
}
_INVOCATION_PREFIXES = ("-o", "-M", "-print-", "-B", "-b", "-V", "-specs=")
_INVOCATION_ALIASES = (
    "--assemble",
    "--compile",
    "--dependencies",
    "--output",
    "--prefix",
    "--preprocess",
    "--print-search-dirs",
    "--print-file-name",
    "--print-libgcc-file-name",
    "--print-missing-file-dependencies",
    "--print-multi-lib",
    "--print-multi-directory",
    "--print-prog-name",
    "--specs",
    "--target",
    "--use-version",
    "--user-dependencies",
    "--version",
    "--write-dependencies",
    "--write-user-dependencies",
)
_FORWARDING_ALIASES = ("--for-assembler", "--for-linker", "--for-preprocessor")


def _is_invocation_flag(flag: str) -> bool:
    pending = [flag]
    while pending:
        current = pending.pop()
        option, separator, value = current.partition("=")
        if option in _INVOCATION_FLAGS or current.startswith(_INVOCATION_PREFIXES):
            return True
        if current.startswith("-x") and current not in {"-xc", "-xnone"}:
            return True
        if current.startswith("-d") and "M" in current[2:]:
            return True
        if current.startswith("--"):
            if "--language".startswith(option) and (
                not separator or value not in {"c", "none"}
            ):
                return True
            if current.startswith(("--output", "--print-")) or any(
                alias.startswith(option) for alias in _INVOCATION_ALIASES
            ):
                return True
            if separator and option in _FORWARDING_ALIASES:
                pending.extend(value.split(","))
        if current.startswith(("-Wa,", "-Wp,", "-Wl,")):
            pending.extend(current[4:].split(","))
    return False


def validate_compiler_flags(flags: tuple[str, ...] | None) -> None:
    """Keep translation-unit profiles separate from operation and output control."""
    if any(_is_invocation_flag(flag) for flag in flags or ()):
        raise ValueError("compiler profile cannot change the build operation or output")


def validate_object_configuration(text: str) -> None:
    normalized = text.replace("\r\n", "\n")
    if any(
        character not in "\t\n" and not " " <= character <= "~"
        for character in normalized
    ):
        raise ValueError("unsupported object compiler configuration characters")
    seen = set()
    for line in normalized.split("\n"):
        line = line.strip()
        if re.match(r"#\[=*\[", line):
            raise ValueError("unsupported object compiler bracket comment")
        if not line or line.startswith("#"):
            continue
        if "#" in line:
            raise ValueError("unsupported trailing object compiler comment")
        match = OBJECT_FLAGS_RE.fullmatch(line)
        kind = "flags"
        if match is None:
            match = OBJCOMPILER_RE.fullmatch(line)
            kind = "compiler"
        if match is None:
            raise ValueError("unsupported object compiler configuration statement")
        key, value = match.groups()
        identity = (kind, key)
        if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", key) or identity in seen:
            raise ValueError("ambiguous or duplicate object compiler key")
        seen.add(identity)
        if kind == "flags":
            if not value.split():
                raise ValueError("unsupported value-less object compiler assignment")
            if any(
                not re.fullmatch(r"-[A-Za-z0-9_=+.,:/-]+", flag)
                for flag in value.split()
            ):
                raise ValueError("unsupported object compiler flag syntax")


def parse_compiler_configuration(
    text: str,
) -> tuple[dict[str, list[str]], dict[str, str]]:
    """Decode bounded literal settings without evaluating CMake statements."""
    check_deadline()
    if not isinstance(text, str) or len(text.encode("utf-8")) > 4 * 1024 * 1024:
        raise ValueError("object compiler configuration exceeds the text bound")
    validate_object_configuration(text)
    result = parse_object_flags(text), parse_object_compilers(text)
    check_deadline()
    return result


def load_compiler_configuration(
    root: Path,
) -> tuple[dict[str, list[str]], dict[str, str]]:
    """Read one bounded, literal configuration for settings-producing callers."""
    content = read_file(
        root, CONFIGURATION_PATH, missing_ok=True, max_bytes=4 * 1024 * 1024
    )
    return parse_compiler_configuration(
        content.decode("utf-8") if content is not None else ""
    )


@dataclass(frozen=True)
class CompilerSettings:
    """One effective compiler selection and ordered translation-unit override."""

    compiler_id: str | None
    flags: tuple[str, ...] | None


def has_compiler_annotations(text: str) -> bool:
    """Recognize comment annotations without treating strings as metadata."""
    return any(
        lexeme.group().startswith(("/*", "//"))
        and COMPILER_TAG_RE.search(lexeme.group())
        for lexeme in iter_c_lexemes(text)
    )


def _parse_compiler_annotations(text: str) -> dict:
    result = {}
    for line in text.removeprefix("/*").removesuffix("*/").splitlines():
        line = line.strip().removeprefix("*").removeprefix("//").strip()
        if not COMPILER_TAG_RE.search(line):
            continue
        match = re.fullmatch(r"@(compiler|gcc|cflags|flags)\s+(.+)", line)
        if match is None:
            raise ValueError("compiler annotations require one nonempty tag per line")
        tag, value = match.groups()
        key = "compiler_id" if tag in {"compiler", "gcc"} else "flags"
        if key in result:
            raise ValueError("duplicate compiler or flag annotation")
        if tag == "gcc":
            if not re.fullmatch(r"[0-9]+\.[0-9]+(?:\.[0-9]+)?", value):
                raise ValueError(
                    "@gcc requires a version with a matching PSX catalog ID"
                )
            value = f"gcc-{value}-psx"
        if key == "compiler_id":
            if not re.fullmatch(r"[A-Za-z0-9._-]+", value):
                raise ValueError("malformed compiler annotation ID")
            result[key] = None if value == DEFAULT_COMPILER_ID else value
        else:
            flags = tuple(value.split())
            if any(not re.fullmatch(r"-[A-Za-z0-9_=+.,:/-]+", flag) for flag in flags):
                raise ValueError("unsupported compiler annotation flag syntax")
            result[key] = flags
    return result


def resolve_compiler_settings(
    source: str,
    text: str,
    object_flags: dict[str, list[str]],
    object_compilers: dict[str, str],
) -> CompilerSettings:
    """Resolve attached metadata, then path overrides, then project defaults."""
    key = sanitize_identifier(Path(source).relative_to("src").as_posix())
    compiler_id = object_compilers.get(key)
    defaults = CompilerSettings(
        None if compiler_id == DEFAULT_COMPILER_ID else compiler_id,
        tuple(object_flags[key]) if key in object_flags else None,
    )
    if not has_compiler_annotations(text):
        validate_compiler_flags(defaults.flags)
        return defaults
    records = parse_function_records(text, validate_progress=False)
    if not records:
        raise ValueError("compiler annotations require attached function metadata")
    metadata_spans = {
        (record.metadata_start, record.metadata_end) for record in records
    }
    for lexeme in iter_c_lexemes(text):
        if lexeme.group().startswith(("/*", "//")) and COMPILER_TAG_RE.search(
            lexeme.group()
        ):
            if (lexeme.start(), lexeme.end()) not in metadata_spans:
                raise ValueError(
                    "compiler annotation is outside attached function metadata"
                )
    settings = []
    for record in records:
        annotations = _parse_compiler_annotations(record.metadata)
        settings.append(
            CompilerSettings(
                annotations.get("compiler_id", defaults.compiler_id),
                annotations.get("flags", defaults.flags),
            )
        )
    if any(setting != settings[0] for setting in settings[1:]):
        raise ValueError(
            "function compiler profiles are incompatible within one translation unit"
        )
    validate_compiler_flags(settings[0].flags)
    return settings[0]


def sanitize_identifier(relative: str) -> str:
    """Mirror CMake's string(MAKE_C_IDENTIFIER ...)."""
    return re.sub(r"[^A-Za-z0-9]", "_", relative)


def build_compiler_arguments(
    root: Path, override: tuple[str, ...] | list[str] | None = None
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
    if override is None:
        return common
    base = [flag for flag in common if not flag.startswith(("-Wa", "-O"))]
    return [*base, *override, *(flag for flag in common if flag.startswith("-Wa"))]


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
