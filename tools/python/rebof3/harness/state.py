from __future__ import annotations

from collections.abc import Iterable
from contextlib import contextmanager
from pathlib import Path
from typing import Any
import json
import sqlite3
import time


SCHEMA_VERSION = "rebof3-simple.harness-state/v1"


def now() -> int:
    return int(time.time())


def connect(path: Path) -> sqlite3.Connection:
    path.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(path)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys = ON")
    return conn


@contextmanager
def state_db(path: Path):
    conn = connect(path)
    try:
        init_db(conn)
        yield conn
        conn.commit()
    finally:
        conn.close()


def init_db(conn: sqlite3.Connection) -> None:
    conn.executescript(
        """
        CREATE TABLE IF NOT EXISTS metadata (
          key TEXT PRIMARY KEY,
          value TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS targets (
          id TEXT PRIMARY KEY,
          type TEXT NOT NULL,
          status TEXT NOT NULL,
          priority INTEGER NOT NULL DEFAULT 100,
          summary TEXT NOT NULL,
          source_hint TEXT,
          program_path TEXT,
          entry_hex TEXT,
          payload_json TEXT NOT NULL DEFAULT '{}',
          created_at INTEGER NOT NULL,
          updated_at INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS claims (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          target_id TEXT NOT NULL REFERENCES targets(id) ON DELETE CASCADE,
          owner TEXT NOT NULL,
          status TEXT NOT NULL,
          claimed_at INTEGER NOT NULL,
          expires_at INTEGER NOT NULL,
          notes TEXT
        );

        CREATE TABLE IF NOT EXISTS events (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          target_id TEXT REFERENCES targets(id) ON DELETE SET NULL,
          kind TEXT NOT NULL,
          message TEXT NOT NULL,
          path TEXT,
          created_at INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS locks (
          name TEXT PRIMARY KEY,
          owner TEXT NOT NULL,
          acquired_at INTEGER NOT NULL,
          expires_at INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS binary_maps (
          target_id TEXT PRIMARY KEY REFERENCES targets(id) ON DELETE CASCADE,
          original_bin TEXT,
          compiled_bin TEXT,
          source_hint TEXT,
          map_json TEXT NOT NULL,
          updated_at INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS symbols (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          target_id TEXT NOT NULL REFERENCES targets(id) ON DELETE CASCADE,
          kind TEXT NOT NULL,
          name TEXT,
          address TEXT,
          source TEXT,
          evidence_json TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS xrefs (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          target_id TEXT NOT NULL REFERENCES targets(id) ON DELETE CASCADE,
          kind TEXT NOT NULL,
          from_address TEXT,
          to_address TEXT,
          name TEXT,
          evidence_json TEXT NOT NULL
        );

        CREATE INDEX IF NOT EXISTS idx_targets_status_priority
          ON targets(status, priority, id);
        CREATE INDEX IF NOT EXISTS idx_claims_target_status
          ON claims(target_id, status, expires_at);
        CREATE INDEX IF NOT EXISTS idx_events_target
          ON events(target_id, created_at);
        CREATE INDEX IF NOT EXISTS idx_symbols_target
          ON symbols(target_id, kind, address);
        CREATE INDEX IF NOT EXISTS idx_xrefs_target
          ON xrefs(target_id, kind, from_address, to_address);
        """
    )
    conn.execute(
        "INSERT OR REPLACE INTO metadata(key, value) VALUES('schema', ?)",
        (SCHEMA_VERSION,),
    )
    _migrate_binary_maps(conn)


def _migrate_binary_maps(conn: sqlite3.Connection) -> None:
    columns = {
        str(row["name"])
        for row in conn.execute("PRAGMA table_info(binary_maps)").fetchall()
    }
    if "compiled_bin" not in columns:
        conn.execute("ALTER TABLE binary_maps ADD COLUMN compiled_bin TEXT")
    if "rebuilt_bin" in columns:
        conn.execute(
            "UPDATE binary_maps SET compiled_bin = rebuilt_bin "
            "WHERE compiled_bin IS NULL"
        )


def encode_payload(payload: dict[str, Any]) -> str:
    return json.dumps(payload, sort_keys=True, separators=(",", ":"))


def decode_payload(value: str | None) -> dict[str, Any]:
    if not value:
        return {}
    payload = json.loads(value)
    return payload if isinstance(payload, dict) else {}


def upsert_targets(conn: sqlite3.Connection, targets: Iterable[dict[str, Any]]) -> int:
    count = 0
    timestamp = now()
    for target in targets:
        payload = dict(target.get("payload") or {})
        conn.execute(
            """
            INSERT INTO targets(
              id, type, status, priority, summary, source_hint, program_path,
              entry_hex, payload_json, created_at, updated_at
            )
            VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ON CONFLICT(id) DO UPDATE SET
              type = excluded.type,
              status = CASE
                WHEN targets.status IN ('done', 'blocked') THEN targets.status
                ELSE excluded.status
              END,
              priority = excluded.priority,
              summary = excluded.summary,
              source_hint = excluded.source_hint,
              program_path = excluded.program_path,
              entry_hex = excluded.entry_hex,
              payload_json = excluded.payload_json,
              updated_at = excluded.updated_at
            """,
            (
                str(target["id"]),
                str(target["type"]),
                str(target.get("status") or "queued"),
                int(target.get("priority", 100)),
                str(target.get("summary") or target["id"]),
                target.get("source_hint"),
                target.get("program_path"),
                target.get("entry_hex"),
                encode_payload(payload),
                timestamp,
                timestamp,
            ),
        )
        count += 1
    return count


def prune_stale_targets(
    conn: sqlite3.Connection,
    *,
    target_type: str,
    keep_ids: Iterable[str],
    statuses: tuple[str, ...] = ("queued", "ready"),
) -> int:
    conn.execute("CREATE TEMP TABLE IF NOT EXISTS keep_target_ids(id TEXT PRIMARY KEY)")
    conn.execute("DELETE FROM keep_target_ids")
    conn.executemany(
        "INSERT OR IGNORE INTO keep_target_ids(id) VALUES(?)",
        [(target_id,) for target_id in keep_ids],
    )
    placeholders = ",".join("?" for _ in statuses)
    cursor = conn.execute(
        f"""
        DELETE FROM targets
        WHERE type = ?
          AND status IN ({placeholders})
          AND NOT EXISTS (
            SELECT 1 FROM keep_target_ids WHERE keep_target_ids.id = targets.id
          )
        """,
        (target_type, *statuses),
    )
    conn.execute("DELETE FROM keep_target_ids")
    return int(cursor.rowcount)


def target_row(conn: sqlite3.Connection, target_id: str) -> dict[str, Any] | None:
    row = conn.execute("SELECT * FROM targets WHERE id = ?", (target_id,)).fetchone()
    return None if row is None else row_to_target(row)


def row_to_target(row: sqlite3.Row) -> dict[str, Any]:
    payload = dict(row)
    payload["payload"] = decode_payload(payload.pop("payload_json", "{}"))
    return payload


def list_targets(
    conn: sqlite3.Connection,
    *,
    limit: int = 20,
    status: str | None = None,
    target_type: str | None = None,
) -> list[dict[str, Any]]:
    params: list[Any] = []
    predicates: list[str] = []
    if status:
        predicates.append("status = ?")
        params.append(status)
    if target_type:
        predicates.append("type = ?")
        params.append(target_type)
    where = "" if not predicates else f"WHERE {' AND '.join(predicates)}"
    params.append(limit)
    rows = conn.execute(
        f"""
        SELECT * FROM targets
        {where}
        ORDER BY priority ASC, id ASC
        LIMIT ?
        """,
        params,
    ).fetchall()
    return [row_to_target(row) for row in rows]


def active_claim_for_target(
    conn: sqlite3.Connection, target_id: str
) -> dict[str, Any] | None:
    row = conn.execute(
        """
        SELECT * FROM claims
        WHERE target_id = ? AND status = 'active' AND expires_at > ?
        ORDER BY expires_at DESC
        LIMIT 1
        """,
        (target_id, now()),
    ).fetchone()
    return None if row is None else dict(row)


def active_claims(conn: sqlite3.Connection) -> list[dict[str, Any]]:
    rows = conn.execute(
        """
        SELECT claims.*, targets.summary
        FROM claims
        JOIN targets ON targets.id = claims.target_id
        WHERE claims.status = 'active' AND claims.expires_at > ?
        ORDER BY claims.expires_at ASC, claims.target_id ASC
        """,
        (now(),),
    ).fetchall()
    return [dict(row) for row in rows]


def claim_target(
    conn: sqlite3.Connection,
    *,
    owner: str,
    target_id: str | None = None,
    status: str | None = None,
    target_type: str | None = None,
    lease_minutes: int = 120,
) -> dict[str, Any] | None:
    timestamp = now()
    expires_at = timestamp + (lease_minutes * 60)
    if target_id:
        row = conn.execute(
            "SELECT * FROM targets WHERE id = ?", (target_id,)
        ).fetchone()
    else:
        row = conn.execute(
            """
            SELECT * FROM targets
            WHERE status IN ('queued', 'ready', 'analyzed')
              AND (? IS NULL OR status = ?)
              AND (? IS NULL OR type = ?)
              AND NOT EXISTS (
                SELECT 1 FROM claims
                WHERE claims.target_id = targets.id
                  AND claims.status = 'active'
                  AND claims.expires_at > ?
              )
            ORDER BY priority ASC, id ASC
            LIMIT 1
            """,
            (status, status, target_type, target_type, timestamp),
        ).fetchone()
    if row is None:
        return None
    target = row_to_target(row)
    active = active_claim_for_target(conn, str(target["id"]))
    if active is not None:
        raise RuntimeError(
            f"target already claimed by {active['owner']}: {target['id']}"
        )
    conn.execute(
        """
        INSERT INTO claims(target_id, owner, status, claimed_at, expires_at, notes)
        VALUES(?, ?, 'active', ?, ?, NULL)
        """,
        (target["id"], owner, timestamp, expires_at),
    )
    add_event(
        conn, target_id=str(target["id"]), kind="claim", message=f"claimed by {owner}"
    )
    return target


def finish_target(
    conn: sqlite3.Connection,
    *,
    target_id: str,
    status: str,
    message: str,
    path: str | None = None,
) -> None:
    timestamp = now()
    conn.execute(
        "UPDATE targets SET status = ?, updated_at = ? WHERE id = ?",
        (status, timestamp, target_id),
    )
    conn.execute(
        "UPDATE claims SET status = 'closed' WHERE target_id = ? AND status = 'active'",
        (target_id,),
    )
    add_event(conn, target_id=target_id, kind=status, message=message, path=path)


def add_event(
    conn: sqlite3.Connection,
    *,
    target_id: str | None,
    kind: str,
    message: str,
    path: str | None = None,
) -> None:
    conn.execute(
        "INSERT INTO events(target_id, kind, message, path, created_at) VALUES(?, ?, ?, ?, ?)",
        (target_id, kind, message, path, now()),
    )


def counts_by_status(conn: sqlite3.Connection) -> dict[str, int]:
    rows = conn.execute(
        "SELECT status, COUNT(*) AS count FROM targets GROUP BY status ORDER BY status"
    ).fetchall()
    return {str(row["status"]): int(row["count"]) for row in rows}


def counts_by_type(conn: sqlite3.Connection) -> dict[str, int]:
    rows = conn.execute(
        "SELECT type, COUNT(*) AS count FROM targets GROUP BY type ORDER BY type"
    ).fetchall()
    return {str(row["type"]): int(row["count"]) for row in rows}


def acquire_lock(
    conn: sqlite3.Connection,
    *,
    name: str,
    owner: str,
    lease_minutes: int = 60,
) -> bool:
    timestamp = now()
    expires_at = timestamp + (lease_minutes * 60)
    conn.execute(
        "DELETE FROM locks WHERE name = ? AND expires_at <= ?", (name, timestamp)
    )
    try:
        conn.execute(
            "INSERT INTO locks(name, owner, acquired_at, expires_at) VALUES(?, ?, ?, ?)",
            (name, owner, timestamp, expires_at),
        )
    except sqlite3.IntegrityError:
        return False
    return True


def release_lock(conn: sqlite3.Connection, *, name: str, owner: str) -> None:
    conn.execute("DELETE FROM locks WHERE name = ? AND owner = ?", (name, owner))


def lock_row(conn: sqlite3.Connection, name: str) -> dict[str, Any] | None:
    row = conn.execute("SELECT * FROM locks WHERE name = ?", (name,)).fetchone()
    return None if row is None else dict(row)


def record_binary_map(conn: sqlite3.Connection, payload: dict[str, Any]) -> None:
    target_id = str(payload["target_id"])
    conn.execute(
        """
        INSERT INTO binary_maps(
          target_id, original_bin, compiled_bin, source_hint, map_json, updated_at
        )
        VALUES(?, ?, ?, ?, ?, ?)
        ON CONFLICT(target_id) DO UPDATE SET
          original_bin = excluded.original_bin,
          compiled_bin = excluded.compiled_bin,
          source_hint = excluded.source_hint,
          map_json = excluded.map_json,
          updated_at = excluded.updated_at
        """,
        (
            target_id,
            payload.get("original_bin"),
            payload.get("compiled_bin"),
            payload.get("source_hint"),
            encode_payload(payload),
            now(),
        ),
    )
    conn.execute("DELETE FROM symbols WHERE target_id = ?", (target_id,))
    conn.execute("DELETE FROM xrefs WHERE target_id = ?", (target_id,))
    symbols = payload.get("symbols", [])
    functions = payload.get("functions", [])
    if not isinstance(symbols, list):
        symbols = []
    if not isinstance(functions, list):
        functions = []
    for symbol in symbols + functions:
        if not isinstance(symbol, dict):
            continue
        conn.execute(
            """
            INSERT INTO symbols(target_id, kind, name, address, source, evidence_json)
            VALUES(?, ?, ?, ?, ?, ?)
            """,
            (
                target_id,
                str(symbol.get("kind") or "function"),
                symbol.get("name"),
                symbol.get("address") or symbol.get("entry_hex") or symbol.get("entry"),
                symbol.get("source"),
                encode_payload(symbol),
            ),
        )
    xrefs = payload.get("xrefs", [])
    if not isinstance(xrefs, list):
        xrefs = []
    for xref in xrefs:
        if not isinstance(xref, dict):
            continue
        conn.execute(
            """
            INSERT INTO xrefs(target_id, kind, from_address, to_address, name, evidence_json)
            VALUES(?, ?, ?, ?, ?, ?)
            """,
            (
                target_id,
                str(xref.get("kind") or "xref"),
                xref.get("from_address") or xref.get("from"),
                xref.get("to_address") or xref.get("to"),
                xref.get("name"),
                encode_payload(xref),
            ),
        )
