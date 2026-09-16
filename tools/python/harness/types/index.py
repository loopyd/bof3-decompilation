"""Target-qualified authored type indexing and provenance-rich query payloads."""

from __future__ import annotations

import ast
import re
import sqlite3
from pathlib import Path
from typing import Any

from harness.common.digests import digest
from harness.domain.c_context import collect_declaration_references, declaration_records
from harness.domain.declarations import (
    CDeclaration,
    build_declaration_occurrences,
    reconcile_declarations,
)
from harness.domain.tags import parse_declaration_kind_tag
from harness.types.inputs import authored_type_headers
from harness.types.representation import (
    resolve_field_declaration,
    resolve_identity_declaration,
)

_ASSERT_SIZE = re.compile(r"\bASSERT_SIZE\s*\(\s*([A-Za-z_]\w*)\s*,\s*([^,)]+)\)")
_ASSERT_OFFSET = re.compile(
    r"\bASSERT_OFFSET\s*\(\s*([A-Za-z_]\w*)\s*,\s*([A-Za-z_]\w*)\s*,\s*([^,)]+)\)"
)
_SCALAR_WIDTH = {
    "bool": 1,
    "s8": 1,
    "u8": 1,
    "s16": 2,
    "u16": 2,
    "s32": 4,
    "u32": 4,
    "f32": 4,
    "s64": 8,
    "u64": 8,
    "f64": 8,
}


def owned_headers(root: Path, manifest: Any) -> list[Path]:
    """Compatibility seam: explicit private owners plus shared base aliases."""

    return [
        path.resolve() for path, _provenance in authored_type_headers(root, manifest)
    ]


def _path(root: Path, path: Path) -> str:
    return path.resolve().relative_to(root.resolve()).as_posix()


def _identity(*parts: str) -> str:
    return digest({"parts": parts})


def _integer(expression: str) -> int | None:
    try:
        node = ast.parse(expression.strip(), mode="eval")
    except SyntaxError:
        return None
    allowed = (
        ast.Expression,
        ast.Constant,
        ast.UnaryOp,
        ast.BinOp,
        ast.UAdd,
        ast.USub,
        ast.Add,
        ast.Sub,
        ast.Mult,
        ast.FloorDiv,
        ast.LShift,
        ast.RShift,
        ast.BitOr,
        ast.BitAnd,
    )
    if any(not isinstance(item, allowed) for item in ast.walk(node)):
        return None
    try:
        value = eval(compile(node, "<assert-layout>", "eval"), {"__builtins__": {}}, {})
    except (ArithmeticError, TypeError):
        return None
    return value if isinstance(value, int) and value >= 0 else None


def _field_width(field: Any) -> int | None:
    base = _SCALAR_WIDTH.get(field.type_name)
    extent = _integer(field.array_extent) if field.array_extent else 1
    return base * extent if base is not None and extent is not None else None


def _insert_model(connection, root: Path, target: str, sources) -> None:
    occurrences = []
    provenance_by_origin = {}
    for path, text, provenance, records in sources:
        origin = _path(root, path)
        provenance_by_origin[origin] = provenance
        occurrences.extend(
            build_declaration_occurrences(records, origin=origin, scope=target)
        )
    try:
        entities = reconcile_declarations(occurrences)
    except ValueError as error:
        raise sqlite3.IntegrityError(str(error)) from error
    identifiers = {
        entity.key: _identity(
            target,
            entity.key.namespace,
            entity.key.name,
            entity.key.origin,
            str(entity.key.ordinal),
        )
        for entity in entities
    }
    for path, text, provenance, records in sources:
        origin = _path(root, path)
        for ordinal, record in enumerate(records):
            connection.execute(
                "INSERT INTO type_occurrences VALUES (?, ?, ?, ?, ?, ?, ?)",
                (
                    _identity(target, origin, str(ordinal)),
                    target,
                    origin,
                    ordinal,
                    record.canonical,
                    provenance,
                    record.diagnostic,
                ),
            )
    for entity in entities:
        occurrence = entity.selected_definition or entity.representative
        provenance = provenance_by_origin[occurrence.origin]
        alias_target = identifiers.get(entity.alias_target)
        if entity.alias_target is not None and alias_target is None:
            shared = connection.execute(
                "SELECT id FROM type_declarations WHERE target_id = '__shared__' "
                "AND namespace = ? AND name = ?",
                (entity.alias_target.namespace, entity.alias_target.name),
            ).fetchone()
            alias_target = None if shared is None else shared[0]
        name = (
            entity.key.name or f"<anonymous:{entity.key.origin}:{entity.key.ordinal}>"
        )
        connection.execute(
            "INSERT INTO type_declarations "
            "(id, target_id, name, kind, tag_name, source_path, provenance, canonical, "
            "review_status, byte_size, byte_alignment, diagnostic, namespace, complete, "
            "type_expression, alias_target, alias_kind, definition_occurrence) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NULL, ?, ?, ?, ?, ?, ?, ?)",
            (
                identifiers[entity.key],
                target,
                name,
                entity.kind,
                entity.key.name
                if entity.key.namespace == "tag"
                else (
                    entity.alias_target.name
                    if entity.alias_target is not None
                    and entity.alias_target.namespace == "tag"
                    else None
                ),
                occurrence.origin,
                provenance,
                entity.canonical,
                "diagnostic"
                if entity.diagnostic
                or provenance not in {"header_claim", "shared_base"}
                else "reviewed",
                _SCALAR_WIDTH.get(name)
                if entity.kind == "typedef"
                and entity.alias_kind != "derived"
                and not entity.diagnostic
                else None,
                entity.diagnostic,
                entity.key.namespace,
                int(entity.complete),
                entity.type_expression,
                alias_target,
                entity.alias_kind,
                _identity(
                    target,
                    entity.selected_definition.origin,
                    str(entity.selected_definition.ordinal),
                )
                if entity.selected_definition is not None
                else None,
            ),
        )
    for entity in entities:
        declaration_id = identifiers[entity.key]
        for occurrence in entity.occurrences:
            connection.execute(
                "INSERT INTO type_declaration_occurrences VALUES (?, ?, ?)",
                (
                    declaration_id,
                    _identity(target, occurrence.origin, str(occurrence.ordinal)),
                    "definition"
                    if occurrence.complete and occurrence.key.namespace == "tag"
                    else occurrence.kind,
                ),
            )
        for ordinal, field in enumerate(entity.fields):
            connection.execute(
                "INSERT INTO type_fields VALUES (?, ?, ?, ?, ?, NULL, ?, ?, ?, 'unresolved', ?)",
                (
                    declaration_id,
                    target,
                    ordinal,
                    field.name,
                    field.type_name,
                    _field_width(field),
                    field.array_extent,
                    field.qualifiers,
                    provenance_by_origin[
                        (entity.selected_definition or entity.representative).origin
                    ],
                ),
            )


def _apply_constraint(connection, target, source_path, type_name, field, kind, raw):
    declaration = connection.execute(
        "SELECT id, kind FROM type_declarations WHERE target_id = ? "
        "AND namespace = 'ordinary' AND name = ?",
        (target, type_name),
    ).fetchone()
    declaration_id = None if declaration is None else declaration[0]
    representation = (
        resolve_identity_declaration(connection, declaration_id)
        if declaration is not None and declaration[1] == "typedef"
        else None
    )
    if (
        representation is not None
        and connection.execute(
            "SELECT diagnostic FROM type_declarations WHERE id = ?", (representation,)
        ).fetchone()[0]
    ):
        representation = None
    resolved = _integer(raw)
    resolution = "unresolved"
    owner = None
    previous = None
    if representation is not None and resolved is not None:
        if kind == "size":
            owner = representation
            previous = connection.execute(
                "SELECT byte_size FROM type_declarations WHERE id = ?", (owner,)
            ).fetchone()
        else:
            owner = resolve_field_declaration(connection, declaration_id)
            previous = connection.execute(
                "SELECT field.byte_offset FROM type_fields field JOIN type_declarations declaration "
                "ON declaration.id = field.declaration_id WHERE field.declaration_id = ? "
                "AND field.name = ? AND declaration.kind IN ('struct', 'union')",
                (owner, field),
            ).fetchone()
        if previous is not None:
            conflict_kind = f"assert_{kind}:{field or ''}"
            conflict = connection.execute(
                "SELECT 1 FROM type_conflicts WHERE target_id = ? AND representation_id = ? "
                "AND conflict_kind = ?",
                (target, representation, conflict_kind),
            ).fetchone()
            resolution = "resolved"
            if conflict or previous[0] is not None and previous[0] != resolved:
                resolution = "conflicting"
                connection.execute(
                    "INSERT OR IGNORE INTO type_conflicts "
                    "(target_id, subject, left_value, right_value, source_path, conflict_kind, "
                    "declaration_id, namespace, representation_id) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, 'ordinary', ?)",
                    (
                        target,
                        type_name,
                        str(previous[0]),
                        raw,
                        source_path,
                        conflict_kind,
                        declaration_id,
                        representation,
                    ),
                )
                connection.execute(
                    "UPDATE type_constraints SET resolution = 'conflicting' WHERE target_id = ? "
                    "AND representation_id = ? AND constraint_kind = ? AND field_name IS ?",
                    (target, representation, kind, field),
                )
            value = resolved if resolution == "resolved" else None
            if kind == "size":
                connection.execute(
                    "UPDATE type_declarations SET byte_size = ? WHERE id = ? AND target_id = ?",
                    (value, owner, target),
                )
            else:
                connection.execute(
                    "UPDATE type_fields SET byte_offset = ? WHERE declaration_id = ? AND name = ? "
                    "AND target_id = ?",
                    (value, owner, field, target),
                )
    connection.execute(
        "INSERT OR IGNORE INTO type_constraints "
        "(target_id, type_name, source_path, field_name, constraint_kind, value, expression, provenance, "
        "evidence_class, declaration_id, namespace, resolution, representation_id) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 'assertion', 'representation', ?, 'ordinary', ?, ?)",
        (
            target,
            type_name,
            source_path,
            field,
            kind,
            raw,
            raw,
            declaration_id,
            resolution,
            representation,
        ),
    )


def _insert_constraints(
    connection: sqlite3.Connection, root: Path, target: str, path: Path, text: str
) -> None:
    source_path = _path(root, path)
    for type_name, value in _ASSERT_SIZE.findall(text):
        _apply_constraint(
            connection, target, source_path, type_name, None, "size", value.strip()
        )
    for type_name, field, value in _ASSERT_OFFSET.findall(text):
        _apply_constraint(
            connection, target, source_path, type_name, field, "offset", value.strip()
        )


def _insert_reference_usages(
    connection,
    target,
    source_path,
    owner,
    expression,
    use_kind,
    provenance,
    evidence,
    skip=None,
):
    for reference in collect_declaration_references(expression):
        declaration = connection.execute(
            "SELECT kind FROM type_declarations WHERE target_id IN (?, '__shared__') "
            "AND namespace = ? AND name = ?",
            (target, reference.namespace, reference.name),
        ).fetchone()
        if declaration is None:
            continue
        type_name = (
            f"{declaration[0]} {reference.name}"
            if reference.namespace == "tag"
            else reference.name
        )
        if type_name == skip:
            continue
        connection.execute(
            "INSERT OR IGNORE INTO type_usages VALUES (?, ?, ?, NULL, ?, ?, NULL, ?, ?)",
            (target, source_path, owner, type_name, use_kind, provenance, evidence),
        )


def _insert_usages(
    connection: sqlite3.Connection,
    root: Path,
    target: str,
    path: Path,
    text: str,
    declarations: tuple[CDeclaration, ...],
    provenance: str,
) -> None:
    source_path = _path(root, path)
    for ordinal, declaration in enumerate(declarations):
        for declarator in declaration.declarators:
            name = declarator.name
            if declarator.kind == "extern":
                storage = parse_declaration_kind_tag(text, name)
                type_name, use_kind = declarator.type_expression, "global"
            elif declarator.kind == "prototype":
                storage = None
                type_name, use_kind = declaration.canonical, "prototype"
            else:
                continue
            connection.execute(
                "INSERT OR IGNORE INTO type_usages VALUES (?, ?, ?, NULL, ?, ?, ?, ?, 'declaration')",
                (target, source_path, name, type_name, use_kind, storage, provenance),
            )
            if declarator.kind == "extern" and declarator.relationship != "function":
                _insert_reference_usages(
                    connection,
                    target,
                    source_path,
                    name,
                    declarator.type_expression,
                    "global-type",
                    provenance,
                    "declaration",
                    skip=type_name,
                )
        owner = (
            f"{declaration.kind} {declaration.tag_name}"
            if declaration.tag_name
            else f"<anonymous:{source_path}:{ordinal}>"
        )
        for field in declaration.fields:
            _insert_reference_usages(
                connection,
                target,
                source_path,
                owner,
                field.type_name,
                f"field:{field.name}",
                provenance,
                "field",
            )


def insert_shared_scalar_types(
    connection: sqlite3.Connection, root: Path, *, files=None
) -> None:
    """Insert the explicitly reviewed shared scalar declaration inventory."""
    path, provenance = authored_type_headers(
        root, type("HeaderOwner", (), {"headers": ()})(), validate_paths=files is None
    )[0]
    text = (
        path.read_text(encoding="utf-8", errors="replace")
        if files is None
        else files.text(path, errors="replace")
    )
    _insert_model(
        connection,
        root,
        "__shared__",
        [(path, text, provenance, declaration_records(text))],
    )


def insert_authored_types(
    connection: sqlite3.Connection,
    root: Path,
    target: str,
    manifest: Any,
    *,
    files=None,
) -> None:
    """Reconcile owned declarations before binding layout assertions and usages."""
    sources = []
    for path, provenance in authored_type_headers(
        root, manifest, include_shared=False, validate_paths=files is None
    ):
        text = (
            path.read_text(encoding="utf-8", errors="replace")
            if files is None
            else files.text(path, errors="replace")
        )
        sources.append((path, text, provenance, declaration_records(text)))
    _insert_model(connection, root, target, sources)
    for path, text, provenance, records in sources:
        _insert_constraints(connection, root, target, path, text)
        _insert_usages(connection, root, target, path, text, records, provenance)


__all__ = ["insert_authored_types", "insert_shared_scalar_types", "owned_headers"]
