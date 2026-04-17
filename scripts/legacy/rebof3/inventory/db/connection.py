from __future__ import annotations

import sqlite3
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator

from .migrations import ensure_inventory_schema


def connect_inventory_database(path: Path) -> sqlite3.Connection:
    path.parent.mkdir(parents=True, exist_ok=True)
    connection = sqlite3.connect(path)
    connection.row_factory = sqlite3.Row
    connection.execute("PRAGMA foreign_keys=ON")
    connection.execute("PRAGMA journal_mode=WAL")
    connection.execute("PRAGMA synchronous=NORMAL")
    connection.execute("PRAGMA temp_store=MEMORY")
    return connection


@contextmanager
def inventory_db(path: Path) -> Iterator[sqlite3.Connection]:
    connection = connect_inventory_database(path)
    try:
        ensure_inventory_schema(connection)
        yield connection
    finally:
        connection.close()
