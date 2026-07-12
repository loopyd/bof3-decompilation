"""Repository boundary for the generated SQLite evidence graph."""

from __future__ import annotations

from contextlib import contextmanager
from pathlib import Path
import sqlite3
from typing import Any, Iterator

from .schema import _table_sql, clear, connect, create_schema


class EvidenceRepository:
    """Own connection lifetime, schema setup, and graph transactions.

    Callers use ``connection`` only for read queries that are deliberately
    local to a workflow; writes go through :meth:`insert` and :meth:`edge` so
    transaction ownership stays explicit.
    """

    def __init__(self, path: Path):
        self.path = path
        self.connection: sqlite3.Connection | None = None

    def __enter__(self) -> "EvidenceRepository":
        self.path.parent.mkdir(parents=True, exist_ok=True)
        self.connection = connect(self.path)
        return self

    def __exit__(self, exc_type: Any, exc: Any, traceback: Any) -> None:
        if self.connection is None:
            return
        if exc_type is None:
            self.connection.commit()
        else:
            self.connection.rollback()
        self.connection.close()
        self.connection = None

    def initialize(self) -> None:
        self._require_connection()
        create_schema(self.connection)

    def reset(self) -> None:
        self._require_connection()
        clear(self.connection)
        self.connection.execute("DELETE FROM metadata")

    def insert(
        self, table: str, values: dict[str, Any], *, ignore: bool = False
    ) -> None:
        connection = self._require_connection()
        columns = ", ".join(values)
        placeholders = ", ".join("?" for _ in values)
        verb = "INSERT OR IGNORE" if ignore else "INSERT"
        connection.execute(
            f"{verb} INTO {_table_sql(table)} ({columns}) VALUES ({placeholders})",
            tuple(values.values()),
        )

    def edge(self, source: str, relation: str, target: str) -> None:
        connection = self._require_connection()
        connection.execute(
            "INSERT OR IGNORE INTO edges (source_id, relation, target_id) VALUES (?, ?, ?)",
            (source, relation, target),
        )

    def execute(self, query: str, parameters: tuple[Any, ...] = ()) -> sqlite3.Cursor:
        return self._require_connection().execute(query, parameters)

    @contextmanager
    def transaction(self) -> Iterator[sqlite3.Connection]:
        connection = self._require_connection()
        try:
            yield connection
        except Exception:
            connection.rollback()
            raise
        else:
            connection.commit()

    def _require_connection(self) -> sqlite3.Connection:
        if self.connection is None:
            raise RuntimeError("EvidenceRepository must be used as a context manager")
        return self.connection
