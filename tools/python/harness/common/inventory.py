"""Bounded confined filesystem observations for immutable query inputs."""

from __future__ import annotations

import hashlib
import os
import stat
from dataclasses import dataclass
from pathlib import Path

from harness.common.directory import (
    close_descriptors,
    open_parent_chain,
    verify_parent_chain,
)

MAX_FILE_BYTES = 128 * 1024 * 1024


@dataclass
class CaptureBudget:
    """Bound the explicitly selected file captures within one transaction."""

    files: int = 0
    size: int = 0

    def charge(self, size: int = 0) -> None:
        self.files += 1
        self.size += size
        if self.files > 200000 or self.size > 2 * 1024 * 1024 * 1024:
            raise ValueError("scoped file capture exceeds inventory limit")


def describe_identity(metadata: os.stat_result) -> dict:
    """Bind object identity without directory allocation-size noise."""
    identity = {
        field.removeprefix("st_"): getattr(metadata, field)
        for field in (
            "st_dev",
            "st_ino",
            "st_uid",
            "st_gid",
            "st_nlink",
            "st_size",
            "st_mtime_ns",
            "st_ctime_ns",
        )
    }
    identity["file_type"] = stat.S_IFMT(metadata.st_mode)
    if stat.S_ISDIR(metadata.st_mode):
        del identity["size"]
    return identity


def capture_file(
    root: Path,
    name: str,
    budget: CaptureBudget,
    *,
    missing_ok: bool = False,
    max_bytes: int = MAX_FILE_BYTES,
) -> tuple[dict | None, bytes | None]:
    """Read stable bytes through a pinned chain without following any links."""
    budget.charge()
    try:
        descriptors, leaf = open_parent_chain(root, name)
    except FileNotFoundError:
        if missing_ok:
            return None, None
        raise
    try:
        try:
            descriptor = os.open(
                leaf,
                os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK,
                dir_fd=descriptors[-1],
            )
        except FileNotFoundError:
            if not missing_ok:
                raise
            verify_parent_chain(root, name, descriptors)
            return None, None
        except OSError as error:
            raise ValueError(f"unsafe snapshot file: {name}") from error
        with os.fdopen(descriptor, "rb") as stream:
            before = os.fstat(stream.fileno())
            if not stat.S_ISREG(before.st_mode) or before.st_size > max_bytes:
                raise ValueError(f"unsupported or oversized snapshot file: {name}")
            budget.charge(before.st_size)
            content = stream.read(before.st_size + 1)
            after = os.fstat(stream.fileno())
        attached = os.stat(leaf, dir_fd=descriptors[-1], follow_symlinks=False)
        identity = describe_identity(before)
        if (
            len(content) != before.st_size
            or identity != describe_identity(after)
            or identity != describe_identity(attached)
        ):
            raise ValueError(f"snapshot file changed during capture: {name}")
        verify_parent_chain(root, name, descriptors)
        return {**identity, "sha256": hashlib.sha256(content).hexdigest()}, content
    finally:
        close_descriptors(descriptors)
