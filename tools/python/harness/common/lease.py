"""Cooperative repository writer exclusion for reviewed source transactions."""

from __future__ import annotations

import fcntl
import os
import stat
from contextlib import contextmanager
from contextvars import ContextVar
from dataclasses import dataclass
from functools import wraps
from pathlib import Path
from typing import Callable, Concatenate, Iterator, ParamSpec, TypeVar

from harness.common.directory import open_parent_fd
from harness.common.paths import leaf_stat
from harness.common.root import root_fd

LOCK_PATH = "out/reviews/evidence/transaction.lock"
Parameters = ParamSpec("Parameters")
Result = TypeVar("Result")


@dataclass(frozen=True)
class WriterLease:
    root: Path
    identity: tuple[int, int]
    descriptor: int
    process: int


ACTIVE_WRITER: ContextVar[WriterLease | None] = ContextVar(
    "active_writer", default=None
)


def _verify_lease(lease: WriterLease, root: Path) -> None:
    if os.getpid() != lease.process or root.resolve() != lease.root:
        raise RuntimeError("writer lease belongs to another process or root")
    current_root = os.stat(root, follow_symlinks=False)
    opened = os.fstat(lease.descriptor)
    named = leaf_stat(root, LOCK_PATH)
    if (
        not stat.S_ISDIR(current_root.st_mode)
        or (current_root.st_dev, current_root.st_ino) != lease.identity
        or not stat.S_ISREG(opened.st_mode)
        or opened.st_nlink != 1
        or opened.st_size != 0
        or opened.st_uid != os.geteuid()
        or named is None
        or (named.st_dev, named.st_ino) != (opened.st_dev, opened.st_ino)
    ):
        raise RuntimeError("writer lease identity drifted; explicit recovery required")


def verify_writer(root: Path) -> None:
    """Recheck an active lease; low-level callers without a lease remain unprotected."""
    lease = ACTIVE_WRITER.get()
    if lease is not None:
        _verify_lease(lease, root)


def _open_lease(root: Path) -> int:
    parent, leaf = open_parent_fd(root, LOCK_PATH, create=True)
    descriptor = -1
    try:
        flags = os.O_RDWR | os.O_NOFOLLOW | os.O_CLOEXEC | os.O_NONBLOCK
        try:
            descriptor = os.open(
                leaf, flags | os.O_CREAT | os.O_EXCL, 0o600, dir_fd=parent
            )
        except FileExistsError:
            descriptor = os.open(leaf, flags, dir_fd=parent)
        else:
            os.fsync(descriptor)
            os.fsync(parent)
        return descriptor
    except BaseException:
        if descriptor >= 0:
            os.close(descriptor)
        raise
    finally:
        os.close(parent)


@contextmanager
def acquire_writer(root: Path) -> Iterator[None]:
    """Fail fast on another cooperating writer; retain the lock inode on release."""
    if ACTIVE_WRITER.get() is not None:
        raise RuntimeError("nested source writers are not allowed")
    root_descriptor = root_fd(root)
    try:
        identity = os.fstat(root_descriptor)
        descriptor = _open_lease(root)
        try:
            lease = WriterLease(
                root.resolve(),
                (identity.st_dev, identity.st_ino),
                descriptor,
                os.getpid(),
            )
            _verify_lease(lease, root)
            try:
                fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except BlockingIOError as error:
                raise RuntimeError(
                    "another source transaction holds the writer lease"
                ) from error
            _verify_lease(lease, root)
            token = ACTIVE_WRITER.set(lease)
            try:
                yield
                _verify_lease(lease, root)
            finally:
                ACTIVE_WRITER.reset(token)
        finally:
            os.close(descriptor)
    finally:
        os.close(root_descriptor)


def exclude_writers(
    operation: Callable[Concatenate[Path, Parameters], Result],
) -> Callable[Concatenate[Path, Parameters], Result]:
    @wraps(operation)
    def execute(
        root: Path, *args: Parameters.args, **kwargs: Parameters.kwargs
    ) -> Result:
        with acquire_writer(root):
            return operation(root, *args, **kwargs)

    return execute
