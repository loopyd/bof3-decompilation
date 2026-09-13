"""Bounded confined filesystem observations for immutable query inputs."""

from __future__ import annotations

import hashlib
import os
import stat
from dataclasses import dataclass
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.directory import (
    close_descriptors,
    open_parent_chain,
    validate_repo_path,
    verify_parent_chain,
)

MAX_FILE_BYTES = 128 * 1024 * 1024


@dataclass
class CaptureBudget:
    """Bound traversal, captured bytes and queries across a recursive operation."""

    files: int = 0
    size: int = 0
    queries: int = 0

    def charge(self, size: int = 0) -> None:
        check_deadline()
        self.files += 1
        self.size += size
        if self.files > 200000 or self.size > 2 * 1024 * 1024 * 1024:
            raise ValueError("submodule capture exceeds inventory limit")

    def charge_query(self) -> None:
        check_deadline()
        self.queries += 1
        if self.queries > 512:
            raise ValueError("submodule capture exceeds query limit")


def describe_identity(metadata: os.stat_result) -> dict:
    return {
        field.removeprefix("st_"): getattr(metadata, field)
        for field in (
            "st_dev",
            "st_ino",
            "st_mode",
            "st_uid",
            "st_gid",
            "st_nlink",
            "st_size",
            "st_mtime_ns",
            "st_ctime_ns",
        )
    }


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
            content = stream.read(max_bytes + 1)
            after = os.fstat(stream.fileno())
        attached = os.stat(leaf, dir_fd=descriptors[-1], follow_symlinks=False)
        identity = describe_identity(before)
        if (
            len(content) > max_bytes
            or identity != describe_identity(after)
            or identity != describe_identity(attached)
        ):
            raise ValueError(f"snapshot file changed during capture: {name}")
        verify_parent_chain(root, name, descriptors)
        return {**identity, "sha256": hashlib.sha256(content).hexdigest()}, content
    finally:
        close_descriptors(descriptors)


def scan_tree(
    root: Path,
    name: str | None,
    budget: CaptureBudget,
    *,
    excluded: frozenset[str] = frozenset(),
    opaque: frozenset[str] = frozenset(),
) -> dict[str, dict]:
    """Observe a bounded directory namespace without dereferencing descendants."""
    if name is not None:
        validate_repo_path(name)
    result = {}

    def visit(relative: str, depth: int) -> None:
        if depth > 64:
            raise ValueError("snapshot directory depth exceeds limit")
        sentinel = "/".join(part for part in (name, relative, "snapshot") if part)
        descriptors, _ = open_parent_chain(root, sentinel)
        try:
            descriptor = descriptors[-1]
            before = describe_identity(os.fstat(descriptor))
            budget.charge()
            entries = []
            with os.scandir(descriptor) as directory:
                for entry in directory:
                    budget.charge()
                    entries.append(entry.name)
            for leaf in sorted(entries):
                child = f"{relative}/{leaf}" if relative else leaf
                validate_repo_path(child)
                if child in excluded:
                    continue
                metadata = os.stat(leaf, dir_fd=descriptor, follow_symlinks=False)
                result[child] = describe_identity(metadata)
                if stat.S_ISDIR(metadata.st_mode) and leaf not in opaque:
                    visit(child, depth + 1)
            if before != describe_identity(os.fstat(descriptor)):
                raise ValueError(f"snapshot directory changed during scan: {name}")
            result[relative] = before
            verify_parent_chain(root, sentinel, descriptors)
        finally:
            close_descriptors(descriptors)

    visit("", 0)
    return result
