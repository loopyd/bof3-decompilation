"""Reject artificial allocator and scheduling controls in authored lift inputs."""

from __future__ import annotations

import ast
import re
from pathlib import Path

from harness.common.lexicon import iter_c_lexemes

from .includes import local_include_files

_AID = re.compile(r"\b(?:REGISTER_PIN|CLOBBER_\w*|barrier)\b")
_ASM = re.compile(r"\b(?:asm|__asm|__asm__)\b")
_CONSTRAINTS = re.compile(
    r"\s*(?:(?:volatile|__volatile|__volatile__|inline|goto)\s*)*\([^)]*:"
)
_ASSEMBLY = re.compile(
    r"\s*(?:(?:volatile|__volatile|__volatile__|inline|goto)\s*)*"
    r'\(\s*((?:"[^"\\]*(?:\\.[^"\\]*)*"\s*)+)'
)
_REGISTER = re.compile(r"\$?[A-Za-z_0-9]+")


def validate_matching_text(text: str) -> None:
    """Reject forbidden aids, including aliases, without reading comment text."""
    text = re.sub(r"\\\r?\n", "", text)
    masked = list(text)
    assembly = list(text)
    for lexeme in iter_c_lexemes(text):
        blank = ["\n" if char == "\n" else " " for char in lexeme.group()]
        masked[lexeme.start() : lexeme.end()] = blank
        if lexeme.group().startswith(("/*", "//")):
            assembly[lexeme.start() : lexeme.end()] = blank
    code = "".join(masked)
    literal_code = "".join(assembly)
    aid = _AID.search(code)
    if aid is not None:
        raise ValueError(f"forbidden matching aid: {aid.group()}")
    for invocation in _ASM.finditer(code):
        if _CONSTRAINTS.match(code, invocation.end()):
            raise ValueError("forbidden asm register constraints or clobbers")
        operand = _ASSEMBLY.match(literal_code, invocation.end())
        try:
            template = ast.literal_eval(f"({operand[1]})") if operand else ""
        except (SyntaxError, ValueError):
            template = ""
        if not template.strip("\x00 \t\n\r\v\f") or _REGISTER.fullmatch(template):
            raise ValueError("forbidden register binding or artificial asm barrier")


def validate_matching_source(
    root: Path, source: Path, *, checked: set[Path] | None = None
) -> None:
    """Check the source and local header/template closure before native/cache use."""
    root = root.resolve()
    source = source.resolve()
    paths = [
        source,
        *local_include_files(
            root,
            [source],
            include_roots=(
                root / "src",
                root / "include",
                root / "toolchains/psyq/4.7/include",
            ),
            include_angle=True,
        ),
    ]
    for path in paths:
        if path.is_relative_to(root / "toolchains"):
            continue
        if checked is not None and path in checked:
            continue
        try:
            validate_matching_text(path.read_text(encoding="utf-8"))
        except (OSError, UnicodeError, ValueError) as error:
            raise ValueError(f"{path}: {error}") from error
        if checked is not None:
            checked.add(path)
