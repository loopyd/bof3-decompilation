"""Confined read-only link snapshots and explicit input-leaf resolution."""

from __future__ import annotations

import os
import stat
from collections.abc import Mapping
from contextlib import ExitStack, contextmanager
from dataclasses import dataclass
from pathlib import Path

from harness.common.directory import (
    close_descriptors,
    open_parent_chain,
    verify_parent_chain,
)
from harness.common.deadlines import check_deadline
from harness.common.files import read_leaf_state
from harness.common.inputs import InputBoundaryError


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


@contextmanager
def _observe_symlink(root: Path, name: str, *, missing_ok: bool = False):
    """Retain the verified parent chain until the observed leaf is rechecked."""
    try:
        descriptors, leaf = open_parent_chain(root, name)
    except FileNotFoundError:
        if not missing_ok:
            raise
        yield None, None, None, None
        return
    try:
        parent = descriptors[-1]
        try:
            before = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        except FileNotFoundError:
            if not missing_ok:
                raise
            verify_parent_chain(root, name, descriptors)
            yield None, None, None, None
            return
        linked = stat.S_ISLNK(before.st_mode)
        content = os.readlink(os.fsencode(leaf), dir_fd=parent) if linked else b""
        after = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        snapshot = _snapshot(content, before)
        if snapshot != _snapshot(content, after) or before.st_size != after.st_size:
            raise ValueError(f"workspace symlink changed while reading: {name}")
        verify_parent_chain(root, name, descriptors)
        yield snapshot if linked else None, parent, leaf, before
        after = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
        current = os.readlink(os.fsencode(leaf), dir_fd=parent) if linked else b""
        if snapshot != _snapshot(current, after) or before.st_size != after.st_size:
            raise ValueError(f"workspace symlink changed while reading: {name}")
        verify_parent_chain(root, name, descriptors)
    finally:
        close_descriptors(descriptors)


def read_symlink(root: Path, name: str) -> SymlinkSnapshot | None:
    """Read a stable leaf link without allowing linked parents or dereferencing."""
    with _observe_symlink(root, name, missing_ok=True) as observation:
        return observation[0]


def verify_symlinks(root: Path, workspace: Mapping[str, object]) -> None:
    """Require every captured symlink to remain unchanged without restoring it."""
    for name, snapshot in workspace.items():
        if (
            isinstance(snapshot, SymlinkSnapshot)
            and read_symlink(root, name) != snapshot
        ):
            raise ValueError(f"workspace symlink drift requires parent review: {name}")


def _resolve_target(root: Path, name: str, content: bytes, stack: ExitStack) -> str:
    target = os.fsdecode(content)
    if target.endswith(("/", "/.", "/..")) or target in {".", ".."}:
        raise ValueError(f"input link does not identify a file: {name}")
    path = Path(target)
    if path.is_absolute():
        if not path.is_relative_to(root):
            raise InputBoundaryError(name)
        parts = []
        components = path.relative_to(root).parts
    else:
        parts = list(Path(name).parent.parts)
        components = path.parts
    for component in components:
        check_deadline()
        if component == "..":
            if not parts:
                raise InputBoundaryError(name)
            parent = "/".join((*parts, ".link-parent"))
            descriptors, _ = open_parent_chain(root, parent)
            stack.callback(close_descriptors, descriptors)
            stack.callback(verify_parent_chain, root, parent, descriptors)
            parts.pop()
        else:
            parts.append(component)
    if not parts:
        raise ValueError(f"input link does not identify a file: {name}")
    return "/".join(parts)


def read_linked_state(
    root: Path, name: str, *, missing_ok: bool = False, allow_links: bool = True
) -> tuple[bytes, os.stat_result, Path] | None:
    """Capture bytes, metadata and canonical identity through held descriptors."""

    aliases = set()
    current = name
    with ExitStack() as stack:
        for depth in range(41):
            check_deadline()
            if current in aliases:
                raise ValueError(f"input link cycle: {name}")
            snapshot, parent, leaf, metadata = stack.enter_context(
                _observe_symlink(root, current, missing_ok=missing_ok)
            )
            if parent is None:
                return None
            if snapshot is None:
                content, captured = read_leaf_state(
                    parent, leaf, current, missing_ok=False, expected=metadata
                )
                check_deadline()
                return content, captured, root / current
            if depth == 40:
                raise ValueError(f"input link hop limit exceeded: {name}")
            if not allow_links:
                raise ValueError(f"transaction path is unsafe: {name}")
            aliases.add(current)
            current = _resolve_target(root, current, snapshot.content, stack)
    raise AssertionError("bounded link resolution did not terminate")


def read_linked_input(root: Path, name: str) -> bytes:
    """Read one confined leaf chain, preserving strict canonical input capture."""
    return read_linked_state(root, name)[0]
