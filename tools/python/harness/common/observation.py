"""Observe confined file content, identity and local path-entry mutations."""

from __future__ import annotations

import hashlib
import ctypes
import os
import stat
import struct
from pathlib import Path
from typing import Any

from harness.common.files import read_file
from harness.common.deadlines import check_deadline
from harness.common.paths import leaf_stat

_ENTRY_EVENTS = 0x000003C6
_NAMESPACE_EVENTS = 0x000003C0
_SELF_EVENTS = 0x00002C00
_LOST_EVENTS = 0x0000C000
_DIRECTORY_EVENT = 0x40000000
_ONLY_DIRECTORY = 0x01000000
_EVENT_HEADER = struct.Struct("=iIII")
_DIRECTORY_FLAGS = os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW | os.O_CLOEXEC
_DIRECTORY_ENTRY_LIMIT = 16384
_DIRECTORY_BYTE_LIMIT = 4 * 1024 * 1024


def _capture_directory_metadata(status: os.stat_result) -> tuple[int, ...]:
    return (
        status.st_dev,
        status.st_ino,
        status.st_mode,
        status.st_nlink,
        status.st_uid,
        status.st_gid,
        status.st_size,
        status.st_mtime_ns,
        status.st_ctime_ns,
    )


def observe_directory(path: Path) -> tuple:
    """Capture raw metadata and bounded membership through one directory descriptor."""
    check_deadline()
    descriptor = os.open(path, _DIRECTORY_FLAGS)
    try:
        before = _capture_directory_metadata(os.fstat(descriptor))
        records = {}
        total = 0
        with os.scandir(descriptor) as entries:
            for entry in entries:
                check_deadline()
                name = os.fsencode(entry.name)
                if (
                    not name
                    or name in {b".", b".."}
                    or b"/" in name
                    or b"\0" in name
                    or name in records
                ):
                    raise ValueError(f"invalid directory observation entry: {path}")
                status = entry.stat(follow_symlinks=False)
                kind = stat.S_IFMT(status.st_mode)
                if kind not in {
                    stat.S_IFREG,
                    stat.S_IFDIR,
                    stat.S_IFLNK,
                    stat.S_IFCHR,
                    stat.S_IFBLK,
                    stat.S_IFIFO,
                    stat.S_IFSOCK,
                }:
                    raise ValueError(f"unknown directory entry type: {path}")
                record = (
                    struct.pack("=IQQI", len(name), status.st_dev, status.st_ino, kind)
                    + name
                )
                records[name] = record
                total += len(record)
                if (
                    len(records) > _DIRECTORY_ENTRY_LIMIT
                    or total > _DIRECTORY_BYTE_LIMIT
                ):
                    raise ValueError(f"directory observation exceeds its bound: {path}")
        check_deadline()
        digest = hashlib.sha256()
        for name in sorted(records):
            check_deadline()
            digest.update(records[name])
        after = _capture_directory_metadata(os.fstat(descriptor))
        linked = _capture_directory_metadata(path.stat(follow_symlinks=False))
        for state in (after, linked):
            if before[:6] != state[:6] or before[7:] != state[7:]:
                raise ValueError(f"directory changed during observation: {path}")
        check_deadline()
        result = (*linked, digest.hexdigest())
    finally:
        os.close(descriptor)
    check_deadline()
    return result


def _capture_directory_identity(status: os.stat_result) -> tuple[int, ...]:
    return (
        status.st_dev,
        status.st_ino,
        status.st_mode,
        status.st_uid,
        status.st_gid,
    )


class PathWatch:
    """Latch local Linux input-entry changes across repeated content observations."""

    def __init__(
        self,
        paths: set[Path],
        *,
        directories: set[Path] | None = None,
        namespace_only: bool = False,
    ) -> None:
        if type(namespace_only) is not bool:
            raise ValueError("invalid directory event selection")
        self._membership_events = _NAMESPACE_EVENTS if namespace_only else _ENTRY_EVENTS
        self._descriptor: int | None = None
        self._directories: dict[Path, int] = {}
        self._identities: dict[Path, tuple[int, ...]] = {}
        self._entries: dict[int, set[bytes]] = {}
        self._memberships: set[int] = set()
        self._failure: str | None = None
        directories = set() if directories is None else set(directories)
        inputs = set(paths) | directories
        if not inputs or len(inputs) > 16384:
            raise ValueError("path watch input count is outside supported bounds")
        entries: dict[Path, set[bytes]] = {path: set() for path in directories}
        for path in inputs:
            check_deadline()
            if not path.is_absolute() or ".." in path.parts or path == path.parent:
                raise ValueError("path watch requires absolute named inputs")
            child = path
            while child != child.parent:
                check_deadline()
                entries.setdefault(child.parent, set()).add(os.fsencode(child.name))
                if len(entries) > 32768:
                    raise ValueError("path watch ancestor count exceeds limit")
                child = child.parent
        try:
            library = ctypes.CDLL(None, use_errno=True)
            initialize = library.inotify_init1
            initialize.argtypes = [ctypes.c_int]
            initialize.restype = ctypes.c_int
            add_watch = library.inotify_add_watch
            add_watch.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_uint32]
            add_watch.restype = ctypes.c_int
            descriptor = initialize(os.O_CLOEXEC | os.O_NONBLOCK)
            if descriptor < 0:
                raise OSError(ctypes.get_errno(), "path event observation unavailable")
            self._descriptor = descriptor
            for path in sorted(entries, key=lambda item: (len(item.parts), item)):
                check_deadline()
                if path == path.parent:
                    directory = os.open(path, _DIRECTORY_FLAGS)
                elif path.parent not in self._directories:
                    continue
                else:
                    try:
                        directory = os.open(
                            path.name,
                            _DIRECTORY_FLAGS,
                            dir_fd=self._directories[path.parent],
                        )
                    except FileNotFoundError:
                        continue
                self._directories[path] = directory
                before = os.fstat(directory)
                watch = add_watch(
                    descriptor,
                    os.fsencode(f"/proc/self/fd/{directory}"),
                    _ENTRY_EVENTS | _SELF_EVENTS | _ONLY_DIRECTORY,
                )
                if watch < 0:
                    raise OSError(
                        ctypes.get_errno(), f"cannot watch input parent: {path}"
                    )
                self._entries.setdefault(watch, set()).update(entries[path])
                if path in directories:
                    self._memberships.add(watch)
                after = os.fstat(directory)
                if (
                    _capture_directory_identity(before)
                    != _capture_directory_identity(after)
                    or before.st_mtime_ns != after.st_mtime_ns
                    or before.st_ctime_ns != after.st_ctime_ns
                ):
                    self._fail(f"input directory changed during watch setup: {path}")
                self._identities[path] = _capture_directory_identity(after)
            if directories - self._directories.keys():
                self._fail("input membership directory is missing")
            self.validate()
        except BaseException:
            self.close()
            raise

    def _fail(self, message: str) -> None:
        if self._failure is None:
            self._failure = message
        raise ValueError(self._failure)

    def _consume(self, content: bytes) -> None:
        cursor = 0
        while cursor < len(content):
            check_deadline()
            if len(content) - cursor < _EVENT_HEADER.size:
                self._fail("truncated input watch event header")
            watch, mask, _cookie, length = _EVENT_HEADER.unpack_from(content, cursor)
            cursor += _EVENT_HEADER.size
            if length > len(content) - cursor or length % 4:
                self._fail("malformed input watch event length")
            payload = content[cursor : cursor + length]
            cursor += length
            if length:
                name, separator, padding = payload.partition(b"\0")
                if not separator or not name or any(padding) or b"/" in name:
                    self._fail("malformed input watch event name")
            else:
                name = b""
            if mask & _LOST_EVENTS:
                self._fail("input watch overflowed or lost a directory")
            if (
                watch not in self._entries
                or mask & ~(_ENTRY_EVENTS | _SELF_EVENTS | _DIRECTORY_EVENT)
                or not mask & (_ENTRY_EVENTS | _SELF_EVENTS)
            ):
                self._fail("unknown input watch event")
            if (
                mask & _SELF_EVENTS
                or not name
                or (watch in self._memberships and mask & self._membership_events)
                or name in self._entries[watch]
            ):
                self._fail("watched input entry or ancestor changed")

    def _drain(self) -> None:
        for _attempt in range(64):
            check_deadline()
            try:
                content = os.read(self._descriptor, 65536)
            except BlockingIOError:
                return
            except InterruptedError:
                continue
            except OSError as error:
                self._fail(f"input watch became unreadable: {error}")
            if not content:
                self._fail("input watch returned an empty event stream")
            try:
                self._consume(content)
            except BaseException:
                if self._failure is None:
                    self._failure = "input watch event processing was interrupted"
                raise
        self._fail("input watch event drain exceeded its bound")

    def validate(self) -> None:
        """Latch selected-entry events and the chosen explicit-directory child events."""
        check_deadline()
        if self._failure is not None:
            raise ValueError(self._failure)
        if self._descriptor is None:
            self._fail("input watch is closed")
        self._drain()
        try:
            for path, descriptor in self._directories.items():
                check_deadline()
                linked = (
                    os.stat(path, follow_symlinks=False)
                    if path == path.parent
                    else os.stat(
                        path.name,
                        dir_fd=self._directories[path.parent],
                        follow_symlinks=False,
                    )
                )
                if (
                    _capture_directory_identity(linked) != self._identities[path]
                    or _capture_directory_identity(os.fstat(descriptor))
                    != self._identities[path]
                ):
                    self._fail(f"input watch directory detached or changed: {path}")
        except OSError as error:
            self._fail(f"input watch directory became unavailable: {error}")
        self._drain()
        check_deadline()

    def close(self) -> None:
        """Close owned descriptors without allowing a failed watch to be revived."""
        descriptors = list(self._directories.values())
        if self._descriptor is not None:
            descriptors.append(self._descriptor)
        self._descriptor = None
        self._directories = {}
        error = None
        for descriptor in reversed(descriptors):
            try:
                os.close(descriptor)
            except OSError as caught:
                error = caught
        if error is not None:
            raise error


class DirectoryBatch:
    """Reuse bounded membership within one namespace-watched verification pass."""

    def __init__(self, directories: set[Path]) -> None:
        self._directories = {path for path in directories if path != path.parent}
        self._watch: PathWatch | None = None
        self._memberships: dict[Path, str] = {}
        self._active = False
        self._closed = False
        self._failed = False

    def __enter__(self) -> DirectoryBatch:
        check_deadline()
        if self._active or self._closed:
            raise ValueError("directory batch cannot be reused")
        try:
            if self._directories:
                self._watch = PathWatch(
                    set(), directories=self._directories, namespace_only=True
                )
            self._active = True
        except BaseException:
            self._closed = True
            raise
        return self

    def __exit__(self, error_type, error, traceback) -> None:
        try:
            if error_type is None:
                check_deadline()
                if self._failed:
                    raise ValueError("directory batch observation failed")
                if self._watch is not None:
                    self._watch.validate()
        finally:
            self._active = False
            self._closed = True
            self._memberships.clear()
            if self._watch is not None:
                self._watch.close()

    def observe(self, path: Path) -> tuple:
        """Return fresh raw metadata; membership reuse requires successful batch exit."""
        try:
            check_deadline()
            if not self._active or self._closed or self._failed:
                raise ValueError("directory batch is inactive or failed")
            if path not in self._directories:
                return observe_directory(path)
            if path not in self._memberships:
                observed = observe_directory(path)
                self._memberships[path] = observed[-1]
                return observed
            metadata = _capture_directory_metadata(path.stat(follow_symlinks=False))
            check_deadline()
            return (*metadata, self._memberships[path])
        except BaseException:
            self._failed = True
            raise


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
