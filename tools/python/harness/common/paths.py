"""Canonical path-set validation for review transactions."""

from __future__ import annotations

import hashlib
import os
import stat
from pathlib import Path

from harness.common.directory import validate_repo_path, open_parent_fd


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
