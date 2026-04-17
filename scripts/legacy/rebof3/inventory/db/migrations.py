from __future__ import annotations

import sqlite3
from dataclasses import dataclass

from .schema import MIGRATIONS, SCHEMA_VERSION, SchemaMigration


@dataclass(frozen=True, slots=True)
class MigrationResult:
    applied_versions: tuple[int, ...]
    current_version: int


class MigrationRunner:
    def __init__(self, connection: sqlite3.Connection):
        self.connection = connection

    def current_version(self) -> int:
        try:
            row = self.connection.execute(
                "SELECT MAX(version) AS version FROM schema_migrations"
            ).fetchone()
        except sqlite3.OperationalError as exc:
            if "no such table: schema_migrations" not in str(exc):
                raise
            return 0
        if row is None:
            return 0
        value = row["version"]
        return int(value) if value is not None else 0

    def apply_all(self) -> MigrationResult:
        applied_versions: list[int] = []
        for migration in MIGRATIONS:
            if migration.version <= self.current_version():
                continue
            self._apply_migration(migration)
            applied_versions.append(migration.version)
        return MigrationResult(
            applied_versions=tuple(applied_versions),
            current_version=self.current_version(),
        )

    def _apply_migration(self, migration: SchemaMigration) -> None:
        with self.connection:
            for statement in migration.statements:
                self.connection.execute(statement)
            self.connection.execute(
                "INSERT OR REPLACE INTO schema_migrations(version, name) VALUES(?, ?)",
                (migration.version, migration.name),
            )


def ensure_inventory_schema(connection: sqlite3.Connection) -> MigrationResult:
    runner = MigrationRunner(connection)
    result = runner.apply_all()
    if result.current_version != SCHEMA_VERSION:
        raise RuntimeError(
            f"inventory schema mismatch: expected {SCHEMA_VERSION}, got {result.current_version}"
        )
    return result
