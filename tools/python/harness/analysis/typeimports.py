"""Canonical declaration imports for reviewed Rizin replay recipes."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re

from harness.common.links import read_linked_state
from harness.domain.c_context import (
    collect_declaration_references,
    declaration_records,
    render_declaration_context,
)
from harness.domain.declarations import (
    DeclarationKey,
    build_declaration_occurrences,
    reconcile_declarations,
)
from harness.macros.facts import parse_macro_definitions, parse_macro_uses

_SELECTION = re.compile(r"^# rizin-types: ([A-Za-z_]\w*(?: [A-Za-z_]\w*)*)$")
_INCLUDE = re.compile(r'^#include "(include/[^"\s]+\.h)"$')
_GUARD = re.compile(r"\A\s*#ifndef ([A-Za-z_]\w*)\s*\n#define \1\s*\n")


def _selection(overlay: str) -> tuple[str, ...]:
    lines = [line.strip() for line in overlay.splitlines() if "rizin-types:" in line]
    if not lines:
        return ()
    match = _SELECTION.fullmatch(lines[0])
    if len(lines) != 1 or match is None:
        raise ValueError("reviewed recipe requires one literal rizin-types selection")
    names = tuple(match.group(1).split())
    if len(set(names)) != len(names):
        raise ValueError("duplicate reviewed Rizin type selection")
    return names


def type_headers(overlay: str) -> tuple[str, ...]:
    """Return explicit root-relative header inputs for a selected type context."""
    if not _selection(overlay):
        return ()
    headers = []
    for line in overlay.splitlines():
        if not re.match(r"\s*#\s*include\b", line):
            continue
        match = _INCLUDE.fullmatch(line.strip())
        if (
            match is None
            or "\\" in match.group(1)
            or any(part in {"", ".", ".."} for part in match.group(1).split("/"))
        ):
            raise ValueError(
                "reviewed type headers must be literal canonical include paths"
            )
        headers.append(match.group(1))
    if not headers or len(set(headers)) != len(headers):
        raise ValueError("missing or duplicate reviewed type headers")
    return tuple(headers)


def _rizin_declaration(record) -> str:
    # Rizin has one type namespace. A same-name tag/identity typedef needs
    # only its unchanged named aggregate, rather than a conflicting alias.
    if record.kind in {"struct", "union", "enum"} and record.declarators:
        alias = record.declarators[0]
        if not (
            record.complete
            and record.kind == "struct"
            and record.tag_name
            and len(record.declarators) == 1
            and alias.kind == "typedef"
            and alias.relationship == "identity"
            and alias.name == alias.alias_name == record.tag_name
            and alias.alias_namespace == "tag"
        ):
            raise ValueError("unsupported reviewed Rizin aggregate alias")
        statement = record.statement.removeprefix("typedef ")
        return statement[: statement.rfind("}") + 1] + ";"
    return record.statement


def type_replay(root: Path, overlay: str, *, files=None) -> str:
    """Render selected declarations from live or exclusively captured bytes."""
    headers = type_headers(overlay)
    if not headers:
        return ""
    texts, occurrences, lines = {}, [], []
    for name in headers:
        raw = (
            read_linked_state(root, name)[0]
            if files is None
            else files.read(root / name)
        )
        texts[name] = raw.decode("utf-8")
        lines.append(f"# type-input {name} {hashlib.sha256(raw).hexdigest()}")
        occurrences.extend(
            build_declaration_occurrences(declaration_records(texts[name]), origin=name)
        )
    entities = {entity.key: entity for entity in reconcile_declarations(occurrences)}
    for name in _selection(overlay):
        entity = entities.get(DeclarationKey("", "ordinary", name))
        if entity is None or not entity.complete or entity.diagnostic:
            raise ValueError(
                f"missing, incomplete or ambiguous reviewed Rizin type: {name}"
            )
    context = render_declaration_context(
        tuple(occurrences), " ".join(_selection(overlay))
    )
    records = declaration_records(context)
    selected = build_declaration_occurrences(records, origin="replay")
    available = {item.key for item in selected}
    for item in selected:
        expressions = (
            [
                field.type_name
                + (f"[{field.array_extent}]" if field.array_extent else "")
                for field in item.fields
            ]
            if item.key.namespace == "tag"
            else [item.type_expression]
        )
        for expression in expressions:
            if collect_declaration_references(expression) - available:
                raise ValueError(
                    f"unresolved reviewed Rizin type dependency: {expression}"
                )
    names = {item.key.name for item in selected if item.key.name}
    for name, text in texts.items():
        if any(
            definition.name in names for definition in parse_macro_definitions(text)
        ):
            raise ValueError(f"macro-defined reviewed Rizin type input: {name}")
        guard = _GUARD.match(text)
        permitted = (f"!defined({guard.group(1)})",) if guard else ()
        for use in parse_macro_uses(text, names):
            if use.conditions != permitted:
                raise ValueError(
                    f"conditional reviewed Rizin type input: {name}:{use.source_line}"
                )
    lines.extend(f"td {json.dumps(_rizin_declaration(record))}" for record in records)
    return "\n".join(lines) + "\n"
