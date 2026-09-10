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
