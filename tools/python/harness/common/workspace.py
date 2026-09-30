"""Git-aware workspace baselines for atomic review transactions."""

from __future__ import annotations

import hashlib
import json
import base64
import os
from contextlib import contextmanager
from pathlib import Path
from typing import Any

from harness.common.digests import digest
from harness.common.git import read_git
from harness.common.links import SymlinkSnapshot, verify_symlinks
from harness.common.files import atomic_write, safe_unlink
from harness.common.lease import verify_writer
from harness.common.submodules import (
    SubmoduleSnapshot,
    collect_entries,
    collect_gitlinks,
    is_submodule_dirty,
    read_submodule,
    validate_mutations,
    verify_observations,
)
from harness.common.inventory import CaptureBudget
from harness.common.repositories import isolate_repository
from harness.common.trees import (
    capture_worktree,
    describe_changes,
    parse_entries,
    verify_worktree,
)

WorkspaceSnapshot = dict[str, bytes | SymlinkSnapshot | SubmoduleSnapshot]
ARTIFACT_EXCLUSIONS = frozenset({"out", "sessions/subagent-artifacts", ".pi/subagents"})


@contextmanager
def _capture_root(root: Path, indexed: dict, committed: dict, gitlinks: dict):
    gitdir = Path(read_git(root, ["rev-parse", "--absolute-git-dir"]).strip())
    try:
        relative = gitdir.relative_to(root).as_posix()
    except ValueError as error:
        raise ValueError(
            "external root Git directories require explicit support"
        ) from error
    with isolate_repository(root, relative, CaptureBudget()) as (copy, _):
        if parse_entries(copy.query(["ls-files", "--stage", "-z"])) != indexed:
            raise ValueError("root index changed before isolated capture")
        if (
            committed
            and parse_entries(copy.query(["ls-tree", "-r", "-z", "HEAD"]), tree=True)
            != committed
        ):
            raise ValueError("root HEAD changed before isolated capture")
        snapshot, _ = capture_worktree(
            root, None, copy, indexed, set(gitlinks), ARTIFACT_EXCLUSIONS
        )
        yield copy, snapshot
        verify_worktree(root, None, copy.budget, snapshot, set(gitlinks))


def _is_artifact(name: str) -> bool:
    return any(
        name == prefix or name.startswith(prefix + "/")
        for prefix in ARTIFACT_EXCLUSIONS
    )


_WORKSPACE_STATE_CACHE: dict[Path, tuple[tuple, dict]] = {}
_WORKSPACE_BACKUP_CACHE: dict[Path, tuple[tuple, WorkspaceSnapshot]] = {}


def _workspace_token(root: Path) -> tuple | None:
    """Cheap identity of the git index, HEAD and every worktree input.

    Returns None when the shape cannot be summarised safely (external or
    submodule Git directories), which disables caching for that root. File
    identity, size and mtime are enough to detect any harness write, which
    publishes through atomic replacement.
    """
    git = root / ".git"
    if not git.is_dir():
        return None
    parts: list[tuple] = []
    for dirpath, dirnames, filenames in os.walk(root, followlinks=False):
        relative_dir = Path(dirpath).relative_to(root)
        if relative_dir.parts and relative_dir.parts[0] in ARTIFACT_EXCLUSIONS:
            dirnames[:] = []
            continue
        dirnames[:] = sorted(name for name in dirnames if name != ".git")
        for name in sorted(filenames):
            if name == ".git":
                return None
            path = Path(dirpath) / name
            relative = path.relative_to(root).as_posix()
            if _is_artifact(relative):
                continue
            try:
                status = path.stat(follow_symlinks=False)
            except OSError:
                return None
            parts.append(
                (
                    relative,
                    status.st_dev,
                    status.st_ino,
                    status.st_size,
                    status.st_mtime_ns,
                    status.st_mode,
                )
            )
    try:
        head = (git / "HEAD").read_bytes()
    except OSError:
        return None
    parts.append((".git/HEAD", head))
    refs = [".git/index", ".git/packed-refs"]
    if head.startswith(b"ref: "):
        ref = head[5:].strip().decode("utf-8", "surrogateescape")
        if ref and not ref.startswith("/") and ".." not in ref.split("/"):
            refs.append(f".git/{ref}")
    for name in refs:
        try:
            status = (root / name).stat(follow_symlinks=False)
        except OSError:
            parts.append((name, None))
        else:
            parts.append(
                (
                    name,
                    status.st_dev,
                    status.st_ino,
                    status.st_size,
                    status.st_mtime_ns,
                )
            )
    return tuple(parts)


def workspace_state(root: Path) -> dict[str, dict[str, str | None]]:
    if not (root / ".git").exists():
        return {}
    token = _workspace_token(root)
    if token is not None:
        cached = _WORKSPACE_STATE_CACHE.get(root)
        if cached is not None and cached[0] == token:
            import copy as _copy

            return _copy.deepcopy(cached[1])
    state = {}
    indexed, committed = collect_entries(root)
    gitlinks = {
        name: checksum
        for entries in (committed, indexed)
        for name, (mode, checksum) in entries.items()
        if mode == "160000"
    }
    budget = CaptureBudget()
    with _capture_root(root, indexed, committed, gitlinks) as (copy, snapshot):
        for name, status in describe_changes(copy, snapshot, indexed).items():
            if _is_artifact(name) or any(
                name.rstrip("/") == boundary or name.startswith(boundary + "/")
                for boundary in gitlinks
            ):
                continue
            record = snapshot["files"].get(name)
            state[name] = {
                "status": status,
                "sha256": record.get("sha256") if record else None,
            }
            if record and record["kind"] == "symlink":
                state[name].update(
                    kind="symlink",
                    sha256=hashlib.sha256(
                        base64.b64decode(record["content_base64"])
                    ).hexdigest(),
                )
    for name, checksum in gitlinks.items():
        snapshot = read_submodule(root, name, checksum, budget=budget)
        staged = " "
        if indexed.get(name) != committed.get(name):
            staged = (
                "A" if name not in committed else "D" if name not in indexed else "M"
            )
        dirty = is_submodule_dirty(json.loads(snapshot.content))
        if staged != " " or dirty:
            state[name] = {
                "status": staged + ("M" if dirty else " "),
                "kind": "gitlink",
                "sha256": hashlib.sha256(snapshot.content).hexdigest(),
            }
    result = dict(sorted(state.items()))
    if token is not None:
        _WORKSPACE_STATE_CACHE[root] = (token, result)
    return result


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


def workspace_backup(root: Path) -> WorkspaceSnapshot:
    if not (root / ".git").exists():
        return {}
    token = _workspace_token(root)
    if token is not None:
        cached = _WORKSPACE_BACKUP_CACHE.get(root)
        if cached is not None and cached[0] == token:
            import copy as _copy

            return _copy.deepcopy(cached[1])
    backup: WorkspaceSnapshot = _capture_workspace_backup(root)
    if token is not None:
        _WORKSPACE_BACKUP_CACHE[root] = (token, backup)
    return backup


def _capture_workspace_backup(root: Path) -> WorkspaceSnapshot:
    indexed, committed = collect_entries(root)
    gitlinks = {
        name: checksum
        for entries in (committed, indexed)
        for name, (mode, checksum) in entries.items()
        if mode == "160000"
    }
    budget = CaptureBudget()
    backup = {}
    with _capture_root(root, indexed, committed, gitlinks) as (copy, snapshot):
        for name in snapshot["inventory"]:
            if _is_artifact(name) or any(
                name == boundary or name.startswith(boundary + "/")
                for boundary in gitlinks
            ):
                continue
            record = snapshot["files"].get(name)
            if record is None:
                continue
            if record["kind"] == "symlink":
                backup[name] = SymlinkSnapshot(
                    content=base64.b64decode(record["content_base64"]),
                    **{
                        key: value
                        for key, value in record.items()
                        if key not in {"content_base64", "kind"}
                    },
                )
            else:
                backup[name] = (copy.worktree / name).read_bytes()
    for name, checksum in gitlinks.items():
        backup[name] = read_submodule(root, name, checksum, budget=budget)
    return backup


def verify_workspace(root: Path, backup: WorkspaceSnapshot) -> None:
    """Recheck protected links and dependencies before source publication."""
    verify_symlinks(root, backup)
    gitlinks = collect_gitlinks(root)
    if set(gitlinks) != {
        name for name, value in backup.items() if isinstance(value, SubmoduleSnapshot)
    }:
        raise ValueError("workspace submodule boundaries changed during transaction")
    budget = CaptureBudget()
    for name, snapshot in backup.items():
        if isinstance(snapshot, SubmoduleSnapshot):
            facts = json.loads(snapshot.content)
            if gitlinks.get(name) != facts["gitlink"]:
                raise ValueError(f"workspace submodule boundary drifted: {name}")
            verify_observations(root, facts, budget)


def rollback_workspace(root: Path, backup: WorkspaceSnapshot) -> list[str]:
    errors = []
    quarantines = []
    current = workspace_backup(root)
    for name in current.keys() | backup.keys():
        if (
            isinstance(current.get(name), (SymlinkSnapshot, SubmoduleSnapshot))
            or isinstance(backup.get(name), (SymlinkSnapshot, SubmoduleSnapshot))
        ) and current.get(name) != backup.get(name):
            raise ValueError(
                f"workspace dependency or symlink drift requires parent review: {name}"
            )
    for name in sorted(set(current) - set(backup)):
        try:
            verify_writer(root)
            validate_mutations(root, {name})
            quarantine = safe_unlink(root, name, expected=current[name])
            if quarantine is not None:
                quarantines.append(quarantine)
        except (OSError, ValueError) as error:
            errors.append(f"{name}: {error}")
    for name, content in backup.items():
        if current.get(name) == content:
            continue
        try:
            verify_writer(root)
            validate_mutations(root, {name})
            atomic_write(root, name, content, expected=current.get(name))
        except (OSError, ValueError) as error:
            errors.append(f"{name}: {error}")
    if errors:
        raise RuntimeError("type transaction rollback failed: " + "; ".join(errors))
    return quarantines
