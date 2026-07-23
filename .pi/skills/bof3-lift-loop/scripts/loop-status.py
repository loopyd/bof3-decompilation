#!/usr/bin/env python3
"""Read-only lift-loop dashboard: worktree safety, index state, journal, candidates."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess
from typing import Any


ROOT = Path(__file__).resolve().parents[4]


def command(root: Path, *args: str) -> dict[str, Any]:
    completed = subprocess.run(args, cwd=root, text=True, capture_output=True)
    result: dict[str, Any] = {"command": list(args), "exit_code": completed.returncode}
    stdout = completed.stdout.strip()
    if stdout:
        try:
            result["json"] = json.loads(stdout)
        except json.JSONDecodeError:
            result["stdout"] = stdout[-3000:]
    if completed.stderr.strip():
        result["stderr"] = completed.stderr.strip()[-3000:]
    return result


def journal(path: Path) -> list[dict[str, str]]:
    if not path.is_file():
        return []
    lines = path.read_text().splitlines()
    if not lines or lines[0] != "function\tstatus\tcommit\tnotes":
        return [{"error": f"invalid journal header: {path}"}]
    return [dict(zip(("function", "status", "commit", "notes"), line.split("\t", 3))) for line in lines[1:] if line]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--selection", choices=("quick-wins", "leafs", "duplicates", "hotspots", "pareto"), default="quick-wins")
    parser.add_argument("--limit", type=int, default=5)
    args = parser.parse_args()
    root = args.root.resolve()
    status = command(root, "git", "status", "--short")
    staged = command(root, "git", "diff", "--cached", "--name-only")
    index = command(root, "bin/rev-query", "status", "--json")
    candidates = command(root, "bin/rev-query", args.selection, "--unlifted", "--detail", "minimal", "--limit", str(args.limit), "--json")
    journal_path = root / "out" / "lift-loop" / "results.tsv"
    records = journal(journal_path)
    counts: dict[str, int] = {}
    for row in records:
        if "status" in row:
            counts[row["status"]] = counts.get(row["status"], 0) + 1
    report = {
        "schema": "bof3.skill-lift-loop-status/v1",
        "worktree": {
            "clean": not status.get("stdout"),
            "changes": status.get("stdout", "").splitlines(),
            "staged": staged.get("stdout", "").splitlines(),
        },
        "journal": {"path": journal_path.relative_to(root).as_posix(), "records": records, "counts": counts},
        "index": index,
        "candidates": candidates,
        "next_action": (
            "inspect index failure; do not select a candidate" if index["exit_code"] else
            "clean or explicitly scope the worktree before dispatch" if status.get("stdout") else
            "select one candidate and run function-brief.py" if candidates["exit_code"] == 0 else
            "repair candidate query before dispatch"
        ),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
