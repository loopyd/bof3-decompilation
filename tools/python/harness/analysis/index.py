"""Atomic derived reverse index; never authority for binary layout or names."""

from __future__ import annotations

import sqlite3
from pathlib import Path
from typing import Iterable, Mapping

from ..domain.manifests import load_target_manifests
from ..domain.manifests import TargetManifest
from .index_validation import validate_index, validate_status_index

SCHEMA_VERSION = "bof3.reverse-index/v18"


def index_path(root: Path) -> Path:
    return root / "out" / "index" / "reverse.sqlite"


def rebuild(root: Path) -> Path:
    from .index_build import rebuild as build

    return build(root)


def connect(
    root: Path,
    *,
    manifests: Mapping[str, TargetManifest] | None = None,
    shared: bool = False,
    allow_stale: bool = False,
) -> sqlite3.Connection:
    path = index_path(root)
    if not path.is_file():
        raise FileNotFoundError(
            f"reverse index not found: {path.relative_to(root)}; run just index"
        )
    connection = sqlite3.connect(path, check_same_thread=not shared)
    connection.execute("PRAGMA foreign_keys = ON")
    connection.row_factory = sqlite3.Row
    if allow_stale:
        # Read-only ranking queries only need the cached index. Skipping the
        # every-target snapshot revalidation keeps candidate ranking usable
        # while sibling lanes' writes have invalidated snapshots; callers must
        # still verify candidates live before acceptance.
        return connection
    try:
        validate_index(
            connection,
            root,
            manifests=manifests,
            manifest_loader=load_target_manifests,
        )
    except BaseException:
        connection.close()
        raise
    return connection


def connect_status(root: Path) -> sqlite3.Connection:
    """Open an index with lightweight validation equivalent for status facts."""
    path = index_path(root)
    if not path.is_file():
        raise FileNotFoundError(
            f"reverse index not found: {path.relative_to(root)}; run just index"
        )
    connection = sqlite3.connect(path)
    connection.execute("PRAGMA foreign_keys = ON")
    connection.row_factory = sqlite3.Row
    try:
        validate_status_index(connection, root)
    except BaseException:
        connection.close()
        raise
    return connection


def rows(
    connection: sqlite3.Connection, query: str, params: Iterable[object] = ()
) -> list[dict[str, object]]:
    return [dict(row) for row in connection.execute(query, tuple(params))]


__all__ = [
    "connect",
    "connect_status",
    "index_path",
    "rebuild",
    "rows",
    "validate_index",
]
