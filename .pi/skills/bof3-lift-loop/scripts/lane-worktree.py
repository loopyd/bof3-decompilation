#!/usr/bin/env python3
"""Create, export, and remove parent-managed BOF3 lane worktrees."""

from __future__ import annotations

import argparse
import fcntl
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess


ROOT = Path(__file__).resolve().parents[4]
STATE = ROOT / "out/lift-loop/lanes"
WORKTREES = ROOT.parent / ".bof3-lift-worktrees"
HANDOFFS = ROOT / "out/lift-loop/handoffs"
LEDGERS = ROOT / "out/lift-loop/experiment-ledgers"
SESSIONS = ROOT / ".pi-subagents/sessions/lift-loop"


def ledger_path(selector: str) -> Path:
    return LEDGERS / f"{hashlib.sha256(selector.encode()).hexdigest()}.jsonl"


def read_ledger(selector: str) -> list[dict]:
    path = ledger_path(selector)
    if not path.exists():
        return []
    return [json.loads(line) for line in path.read_text().splitlines() if line.strip()]


def record(args: argparse.Namespace) -> int:
    _, _, state = lane_state(args.key, check_head=False)
    entry = json.loads(args.entry_json)
    if state["selector"] != args.selector or entry.get("selector") != args.selector:
        raise SystemExit("ledger selector mismatch")
    if entry.get("lane_key") != args.key or not isinstance(entry.get("row"), dict):
        raise SystemExit("invalid ledger entry")
    LEDGERS.mkdir(parents=True, exist_ok=True)
    path = ledger_path(args.selector)
    with path.open("a") as output:
        fcntl.flock(output, fcntl.LOCK_EX)
        output.write(json.dumps(entry, sort_keys=True) + "\n")
    print(json.dumps({"recorded": True, "entries": len(read_ledger(args.selector))}))
    return 0


def ledger(args: argparse.Namespace) -> int:
    print(json.dumps({"selector": args.selector, "entries": read_ledger(args.selector)}))
    return 0


INTEGRATION_LOCK = ROOT / ".git/bof3-lift-integrate.lock"
FORBIDDEN_PREFIXES = ("build/", "src/emi/", ".pi-subagents/", "out/")


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


def session_path(key: str) -> Path:
    path = SESSIONS / key
    if path.parent.resolve() != SESSIONS.resolve() or path.is_symlink():
        raise SystemExit("invalid lane session path")
    return path


def identity(key: str, selector: str, base: str, worktree: str, session_dir: str) -> str:
    return hashlib.sha256(json.dumps([key, selector, base, worktree, session_dir]).encode()).hexdigest()


def lane_state(key: str, *, check_head: bool = True) -> tuple[Path, Path, dict]:
    state_path, expected = paths(key)
    expected_session = session_path(key)
    state = json.loads(state_path.read_text())
    if (
        state.get("key") != key
        or Path(state.get("worktree", "")).resolve() != expected
        or Path(state.get("session_dir", "")) != expected_session
        or state.get("launch") != {"cwd": str(expected), "worktree": False, "sessionDir": str(expected_session), "async": True}
        or not expected_session.is_dir()
        or expected_session.is_symlink()
        or (check_head and state.get("base") != git("rev-parse", "HEAD", cwd=expected, capture=True))
        or state.get("identity") != identity(key, state.get("selector", ""), state.get("base", ""), str(expected), str(expected_session))
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
    session_dir = session_path(args.key)
    if state_path.exists() or worktree.exists() or session_dir.exists() or session_dir.is_symlink():
        raise SystemExit(f"lane already exists: {args.key}")
    if not args.allow_dirty and git("status", "--porcelain", capture=True):
        raise SystemExit("parent worktree must be clean")
    STATE.mkdir(parents=True, exist_ok=True)
    WORKTREES.mkdir(parents=True, exist_ok=True)
    base = git("rev-parse", "HEAD", capture=True)
    subprocess.run(("git", "worktree", "add", "--detach", str(worktree), base), cwd=ROOT, check=True, stdout=subprocess.DEVNULL)
    try:
        subprocess.run(("python3", ".pi/scripts/bootstrap-bof3-lane.py"), cwd=worktree, check=True)
        session_dir.mkdir(parents=True)
        launch = {"cwd": str(worktree), "worktree": False, "sessionDir": str(session_dir), "async": True}
        state = {"key": args.key, "selector": args.selector, "base": base, "worktree": str(worktree), "session_dir": str(session_dir), "launch": launch}
        state["identity"] = identity(args.key, args.selector, base, str(worktree), str(session_dir))
        state_path.write_text(json.dumps(state, indent=2) + "\n")
    except BaseException:
        git("worktree", "remove", "--force", str(worktree))
        state_path.unlink(missing_ok=True)
        if session_dir.is_dir() and not session_dir.is_symlink():
            shutil.rmtree(session_dir)
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


def integrate(args: argparse.Namespace) -> int:
    INTEGRATION_LOCK.parent.mkdir(parents=True, exist_ok=True)
    with INTEGRATION_LOCK.open("w") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        if git("status", "--porcelain", capture=True):
            raise SystemExit("parent worktree must be clean")
        state_path, worktree, state = lane_state(args.key)
        if state["selector"] != args.selector:
            raise SystemExit("lane selector mismatch")
        if state["base"] != git("rev-parse", "HEAD", capture=True):
            raise SystemExit("parent HEAD advanced; rerun lane")
        export(argparse.Namespace(key=args.key, selector=args.selector))
        manifest = json.loads((HANDOFFS / f"{args.key}.json").read_text())
        patch = Path(manifest["patch"])
        if hashlib.sha256(patch.read_bytes()).hexdigest() != manifest["patch_sha256"]:
            raise SystemExit("handoff patch digest mismatch")
        changed_paths = [entry[3:] for entry in manifest["changed"]]
        if not changed_paths:
            raise SystemExit("lane has no changes to integrate")
        forbidden = [path for path in changed_paths if path.startswith(FORBIDDEN_PREFIXES) or Path(path).is_absolute()]
        if forbidden:
            raise SystemExit("forbidden lane paths: " + ", ".join(forbidden))
        try:
            subprocess.run(("git", "apply", "--check", str(patch)), cwd=ROOT, check=True)
            subprocess.run(("git", "apply", str(patch)), cwd=ROOT, check=True)
            subprocess.run(("git", "diff", "--check"), cwd=ROOT, check=True)
            git("add", "--", *changed_paths)
            staged = git("diff", "--cached", "--name-only", capture=True).splitlines()
            if sorted(staged) != sorted(changed_paths):
                raise RuntimeError("staged paths differ from reviewed handoff")
            git("commit", "-m", args.message)
        except BaseException:
            git("reset", "--hard", "HEAD")
            subprocess.run(("git", "clean", "-fd", "--", *changed_paths), cwd=ROOT, check=False, stdout=subprocess.DEVNULL)
            raise
        commit = git("rev-parse", "HEAD", capture=True)
        removed = True
        try:
            git("worktree", "remove", "--force", str(worktree))
            state_path.unlink(missing_ok=True)
            session_dir = session_path(args.key)
            if session_dir.is_dir():
                shutil.rmtree(session_dir)
            git("worktree", "prune")
        except BaseException:
            removed = False
        print(json.dumps({"key": args.key, "selector": args.selector, "integrated": True, "commit": commit, "lane_removed": removed}))
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
    session_dir = session_path(args.key)
    if session_dir.is_dir():
        shutil.rmtree(session_dir)
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
    integrate_parser = sub.add_parser("integrate")
    integrate_parser.add_argument("--key", required=True)
    integrate_parser.add_argument("--selector", required=True)
    integrate_parser.add_argument("--message", required=True)
    remove_parser = sub.add_parser("remove")
    remove_parser.add_argument("--key", required=True)
    record_parser = sub.add_parser("record")
    record_parser.add_argument("--key", required=True)
    record_parser.add_argument("--selector", required=True)
    record_parser.add_argument("--entry-json", required=True)
    ledger_parser = sub.add_parser("ledger")
    ledger_parser.add_argument("--selector", required=True)
    args = parser.parse_args()
    commands = {"create": create, "export": export, "integrate": integrate, "remove": remove, "record": record, "ledger": ledger}
    return commands[args.command](args)


if __name__ == "__main__":
    raise SystemExit(main())
