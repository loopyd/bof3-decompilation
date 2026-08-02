"""Disposable content-addressed summaries for target decompilation audits."""
import sqlite3
from typing import Any, Iterable

from ..domain.manifests import TargetManifest

_SCHEMA = "harness.decomp-status-cache/v1"


def _hash_file(path: Path) -> str:
def _paths(root: Path, manifest: TargetManifest) -> Iterable[Path]:
    yield root / "CMakeLists.txt"
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


def target_fingerprint(root: Path, manifest: TargetManifest) -> str:
    """Hash every shared and target-local input that can affect its object."""

    digest = hashlib.sha256(_SCHEMA.encode())
    for path in _paths(root, manifest):
        relative = path.relative_to(root).as_posix()
        digest.update(relative.encode())
        digest.update(b"\0")
        digest.update(_hash_file(path).encode() if path.is_file() else b"missing")
        digest.update(b"\0")
    binary = root / manifest.binary
    digest.update(manifest.binary.encode())
    digest.update(b"\0")
    digest.update(_hash_file(binary).encode() if binary.is_file() else b"missing")
    return digest.hexdigest()


def source_fingerprint(source: Path, target_fingerprint: str) -> str:
    digest = hashlib.sha256(target_fingerprint.encode())
    digest.update(b"\0")
    digest.update(_hash_file(source).encode())
    return digest.hexdigest()


class StatusCache:
    """SQLite-backed, throwaway cache whose misses are always safe to recompute."""

    def __init__(self, root: Path) -> None:
        self.path = root / "out" / "matching" / "status-cache.sqlite"
        self.path.parent.mkdir(parents=True, exist_ok=True)
        self.connection = sqlite3.connect(self.path)
        self.connection.execute(
            "CREATE TABLE IF NOT EXISTS metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL)"
        )
        row = self.connection.execute(
        if row is not None and row[0] != _SCHEMA:
            self.connection.execute("DROP TABLE IF EXISTS results")
            self.connection.execute("DELETE FROM metadata")
        self.connection.execute(
            "INSERT OR REPLACE INTO metadata (key, value) VALUES ('schema', ?) ",
            (_SCHEMA,),
        )
        self.connection.execute(
            "CREATE TABLE IF NOT EXISTS results ("
            "target TEXT NOT NULL, source TEXT NOT NULL, fingerprint TEXT NOT NULL, "
            "record TEXT NOT NULL, PRIMARY KEY (target, source))"
        )
        self.connection.commit()

    def get(
        self, target: str, source: str, fingerprint: str
    ) -> dict[str, Any] | None:
        row = self.connection.execute(
            "SELECT record FROM results WHERE target = ? AND source = ? AND fingerprint = ?",
            (target, source, fingerprint),
        ).fetchone()
        if row is None:
            return None
        try:
            record = json.loads(row[0])
        except json.JSONDecodeError:
            return None
        return record if isinstance(record, dict) else None

    def put(
        self, target: str, source: str, fingerprint: str, record: dict[str, Any]
    ) -> None:
        self.connection.execute(
            "INSERT OR REPLACE INTO results (target, source, fingerprint, record) "
            "VALUES (?, ?, ?, ?)",
            (target, source, fingerprint, json.dumps(record, sort_keys=True)),
        )
        self.connection.commit()

    def close(self) -> None:
        self.connection.close()
