"""Fresh reverse-index type context for decompiler exporters."""

from __future__ import annotations

import sqlite3
from pathlib import Path

from ..domain.c_context import public_declaration_context, scalar_declaration_context
from .index import connect
from .type_inputs import SCALAR_HEADER

_BOOTSTRAP_WARNING = (
    "/* WARNING: reverse type index unavailable during explicit bootstrap; "
    "using include/base/types.h only. */\n"
)


def _connect_or_bootstrap(root: Path) -> sqlite3.Connection | None:
    """Open the registry, falling back only when no usable index exists.

    A schema-valid repository index that fails freshness validation must stay a
    hard error. Tiny bootstrap/test roots may have no index or a placeholder
    file before index setup; those roots receive the explicit scalar fallback.
    """

    bootstrap_root = not (root / ".git").exists()
    try:
        return connect(root)
    except FileNotFoundError:
        if bootstrap_root:
            return None
        raise
    except ValueError:
        if not bootstrap_root:
            raise
        path = root / "out/index/reverse.sqlite"
        if path.is_file():
            try:
                with sqlite3.connect(path) as connection:
                    schema = connection.execute(
                        "SELECT value FROM metadata WHERE key = 'schema'"
                    ).fetchone()
            except sqlite3.Error:
                return None
            if schema is None:
                return None
        raise


def scalar_context_from_connection(connection: sqlite3.Connection) -> str:
    """Render shared scalar aliases from the validated registry."""

    rows = connection.execute(
        "SELECT canonical FROM type_declarations WHERE target_id = '__shared__' "
        "AND provenance = 'shared_base' ORDER BY source_path, name"
    ).fetchall()
    declarations = list(dict.fromkeys(str(row[0]) for row in rows))
    if not declarations:
        raise ValueError(
            "reverse index has no shared scalar type declarations; run just index"
        )
    return "\n".join(declarations) + "\n"


def type_context_from_connection(
    connection: sqlite3.Connection, target: str, source: str
) -> str:
    """Close source dependencies across all explicitly owned registry headers."""

    rows = connection.execute(
        "SELECT canonical FROM type_declarations WHERE target_id IN ('__shared__', ?) "
        "AND kind != 'enumerator' ORDER BY CASE target_id WHEN '__shared__' THEN 0 ELSE 1 END, "
        "source_path, name",
        (target,),
    ).fetchall()
    declarations = list(dict.fromkeys(str(row[0]) for row in rows))
    if not declarations:
        raise ValueError(
            f"reverse index has no type declarations for {target}; run just index"
        )
    return public_declaration_context("\n".join(declarations), source, base="")


def _bootstrap_scalar_context(root: Path) -> str:
    return _BOOTSTRAP_WARNING + scalar_declaration_context(
        (root / SCALAR_HEADER).read_text(encoding="utf-8")
    )


def scalar_context(root: Path) -> str:
    """Read fresh registry scalars, or diagnose bootstrap scalar fallback."""

    connection = _connect_or_bootstrap(root)
    if connection is None:
        return _bootstrap_scalar_context(root)
    try:
        return scalar_context_from_connection(connection)
    finally:
        connection.close()


def type_context(root: Path, target: str, source: str) -> str:
    """Read fresh target registry context, or diagnose bootstrap fallback."""

    connection = _connect_or_bootstrap(root)
    if connection is None:
        return _bootstrap_scalar_context(root)
    try:
        return type_context_from_connection(connection, target, source)
    finally:
        connection.close()


__all__ = [
    "scalar_context",
    "scalar_context_from_connection",
    "type_context",
    "type_context_from_connection",
]
