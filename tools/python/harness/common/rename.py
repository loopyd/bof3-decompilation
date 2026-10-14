"""Publication requires native no-replace support and never overwrites existing names."""

from __future__ import annotations

import ctypes
import errno
import os
import stat
from dataclasses import dataclass

_RENAME_NOREPLACE = 1


class UnsupportedRenameNoReplaceError(OSError):
    """The platform or filesystem cannot rename without replacement."""


@dataclass(frozen=True)
class Publication:
    method: str


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


def _state(value: os.stat_result) -> tuple[int, ...]:
    return (
        value.st_dev,
        value.st_ino,
        stat.S_IFMT(value.st_mode),
        value.st_nlink,
        value.st_size,
        value.st_mtime_ns,
        value.st_ctime_ns,
    )


def _observe(directory: int, leaf: str) -> tuple[int, ...] | None:
    try:
        return _state(os.stat(leaf, dir_fd=directory, follow_symlinks=False))
    except FileNotFoundError:
        return None


def describe_locations(directory: int, *leaves: str) -> str:
    """Report observed names without inferring a failed publication's outcome."""
    locations = []
    for leaf in leaves:
        try:
            observed = _observe(directory, leaf)
            description = (
                "absent" if observed is None else f"{observed[0]}:{observed[1]}"
            )
        except OSError:
            description = "unavailable"
        locations.append(f"{leaf}={description}")
    return f"observed locations: {', '.join(locations)}"


def publish_file(
    source: str, destination: str, *, src_dir_fd: int, dst_dir_fd: int
) -> Publication:
    """Move using native no-replace rename, or leave both paths untouched."""
    if any(
        not name or name in {".", ".."} or "/" in name or "\0" in name
        for name in (source, destination)
    ):
        raise ValueError("publication requires relative leaf names")
    rename_noreplace(source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd)
    return Publication("renameat2")
