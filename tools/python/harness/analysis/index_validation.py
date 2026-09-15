"""Lightweight reverse-index freshness and integrity validation."""

from __future__ import annotations

import sqlite3
from pathlib import Path
from typing import TYPE_CHECKING, Mapping, Sequence

from harness.common.inputs import InputBoundaryError

if TYPE_CHECKING:
    from ..domain.manifests import TargetManifest


def _digest(rows: Sequence[Sequence[str]]) -> str:
    import hashlib

    payload = "\n".join("\0".join(row) for row in rows).encode()
    return hashlib.sha256(payload).hexdigest()


def repository_input(root: Path, path: Path) -> Path:
    """Return an existing regular repository input contained by canonical root."""
    root = root.resolve()
    lexical = path if path.is_absolute() else root / path
    try:
        resolved = lexical.resolve(strict=True)
    except OSError as exc:
        raise ValueError(f"missing repository input: {lexical}") from exc
    if not resolved.is_relative_to(root) or not resolved.is_file():
        raise ValueError(f"repository input escapes root: {lexical}")
    return lexical


def _validate_structure(connection: sqlite3.Connection) -> None:
    from .index import SCHEMA_VERSION
    from .schema import collect_indexes, required_schema

    quick_check = connection.execute("PRAGMA quick_check(1)").fetchone()
    if quick_check is None or quick_check[0] != "ok":
        raise ValueError(f"reverse index integrity check failed: {quick_check}")
    if connection.execute("PRAGMA foreign_key_check").fetchall():
        raise ValueError("reverse index foreign key check failed")
    schema = required_schema()
    tables = {
        row[0]
        for row in connection.execute(
            "SELECT name FROM sqlite_master WHERE type = 'table'"
        )
    }
    if not schema.keys() <= tables:
        raise ValueError("reverse index required tables missing")
    indexes = collect_indexes(connection)
    for table, (
        expected_columns,
        expected_foreign_keys,
        expected_indexes,
    ) in schema.items():
        actual_columns = tuple(
            tuple(row[1:7])
            for row in connection.execute(f'PRAGMA table_xinfo("{table}")')
        )
        actual_foreign_keys = tuple(
            tuple(row)
            for row in connection.execute(f'PRAGMA foreign_key_list("{table}")')
        )
        if (
            actual_columns != expected_columns
            or actual_foreign_keys != expected_foreign_keys
            or indexes.get(table, ()) != expected_indexes
        ):
            raise ValueError(f"reverse index table schema mismatch: {table}")
    row = connection.execute(
        "SELECT value FROM metadata WHERE key = 'schema'"
    ).fetchone()
    if row is None or row[0] != SCHEMA_VERSION:
        found = row[0] if row is not None else "missing"
        raise ValueError(
            f"reverse index schema mismatch: expected {SCHEMA_VERSION}, got {found}; run just index"
        )


def _validate_selected_maps(
    connection: sqlite3.Connection, target: str, expected: Sequence[Sequence[str]]
) -> None:
    actual = [
        tuple(row)
        for row in connection.execute(
            "SELECT source_path, sha256 FROM selected_map_fingerprints "
            "WHERE target_id = ? ORDER BY source_path",
            (target,),
        )
    ]
    if actual != sorted(tuple(row) for row in expected):
        raise ValueError(
            f"stale reverse index selected symbol maps for {target}; run just index"
        )


def validate_status_index(connection: sqlite3.Connection, root: Path) -> None:
    """Validate status against the same repository-derived facts as connect()."""
    from harness.macros.index import macro_input_digest
    from harness.types.inputs import type_input_digest

    from ..domain.repository_layout import load_repository_layout
    from .index_snapshot import snapshot_for

    try:
        _validate_structure(connection)
        repository = load_repository_layout(root)
        manifests = repository.manifests
        targets = connection.execute(
            "SELECT id, binary, binary_sha256, snapshot, snapshot_sha256 FROM targets"
        ).fetchall()
        if {row[0] for row in targets} != set(manifests):
            raise ValueError("stale reverse index target coverage; run just index")

        digest_file = repository.files.digest

        def derive(row):
            target, binary, binary_digest, snapshot, snapshot_digest = row
            binary_path, snapshot_path = root / binary, root / snapshot
            if not binary_path.is_file() or digest_file(binary_path) != binary_digest:
                raise ValueError(
                    f"stale reverse index binary for {target}; run just index"
                )
            if (
                not snapshot_path.is_file()
                or digest_file(snapshot_path) != snapshot_digest
            ):
                raise ValueError(
                    f"stale reverse index snapshot for {target}; run just index"
                )
            snapshot_for(
                root,
                target,
                binary_path,
                manifest=manifests[target],
                binary_sha256=binary_digest,
                layout=repository.splats[target],
                read_file=repository.files.read,
            )
            types = list(repository.type_input_rows[target])
            macros = list(repository.macro_input_rows[target])
            return (
                target,
                types,
                type_input_digest(types),
                macros,
                macro_input_digest(macros),
            )

        derived = [derive(row) for row in targets]
        for (
            target,
            expected_types,
            type_digest,
            expected_macros,
            macro_digest,
        ) in derived:
            _validate_selected_maps(
                connection, target, repository.selected_map_rows[target]
            )
            actual_types = [
                tuple(row)
                for row in connection.execute(
                    "SELECT source_path, sha256, input_kind FROM type_input_fingerprints "
                    "WHERE target_id = ? ORDER BY source_path",
                    (target,),
                )
            ]
            stored_type = connection.execute(
                "SELECT value FROM metadata WHERE key = ?", (f"type_inputs:{target}",)
            ).fetchone()
            if (
                actual_types != expected_types
                or stored_type is None
                or stored_type[0] != type_digest
            ):
                raise ValueError(
                    f"stale reverse index type inputs for {target}; run just index"
                )
            actual_macros = [
                tuple(row)
                for row in connection.execute(
                    "SELECT source_path, sha256, input_kind, owner_target "
                    "FROM macro_input_fingerprints WHERE target_id = ? "
                    "ORDER BY source_path, owner_target",
                    (target,),
                )
            ]
            stored_macro = connection.execute(
                "SELECT value FROM metadata WHERE key = ?", (f"macro_inputs:{target}",)
            ).fetchone()
            if (
                actual_macros != expected_macros
                or stored_macro is None
                or stored_macro[0] != macro_digest
            ):
                raise ValueError(
                    f"stale reverse index macro inputs for {target}; run just index"
                )
    except (sqlite3.DatabaseError, ValueError, OSError, KeyError) as exc:
        if isinstance(exc, InputBoundaryError):
            raise ValueError(f"repository input escapes root: {exc.path}") from exc
        if isinstance(exc, ValueError):
            raise
        raise ValueError("invalid reverse index; run just index") from exc


def validate_index(
    connection: sqlite3.Connection,
    root: Path,
    *,
    manifests: Mapping[str, TargetManifest] | None = None,
    manifest_loader=None,
) -> None:
    """Validate index structure and every repository-owned input fingerprint."""
    from harness.macros.index import macro_input_digest, macro_input_rows
    from harness.types.inputs import type_input_digest, type_input_rows

    from ..domain.manifests import load_target_manifests
    from ..io import file_sha256
    from .index_snapshot import snapshot_for

    try:
        _validate_structure(connection)
        # Canonical registered-input traversal owns containment for both open
        # modes; full validation must never maintain a weaker parallel list.
        from ..domain.repository_layout import load_repository_layout

        repository = load_repository_layout(root)
        loader = load_target_manifests if manifest_loader is None else manifest_loader
        loaded = loader(root) if manifests is None else manifests
        if set(loaded) != set(repository.manifests):
            raise ValueError("stale reverse index target coverage; run just index")
        for target, manifest in loaded.items():
            repository_input(root, Path(f"config/targets/{target}/target.toml"))
            repository_input(root, Path(manifest.binary))
            repository_input(root, Path(manifest.splat))
            for claim in (
                *manifest.sources,
                *manifest.support_sources,
                *manifest.headers,
                manifest.psyq_source,
            ):
                if claim:
                    repository_input(root, Path(claim))
        seen: set[str] = set()
        indexed = connection.execute(
            "SELECT id, binary, binary_sha256, snapshot, snapshot_sha256 FROM targets"
        )
        for (
            target,
            binary_name,
            binary_digest,
            snapshot_name,
            snapshot_digest,
        ) in indexed:
            seen.add(target)
            binary = repository_input(root, Path(binary_name))
            snapshot = repository_input(root, Path(snapshot_name))
            if file_sha256(binary) != binary_digest:
                raise ValueError(
                    f"stale reverse index binary for {target}; run just index"
                )
            if file_sha256(snapshot) != snapshot_digest:
                raise ValueError(
                    f"stale reverse index snapshot for {target}; run just index"
                )
            snapshot_for(
                root,
                target,
                binary,
                manifest=loaded[target],
                binary_sha256=binary_digest,
            )
            _validate_selected_maps(
                connection, target, repository.selected_map_rows[target]
            )
            inputs = connection.execute(
                "SELECT source_path, sha256, input_kind FROM type_input_fingerprints WHERE target_id = ? ORDER BY source_path",
                (target,),
            ).fetchall()
            expected_inputs = type_input_rows(root, loaded[target])
            digest = connection.execute(
                "SELECT value FROM metadata WHERE key = ?", (f"type_inputs:{target}",)
            ).fetchone()
            if (
                [tuple(row) for row in inputs] != expected_inputs
                or digest is None
                or digest[0] != type_input_digest(expected_inputs)
            ):
                raise ValueError(
                    f"stale reverse index type inputs for {target}; run just index"
                )
            macros = connection.execute(
                "SELECT source_path, sha256, input_kind, owner_target FROM macro_input_fingerprints WHERE target_id = ? ORDER BY source_path, owner_target",
                (target,),
            ).fetchall()
            expected_macros = macro_input_rows(root, target, loaded[target])
            macro_digest = connection.execute(
                "SELECT value FROM metadata WHERE key = ?", (f"macro_inputs:{target}",)
            ).fetchone()
            if (
                [tuple(row) for row in macros] != expected_macros
                or macro_digest is None
                or macro_digest[0] != macro_input_digest(expected_macros)
            ):
                raise ValueError(
                    f"stale reverse index macro inputs for {target}; run just index"
                )
        if seen != set(loaded):
            raise ValueError("stale reverse index target coverage; run just index")
    except (sqlite3.DatabaseError, ValueError, KeyError) as exc:
        if isinstance(exc, InputBoundaryError):
            raise ValueError(f"repository input escapes root: {exc.path}") from exc
        if isinstance(exc, ValueError):
            raise
        raise ValueError("invalid reverse index; run just index") from exc
