"""Public C declaration context for standalone tooling payloads."""

from __future__ import annotations

import re

_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
_C_KEYWORDS = frozenset(
    "auto break case char const continue default do double else enum extern float for "
    "goto if int long register return short signed sizeof static struct switch "
    "typedef union unsigned void volatile while".split()
)


def _statements(text: str) -> list[str]:
    """Collect public typedef/extern declarations without splitting structs."""

    rows: list[str] = []
    pending = ""
    depth = 0
    for line in text.splitlines():
        if not pending and line.lstrip().startswith("#"):
            continue
        if not pending and not line.strip():
            continue
        pending += line + "\n"
        for character in line:
            if character == "{":
                depth += 1
            elif character == "}" and depth:
                depth -= 1
        if depth == 0 and ";" in line:
            statement = " ".join(pending.split())
            if statement.startswith(("typedef ", "extern ")):
                rows.append(statement)
            pending = ""
    return rows


def _name(statement: str) -> str | None:
    identifiers = _IDENTIFIER.findall(statement.rstrip("; \n\t"))
    if not identifiers:
        return None
    if statement.startswith("typedef "):
        return identifiers[-1]
    function = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(", statement)
    if function is not None and not re.search(
        rf"\(\s*\*\s*{re.escape(function.group(1))}\s*\)", statement
    ):
        return function.group(1)
    return identifiers[-1]


def public_declaration_context(preprocessed: str, source: str, *, base: str) -> str:
    """Return source-referenced public declarations and their type dependencies.

    ``preprocessed`` is compiler output, so declarations may originate in any
    project header. The caller remains responsible for rejecting private-header
    identifiers before this lexical, dependency-closed selection is exported.
    """

    declarations: dict[str, str] = {}
    for statement in _statements(preprocessed):
        name = _name(statement)
        if name is not None and name not in declarations:
            declarations[name] = statement

    wanted = set(_IDENTIFIER.findall(source)) - _C_KEYWORDS
    selected: set[str] = set()
    pending = list(wanted)
    while pending:
        name = pending.pop()
        if name in selected or name not in declarations:
            continue
        selected.add(name)
        pending.extend(
            dependency
            for dependency in _IDENTIFIER.findall(declarations[name])
            if dependency not in selected and dependency not in _C_KEYWORDS
        )

    return base + "\n".join(
        statement for name, statement in declarations.items() if name in selected
    ) + "\n"
