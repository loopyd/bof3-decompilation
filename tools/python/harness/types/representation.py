"""Resolve explicitly indexed identity aliases without inferring pointee layout."""

from __future__ import annotations

import sqlite3


def resolve_identity_declaration(
    connection: sqlite3.Connection,
    declaration_id: str,
    *,
    identities: dict[str, str] | None = None,
) -> str:
    """Resolve aliases; share identities only within one unchanged query snapshot."""
    if identities is None:
        identities = {}
    visited: set[str] = set()
    while declaration_id not in visited:
        if declaration_id in identities:
            declaration_id = identities[declaration_id]
            break
        visited.add(declaration_id)
        row = connection.execute(
            "SELECT namespace, kind, complete, alias_kind, alias_target, diagnostic "
            "FROM type_declarations WHERE id = ?",
            (declaration_id,),
        ).fetchone()
        if row is None:
            raise ValueError(f"missing type declaration: {declaration_id}")
        namespace, kind, complete, alias_kind, alias_target, diagnostic = row
        if (
            diagnostic
            or kind != "typedef"
            or alias_kind != "identity"
            or alias_target is None
        ):
            break
        declaration_id = alias_target
    else:
        raise ValueError(f"cyclic type identity alias: {declaration_id}")
    identities.update((visited_id, declaration_id) for visited_id in visited)
    return declaration_id


def resolve_field_declaration(
    connection: sqlite3.Connection,
    declaration_id: str,
    *,
    identities: dict[str, str] | None = None,
) -> str | None:
    """Find definition-owned fields without copying them onto derived aliases."""
    declaration_id = resolve_identity_declaration(
        connection, declaration_id, identities=identities
    )
    namespace, kind, complete, diagnostic = connection.execute(
        "SELECT namespace, kind, complete, diagnostic FROM type_declarations WHERE id = ?",
        (declaration_id,),
    ).fetchone()
    return (
        declaration_id
        if namespace == "tag"
        and kind in {"struct", "union", "enum"}
        and complete
        and not diagnostic
        else None
    )
