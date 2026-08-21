"""Git-aware workspace baselines for atomic review transactions."""

from __future__ import annotations

import subprocess
from pathlib import Path
from typing import Any

from ..domain.receipts import sha256_file
from .type_candidate_review import digest


def workspace_state(root: Path) -> dict[str, dict[str, str | None]]:
    if not (root / ".git").exists():
        return {}
    result = subprocess.run(
        ["git", "status", "--porcelain=v1", "-z", "--untracked-files=all"],
        cwd=root,
        capture_output=True,
        check=True,
    )
    records = result.stdout.split(b"\0")
    state = {}
    index = 0
    while index < len(records) and records[index]:
        record = records[index]
        status = record[:2].decode("ascii")
        name = record[3:].decode(errors="surrogateescape")
        index += 1
        if status[0] in "RC" or status[1] in "RC":
            index += 1
        if name.startswith(("out/", "sessions/subagent-artifacts/", ".pi/subagents/")):
            continue
        path = root / name
        state[name] = {
            "status": status,
            "sha256": sha256_file(path) if path.is_file() else None,
        }
    return dict(sorted(state.items()))


def workspace_baseline(root: Path) -> dict[str, Any]:
    current = workspace_state(root)
    return {"state": current, "digest": digest(current), "adopted": bool(current)}


def adopted_baseline(root: Path, request: dict[str, Any]) -> dict[str, Any]:
    baseline = workspace_baseline(root)
    adopted = request.get("adopted_baseline")
    if baseline["state"] and adopted != baseline["digest"]:
        raise ValueError("dirty worktree requires current adopted_baseline")
    if not baseline["state"] and adopted not in {None, digest({})}:
        raise ValueError("adopted_baseline does not match clean worktree")
    return baseline
