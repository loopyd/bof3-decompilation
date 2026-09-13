"""Durable workspace and Git-index guards for parent-mediated recovery."""

from __future__ import annotations

import base64
import hashlib
import os
import stat
from pathlib import Path
from typing import Any

from harness.common.directory import validate_repo_path
from harness.common.git import GitIndexSnapshot, git_index_backup
from harness.common.paths import leaf_stat, file_state
from harness.common.workspace import (
    WorkspaceSnapshot,
    workspace_backup,
    workspace_state,
)
from harness.common.links import SymlinkSnapshot, read_symlink
from harness.common.submodules import (
    SubmoduleSnapshot,
    collect_gitlinks,
    read_submodule,
    validate_snapshot,
)
from harness.common.inventory import CaptureBudget

LEGACY_SCHEMA = "bof3.recovery-safeguards/v1"
LINK_SCHEMA = "bof3.recovery-safeguards/v2"
SCHEMA = "bof3.recovery-safeguards/v3"


def verify_restored_state(
    root: Path, manifest: dict, index: GitIndexSnapshot | None, safeguards: dict | None
) -> None:
    """Verify owned PRE and the adopted workspace/index without restoring external state."""
    if (
        file_state(root, set(manifest["allowed_paths"])) != manifest["pre_state"]
        or git_index_backup(root) != index
        or workspace_state(root) != manifest["workspace_baseline"]["state"]
        or (
            safeguards is not None
            and inspect_safeguards(root, safeguards, set())["matches"] is not True
        )
    ):
        raise ValueError("workspace or Git index changed concurrently")


def _encode_index(backup: GitIndexSnapshot) -> dict[str, Any]:
    return {
        "path": str(backup.path),
        "content_base64": base64.b64encode(backup.content).decode("ascii")
        if backup.content is not None
        else None,
        "state": list(backup.state) if backup.state is not None else None,
    }


def _has_index_lock(backup: GitIndexSnapshot) -> bool:
    try:
        os.lstat(backup.path.with_name(backup.path.name + ".lock"))
    except FileNotFoundError:
        return False
    return True


def _describe_workspace(
    root: Path,
    workspace: WorkspaceSnapshot,
    changed: set[str],
    *,
    include_content: bool = False,
) -> dict:
    result = {}
    gitlinks = (
        collect_gitlinks(root)
        if any(isinstance(value, SubmoduleSnapshot) for value in workspace.values())
        else {}
    )
    budget = CaptureBudget()
    for name, content in workspace.items():
        if isinstance(content, SubmoduleSnapshot):
            if any(
                path == name
                or path.startswith(name + "/")
                or name.startswith(path + "/")
                for path in changed
            ):
                raise ValueError(
                    f"transaction mutation overlaps protected submodule: {name}"
                )
            if (
                name not in gitlinks
                or read_submodule(root, name, gitlinks[name], budget=budget) != content
            ):
                raise ValueError(f"recovery safeguard submodule drifted: {name}")
            result[name] = {
                "kind": "gitlink",
                "sha256": hashlib.sha256(content.content).hexdigest(),
            }
            if include_content:
                result[name]["content_base64"] = base64.b64encode(
                    content.content
                ).decode("ascii")
            continue
        if name in changed:
            continue
        validate_repo_path(name)
        if isinstance(content, SymlinkSnapshot):
            if read_symlink(root, name) != content:
                raise ValueError(f"recovery safeguard symlink drifted: {name}")
            result[name] = {
                "kind": "symlink",
                "sha256": hashlib.sha256(content.content).hexdigest(),
                "mode": stat.S_IMODE(content.mode),
                **{
                    key: getattr(content, key)
                    for key in (
                        "device",
                        "inode",
                        "uid",
                        "gid",
                        "links",
                        "mtime_ns",
                        "ctime_ns",
                    )
                },
            }
            if include_content:
                result[name]["content_base64"] = base64.b64encode(
                    content.content
                ).decode("ascii")
            continue
        metadata = leaf_stat(root, name)
        if metadata is None:
            raise ValueError(f"recovery safeguard path disappeared: {name}")
        result[name] = {
            "kind": "file",
            "sha256": hashlib.sha256(content).hexdigest(),
            "mode": stat.S_IMODE(metadata.st_mode),
            "device": metadata.st_dev,
            "inode": metadata.st_ino,
            "uid": metadata.st_uid,
            "gid": metadata.st_gid,
            "links": metadata.st_nlink,
        }
        if include_content:
            result[name]["content_base64"] = base64.b64encode(content).decode("ascii")
    return result


def capture_safeguards(
    root: Path,
    changed: set[str],
    workspace: WorkspaceSnapshot | None,
    index: GitIndexSnapshot | None,
) -> dict[str, Any] | None:
    if index is None:
        return None
    if workspace is None:
        raise ValueError("recovery index snapshot requires a workspace snapshot")
    if _has_index_lock(index):
        raise ValueError("Git index lock prevents recovery safeguard capture")
    return {
        "schema": SCHEMA,
        "untouched": _describe_workspace(
            root, workspace, changed, include_content=True
        ),
        "index": _encode_index(index),
    }


def _validate_safeguards(value: object, changed: set[str]) -> dict:
    if not isinstance(value, dict) or set(value) != {"schema", "untouched", "index"}:
        raise ValueError("invalid recovery safeguards")
    if value["schema"] not in {SCHEMA, LINK_SCHEMA, LEGACY_SCHEMA} or not isinstance(
        value["untouched"], dict
    ):
        raise ValueError("invalid recovery safeguards schema")
    for name, entry in value["untouched"].items():
        validate_repo_path(name)
        fields = {
            "sha256",
            "mode",
            "device",
            "inode",
            "uid",
            "gid",
            "links",
            "content_base64",
        }
        if value["schema"] != LEGACY_SCHEMA:
            if not isinstance(entry, dict) or entry.get("kind") not in {
                "file",
                "symlink",
            } | ({"gitlink"} if value["schema"] == SCHEMA else set()):
                raise ValueError("invalid recovery untouched kind")
            fields.add("kind")
            if entry["kind"] == "symlink":
                fields.update({"mtime_ns", "ctime_ns"})
            if entry["kind"] == "gitlink":
                fields = {"kind", "sha256", "content_base64"}
        if name in changed or not isinstance(entry, dict) or set(entry) != fields:
            raise ValueError("invalid recovery untouched path")
        checksum = entry["sha256"]
        if (
            not isinstance(checksum, str)
            or len(checksum) != 64
            or any(character not in "0123456789abcdef" for character in checksum)
            or any(
                type(entry[key]) is not int or entry[key] < 0
                for key in entry
                if key not in {"sha256", "content_base64", "kind"}
            )
        ):
            raise ValueError("invalid recovery untouched identity")
        if entry.get("kind") != "gitlink" and (
            entry["mode"] > 0o7777 or entry["links"] < 1
        ):
            raise ValueError("invalid recovery untouched mode or link count")
        if not isinstance(entry["content_base64"], str):
            raise ValueError("invalid recovery untouched encoding")
        content = base64.b64decode(entry["content_base64"], validate=True)
        if (
            base64.b64encode(content).decode("ascii") != entry["content_base64"]
            or hashlib.sha256(content).hexdigest() != checksum
        ):
            raise ValueError("invalid recovery untouched image")
        if entry.get("kind") == "gitlink":
            if any(
                path == name
                or path.startswith(name + "/")
                or name.startswith(path + "/")
                for path in changed
            ):
                raise ValueError("recovery mutation overlaps protected submodule")
            validate_snapshot(name, content)
    index = value["index"]
    if not isinstance(index, dict) or set(index) != {"path", "content_base64", "state"}:
        raise ValueError("invalid recovery index fields")
    if not isinstance(index["path"], str) or not Path(index["path"]).is_absolute():
        raise ValueError("invalid recovery index path")
    if index["content_base64"] is None:
        if index["state"] is not None:
            raise ValueError("invalid absent recovery index")
        return value
    if not isinstance(index["content_base64"], str):
        raise ValueError("invalid recovery index encoding")
    data = base64.b64decode(index["content_base64"], validate=True)
    state = index["state"]
    if (
        base64.b64encode(data).decode("ascii") != index["content_base64"]
        or not isinstance(state, list)
        or len(state) != 8
        or any(type(item) is not int for item in state)
        or any(item < 0 for item in state[:6])
        or not stat.S_ISREG(state[2])
        or state[5] != len(data)
    ):
        raise ValueError("invalid recovery index image or identity")
    return value


def inspect_safeguards(root: Path, value: object, changed: set[str]) -> dict[str, Any]:
    if value is None:
        return {"available": False, "matches": False}
    expected = _validate_safeguards(value, changed)
    index = git_index_backup(root)
    if index is None:
        return {"available": True, "matches": False, "git_present": False}
    current = _describe_workspace(root, workspace_backup(root), changed)
    if expected["schema"] == LEGACY_SCHEMA:
        for entry in current.values():
            if entry["kind"] == "file":
                del entry["kind"]
    previous = {
        name: {key: value for key, value in entry.items() if key != "content_base64"}
        for name, entry in expected["untouched"].items()
    }
    added = sorted(current.keys() - previous.keys())
    removed = sorted(previous.keys() - current.keys())
    drifted = sorted(
        name
        for name in current.keys() & previous.keys()
        if current[name] != previous[name]
    )
    index_matches = _encode_index(index) == expected["index"]
    index_locked = _has_index_lock(index)
    return {
        "available": True,
        "matches": not (added or removed or drifted or index_locked) and index_matches,
        "git_present": True,
        "untouched": {"added": added, "removed": removed, "drifted": drifted},
        "index_matches": index_matches,
        "index_locked": index_locked,
    }
