from __future__ import annotations

from pathlib import Path
from typing import Any
import sqlite3

from ..jsonio import write_json
from . import state
from .config import HarnessConfig
from .tools import tool_health


def build_report(config: HarnessConfig, conn: sqlite3.Connection) -> dict[str, Any]:
    queued = state.list_targets(conn, limit=10, status="queued")
    blockers = state.list_targets(conn, limit=10, status="blocked")
    return {
        "schema": "rebof3-simple.harness-report/v1",
        "config": str(config.path),
        "database": str(config.database),
        "counts_by_status": state.counts_by_status(conn),
        "counts_by_type": state.counts_by_type(conn),
        "active_claims": state.active_claims(conn),
        "next_targets": queued,
        "blockers": blockers,
        "tool_health": [item.to_dict() for item in tool_health(config)],
    }


def render_report_markdown(report: dict[str, Any]) -> str:
    lines = [
        "# Harness Report",
        "",
        f"- Database: `{report['database']}`",
        "",
        "## Status Counts",
        "",
    ]
    for status, count in report["counts_by_status"].items():
        lines.append(f"- `{status}`: {count}")
    lines.extend(["", "## Type Counts", ""])
    for type_name, count in report["counts_by_type"].items():
        lines.append(f"- `{type_name}`: {count}")
    lines.extend(["", "## Next Targets", ""])
    for target in report["next_targets"]:
        lines.append(
            f"- `{target['id']}` priority `{target['priority']}` - {target['summary']}"
        )
    lines.extend(["", "## Active Claims", ""])
    if not report["active_claims"]:
        lines.append("- none")
    for claim in report["active_claims"]:
        lines.append(f"- `{claim['target_id']}` owned by `{claim['owner']}`")
    lines.extend(["", "## Blockers", ""])
    if not report["blockers"]:
        lines.append("- none")
    for blocker in report["blockers"]:
        lines.append(f"- `{blocker['id']}` - {blocker['summary']}")
    return "\n".join(lines) + "\n"


def write_report(
    config: HarnessConfig, conn: sqlite3.Connection
) -> tuple[dict[str, Any], Path, Path]:
    payload = build_report(config, conn)
    json_path = config.out_dir / "report.json"
    md_path = config.out_dir / "report.md"
    write_json(json_path, payload)
    md_path.write_text(render_report_markdown(payload), encoding="utf-8")
    return payload, json_path, md_path


def choose_resume_action(
    config: HarnessConfig, conn: sqlite3.Connection
) -> dict[str, Any]:
    if not config.database.exists():
        return {
            "action": "setup",
            "command": "bin/harness setup",
            "reason": "harness database does not exist",
        }
    counts = state.counts_by_status(conn)
    total = sum(counts.values())
    if total == 0:
        return {
            "action": "catalog",
            "command": "bin/harness catalog",
            "reason": "no harness targets are recorded",
        }
    claims = state.active_claims(conn)
    if claims:
        claim = claims[0]
        return {
            "action": "continue-claim",
            "command": f"bin/harness target init {claim['target_id']}",
            "reason": f"active claim owned by {claim['owner']}",
            "target_id": claim["target_id"],
        }
    queued = state.list_targets(conn, limit=1, status="queued")
    if queued:
        return {
            "action": "claim",
            "command": "bin/harness claim",
            "reason": "queued targets are available",
            "target_id": queued[0]["id"],
        }
    blocked = state.list_targets(conn, limit=1, status="blocked")
    if blocked:
        return {
            "action": "review-blockers",
            "command": "bin/harness report",
            "reason": "no queued targets remain, but blockers exist",
            "target_id": blocked[0]["id"],
        }
    return {
        "action": "checkpoint",
        "command": "bin/harness checkpoint",
        "reason": "no queued targets or active claims remain",
    }
