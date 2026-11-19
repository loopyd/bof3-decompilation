"""Execute assessed cleanup operations under repository and Cargo exclusion."""

from __future__ import annotations

import fcntl
import os
import shutil
import stat
from contextlib import ExitStack
from collections import Counter
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Callable

from harness.cleanup import inventory
from harness.cleanup.inventory import Operation, Plan, estimate_bytes, git, snapshot
from harness.common.directory import (
    close_descriptors,
    open_parent_chain,
    verify_parent_chain,
)
from harness.common.lease import acquire_writer


@dataclass(frozen=True)
class Result:
    completed: tuple[Operation, ...]
    observed_bytes: int
    error: str = ""

    @property
    def estimated_bytes(self) -> int:
        return estimate_bytes(self.completed)


def lock_cargo(plan: Plan, stack: ExitStack) -> None:
    for base in plan.cargo_roots:
        if base.resolve() != base:
            raise ValueError(f"Cargo directory changed: {base}")
        for profile in ("debug", "release"):
            path = base / profile / ".cargo-lock"
            if not path.exists():
                continue
            parent, leaf = open_parent_chain(
                plan.root, path.relative_to(plan.root).as_posix()
            )
            try:
                verify_parent_chain(
                    plan.root, path.relative_to(plan.root).as_posix(), parent
                )
                descriptor = os.open(
                    leaf, os.O_RDWR | os.O_NOFOLLOW | os.O_CLOEXEC, dir_fd=parent[-1]
                )
                stack.callback(os.close, descriptor)
                if not stat.S_ISREG(os.fstat(descriptor).st_mode):
                    raise ValueError(f"Invalid Cargo lock: {path}")
                try:
                    fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
                except BlockingIOError as error:
                    raise ValueError(f"Cargo build holds {path}") from error
            finally:
                close_descriptors(parent)


def revalidate(plan: Plan, operation: Operation, completed: list[Operation]) -> None:
    root = plan.root
    if git(root, "rev-parse", "HEAD").stdout.strip() != plan.head:
        raise ValueError("Git HEAD changed since assessment")
    if any(
        name == operation.path or name.startswith(operation.path + "/")
        for name in inventory.tracked_paths(root)
    ):
        raise ValueError(f"Cleanup path is now indexed or committed: {operation.path}")
    removed_links = Counter(
        (entry.device, entry.inode)
        for previous in completed
        for entry in previous.entries
        if entry.kind != stat.S_IFDIR
    )
    expected = tuple(
        replace(entry, links=entry.links - removed_links[(entry.device, entry.inode)])
        if entry.kind != stat.S_IFDIR
        else entry
        for entry in operation.entries
    )
    if (
        snapshot(root, operation.path, worktree=operation.category == "worktree")
        != expected
    ):
        raise ValueError(f"Cleanup candidate changed: {operation.path}")
    if (
        operation.category not in {"dump", "worktree"}
        and git(
            root, "check-ignore", "-q", "--", operation.path, check=False
        ).returncode
    ):
        raise ValueError(f"Cleanup path is no longer ignored: {operation.path}")
    activity = inventory.read_activity(root)
    path = root / operation.path
    if any(used == path or used.is_relative_to(path) for used in activity.paths):
        raise ValueError(f"Cleanup candidate is active: {operation.path}")
    if operation.category in {"cache", "binary"} and activity.rust_busy:
        raise ValueError("An active Rust build appeared after assessment")
    if operation.category == "worktree":
        inventory.check_worktree(
            root, {"worktree": str(path), "HEAD": operation.worktree_head}, activity
        )


def remove_artifact(root: Path, operation: Operation) -> None:
    descriptors, leaf = open_parent_chain(root, operation.path)
    try:
        verify_parent_chain(root, operation.path, descriptors)
        actual = os.stat(leaf, dir_fd=descriptors[-1], follow_symlinks=False)
        expected = next(
            entry for entry in operation.entries if entry.path == operation.path
        )
        if (actual.st_dev, actual.st_ino) != (expected.device, expected.inode):
            raise ValueError(f"Cleanup candidate identity changed: {operation.path}")
        if stat.S_ISDIR(actual.st_mode):
            if not shutil.rmtree.avoids_symlink_attacks:
                raise ValueError("Cleanup requires symlink-safe directory removal")
            shutil.rmtree(leaf, dir_fd=descriptors[-1])
        else:
            os.unlink(leaf, dir_fd=descriptors[-1])
    finally:
        close_descriptors(descriptors)


def execute(plan: Plan, *, emit: Callable[[str], None] = print) -> Result:
    completed = []
    before = shutil.disk_usage(plan.root).free
    error = ""
    try:
        with acquire_writer(plan.root), ExitStack() as stack:
            lock_cargo(plan, stack)
            for operation in plan.operations:
                revalidate(plan, operation, completed)
                emit(f"REMOVE [{operation.category}] {operation.path}")
                if operation.category == "worktree":
                    git(
                        plan.root,
                        "worktree",
                        "remove",
                        "--",
                        str(plan.root / operation.path),
                    )
                else:
                    remove_artifact(plan.root, operation)
                completed.append(operation)
    except (ValueError, RuntimeError, OSError) as exception:
        error = str(exception)
    return Result(tuple(completed), shutil.disk_usage(plan.root).free - before, error)
