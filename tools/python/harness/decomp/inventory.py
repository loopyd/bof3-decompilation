"""Observed workspace and owned PRE inputs for review-pending lift missions."""

from __future__ import annotations

import base64
import hashlib
import os
import stat
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.directory import open_parent_fd, validate_repo_path
from harness.common.files import read_file
from harness.common.git import git_index_backup, read_git

EXCLUSIONS = ("out/", "sessions/subagent-artifacts/", ".pi/subagents/")


def observe_path(root: Path, name: str) -> dict | None:
    validate_repo_path(name)
    try:
        parent, leaf = open_parent_fd(root, name)
    except FileNotFoundError:
        return None
    try:
        try:
            before = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        except FileNotFoundError:
            return None
        if stat.S_ISLNK(before.st_mode):
            payload = {"symlink": os.readlink(leaf, dir_fd=parent)}
        elif stat.S_ISDIR(before.st_mode):
            payload = {"directory": True}
        elif stat.S_ISREG(before.st_mode):
            descriptor = os.open(
                leaf, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK, dir_fd=parent
            )
            with os.fdopen(descriptor, "rb") as stream:
                opened = os.fstat(stream.fileno())
                if (opened.st_dev, opened.st_ino) != (before.st_dev, before.st_ino):
                    raise ValueError(f"lift inventory open raced: {name}")
                checksum = hashlib.sha256()
                while chunk := stream.read(1024 * 1024):
                    check_deadline()
                    checksum.update(chunk)
                payload = {"sha256": checksum.hexdigest()}
        else:
            raise ValueError(f"unsupported lift inventory entry: {name}")
        after = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        fields = (
            "st_dev",
            "st_ino",
            "st_mode",
            "st_nlink",
            "st_uid",
            "st_gid",
            "st_size",
            "st_mtime_ns",
            "st_ctime_ns",
        )
        if any(getattr(before, field) != getattr(after, field) for field in fields):
            raise ValueError(f"lift inventory observation raced: {name}")
        return {
            **payload,
            "device": after.st_dev,
            "inode": after.st_ino,
            "mode": stat.S_IMODE(after.st_mode),
            "links": after.st_nlink,
            "uid": after.st_uid,
            "gid": after.st_gid,
            "mtime_ns": after.st_mtime_ns,
            "ctime_ns": after.st_ctime_ns,
        }
    finally:
        os.close(parent)


def list_workspace(root: Path) -> set[str]:
    names = set()
    pending = [""]
    while pending:
        prefix = pending.pop()
        check_deadline()
        output = read_git(
            root,
            [
                "-C",
                str(root / prefix),
                "ls-files",
                "-co",
                "--exclude-standard",
                "-z",
            ],
        )
        for leaf in output.split("\0"):
            if not leaf:
                continue
            name = f"{prefix}/{leaf}" if prefix else leaf
            validate_repo_path(name)
            if name in names or name.startswith(EXCLUSIONS):
                continue
            names.add(name)
            try:
                metadata = (root / name).lstat()
            except FileNotFoundError:
                continue
            if stat.S_ISDIR(metadata.st_mode) and (root / name / ".git").exists():
                pending.append(name)
    return names


def capture_inventory(root: Path, paths: list[str]) -> dict:
    check_deadline()
    if not (root / ".git").exists():
        raise ValueError("native lifting requires a Git-aware workspace")
    names = list_workspace(root) | set(paths)
    entries = {}
    for name in sorted(names):
        check_deadline()
        entries[name] = observe_path(root, name)
    index = git_index_backup(root)
    if index is None or index.path.with_name(index.path.name + ".lock").exists():
        raise ValueError("lift mission requires an available unlocked Git index")
    return {
        "entries": entries,
        "index": {
            "path": str(index.path),
            "state": list(index.state) if index.state is not None else None,
            "content_base64": base64.b64encode(index.content).decode()
            if index.content is not None
            else None,
        },
    }


def capture_owned(root: Path, paths: list[str], inventory: dict) -> dict:
    owned = {}
    for name in paths:
        state = inventory["entries"][name]
        if state is not None and ("sha256" not in state or state["links"] != 1):
            raise ValueError(f"lift path is not a single-link regular file: {name}")
        content = read_file(root, name, missing_ok=True)
        if observe_path(root, name) != state or (
            state is not None and hashlib.sha256(content).hexdigest() != state["sha256"]
        ):
            raise ValueError(f"lift PRE changed during capture: {name}")
        owned[name] = {
            "state": state,
            "content_base64": base64.b64encode(content).decode()
            if content is not None
            else None,
        }
    return owned


def verify_scope(before: dict, after: dict, paths: list[str]) -> list[str]:
    if before["index"] != after["index"]:
        raise ValueError("lift Git index drifted; preserve state for parent inspection")
    changed = sorted(
        name
        for name in set(before["entries"]) | set(after["entries"])
        if before["entries"].get(name) != after["entries"].get(name)
    )
    outside = sorted(set(changed) - set(paths))
    if outside:
        raise ValueError(
            f"out-of-scope lift changes retained for inspection: {outside}"
        )
    for name in paths:
        state = after["entries"].get(name)
        if state is not None and ("sha256" not in state or state["links"] != 1):
            raise ValueError(f"unsafe retained lift path: {name}")
        original = before["entries"].get(name)
        if original is not None and (
            state is None
            or any(original[key] != state[key] for key in ("mode", "uid", "gid"))
        ):
            raise ValueError(f"lift removed or changed owned mode/ownership: {name}")
    return changed
