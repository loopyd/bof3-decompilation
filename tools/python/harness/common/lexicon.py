"""Ordered C comments and literals for lexical source inspection."""

from __future__ import annotations

import re
from collections.abc import Iterator

_LEXEME = re.compile(
    r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
    re.DOTALL,
)


def iter_c_lexemes(text: str) -> Iterator[re.Match[str]]:
    """Yield comments/literals in source order without interpreting their contents."""

    return _LEXEME.finditer(text)


def mask_c_text(text: str) -> str:
    """Blank C comments and literals while preserving offsets and newlines."""
    parts: list[str] = []
    start = 0
    for match in iter_c_lexemes(text):
        parts.append(text[start : match.start()])
        parts.append(re.sub(r"[^\n]", " ", match.group()))
        start = match.end()
    return "".join(parts) + text[start:]
