"""Export target-qualified reverse-index counts for the public dashboard."""

from __future__ import annotations

import json
from contextlib import closing
from datetime import datetime, timezone
from pathlib import Path

from harness.analysis.index import SCHEMA_VERSION, connect_status

STATUSES = {
    "exact": "completed",
    "partial": "partial",
    "unlifted": "unlifted",
    "invalid": "unlifted",
}


def collect_targets(connection) -> list[dict]:
    targets = {}
    for row in connection.execute("SELECT id FROM targets ORDER BY id"):
        target = row[0]
        parts = target.split("/")
        if parts[0] not in {"exe", "emi"}:
            raise ValueError(f"unsupported dashboard target: {target}")
        targets[target] = {
            "id": target,
            "kind": parts[0],
            "family": parts[1] if parts[0] == "emi" else "executables",
            "archive": "/".join(parts[2:-1]) if parts[0] == "emi" else parts[-1],
            "entry": parts[-1] if parts[0] == "emi" else None,
            "completed": 0,
            "partial": 0,
            "unlifted": 0,
            "invalid": 0,
            "total": 0,
        }
    for target, status, count in connection.execute(
        "SELECT target_id, lift_status, COUNT(*) FROM functions GROUP BY target_id, lift_status"
    ):
        if status not in STATUSES:
            raise ValueError(f"unknown indexed lift status: {status!r}")
        targets[target][STATUSES[status]] += count
        targets[target]["total"] += count
        if status == "invalid":
            targets[target]["invalid"] += count
    return list(targets.values())


def write_snapshot(root: Path, output: Path, revision: str) -> dict:
    with closing(connect_status(root)) as connection:
        targets = collect_targets(connection)
    counts = {
        key: sum(target[key] for target in targets)
        for key in ("completed", "partial", "unlifted", "invalid", "total")
    }
    snapshot = {
        "schema": "bof3.decomp-status/v1",
        "index_schema": SCHEMA_VERSION,
        "revision": revision,
        "published_at": datetime.now(timezone.utc).isoformat(),
        "counts": counts,
        "targets": targets,
    }
    output.write_text(json.dumps(snapshot, indent=2) + "\n", encoding="utf-8")
    return snapshot
