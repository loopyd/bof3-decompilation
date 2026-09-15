"""Canonical path-set validation for review transactions."""

from __future__ import annotations

import hashlib
import os
import stat
from pathlib import Path, PurePosixPath

from harness.common.directory import validate_repo_path, open_parent_fd


def format_relative_path(root: Path, path: Path) -> str:
    """Format a lexical descendant without resolving filesystem paths."""
    if not isinstance(path, PurePosixPath):
        return path.relative_to(root).as_posix()
    parent = path.with_segments(root)
    parts = path.parts
    count = len(parent.parts)
    if path.with_segments(*parts[:count]) != parent or (not count and path.anchor):
        raise ValueError(f"{str(path)!r} is not in the subpath of {str(parent)!r}")
    return path.with_segments(*parts[count:]).as_posix()


def require_absent(root: Path, name: str) -> None:
    """Reject any occupied confined leaf, including symlinks, without reading it."""
    parent, leaf = open_parent_fd(root, name)
    try:
        try:
            os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        except FileNotFoundError:
            return
        raise FileExistsError(name)
    finally:
        os.close(parent)


def leaf_stat(root: Path, name: str) -> os.stat_result | None:
    try:
        parent, leaf = open_parent_fd(root, name)
    except FileNotFoundError:
        return None
    try:
        try:
            value = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        except FileNotFoundError:
            return None
        if stat.S_ISLNK(value.st_mode) or not stat.S_ISREG(value.st_mode):
            raise ValueError(f"transaction path is not a regular file: {name}")
        return value
    finally:
        os.close(parent)


def file_state(root: Path, paths: set[str]) -> dict[str, str | None]:
    from harness.common.files import read_file

    result = {}
    for name in sorted(validate_paths(root, paths)):
        content = read_file(root, name, missing_ok=True)
        result[name] = (
            hashlib.sha256(content).hexdigest() if content is not None else None
        )
    return result


def validate_paths(root: Path, paths: object) -> set[str]:
    if not isinstance(paths, (set, list, tuple)) or any(
        not isinstance(name, str) for name in paths
    ):
        raise ValueError("transaction paths are invalid")
    result = {validate_repo_path(name) for name in paths}
    if len(result) != len(paths):
        raise ValueError("transaction paths must be unique")
    for name in result:
        leaf_stat(root, name)
    return result
