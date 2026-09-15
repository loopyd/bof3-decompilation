"""Disposable content-addressed summaries for target decompilation audits."""

from __future__ import annotations

import hashlib
import json
from dataclasses import asdict
from pathlib import Path
import sqlite3
from typing import Any, Iterable

from harness.common.files import read_file
from harness.common.directory import validate_repo_path
from harness.domain.ids import normalize_target_id

from ..io import file_sha256
from ..domain.manifests import TargetManifest

_SCHEMA = "harness.decomp-status-cache/v7"
_CONFIGURATION = (
    "config/compiler/object-flags.cmake",
    "config/compiler/variants.json",
)
_LIMIT = 4 * 1024 * 1024


def _paths(root: Path, manifest: TargetManifest) -> Iterable[Path]:
    yield root / "CMakeLists.txt"
    for name in _CONFIGURATION:
        yield root / name
    yield root / "config" / "targets" / manifest.id.value / "target.toml"
    yield root / manifest.splat
    yield root / "config" / "targets" / manifest.id.value / "symbols.txt"
    yield root / "config" / "targets" / "shared" / "symbols.txt"
    yield root / "config" / "sdk" / f"psyq-{manifest.psyq_space}.txt"
    for directory in (
        root / "config" / "compiler",
        root / "include",
        root / "src" / "shared",
        root / manifest.source_dir,
    ):
        if directory.is_dir():
            yield from sorted(
                path
                for path in directory.rglob("*")
                if path.is_file() and path.suffix in {".cmake", ".h", ".inc"}
            )
    if manifest.headers:
        # Explicitly claimed private headers (may live outside source_dir).
        from ..domain.claims import manifest_header_paths

        for path in manifest_header_paths(root, manifest):
            if path.is_file() and path.suffix in {".cmake", ".h", ".inc"}:
                yield path
    # Explicit claim identity is part of the target fingerprint: adding or
    # removing a claimed source/support path invalidates the whole target
    # cache even when the files live outside source_dir.  Content of claimed
    # sources is covered per-source by source_fingerprint.
    for claimed in manifest.sources + manifest.support_sources:
        yield root / claimed


def target_fingerprint(root: Path, manifest: TargetManifest) -> str:
    """Bind the supplied target model, configuration and enumerated audit inputs."""

    digest = hashlib.sha256(_SCHEMA.encode())
    digest.update(b"\0")
    digest.update(
        json.dumps(
            asdict(manifest), sort_keys=True, separators=(",", ":"), allow_nan=False
        ).encode()
    )
    digest.update(b"\0")
    for path in dict.fromkeys(_paths(root, manifest)):
        relative = path.relative_to(root).as_posix()
        digest.update(relative.encode())
        digest.update(b"\0")
        if relative in _CONFIGURATION:
            content = read_file(root, relative, missing_ok=True, max_bytes=_LIMIT)
            fingerprint = (
                hashlib.sha256(content).hexdigest().encode()
                if content is not None
                else b"missing"
            )
        else:
            fingerprint = file_sha256(path).encode() if path.is_file() else b"missing"
        digest.update(fingerprint)
        digest.update(b"\0")
    binary = root / manifest.binary
    digest.update(manifest.binary.encode())
    digest.update(b"\0")
    digest.update(file_sha256(binary).encode() if binary.is_file() else b"missing")
    return digest.hexdigest()


def source_fingerprint(source: Path, target_fingerprint: str) -> str:
    digest = hashlib.sha256(target_fingerprint.encode())
    digest.update(b"\0")
    digest.update(file_sha256(source).encode())
    return digest.hexdigest()


def _validate_identity(target: str, source: str, address: int) -> dict[str, str]:
    if not isinstance(target, str) or normalize_target_id(target).value != target:
        raise ValueError("status cache target must be canonical")
    validate_repo_path(source)
    if type(address) is not int or not 0 <= address <= 0xFFFFFFFF:
        raise ValueError("status cache address must be an unsigned 32-bit integer")
    return {"target": target, "source": source, "address": f"0x{address:08X}"}


class MatchStatusCache:
    """SQLite-backed, throwaway cache whose misses are always safe to recompute."""

    def __init__(self, root: Path) -> None:
        self.path = root / "out" / "matching" / "status-cache.sqlite"
        self.path.parent.mkdir(parents=True, exist_ok=True)
        self.connection = sqlite3.connect(self.path)
        self.connection.execute(
            "CREATE TABLE IF NOT EXISTS metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL)"
        )
        row = self.connection.execute(
            "SELECT value FROM metadata WHERE key = 'schema'"
        ).fetchone()
        if row is None or row[0] != _SCHEMA:
            self.connection.execute("DROP TABLE IF EXISTS results")
            self.connection.execute("DELETE FROM metadata")
        self.connection.execute(
            "INSERT OR REPLACE INTO metadata (key, value) VALUES ('schema', ?) ",
            (_SCHEMA,),
        )
        self.connection.execute(
            "CREATE TABLE IF NOT EXISTS results ("
            "target TEXT NOT NULL, source TEXT NOT NULL, address INTEGER NOT NULL, "
            "fingerprint TEXT NOT NULL, record TEXT NOT NULL, "
            "PRIMARY KEY (target, source, address))"
        )
        self.connection.commit()

    def get(
        self, target: str, source: str, address: int, fingerprint: str
    ) -> dict[str, Any] | None:
        expected = _validate_identity(target, source, address)
        row = self.connection.execute(
            "SELECT record FROM results WHERE target = ? AND source = ? "
            "AND address = ? AND fingerprint = ?",
            (target, source, address, fingerprint),
        ).fetchone()
        if row is None:
            return None
        try:
            record = json.loads(row[0])
        except json.JSONDecodeError:
            return None
        if not isinstance(record, dict) or any(
            record.get(name) != value for name, value in expected.items()
        ):
            return None
        return record

    def put(
        self,
        target: str,
        source: str,
        address: int,
        fingerprint: str,
        record: dict[str, Any],
    ) -> None:
        expected = _validate_identity(target, source, address)
        if not isinstance(record, dict) or any(
            record.get(name) != value for name, value in expected.items()
        ):
            raise ValueError("status cache record differs from function identity")
        self.connection.execute(
            "INSERT OR REPLACE INTO results (target, source, address, fingerprint, record) "
            "VALUES (?, ?, ?, ?, ?)",
            (target, source, address, fingerprint, json.dumps(record, sort_keys=True)),
        )
        self.connection.commit()

    def close(self) -> None:
        self.connection.close()
