#!/usr/bin/env python3
"""Lift-loop dashboard that repairs stale generated Rizin/index evidence by default."""

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


def targets(root: Path) -> list[str]:
    """Read target IDs from manifests without depending on generated index state."""

    import tomllib

    return sorted(
        tomllib.loads(path.read_text())["id"]
        for path in (root / "config" / "targets").glob("**/target.toml")
    )


def recover_index(root: Path) -> dict[str, Any]:
    """Refresh only ignored/generated analysis artifacts, never authored files."""

    snapshots = [command(root, "bin/rz-project", "status", target, "--json") for target in targets(root)]
    stale = [
        result["command"][2]
        for result in snapshots
        if not result.get("json", {}).get("fresh", False)
    ]
    analyses = [command(root, "bin/rz-project", "analyze", target) for target in stale]
    rebuild = command(root, "bin/index")
    return {"snapshot_status": snapshots, "stale_targets": stale, "analyses": analyses, "index_rebuild": rebuild}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--selection", choices=("quick-wins", "leafs", "duplicates", "hotspots", "pareto"), default="quick-wins")
    parser.add_argument("--limit", type=int, default=5)
    parser.add_argument("--no-recover", action="store_true", help="report stale generated evidence without rebuilding it")
    args = parser.parse_args()
    root = args.root.resolve()
    status = command(root, "git", "status", "--short")
    staged = command(root, "git", "diff", "--cached", "--name-only")
    index = command(root, "bin/rev-query", "--json", "status")
    candidates = command(root, "bin/rev-query", args.selection, "--unlifted", "--detail", "minimal", "--limit", str(args.limit), "--json")
    recovery = None
    if candidates["exit_code"] and not args.no_recover:
        recovery = recover_index(root)
        index = command(root, "bin/rev-query", "--json", "status")
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
        "recovery": recovery,
        "next_action": (
            "inspect index failure after generated-evidence recovery; do not select a candidate" if index["exit_code"] else
            "clean or explicitly scope the worktree before dispatch" if status.get("stdout") else
            "select one candidate and run function-brief.py" if candidates["exit_code"] == 0 else
            "inspect candidate query failure after generated-evidence recovery"
        ),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
