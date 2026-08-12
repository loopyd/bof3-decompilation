#!/usr/bin/env python3
"""Create, export, and remove parent-managed BOF3 lane worktrees."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


ROOT = Path(__file__).resolve().parents[4]
STATE = ROOT / "out/lift-loop/lanes"
WORKTREES = ROOT.parent / ".bof3-lift-worktrees"
HANDOFFS = ROOT / "out/lift-loop/handoffs"


def git(*args: str, cwd: Path = ROOT, capture: bool = False) -> str:
    result = subprocess.run(("git", *args), cwd=cwd, check=True, text=True, capture_output=capture)
    return result.stdout.strip() if capture else ""


def paths(key: str) -> tuple[Path, Path]:
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]{0,79}", key):
        raise SystemExit("invalid lane key")
    state = (STATE / f"{key}.json").resolve()
    worktree = (WORKTREES / key).resolve()
    state.relative_to(STATE.resolve())
    worktree.relative_to(WORKTREES.resolve())
    return state, worktree


def identity(key: str, selector: str, base: str, worktree: str) -> str:
    return hashlib.sha256(json.dumps([key, selector, base, worktree]).encode()).hexdigest()


def lane_state(key: str, *, check_head: bool = True) -> tuple[Path, Path, dict]:
    state_path, expected = paths(key)
    state = json.loads(state_path.read_text())
    if (
        state.get("key") != key
        or Path(state.get("worktree", "")).resolve() != expected
        or (check_head and state.get("base") != git("rev-parse", "HEAD", cwd=expected, capture=True))
        or state.get("identity") != identity(key, state.get("selector", ""), state.get("base", ""), str(expected))
    ):
        raise SystemExit("lane state identity mismatch")
    if expected not in [Path(line.removeprefix("worktree ")).resolve() for line in git("worktree", "list", "--porcelain", capture=True).splitlines() if line.startswith("worktree ")]:
        raise SystemExit("lane worktree is not registered")
    return state_path, expected, state


def status_entries(worktree: Path) -> list[tuple[str, str]]:
    raw = subprocess.run(("git", "status", "--porcelain", "-z", "--untracked-files=all"), cwd=worktree, check=True, capture_output=True).stdout
    entries = []
    parts = raw.split(b"\0")
    index = 0
    while index < len(parts) and parts[index]:
        entry = parts[index].decode(errors="surrogateescape")
        code, path = entry[:2], entry[3:]
        if code[0] in "RC":
            index += 1
            path = parts[index].decode(errors="surrogateescape")
        entries.append((code, path))
        index += 1
    return entries


def create(args: argparse.Namespace) -> int:
    state_path, worktree = paths(args.key)
    if state_path.exists() or worktree.exists():
        raise SystemExit(f"lane already exists: {args.key}")
    if not args.allow_dirty and git("status", "--porcelain", capture=True):
        raise SystemExit("parent worktree must be clean")
    STATE.mkdir(parents=True, exist_ok=True)
    WORKTREES.mkdir(parents=True, exist_ok=True)
    base = git("rev-parse", "HEAD", capture=True)
    subprocess.run(("git", "worktree", "add", "--detach", str(worktree), base), cwd=ROOT, check=True, stdout=subprocess.DEVNULL)
    try:
        subprocess.run(("python3", ".pi/scripts/bootstrap-bof3-lane.py"), cwd=worktree, check=True)
        state = {"key": args.key, "selector": args.selector, "base": base, "worktree": str(worktree)}
        state["identity"] = identity(args.key, args.selector, base, str(worktree))
        state_path.write_text(json.dumps(state, indent=2) + "\n")
    except BaseException:
        git("worktree", "remove", "--force", str(worktree))
        raise
    print(json.dumps(state))
    return 0


def export(args: argparse.Namespace) -> int:
    state_path, worktree, state = lane_state(args.key)
    if state["selector"] != args.selector:
        raise SystemExit("lane selector mismatch")
    if git("diff", "--cached", "--name-only", cwd=worktree, capture=True):
        raise SystemExit("lane index must be clean")
    entries = status_entries(worktree)
    ignored = subprocess.run(
        ("git", "ls-files", "--others", "--ignored", "--exclude-standard", "-z", "--", "build", "src/emi", ".pi-subagents"),
        cwd=worktree, check=True, capture_output=True,
    ).stdout.split(b"\0")
    forbidden = [path.decode(errors="surrogateescape") for path in ignored if path]
    if forbidden:
        raise SystemExit("forbidden ignored lane artifacts: " + ", ".join(forbidden))
    changed = [code + " " + path for code, path in entries]
    HANDOFFS.mkdir(parents=True, exist_ok=True)
    patch = HANDOFFS / f"{args.key}.patch"
    subprocess.run(("git", "diff", "--binary", "--no-ext-diff", "HEAD"), cwd=worktree, check=True, stdout=patch.open("wb"))
    untracked = [path for code, path in entries if code == "??"]
    if untracked:
        subprocess.run(("git", "add", "-N", "--", *untracked), cwd=worktree, check=True)
        subprocess.run(("git", "diff", "--binary", "--no-ext-diff", "HEAD"), cwd=worktree, check=True, stdout=patch.open("wb"))
        subprocess.run(("git", "reset", "--", *untracked), cwd=worktree, check=True, stdout=subprocess.DEVNULL)
    data = state | {
        "patch": str(patch),
        "patch_sha256": hashlib.sha256(patch.read_bytes()).hexdigest(),
        "changed": changed,
    }
    manifest = HANDOFFS / f"{args.key}.json"
    manifest.write_text(json.dumps(data, indent=2) + "\n")
    print(json.dumps(data))
    return 0


def remove(args: argparse.Namespace) -> int:
    state_path, worktree = paths(args.key)
    registered_paths = {
        Path(line.removeprefix("worktree ")).resolve()
        for line in git("worktree", "list", "--porcelain", capture=True).splitlines()
        if line.startswith("worktree ")
    }
    registered = worktree in registered_paths
    if state_path.exists():
        _, worktree, _ = lane_state(args.key, check_head=False)
        registered = True
    elif not registered:
        raise SystemExit("unknown lane")
    if registered:
        git("worktree", "remove", "--force", str(worktree))
    state_path.unlink(missing_ok=True)
    git("worktree", "prune")
    print(json.dumps({"key": args.key, "removed": True}))
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    make = sub.add_parser("create")
    make.add_argument("--key", required=True)
    make.add_argument("--selector", required=True)
    make.add_argument("--allow-dirty", action="store_true", help=argparse.SUPPRESS)
    export_parser = sub.add_parser("export")
    export_parser.add_argument("--key", required=True)
    export_parser.add_argument("--selector", required=True)
    remove_parser = sub.add_parser("remove")
    remove_parser.add_argument("--key", required=True)
    args = parser.parse_args()
    return create(args) if args.command == "create" else export(args) if args.command == "export" else remove(args)


if __name__ == "__main__":
    raise SystemExit(main())
