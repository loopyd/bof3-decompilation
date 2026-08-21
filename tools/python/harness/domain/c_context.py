"""Lexical C declaration records and dependency-closed standalone context.

This is deliberately not a C compiler.  It recognizes the declaration forms
used by tracked BOF3 headers, preserves unsupported declarations as diagnostics,
and never infers layout from spelling alone.
"""

from __future__ import annotations

import re
from dataclasses import dataclass

_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
_SPACE_BEFORE_PUNCTUATION = re.compile(r"\s+([(),;])")
_SPACE_AFTER_PUNCTUATION = re.compile(r"([(),;])\s+")
_C_KEYWORDS = frozenset(
    "auto break case char const continue default do double else enum extern float for "
    "goto if int long register return short signed sizeof static struct switch "
    "typedef union unsigned void volatile while".split()
)
_AGGREGATE = re.compile(r"^(?:typedef\s+)?(struct|union|enum)\s*([A-Za-z_]\w*)?")
_ARRAY = re.compile(r"\[\s*([^]]*)\s*\]\s*$")
_FUNCTION_POINTER = re.compile(r"\(\s*\*\s*([A-Za-z_]\w*)\s*\)")
_COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)


@dataclass(frozen=True)
class CField:
    """One lexical aggregate member; offset/width remain analysis facts."""

    name: str
    type_name: str
    array_extent: str | None = None
    qualifiers: str = ""


@dataclass(frozen=True)
class CDeclaration:
    """One normalized public declaration from an owned header."""

    names: tuple[str, ...]
    kind: str
    canonical: str
    tag_name: str | None = None
    fields: tuple[CField, ...] = ()
    diagnostic: str | None = None


def declaration_statements(text: str) -> list[str]:
    """Collect public declarations without splitting aggregate bodies."""

    rows: list[str] = []
    pending = ""
    depth = 0
    for line in text.splitlines():
        stripped = line.lstrip()
        if not pending and stripped.startswith("#"):
            continue
        if not pending and (not stripped or stripped.startswith(("/*", "*", "//"))):
            continue
        pending += line + "\n"
        for character in line:
            if character == "{":
                depth += 1
            elif character == "}" and depth:
                depth -= 1
        if depth == 0 and ";" in line:
            statement = " ".join(pending.split())
            if statement.startswith(
                ("typedef ", "extern ", "struct ", "union ", "enum ")
            ) or re.search(r"\b[A-Za-z_]\w*\s*\([^;{}]*\)\s*;$", statement):
                rows.append(statement)
            pending = ""
    return rows


def declaration_names(statement: str) -> tuple[str, ...]:
    """Return every name introduced by one collected declaration."""

    body = _COMMENT.sub("", statement).rstrip("; \n\t")
    if statement.startswith("typedef "):
        pointers = _FUNCTION_POINTER.findall(body)
        if pointers:
            return tuple(dict.fromkeys(pointers))
    if re.fullmatch(r"(?:struct|union|enum)\s+[A-Za-z_]\w*\s*;", statement):
        return (statement.split()[1].rstrip(";"),)
    for function in re.finditer(r"\b([A-Za-z_]\w*)\s*\(", body):
        prefix = body[: function.start()].rstrip()
        if not re.search(r"\(\s*\*\s*$", prefix):
            return (function.group(1),)
    declarators = body[body.rfind("}") + 1 :] if "}" in body else body
    names = []
    for declarator in _split_top_level(declarators, ","):
        identifiers = _IDENTIFIER.findall(declarator)
        if identifiers:
            names.append(identifiers[-1])
    return tuple(dict.fromkeys(names[-1:] if "," not in declarators else names))


def canonical_declaration(statement: str) -> str:
    compact = " ".join(statement.split())
    compact = _SPACE_BEFORE_PUNCTUATION.sub(r"\1", compact)
    return _SPACE_AFTER_PUNCTUATION.sub(r"\1", compact)


def _split_top_level(text: str, delimiter: str) -> list[str]:
    rows, start, braces, parens, brackets = [], 0, 0, 0, 0
    for index, character in enumerate(text):
        braces += (character == "{") - (character == "}")
        parens += (character == "(") - (character == ")")
        brackets += (character == "[") - (character == "]")
        if character == delimiter and braces == parens == brackets == 0:
            rows.append(text[start:index])
            start = index + 1
    rows.append(text[start:])
    return rows


def _field_records(statement: str, kind: str) -> tuple[CField, ...]:
    if "{" not in statement or "}" not in statement:
        return ()
    body = statement[statement.find("{") + 1 : statement.rfind("}")]
    if kind == "enum":
        return tuple(
            CField(name=match.group(1), type_name="enumerator")
            for item in _split_top_level(body, ",")
            if (match := re.match(r"\s*([A-Za-z_]\w*)", item))
        )
    fields: list[CField] = []
    for item in _split_top_level(body, ";"):
        item = item.strip()
        if not item:
            continue
        nested_tail = item[item.rfind("}") + 1 :].strip() if "}" in item else item
        pointer = _FUNCTION_POINTER.search(nested_tail)
        if pointer:
            fields.append(CField(pointer.group(1), "function_pointer"))
            continue
        match = re.search(r"([A-Za-z_]\w*)\s*(\[[^]]*\])?\s*$", nested_tail)
        if match is None:
            continue
        name, array = match.group(1), match.group(2)
        prefix = nested_tail[: match.start(1)].strip()
        qualifiers = " ".join(
            word for word in ("const", "volatile") if re.search(rf"\b{word}\b", prefix)
        )
        type_name = re.sub(r"\b(?:const|volatile)\b", "", prefix).strip() or "unknown"
        extent = None
        if array:
            extent_match = _ARRAY.search(array)
            extent = extent_match.group(1).strip() if extent_match else None
        fields.append(CField(name, " ".join(type_name.split()), extent, qualifiers))
    return tuple(fields)


def declaration_records(text: str) -> tuple[CDeclaration, ...]:
    """Return deterministic declaration records for tracked C header forms."""

    records: list[CDeclaration] = []
    for statement in declaration_statements(text):
        canonical = canonical_declaration(statement)
        names = declaration_names(statement)
        aggregate = _AGGREGATE.match(statement)
        tag = aggregate.group(2) if aggregate else None
        if aggregate:
            kind = aggregate.group(1)
        elif statement.startswith("typedef "):
            kind = "typedef"
        elif statement.startswith("extern "):
            kind = "extern"
        else:
            kind = "prototype"
        diagnostic = None if names else "unsupported declaration name"
        records.append(
            CDeclaration(
                names=names,
                kind=kind,
                canonical=canonical,
                tag_name=tag,
                fields=_field_records(statement, kind),
                diagnostic=diagnostic,
            )
        )
    return tuple(records)


def scalar_declaration_context(text: str) -> str:
    """Return scalar typedef declarations from the tracked base type header."""

    names = {"bool", "s8", "s16", "s32", "s64", "u8", "u16", "u32", "u64", "f32", "f64"}
    return (
        "\n".join(
            statement
            for statement in declaration_statements(text)
            if set(declaration_names(statement)) & names
        )
        + "\n"
    )


def public_declaration_context(preprocessed: str, source: str, *, base: str) -> str:
    """Return source-referenced public declarations and type dependencies."""

    base_names = {
        name for declaration in declaration_records(base) for name in declaration.names
    }
    declarations: dict[str, str] = {}
    ordered_statements: list[str] = []
    for statement in declaration_statements(preprocessed):
        names = tuple(
            name for name in declaration_names(statement) if name not in base_names
        )
        if not names:
            continue
        for name in names:
            previous = declarations.get(name)
            if previous is not None and canonical_declaration(
                previous
            ) != canonical_declaration(statement):
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


__all__ = [
    "CDeclaration",
    "CField",
    "canonical_declaration",
    "declaration_names",
    "declaration_records",
    "declaration_statements",
    "public_declaration_context",
    "scalar_declaration_context",
]
