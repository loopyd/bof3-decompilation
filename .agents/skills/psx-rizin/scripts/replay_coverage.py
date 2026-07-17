#!/usr/bin/env python3
"""Validate and summarize a PSX replay/scenario coverage matrix."""

from __future__ import annotations

import argparse
import csv
import sys
from collections import Counter
from pathlib import Path

VALID_STATUSES = {
    "passed",
    "failed",
    "blocked",
    "duplicate",
    "not_applicable",
    "pending",
}
REQUIRED_COLUMNS = {
    "replay_id",
    "source",
    "status",
    "emulator",
    "disc_sha256",
    "bios_sha256",
    "expected_event",
    "functions_hit",
    "trace_path",
}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("matrix", type=Path)
    parser.add_argument("--report", type=Path)
    parser.add_argument("--allow-incomplete", action="store_true")
    args = parser.parse_args()

    errors: list[str] = []
    rows: list[dict[str, str]] = []
    try:
        with args.matrix.open("r", encoding="utf-8-sig", newline="") as stream:
            reader = csv.DictReader(stream)
            fields = set(reader.fieldnames or [])
            missing_columns = sorted(REQUIRED_COLUMNS - fields)
            if missing_columns:
                errors.append("missing columns: " + ", ".join(missing_columns))
            rows = [{key: (value or "").strip() for key, value in row.items()} for row in reader]
    except OSError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    ids: Counter[str] = Counter()
    statuses: Counter[str] = Counter()
    missing_fields: Counter[str] = Counter()
    function_hits: set[str] = set()
    overlays: set[str] = set()

    for number, row in enumerate(rows, start=2):
        replay_id = row.get("replay_id", "")
        if replay_id:
            ids[replay_id] += 1
        else:
            errors.append(f"row {number}: empty replay_id")

        status = row.get("status", "").lower()
        statuses[status or "<empty>"] += 1
        if status not in VALID_STATUSES:
            errors.append(f"row {number}: invalid status {status!r}")

        for field in REQUIRED_COLUMNS:
            if not row.get(field, ""):
                missing_fields[field] += 1

        for name in row.get("functions_hit", "").replace(";", ",").split(","):
            if name.strip():
                function_hits.add(name.strip())
        for name in row.get("overlays_loaded", "").replace(";", ",").split(","):
            if name.strip():
                overlays.add(name.strip())

    for replay_id, count in ids.items():
        if count > 1:
            errors.append(f"duplicate replay_id {replay_id!r}: {count} rows")

    completed = sum(statuses[name] for name in ("passed", "failed", "blocked", "duplicate", "not_applicable"))
    total = len(rows)
    lines = [
        "# Replay coverage report",
        "",
        f"- Matrix: `{args.matrix}`",
        f"- Total scenarios: **{total}**",
        f"- Explicitly resolved: **{completed}**",
        f"- Pending: **{statuses['pending']}**",
        f"- Unique functions hit/listed: **{len(function_hits)}**",
        f"- Unique overlays loaded/listed: **{len(overlays)}**",
        "",
        "## Status counts",
        "",
        "| status | count |",
        "|---|---:|",
    ]
    for status, count in sorted(statuses.items()):
        lines.append(f"| {status} | {count} |")

    lines.extend(["", "## Missing required fields", "", "| field | rows missing |", "|---|---:|"])
    for field in sorted(REQUIRED_COLUMNS):
        lines.append(f"| {field} | {missing_fields[field]} |")

    lines.extend(["", "## Validation errors", ""])
    if errors:
        lines.extend(f"- {error}" for error in errors)
    else:
        lines.append("None.")

    lines.extend(
        [
            "",
            "## Interpretation",
            "",
            "A scenario is covered only when its status and evidence paths are explicit. "
            "A function name in `functions_hit` proves only that the scenario recorded it, not full branch coverage.",
        ]
    )

    report = "\n".join(lines) + "\n"
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(report, encoding="utf-8")
        print(args.report)
    else:
        print(report, end="")

    incomplete = bool(errors) or statuses["pending"] > 0 or any(missing_fields.values())
    return 0 if args.allow_incomplete or not incomplete else 1


if __name__ == "__main__":
    raise SystemExit(main())
