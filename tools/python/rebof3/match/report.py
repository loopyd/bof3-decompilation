from __future__ import annotations

from pathlib import Path
from typing import Any

from ..jsonio import read_json, write_json


STATUS_ORDER = {
    "exact_match": 0,
    "different": 1,
    "missing_actual": 2,
    "missing_expected": 3,
    "blocked_build_failed": 4,
}


def report_row(payload: dict[str, Any], path: Path) -> dict[str, Any]:
    return {
        "program_path": payload.get("program_path"),
        "entry_hex": payload.get("entry_hex"),
        "status": payload.get("status"),
        "exact_match": bool(payload.get("exact_match")),
        "workspace_dir": payload.get("workspace_dir"),
        "diff_json": str(path),
        "expected_artifact": payload.get("expected_artifact"),
        "actual_artifact": payload.get("actual_artifact"),
    }


def collect_match_reports(match_root: Path) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for path in sorted(match_root.glob("**/diff.json")):
        payload = read_json(path)
        if not isinstance(payload, dict):
            continue
        rows.append(report_row(payload, path))
    rows.sort(
        key=lambda row: (
            STATUS_ORDER.get(str(row.get("status")), 99),
            str(row.get("program_path") or ""),
            str(row.get("entry_hex") or ""),
        )
    )
    return rows


def render_report_tsv(rows: list[dict[str, Any]]) -> str:
    header = [
        "status",
        "program_path",
        "entry_hex",
        "exact_match",
        "workspace_dir",
        "diff_json",
    ]
    lines = ["\t".join(header)]
    for row in rows:
        lines.append("\t".join(str(row.get(column) or "") for column in header))
    return "\n".join(lines) + "\n"


def write_match_report(
    *,
    match_root: Path,
    output_json: Path,
    output_tsv: Path,
) -> dict[str, Any]:
    rows = collect_match_reports(match_root)
    payload = {
        "schema": "rebof3-simple.match-report/v1",
        "match_root": str(match_root),
        "count": len(rows),
        "rows": rows,
    }
    write_json(output_json, payload)
    output_tsv.parent.mkdir(parents=True, exist_ok=True)
    output_tsv.write_text(render_report_tsv(rows), encoding="utf-8")
    return payload
