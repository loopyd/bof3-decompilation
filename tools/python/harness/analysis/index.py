"""Atomic cross-target reverse-engineering index.

The index is derived entirely from target-qualified Rizin snapshots and maps.
It is a query cache, never an authority for binary layout or symbol names.
"""

from __future__ import annotations

import sqlite3
from pathlib import Path
from typing import Iterable, Mapping

from ..discovery import file_sha256
from ..domain import load_target_manifests
from ..domain.manifests import TargetManifest
from .index_snapshot import snapshot_for
from .type_inputs import type_input_digest, type_input_rows
from .macro_index import macro_input_digest, macro_input_rows


SCHEMA_VERSION = "bof3.reverse-index/v9"


def index_path(root: Path) -> Path:
    return root / "out" / "index" / "reverse.sqlite"


def rebuild(root: Path) -> Path:
    from .index_build import rebuild as build

    return build(root)


def connect(
    root: Path, *, manifests: Mapping[str, TargetManifest] | None = None
) -> sqlite3.Connection:
    path = index_path(root)
    if not path.is_file():
        raise FileNotFoundError(
            f"reverse index not found: {path.relative_to(root)}; run just index"
        )
    connection = sqlite3.connect(path)
    connection.row_factory = sqlite3.Row
    try:
        row = connection.execute(
            "SELECT value FROM metadata WHERE key = 'schema'"
        ).fetchone()
    except sqlite3.Error as exc:
        connection.close()
        raise ValueError("invalid reverse index; run just index") from exc
    if row is None or row[0] != SCHEMA_VERSION:
        connection.close()
        found = row[0] if row is not None else "missing"
        raise ValueError(
            f"reverse index schema mismatch: expected {SCHEMA_VERSION}, got {found}; run just index"
        )
    try:
        indexed_targets = connection.execute(
            "SELECT id, binary, binary_sha256, snapshot, snapshot_sha256 FROM targets"
        )
        loaded_manifests = (
            load_target_manifests(root) if manifests is None else manifests
        )
        seen_targets: set[str] = set()
        for (
            target,
            binary_name,
            binary_digest,
            snapshot_name,
            snapshot_digest,
        ) in indexed_targets:
            seen_targets.add(target)
            binary = root / binary_name
            snapshot = root / snapshot_name
            if not binary.is_file() or file_sha256(binary) != binary_digest:
                raise ValueError(
                    f"stale reverse index binary for {target}; run just index"
                )
            if not snapshot.is_file() or file_sha256(snapshot) != snapshot_digest:
                raise ValueError(
                    f"stale reverse index snapshot for {target}; run just index"
                )
            snapshot_for(root, target, binary, manifest=loaded_manifests[target])
            input_rows = connection.execute(
                "SELECT source_path, sha256, input_kind FROM type_input_fingerprints "
                "WHERE target_id = ? ORDER BY source_path",
                (target,),
            ).fetchall()
            expected_inputs = type_input_rows(root, loaded_manifests[target])
            if [tuple(row) for row in input_rows] != expected_inputs:
                raise ValueError(
                    f"stale reverse index type inputs for {target}; run just index"
                )
            digest_row = connection.execute(
                "SELECT value FROM metadata WHERE key = ?", (f"type_inputs:{target}",)
            ).fetchone()
            if digest_row is None or digest_row[0] != type_input_digest(
                expected_inputs
            ):
                raise ValueError(
                    f"stale reverse index type input digest for {target}; run just index"
                )
            macro_rows = connection.execute(
                "SELECT source_path, sha256, input_kind, owner_target "
                "FROM macro_input_fingerprints WHERE target_id = ? "
                "ORDER BY source_path, owner_target",
                (target,),
            ).fetchall()
            expected_macros = macro_input_rows(root, target, loaded_manifests[target])
            if [tuple(row) for row in macro_rows] != expected_macros:
                raise ValueError(
                    f"stale reverse index macro inputs for {target}; run just index"
                )
            macro_digest = connection.execute(
                "SELECT value FROM metadata WHERE key = ?", (f"macro_inputs:{target}",)
            ).fetchone()
            if macro_digest is None or macro_digest[0] != macro_input_digest(
                expected_macros
            ):
                raise ValueError(
                    f"stale reverse index macro input digest for {target}; run just index"
                )
        if seen_targets != set(loaded_manifests):
            raise ValueError("stale reverse index target coverage; run just index")
    except BaseException:
        connection.close()
        raise
    return connection


def rows(
    connection: sqlite3.Connection, query: str, params: Iterable[object] = ()
) -> list[dict[str, object]]:
    return [dict(row) for row in connection.execute(query, tuple(params))]


__all__ = ["connect", "index_path", "rebuild", "rows"]
