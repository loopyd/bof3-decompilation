#!/usr/bin/env python3
"""Record and restore the best owned-file state for one lift-loop lane."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import shutil
import subprocess


ROOT = Path(__file__).resolve().parents[4]


def run_json(*args: str) -> dict:
    result = subprocess.run(args, cwd=ROOT, text=True, capture_output=True)
    if result.returncode not in (0, 1):
        raise subprocess.CalledProcessError(result.returncode, args, result.stdout, result.stderr)
    return json.loads(result.stdout)


def metric(selector: str, reported: float | None) -> dict:
    diff = run_json("bin/asm-diff", "--json", selector)
    first = diff.get("first_mismatch") or {}
    instruction_count = diff.get("instruction_count") or {}
    live_score = float(instruction_count.get("match_percent", 100.0 if diff.get("exact_match") else 0.0))
    return {
        "match_percent": live_score,
        "reported_match_percent": reported,
        "report_matches_live": reported is None or abs(live_score - reported) < 0.005,
        "exact": bool(diff.get("exact_match")),
        "current_size": diff.get("current_size"),
        "original_size": diff.get("original_size"),
        "size_delta": diff.get("size_delta"),
        "source": diff.get("source"),
        "first_mismatch": {
            "original_offset": first.get("original_offset"),
            "current_offset": first.get("current_offset"),
            "original": first.get("original"),
            "current": first.get("current"),
        },
    }


def checkpoint_dir(lane: str) -> Path:
    return ROOT / "out" / "lift-loop" / "checkpoints" / lane


def dirty_paths() -> set[str]:
    status = subprocess.run(
        ("git", "status", "--porcelain", "-z"), cwd=ROOT, capture_output=True, check=True
    ).stdout.decode(errors="surrogateescape").split("\0")
    return {
        entry[3:].split(" -> ")[-1]
        for entry in status
        if entry and entry[3:].split(" -> ")[-1].startswith(
            ("src/", "include/", "config/targets/", "docs/specs/", "docs/agents/")
        )
    }


def capture(args: argparse.Namespace) -> int:
    lane_dir = checkpoint_dir(args.lane)
    attempt_dir = lane_dir / f"attempt-{args.attempt}"
    outcome_path = attempt_dir / "outcome.json"
    if args.scan_worktree:
        best_path = lane_dir / "best.json"
        baseline = set(json.loads(best_path.read_text()).get("worktree_paths", []))
        args.files.extend(sorted(dirty_paths() - baseline))
    if outcome_path.is_file():
        outcome = json.loads(outcome_path.read_text())
        print(json.dumps(outcome))
        return int(outcome["exit_code"])
    if args.attempt == 1 and lane_dir.exists():
        shutil.rmtree(lane_dir)
    if attempt_dir.exists():
        shutil.rmtree(attempt_dir)
    files_dir = attempt_dir / "files"
    files_dir.mkdir(parents=True)
    evidence = None if args.paths_only else metric(args.selector, args.match)
    paths = set(args.files)
    if evidence is not None:
        source = Path(evidence["source"])
        paths.add(source.relative_to(ROOT).as_posix())
    for record_path in lane_dir.glob("attempt-*/record.json"):
        record = json.loads(record_path.read_text())
        paths.update(state["path"] for state in record["files"])
    paths = sorted(paths)
    states = []
    for name in paths:
        path = (ROOT / name).resolve()
        path.relative_to(ROOT)
        exists = path.is_file()
        states.append({"path": name, "exists": exists})
        if exists:
            destination = files_dir / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, destination)
    record = {"selector": args.selector, "attempt": args.attempt, "files": states, "metric": evidence}
    (attempt_dir / "record.json").write_text(json.dumps(record, indent=2) + "\n")
    if args.paths_only:
        outcome = {"paths_recorded": paths, "exit_code": 0}
        outcome_path.write_text(json.dumps(outcome, indent=2) + "\n")
        print(json.dumps(outcome))
        return 0

    assert evidence is not None
    best_path = lane_dir / "best.json"
    best = json.loads(best_path.read_text()) if best_path.is_file() else None
    best_score = best["metric"]["match_percent"] if best else None
    live_score = evidence["match_percent"]
    improved = best is None or live_score > best_score
    observable = best is None or {
        key: value for key, value in evidence.items() if key != "reported_match_percent"
    } != {
        key: value for key, value in best["metric"].items() if key != "reported_match_percent"
    }
    if improved and not args.no_promote:
        best = record | {
            "checkpoint": attempt_dir.relative_to(lane_dir).as_posix(),
            "worktree_paths": sorted(dirty_paths()),
        }
        best_path.write_text(json.dumps(best, indent=2) + "\n")
    below_floor = args.require_at_least is not None and live_score < args.require_at_least
    exit_code = 2 if not evidence["report_matches_live"] else 1 if (args.require_improvement and not improved) or below_floor else 0
    outcome = {
        "improved": improved, "observable_change": observable,
        "current": record, "best": best, "exit_code": exit_code,
    }
    outcome_path.write_text(json.dumps(outcome, indent=2) + "\n")
    print(json.dumps(outcome))
    return exit_code


def restore(args: argparse.Namespace) -> int:
    lane_dir = checkpoint_dir(args.lane)
    best = json.loads((lane_dir / "best.json").read_text())
    source = lane_dir / best["checkpoint"] / "files"
    best_states = {state["path"]: state for state in best["files"]}
    known_paths = set(best_states)
    for record_path in lane_dir.glob("attempt-*/record.json"):
        record = json.loads(record_path.read_text())
        known_paths.update(state["path"] for state in record["files"])
    for name in sorted(known_paths):
        state = best_states.get(name)
        path = (ROOT / name).resolve()
        path.relative_to(ROOT)
        if state and state["exists"]:
            path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source / name, path)
        elif state:
            if path.exists():
                path.unlink()
        else:
            tracked = subprocess.run(
                ("git", "cat-file", "-e", f"HEAD:{name}"), cwd=ROOT
            ).returncode == 0
            if tracked:
                subprocess.run(("git", "restore", "--source=HEAD", "--", name), cwd=ROOT, check=True)
            elif path.exists():
                path.unlink()
    print(json.dumps(best))
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    save = sub.add_parser("capture")
    save.add_argument("--lane", required=True)
    save.add_argument("--selector", required=True)
    save.add_argument("--attempt", required=True, type=int)
    save.add_argument("--match", type=float)
    save.add_argument("--paths-only", action="store_true")
    save.add_argument("--scan-worktree", action="store_true")
    save.add_argument("--no-promote", action="store_true")
    save.add_argument("--require-improvement", action="store_true")
    save.add_argument("--require-at-least", type=float)
    save.add_argument("files", nargs="*")
    load = sub.add_parser("restore")
    load.add_argument("--lane", required=True)
    args = parser.parse_args()
    return capture(args) if args.command == "capture" else restore(args)


if __name__ == "__main__":
    raise SystemExit(main())
