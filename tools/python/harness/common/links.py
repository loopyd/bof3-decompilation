"""Confined, read-only snapshots of unrelated workspace symlinks."""

from __future__ import annotations

import os
import stat
from collections.abc import Mapping
from dataclasses import dataclass
from pathlib import Path

from harness.common.directory import (
    close_descriptors,
    open_parent_chain,
    verify_parent_chain,
)


@dataclass(frozen=True)
class SymlinkSnapshot:
    """Literal link bytes and identity, never the referent's contents."""

    content: bytes
    mode: int
    device: int
    inode: int
    uid: int
    gid: int
    links: int
    mtime_ns: int
    ctime_ns: int


def _snapshot(content: bytes, metadata: os.stat_result) -> SymlinkSnapshot:
    return SymlinkSnapshot(
        content,
        metadata.st_mode,
        metadata.st_dev,
        metadata.st_ino,
        metadata.st_uid,
        metadata.st_gid,
        metadata.st_nlink,
        metadata.st_mtime_ns,
        metadata.st_ctime_ns,
    )


def read_symlink(root: Path, name: str) -> SymlinkSnapshot | None:
    """Read a stable leaf link without allowing linked parents or dereferencing."""
    try:
        descriptors, leaf = open_parent_chain(root, name)
    except FileNotFoundError:
        return None
    try:
        parent = descriptors[-1]
        try:
            before = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        except FileNotFoundError:
            verify_parent_chain(root, name, descriptors)
            return None
        if not stat.S_ISLNK(before.st_mode):
            verify_parent_chain(root, name, descriptors)
            return None
        content = os.readlink(os.fsencode(leaf), dir_fd=parent)
        after = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        snapshot = _snapshot(content, before)
        if snapshot != _snapshot(content, after):
            raise ValueError(f"workspace symlink changed while reading: {name}")
        verify_parent_chain(root, name, descriptors)
        return snapshot
    finally:
        close_descriptors(descriptors)


def verify_symlinks(root: Path, workspace: Mapping[str, object]) -> None:
    """Require every captured symlink to remain unchanged without restoring it."""
    for name, snapshot in workspace.items():
        if (
            isinstance(snapshot, SymlinkSnapshot)
            and read_symlink(root, name) != snapshot
        ):
            raise ValueError(f"workspace symlink drift requires parent review: {name}")
