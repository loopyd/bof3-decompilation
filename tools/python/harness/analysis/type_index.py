"""Target-qualified authored type indexing and provenance-rich query payloads."""

from __future__ import annotations

import ast
import re
import sqlite3
from pathlib import Path
from typing import Any

from ..domain.c_context import CDeclaration, declaration_records
from ..domain.tags import parse_declaration_kind_tag
from .type_inference import infer_type_candidates
from .type_inputs import authored_type_headers
from .type_queries import type_candidates_payload, type_usages_payload, types_payload

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


def _declaration_names(declaration: CDeclaration, ordinal: int) -> tuple[str, ...]:
    if declaration.names:
        return declaration.names
    if declaration.tag_name:
        return (declaration.tag_name,)
    if declaration.kind == "enum" and declaration.fields:
        return tuple(field.name for field in declaration.fields)
    return (f"__diagnostic_{ordinal:04d}",)


def _insert_declaration(
    connection: sqlite3.Connection,
    root: Path,
    target: str,
    path: Path,
    declaration: CDeclaration,
    provenance: str,
    ordinal: int,
) -> None:
    source_path = _path(root, path)
    for name in _declaration_names(declaration, ordinal):
        kind = (
            "enumerator"
            if not declaration.names and declaration.kind == "enum"
            else declaration.kind
        )
        canonical = declaration.canonical
        existing = connection.execute(
            "SELECT canonical FROM type_declarations WHERE target_id = ? AND name = ? "
            "AND kind = ? AND source_path = ?",
            (target, name, kind, source_path),
        ).fetchone()
        if existing is not None:
            if existing[0] != canonical:
                connection.execute(
                    "INSERT OR IGNORE INTO type_conflicts VALUES (?, ?, ?, ?, ?, 'declaration_collision')",
                    (target, name, existing[0], canonical, source_path),
                )
            continue
        declaration_id = f"{target}:{source_path}:{kind}:{name}"
        connection.execute(
            "INSERT INTO type_declarations VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, NULL, NULL, ?)",
            (
                declaration_id,
                target,
                name,
                kind,
                declaration.tag_name,
                source_path,
                provenance,
                canonical,
                "reviewed"
                if provenance in {"header_claim", "shared_base"}
                else "diagnostic",
                declaration.diagnostic,
            ),
        )
        for field_ordinal, field in enumerate(declaration.fields):
            connection.execute(
                "INSERT INTO type_fields VALUES (?, ?, ?, ?, ?, NULL, ?, ?, ?, 'unresolved', ?)",
                (
                    declaration_id,
                    target,
                    field_ordinal,
                    field.name,
                    field.type_name,
                    _field_width(field),
                    field.array_extent,
                    field.qualifiers,
                    provenance,
                ),
            )


def _insert_constraints(
    connection: sqlite3.Connection, root: Path, target: str, path: Path, text: str
) -> None:
    source_path = _path(root, path)
    for type_name, value in _ASSERT_SIZE.findall(text):
        raw = value.strip()
        connection.execute(
            "INSERT OR IGNORE INTO type_constraints VALUES (?, ?, ?, NULL, 'size', ?, ?, 'assertion', 'representation')",
            (target, type_name, source_path, raw, raw),
        )
        resolved = _integer(raw)
        if resolved is not None:
            connection.execute(
                "UPDATE type_declarations SET byte_size = ? WHERE target_id = ? AND name = ? AND source_path = ?",
                (resolved, target, type_name, source_path),
            )
    for type_name, field, value in _ASSERT_OFFSET.findall(text):
        raw = value.strip()
        connection.execute(
            "INSERT OR IGNORE INTO type_constraints VALUES (?, ?, ?, ?, 'offset', ?, ?, 'assertion', 'representation')",
            (target, type_name, source_path, field, raw, raw),
        )
        resolved = _integer(raw)
        if resolved is not None:
            connection.execute(
                "UPDATE type_fields SET byte_offset = ? WHERE target_id = ? AND "
                "declaration_id IN (SELECT id FROM type_declarations WHERE target_id = ? "
                "AND name = ? AND source_path = ?) AND name = ?",
                (resolved, target, target, type_name, source_path, field),
            )


def _extern_type(canonical: str, name: str) -> str:
    text = canonical.removeprefix("extern ").rstrip(";")
    prefix = text[: text.find(name)].strip()
    return " ".join(prefix.split()) or "unknown"


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
    declared = {name for declaration in declarations for name in declaration.names}
    for declaration in declarations:
        for name in declaration.names:
            if name in {"void", "const", "volatile"}:
                continue
            if declaration.kind == "extern":
                storage = parse_declaration_kind_tag(text, name)
                type_name, use_kind = (
                    _extern_type(declaration.canonical, name),
                    "global",
                )
            elif declaration.kind == "prototype":
                storage = None
                type_name, use_kind = declaration.canonical, "prototype"
            else:
                continue
            connection.execute(
                "INSERT OR IGNORE INTO type_usages VALUES (?, ?, ?, NULL, ?, ?, ?, ?, 'declaration')",
                (target, source_path, name, type_name, use_kind, storage, provenance),
            )
        owner = next(iter(declaration.names), declaration.tag_name)
        if owner is None:
            continue
        for field in declaration.fields:
            for dependency in (
                set(re.findall(r"\b[A-Za-z_]\w*\b", field.type_name)) & declared
            ):
                connection.execute(
                    "INSERT OR IGNORE INTO type_usages VALUES (?, ?, ?, NULL, ?, ?, NULL, ?, 'field')",
                    (
                        target,
                        source_path,
                        owner,
                        dependency,
                        f"field:{field.name}",
                        provenance,
                    ),
                )


def insert_shared_scalar_types(connection: sqlite3.Connection, root: Path) -> None:
    """Index the explicitly shared scalar header once under a synthetic owner."""

    path, provenance = authored_type_headers(root, type("M", (), {"headers": ()})())[0]
    text = path.read_text(encoding="utf-8", errors="replace")
    declarations = declaration_records(text)
    for ordinal, declaration in enumerate(declarations):
        _insert_declaration(
            connection, root, "__shared__", path, declaration, provenance, ordinal
        )


def insert_authored_types(
    connection: sqlite3.Connection, root: Path, target: str, manifest: Any
) -> None:
    """Populate declarations, layout constraints, diagnostics, and usages."""

    seen: dict[tuple[str, str], str] = {}
    for path, provenance in authored_type_headers(root, manifest, include_shared=False):
        text = path.read_text(encoding="utf-8", errors="replace")
        declarations = declaration_records(text)
        for ordinal, declaration in enumerate(declarations):
            for name in _declaration_names(declaration, ordinal):
                key = (declaration.kind, name.casefold())
                previous = seen.get(key)
                if (
                    previous is not None
                    and previous != declaration.canonical
                    and not (
                        declaration.kind in {"struct", "union", "enum"}
                        and "{" not in previous
                        and "{" in declaration.canonical
                    )
                ):
                    connection.execute(
                        "INSERT OR IGNORE INTO type_conflicts VALUES (?, ?, ?, ?, ?, 'declaration_collision')",
                        (
                            target,
                            name,
                            previous,
                            declaration.canonical,
                            _path(root, path),
                        ),
                    )
                    if declaration.kind in {"typedef", "struct", "union", "enum"}:
                        raise sqlite3.IntegrityError(
                            f"conflicting {declaration.kind} declaration for {name}"
                        )
                else:
                    seen[key] = declaration.canonical
            _insert_declaration(
                connection, root, target, path, declaration, provenance, ordinal
            )
        _insert_constraints(connection, root, target, path, text)
        _insert_usages(connection, root, target, path, text, declarations, provenance)


__all__ = [
    "infer_type_candidates",
    "insert_authored_types",
    "insert_shared_scalar_types",
    "owned_headers",
    "type_candidates_payload",
    "type_usages_payload",
    "types_payload",
]
