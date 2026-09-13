"""Native no-replace moves and cooperative publication on unsupported filesystems."""

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
    """Describe the backend used for a completed single-link publication."""

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
        value.st_mode,
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


def _publish_reserved(
    source: str, destination: str, *, src_dir_fd: int, dst_dir_fd: int
) -> None:
    """Replace an exclusive reservation; external writers can race checked names."""
    source_fd = os.open(
        source, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK, dir_fd=src_dir_fd
    )
    try:
        source_state = _state(os.fstat(source_fd))
        if not stat.S_ISREG(source_state[2]) or source_state[3] != 1:
            raise ValueError("cooperative publication requires a single-link file")
        reservation = os.open(
            destination,
            os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW | os.O_NONBLOCK,
            0o600,
            dir_fd=dst_dir_fd,
        )
        try:
            reserved_state = _state(os.fstat(reservation))
            eligible = stat.S_ISREG(reserved_state[2]) and reserved_state[3:5] == (1, 0)
            moved = False
            try:
                if (
                    not eligible
                    or _observe(dst_dir_fd, destination) != reserved_state
                    or _state(os.fstat(reservation)) != reserved_state
                ):
                    raise RuntimeError(
                        f"publication reservation changed: {destination}"
                    )
                if (
                    _observe(src_dir_fd, source) != source_state
                    or _state(os.fstat(source_fd)) != source_state
                ):
                    raise RuntimeError(f"publication source changed: {source}")
                os.rename(
                    source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
                )
                moved = True
                published = _observe(dst_dir_fd, destination)
                if (
                    published is None
                    or published[:-1] != source_state[:-1]
                    or _state(os.fstat(source_fd))[:-1] != source_state[:-1]
                    or _observe(src_dir_fd, source) is not None
                ):
                    raise RuntimeError(
                        f"publication verification failed: {source} -> {destination}; "
                        "preserve both locations for recovery"
                    )
                os.fsync(dst_dir_fd)
                os.fsync(src_dir_fd)
            except BaseException as error:
                if not moved and eligible:
                    try:
                        if (
                            _observe(dst_dir_fd, destination) == reserved_state
                            and _state(os.fstat(reservation)) == reserved_state
                        ):
                            os.unlink(destination, dir_fd=dst_dir_fd)
                            os.fsync(dst_dir_fd)
                    except OSError as cleanup_error:
                        error.add_note(
                            f"reservation cleanup uncertain at {destination}: "
                            f"{cleanup_error}"
                        )
                raise
        finally:
            os.close(reservation)
    finally:
        os.close(source_fd)


def publish_file(
    source: str, destination: str, *, src_dir_fd: int, dst_dir_fd: int
) -> Publication:
    """Move natively or cooperatively; fallback is not CAS against external writers."""
    if any(
        not name or name in {".", ".."} or "/" in name or "\0" in name
        for name in (source, destination)
    ):
        raise ValueError("publication requires relative leaf names")
    try:
        rename_noreplace(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )
        return Publication("renameat2")
    except UnsupportedRenameNoReplaceError:
        _publish_reserved(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )
        return Publication("reserved-rename")
