"""Public C declaration context for standalone tooling payloads."""

from __future__ import annotations

import re

_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
_SPACE_BEFORE_PUNCTUATION = re.compile(r"\s+([(),;*])")
_SPACE_AFTER_PUNCTUATION = re.compile(r"([(),;])\s+")
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
            if statement.startswith(("typedef ", "extern ")) or re.search(
                r"\b[A-Za-z_][A-Za-z0-9_]*\s*\([^;{}]*\)\s*;$", statement
            ):
                rows.append(statement)
            pending = ""
    return rows


def _names(statement: str) -> tuple[str, ...]:
    """Return every name introduced by one collected declaration."""

    body = statement.rstrip("; \n\t")
    if statement.startswith("typedef "):
        function_pointers = re.findall(r"\(\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)", body)
        if function_pointers:
            return tuple(dict.fromkeys(function_pointers))
    for function in re.finditer(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(", body):
        prefix = body[: function.start()].rstrip()
        if not re.search(r"\(\s*\*\s*$", prefix):
            return (function.group(1),)

    declarators = body[body.rfind("}") + 1 :] if "}" in body else body
    names = []
    for declarator in declarators.split(","):
        identifiers = _IDENTIFIER.findall(declarator)
        if identifiers:
            names.append(identifiers[-1])
    return tuple(dict.fromkeys(names[-1:] if "," not in declarators else names))


def _canonical(statement: str) -> str:
    compact = " ".join(statement.split())
    compact = _SPACE_BEFORE_PUNCTUATION.sub(r"\1", compact)
    return _SPACE_AFTER_PUNCTUATION.sub(r"\1", compact)


def public_declaration_context(preprocessed: str, source: str, *, base: str) -> str:
    """Return source-referenced public declarations and their type dependencies.

    ``preprocessed`` is compiler output, so declarations may originate in any
    project header. The caller remains responsible for rejecting private-header
    identifiers before this lexical, dependency-closed selection is exported.
    """

    base_names = {name for statement in _statements(base) for name in _names(statement)}
    declarations: dict[str, str] = {}
    ordered_statements: list[str] = []
    for statement in _statements(preprocessed):
        names = tuple(name for name in _names(statement) if name not in base_names)
        if not names:
            continue
        for name in names:
            previous = declarations.get(name)
            if previous is not None and _canonical(previous) != _canonical(statement):
                raise ValueError(
                    f"conflicting declarations for {name}: {previous} != {statement}"
                )
        if statement not in ordered_statements:
            ordered_statements.append(statement)
        for name in names:
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

    selected_statements = {
        declarations[name] for name in selected if name in declarations
    }
    return (
        base
        + "\n".join(
            statement
            for statement in ordered_statements
            if statement in selected_statements
        )
        + "\n"
    )
