"""Atomic cross-target reverse-engineering index.

The index is derived entirely from target-qualified Rizin snapshots and maps.
It is a query cache, never an authority for binary layout or symbol names.
"""

from __future__ import annotations

import hashlib
import os
import sqlite3
import tempfile
from collections import defaultdict
from pathlib import Path
from typing import Iterable

from .canonical import load_map, map_path
from .domain import load_target_manifests
from .snapshot import read_snapshot, snapshot_path, validate_snapshot_identity


SCHEMA_VERSION = "bof3.reverse-index/v2"


def index_path(root: Path) -> Path:
    return root / "out" / "index" / "reverse.sqlite"


def _hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _trivial_kind(data: bytes) -> str | None:
    if data == b"\x08\x00\xe0\x03\x00\x00\x00\x00":
        return "return_void"
    return None


def _schema(connection: sqlite3.Connection) -> None:
    connection.executescript(
        """
        PRAGMA foreign_keys = ON;
        CREATE TABLE metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL);
        CREATE TABLE targets (
            id TEXT PRIMARY KEY,
            binary TEXT NOT NULL,
            binary_sha256 TEXT NOT NULL,
            load_address INTEGER NOT NULL,
            engine TEXT NOT NULL,
            engine_version TEXT NOT NULL,
            snapshot TEXT NOT NULL,
            snapshot_sha256 TEXT NOT NULL
        );
        CREATE TABLE symbols (
            target_id TEXT NOT NULL REFERENCES targets(id),
            address INTEGER NOT NULL,
            name TEXT NOT NULL,
            kind TEXT NOT NULL,
            PRIMARY KEY (target_id, address),
            UNIQUE (target_id, name)
        );
        CREATE TABLE functions (
            id TEXT PRIMARY KEY,
            target_id TEXT NOT NULL REFERENCES targets(id),
            address INTEGER NOT NULL,
            size INTEGER NOT NULL,
            name TEXT NOT NULL,
            exact_sha256 TEXT NOT NULL,
            reviewed INTEGER NOT NULL,
            lifted INTEGER NOT NULL,
            source TEXT,
            instruction_count INTEGER NOT NULL,
            basic_blocks INTEGER,
            cfg_edges INTEGER,
            cyclomatic_complexity INTEGER,
            loops INTEGER,
            stack_frame INTEGER,
            local_count INTEGER,
            argument_count INTEGER,
            trivial_kind TEXT
        );
        CREATE INDEX functions_target_address ON functions(target_id, address);
        CREATE INDEX functions_hash ON functions(exact_sha256);
        CREATE TABLE calls (
            caller TEXT NOT NULL REFERENCES functions(id),
            callee TEXT NOT NULL REFERENCES functions(id),
            callsite INTEGER NOT NULL,
            PRIMARY KEY(caller, callee, callsite)
        );
        CREATE TABLE xrefs (
            target_id TEXT NOT NULL REFERENCES targets(id),
            source INTEGER NOT NULL,
            destination INTEGER NOT NULL,
            kind TEXT NOT NULL,
            PRIMARY KEY(target_id, source, destination, kind)
        );
        CREATE TABLE unresolved_calls (
            caller TEXT NOT NULL REFERENCES functions(id),
            target_address INTEGER NOT NULL,
            callsite INTEGER NOT NULL,
            kind TEXT NOT NULL,
            PRIMARY KEY(caller, target_address, callsite, kind)
        );
        CREATE TABLE data_references (
            target_id TEXT NOT NULL REFERENCES targets(id),
            function_id TEXT REFERENCES functions(id),
            address INTEGER NOT NULL,
            symbol TEXT,
            PRIMARY KEY(target_id, function_id, address)
        );
        CREATE TABLE duplicate_groups (
            hash TEXT PRIMARY KEY,
            size INTEGER NOT NULL,
            members INTEGER NOT NULL
        );
        CREATE TABLE duplicate_members (
            hash TEXT NOT NULL REFERENCES duplicate_groups(hash),
            function_id TEXT NOT NULL REFERENCES functions(id),
            PRIMARY KEY(hash, function_id)
        );
        CREATE TABLE psyq_evidence (
            target_id TEXT NOT NULL REFERENCES targets(id),
            address INTEGER NOT NULL,
            name TEXT NOT NULL,
            confidence TEXT NOT NULL,
            evidence TEXT NOT NULL,
            PRIMARY KEY(target_id, address, name)
        );
        """
    )


def _snapshot_for(root: Path, target: str, binary: Path):
    path = snapshot_path(root, target)
    if not path.is_file():
        raise ValueError(f"missing Rizin snapshot: {path.relative_to(root)}")
    snapshot = read_snapshot(path)
    errors = validate_snapshot_identity(snapshot)
    if errors:
        raise ValueError(
            f"invalid Rizin snapshot {path.relative_to(root)}: {'; '.join(errors)}"
        )
    if snapshot.target != target:
        raise ValueError(f"stale Rizin snapshot target: {path.relative_to(root)}")
    if snapshot.engine.get("name") != "rizin":
        raise ValueError(
            f"snapshot was not produced by Rizin: {path.relative_to(root)}"
        )
    if snapshot.inputs.get("binary_sha256") != _hash(binary):
        raise ValueError(f"stale Rizin snapshot bytes: {path.relative_to(root)}")
    from .rizin_project import prepare_target

    if (
        snapshot.inputs.get("replay_sha256")
        != prepare_target(root, target).replay_sha256
    ):
        raise ValueError(f"stale Rizin snapshot recipe: {path.relative_to(root)}")
    return path, snapshot


def rebuild(root: Path) -> Path:
    """Rebuild the index atomically; retain the previous complete file on error."""

    manifests = load_target_manifests(root)
    records = []
    for target, manifest in sorted(manifests.items()):
        binary = root / manifest.binary
        if not binary.is_file():
            raise ValueError(f"missing target binary: {manifest.binary}")
        records.append((target, manifest, binary, *_snapshot_for(root, target, binary)))
    output = index_path(root)
    output.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary = tempfile.mkstemp(
        dir=output.parent, prefix=".reverse.", suffix=".sqlite"
    )
    os.close(descriptor)
    temporary_path = Path(temporary)
    try:
        connection = sqlite3.connect(temporary_path)
        try:
            _schema(connection)
            connection.execute(
                "INSERT INTO metadata VALUES (?, ?)", ("schema", SCHEMA_VERSION)
            )
            for target, manifest, binary, path, snapshot in records:
                binary_bytes = binary.read_bytes()
                connection.execute(
                    "INSERT INTO targets VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                    (
                        target,
                        manifest.binary,
                        _hash(binary),
                        manifest.load_address,
                        snapshot.engine["name"],
                        snapshot.engine.get("version", ""),
                        path.relative_to(root).as_posix(),
                        _hash(path),
                    ),
                )
                for symbol in load_map(map_path(root, target)):
                    kind = (
                        "function"
                        if symbol.canonical_name.startswith("func_")
                        else "data"
                    )
                    connection.execute(
                        "INSERT INTO symbols VALUES (?, ?, ?, ?)",
                        (target, symbol.address, symbol.canonical_name, kind),
                    )
                for function in snapshot.functions:
                    connection.execute(
                        """INSERT INTO functions (
                            id, target_id, address, size, name, exact_sha256,
                            reviewed, lifted, source, instruction_count,
                            basic_blocks, cfg_edges, cyclomatic_complexity,
                            loops, stack_frame, local_count, argument_count,
                            trivial_kind
                        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)""",
                        (
                            function.id,
                            target,
                            function.address,
                            function.analyzer_size,
                            function.analyzer_name,
                            function.exact_sha256,
                            int(function.is_reviewed),
                            int(function.is_lifted),
                            function.source,
                            (function.analyzer_size + 3) // 4,
                            function.basic_blocks,
                            function.edges,
                            function.cyclomatic_complexity,
                            function.loops,
                            function.stack_frame,
                            function.local_count,
                            function.argument_count,
                            _trivial_kind(
                                binary_bytes[
                                    function.address - manifest.load_address :
                                    function.address - manifest.load_address
                                    + function.analyzer_size
                                ]
                            ),
                        ),
                    )
                for call in snapshot.calls:
                    connection.execute(
                        "INSERT INTO calls VALUES (?, ?, ?)",
                        (call.caller, call.callee, call.callsite),
                    )
                    connection.execute(
                        "INSERT OR IGNORE INTO xrefs VALUES (?, ?, ?, ?)",
                        (
                            target,
                            call.callsite,
                            int(call.callee.rsplit("@", 1)[1], 16),
                            "call",
                        ),
                    )
                for unresolved in snapshot.unresolved_calls:
                    connection.execute(
                        "INSERT INTO unresolved_calls VALUES (?, ?, ?, ?)",
                        (
                            unresolved.caller,
                            unresolved.target_address,
                            unresolved.callsite,
                            unresolved.kind,
                        ),
                    )
                    connection.execute(
                        "INSERT OR IGNORE INTO xrefs VALUES (?, ?, ?, ?)",
                        (
                            target,
                            unresolved.callsite,
                            unresolved.target_address,
                            unresolved.kind,
                        ),
                    )
            rows = connection.execute(
                "SELECT exact_sha256, size, id FROM functions ORDER BY exact_sha256, id"
            ).fetchall()
            groups: dict[tuple[str, int], list[str]] = defaultdict(list)
            for digest, size, function_id in rows:
                groups[(digest, size)].append(function_id)
            for (digest, size), members in groups.items():
                if len(members) < 2:
                    continue
                connection.execute(
                    "INSERT INTO duplicate_groups VALUES (?, ?, ?)",
                    (digest, size, len(members)),
                )
                connection.executemany(
                    "INSERT INTO duplicate_members VALUES (?, ?)",
                    ((digest, member) for member in members),
                )
            connection.commit()
        finally:
            connection.close()
        temporary_path.replace(output)
    except BaseException:
        temporary_path.unlink(missing_ok=True)
        raise
    return output


def connect(root: Path) -> sqlite3.Connection:
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
        raise ValueError(
            "invalid reverse index; run just index"
        ) from exc
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
        for target, binary_name, binary_digest, snapshot_name, snapshot_digest in indexed_targets:
            binary = root / binary_name
            snapshot = root / snapshot_name
            if not binary.is_file() or _hash(binary) != binary_digest:
                raise ValueError(f"stale reverse index binary for {target}; run just index")
            if not snapshot.is_file() or _hash(snapshot) != snapshot_digest:
                raise ValueError(f"stale reverse index snapshot for {target}; run just index")
            _snapshot_for(root, target, binary)
    except BaseException:
        connection.close()
        raise
    return connection


def rows(
    connection: sqlite3.Connection, query: str, params: Iterable[object] = ()
) -> list[dict[str, object]]:
    return [dict(row) for row in connection.execute(query, tuple(params))]


__all__ = ["connect", "index_path", "rebuild", "rows"]
