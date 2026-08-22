"""Portable descriptor-relative publication without replacement."""

from __future__ import annotations

import ctypes
from dataclasses import dataclass
import errno
import os
import stat

_RENAME_NOREPLACE = 1


class UnsupportedRenameNoReplaceError(OSError):
    """The platform or filesystem cannot rename without replacement."""


@dataclass(frozen=True)
class Publication:
    """How a source was published and where its retained recovery inode remains."""

    method: str
    recovery_path: str | None


def rename_noreplace(
    source: str, destination: str, *, src_dir_fd: int, dst_dir_fd: int
) -> None:
    """Rename without replacing, or fail before mutating either path."""
    try:
        renameat2 = getattr(ctypes.CDLL(None, use_errno=True), "renameat2", None)
    except OSError as error:
        raise UnsupportedRenameNoReplaceError(
            "renameat2(RENAME_NOREPLACE) is unavailable on this platform"
        ) from error
    if renameat2 is None:
        raise UnsupportedRenameNoReplaceError(
            "renameat2(RENAME_NOREPLACE) is unavailable on this platform"
        )
    result = renameat2(
        src_dir_fd,
        os.fsencode(source),
        dst_dir_fd,
        os.fsencode(destination),
        _RENAME_NOREPLACE,
    )
    if not result:
        return
    error = ctypes.get_errno()
    if error in {errno.ENOSYS, errno.EINVAL, errno.EOPNOTSUPP, errno.ENOTSUP}:
        raise UnsupportedRenameNoReplaceError(
            "renameat2(RENAME_NOREPLACE) is unsupported by this platform or filesystem"
        )
    raise OSError(error, os.strerror(error), destination)


def require_native_noreplace(directory: int, existing_leaf: str) -> None:
    """Probe native support without mutating an existing leaf."""
    try:
        rename_noreplace(
            existing_leaf,
            existing_leaf,
            src_dir_fd=directory,
            dst_dir_fd=directory,
        )
    except FileExistsError:
        return
    except UnsupportedRenameNoReplaceError as error:
        raise UnsupportedRenameNoReplaceError(
            "unsupported filesystem: existing-file transactions require native "
            "renameat2(RENAME_NOREPLACE)"
        ) from error
    raise RuntimeError("renameat2 no-replace preflight unexpectedly mutated a path")


def publish_noreplace(
    source: str, destination: str, *, src_dir_fd: int, dst_dir_fd: int
) -> Publication:
    """Publish a regular source exclusively, retaining it when hard-link fallback is used."""
    try:
        rename_noreplace(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )
        return Publication("renameat2", None)
    except UnsupportedRenameNoReplaceError:
        source_state = os.stat(source, dir_fd=src_dir_fd, follow_symlinks=False)
        if not stat.S_ISREG(source_state.st_mode):
            raise UnsupportedRenameNoReplaceError(
                "portable no-replace fallback supports regular files only"
            )
        os.link(
            source,
            destination,
            src_dir_fd=src_dir_fd,
            dst_dir_fd=dst_dir_fd,
            follow_symlinks=False,
        )
        return Publication("hard-link", source)
