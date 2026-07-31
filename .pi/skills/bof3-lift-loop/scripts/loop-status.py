#!/usr/bin/env python3
"""Lift-loop dashboard — inspection only by default; --recover repairs stale evidence."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess
import sys
from typing import Any


ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / "tools" / "python"))

from harness.domain import load_target_manifests  # noqa: E402


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
    records = []
    for number, line in enumerate(lines[1:], start=2):
        if not line:
            continue
        fields = line.split("\t", 3)
        if len(fields) != 4:
            records.append({"error": f"invalid journal row {number}: {path}"})
        else:
            function, status, commit, notes = fields
            records.append(
                {
                    "function": function,
                    "status": status,
                    "commit": commit,
                    "notes": notes,
                }
            )
    return records


def targets(root: Path) -> list[str]:
    """Read target IDs from typed manifests without generated index state."""
    return sorted(load_target_manifests(root))


def snapshot_statuses(
    root: Path, all_targets: list[str] | None = None
) -> tuple[list[dict[str, Any]], list[str]]:
    tids = all_targets if all_targets is not None else targets(root)
    results = [
        command(root, "bin/rz-project", "status", target, "--json") for target in tids
    ]
    return results, [
        result["command"][2]
        for result in results
        if not result.get("json", {}).get("fresh", False)
    ]


def recover_index(
    root: Path, stale: list[str], all_targets: list[str]
) -> dict[str, Any]:
    """Serially repair snapshots; recheck before the one index rebuild."""
    analyses = [command(root, "bin/rz-project", "analyze", target) for target in stale]
    if any(result["exit_code"] for result in analyses):
        return {"analyses": analyses, "index_rebuild": None, "recheck_snapshots": []}
    recheck, stale_after = snapshot_statuses(root, all_targets)
    if stale_after:
        return {
            "analyses": analyses,
            "index_rebuild": None,
            "recheck_snapshots": recheck,
        }
    rebuild = command(root, "bin/index")
    return {
        "analyses": analyses,
        "index_rebuild": rebuild,
        "recheck_snapshots": recheck,
    }


def main(
    *, _targets_override: list[str] | None = None, _argv: list[str] | None = None
) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument(
        "--selection",
        choices=("quick-wins", "leafs", "duplicates", "hotspots", "pareto"),
        default="quick-wins",
    )
    parser.add_argument("--limit", type=int, default=5)
    parser.add_argument(
        "--recover",
        action="store_true",
        help="repair stale generated evidence before querying candidates",
    )
    args = parser.parse_args(args=_argv)
    root = args.root.resolve()
    status = command(root, "git", "status", "--short")
    staged = command(root, "git", "diff", "--cached", "--name-only")
    all_targets = _targets_override if _targets_override is not None else targets(root)
    snapshots, stale = snapshot_statuses(root, all_targets)
    recovery = None
    suppressed: dict[str, Any] | None = None
    index: dict[str, Any] = {"command": ["(skipped)"], "exit_code": 1}
    candidates: dict[str, Any] = {"command": ["(skipped)"], "exit_code": 1}

    if stale:
        if not args.recover:
            suppressed = {
                "reason": "stale_snapshot",
                "stale_targets": stale,
                "hint": "use --recover to repair stale generated evidence",
            }
        else:
            recovery = recover_index(root, stale, all_targets)
            rebuild = recovery["index_rebuild"]
            if rebuild is None or rebuild["exit_code"]:
                suppressed = {
                    "reason": "recovery_incomplete",
                    "stale_targets": stale,
                    "hint": "analysis or index recovery failed; check recovery logs above",
                }
            else:
                snapshots, stale_after = snapshot_statuses(root, all_targets)
                if stale_after:
                    suppressed = {
                        "reason": "recovery_incomplete",
                        "stale_targets": stale_after,
                        "hint": "one or more targets still stale after --recover; check recovery logs above",
                    }

    if suppressed is None:
        index = command(root, "bin/rev-query", "--json", "status")
        if index["exit_code"] and args.recover:
            recovery = recover_index(root, [], all_targets)
            rebuild = recovery["index_rebuild"]
            if rebuild is not None and rebuild["exit_code"] == 0:
                index = command(root, "bin/rev-query", "--json", "status")
        if index["exit_code"]:
            suppressed = {
                "reason": "stale_or_invalid_index",
                "hint": "repair reverse-index evidence before selecting a candidate",
            }
        else:
            candidates = command(
                root,
                "bin/rev-query",
                args.selection,
                "--unlifted",
                "--detail",
                "minimal",
                "--limit",
                str(args.limit),
                "--json",
            )

    journal_path = root / "out" / "lift-loop" / "results.tsv"
    records = journal(journal_path)
    counts: dict[str, int] = {}
    for row in records:
        if "status" in row:
            counts[row["status"]] = counts.get(row["status"], 0) + 1
    report = {
        "schema": "bof3.skill-lift-loop-status/v1",
        "snapshots": snapshots,
        "stale_targets": stale,
        "worktree": {
            "clean": not status.get("stdout"),
            "changes": status.get("stdout", "").splitlines(),
            "staged": staged.get("stdout", "").splitlines(),
        },
        "journal": {
            "path": journal_path.relative_to(root).as_posix(),
            "records": records,
            "counts": counts,
        },
        "index": index,
        "candidates": candidates,
        "suppressed_candidates": suppressed,
        "recovery": recovery,
        "next_action": (
            "inspect stale snapshot evidence; run with --recover to repair"
            if suppressed and recovery is None
            else "inspect recovery failure; do not select a candidate"
            if suppressed
            else "clean or explicitly scope the worktree before dispatch"
            if status.get("stdout")
            else "select one candidate and run function-brief.py"
            if candidates["exit_code"] == 0
            else "inspect candidate query failure"
        ),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
