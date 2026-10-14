"""Git state and dirty-path digests for scoped review transactions."""

from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Any

from harness.common.digests import digest
from harness.common.directory import validate_repo_path
from harness.common.files import read_file
from harness.common.git import capture_git_state, read_git, read_head
from harness.common.links import read_symlink
from harness.common.paths import leaf_stat
from harness.common.submodules import collect_entries

WorkspaceState = dict[str, dict[str, Any]]
ARTIFACT_EXCLUSIONS = frozenset(
    {"build", "out", "sessions/subagent-artifacts", ".pi/subagents"}
)
MAX_DIRTY_FILE_BYTES = 128 * 1024 * 1024


def _is_artifact(name: str) -> bool:
    return any(
        name == prefix or name.startswith(prefix + "/")
        for prefix in ARTIFACT_EXCLUSIONS
    )


def _path_state(root: Path, name: str, status: str, gitlinks: set[str]) -> dict:
    if name in gitlinks:
        # Dependency contents are pinned only when a transaction consumes them.
        head = read_head(root / name) if (root / name / ".git").exists() else None
        return {
            "status": status,
            "sha256": hashlib.sha256(head.encode("ascii")).hexdigest()
            if head is not None
            else None,
            "kind": "gitlink",
        }
    link = read_symlink(root, name)
    if link is not None:
        return {
            "status": status,
            "sha256": hashlib.sha256(link.content).hexdigest(),
            "kind": "symlink",
        }
    metadata = leaf_stat(root, name)
    bound = min(metadata.st_size, MAX_DIRTY_FILE_BYTES) if metadata is not None else 0
    content = read_file(root, name, missing_ok=True, max_bytes=bound)
    return {
        "status": status,
        "sha256": hashlib.sha256(content).hexdigest() if content is not None else None,
    }


def workspace_state(root: Path) -> WorkspaceState:
    """Hash reported dirty paths without copying clean sources or dependency trees."""
    before = capture_git_state(root)
    if before is None:
        return {}
    indexed, committed = collect_entries(root)
    if any(
        name == "build" or name.startswith("build/")
        for name in indexed.keys() | committed.keys()
    ):
        raise ValueError("tracked build paths prevent artifact exclusion")
    gitlinks = {
        name
        for entries in (indexed, committed)
        for name, (mode, _) in entries.items()
        if mode == "160000"
    }
    output = read_git(
        root,
        [
            "status",
            "--porcelain=v1",
            "-z",
            "--untracked-files=all",
            "--ignore-submodules=dirty",
        ],
    )
    fields = iter(output.split("\0"))
    state = {}
    for entry in fields:
        if not entry:
            continue
        if len(entry) < 4 or entry[2] != " ":
            raise ValueError("invalid Git workspace status")
        status, name = entry[:2], validate_repo_path(entry[3:])
        if "R" in status or "C" in status:
            original = validate_repo_path(next(fields))
            if "R" in status and not _is_artifact(original):
                state[original] = _path_state(root, original, "D ", gitlinks)
        if not _is_artifact(name):
            state[name] = _path_state(root, name, status, gitlinks)
    if capture_git_state(root) != before:
        raise ValueError("Git index or HEAD changed during workspace observation")
    return state


def verify_workspace(root: Path, state: WorkspaceState, changed: set[str]) -> None:
    """Check unrelated Git state without restoring unrelated files."""
    current = workspace_state(root)
    if {name: value for name, value in current.items() if name not in changed} != {
        name: value for name, value in state.items() if name not in changed
    }:
        raise ValueError("transaction changed an unrelated workspace path")


def workspace_baseline(root: Path) -> dict[str, Any]:
    state = workspace_state(root)
    return {"state": state, "digest": digest(state), "adopted": bool(state)}


def adopted_baseline(root: Path, request: dict[str, Any]) -> dict[str, Any]:
    baseline = workspace_baseline(root)
    adopted = request.get("adopted_baseline")
    if baseline["state"] and adopted != baseline["digest"]:
        raise ValueError("dirty worktree requires the exact adopted_baseline digest")
    if not baseline["state"] and adopted not in {None, baseline["digest"]}:
        raise ValueError("adopted_baseline does not match the clean worktree")
    return baseline
