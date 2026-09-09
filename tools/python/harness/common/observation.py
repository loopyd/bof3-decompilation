"""Observe confined file content and identity without granting mutation authority."""

from __future__ import annotations

import hashlib
import stat
from pathlib import Path
from typing import Any

from harness.common.files import read_file
from harness.common.paths import leaf_stat


def observe_file(root: Path, name: str) -> dict[str, Any] | None:
    before = leaf_stat(root, name)
    content = read_file(root, name, missing_ok=True)
    after = leaf_stat(root, name)
    fields = (
        "st_dev",
        "st_ino",
        "st_mode",
        "st_nlink",
        "st_size",
        "st_mtime_ns",
        "st_ctime_ns",
    )
    if tuple(getattr(before, field, None) for field in fields) != tuple(
        getattr(after, field, None) for field in fields
    ) or (before is None) != (content is None):
        raise ValueError(f"file observation raced with a writer: {name}")
    if after is None:
        return None
    return {
        "sha256": hashlib.sha256(content).hexdigest(),
        "mode": stat.S_IMODE(after.st_mode),
        "device": after.st_dev,
        "inode": after.st_ino,
        "links": after.st_nlink,
    }
