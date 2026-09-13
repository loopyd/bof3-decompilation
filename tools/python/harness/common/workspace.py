"""Git-aware workspace baselines for atomic review transactions."""

from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Any

from harness.common.digests import digest
from harness.common.git import read_git
from harness.common.links import read_symlink
from harness.common.files import read_file


def workspace_state(root: Path) -> dict[str, dict[str, str | None]]:
    if not (root / ".git").exists():
        return {}
    command = [
        "--no-optional-locks",
        "status",
        "--porcelain=v1",
        "-z",
        "--untracked-files=all",
    ]
    records = (
        read_git(root, command).encode("utf-8", errors="surrogateescape").split(b"\0")
    )
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
        link = read_symlink(root, name)
        content = (
            link.content if link is not None else read_file(root, name, missing_ok=True)
        )
        state[name] = {
            "status": status,
            "sha256": hashlib.sha256(content).hexdigest()
            if content is not None
            else None,
        }
        if link is not None:
            state[name]["kind"] = "symlink"
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
