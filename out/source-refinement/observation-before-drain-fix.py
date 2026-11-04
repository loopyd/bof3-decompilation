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
from harness.common.paths import leaf_stat

_ENTRY_EVENTS = 0x000003C6
_NAMESPACE_EVENTS = 0x000003C0
_SELF_EVENTS = 0x00002C00
_LOST_EVENTS = 0x0000C000
_DIRECTORY_EVENT = 0x40000000
_ONLY_DIRECTORY = 0x01000000
_EVENT_HEADER = struct.Struct("=iIII")
_DIRECTORY_FLAGS = os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW | os.O_CLOEXEC


def capture_directory_identity(status: os.stat_result) -> tuple[int, ...]:
    return (
        status.st_dev,
        status.st_ino,
        stat.S_IFMT(status.st_mode),
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
        self._watch_paths: dict[int, set[Path]] = {}
        self._watch_of: dict[Path, int] = {}
        self._memberships: set[int] = set()
        self._inputs: set[Path] = set()
        self._failure: str | None = None
        try:
            library = ctypes.CDLL(None, use_errno=True)
            initialize = library.inotify_init1
            initialize.argtypes = [ctypes.c_int]
            initialize.restype = ctypes.c_int
            self._add_watch = library.inotify_add_watch
            self._add_watch.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_uint32]
            self._add_watch.restype = ctypes.c_int
            descriptor = initialize(os.O_CLOEXEC | os.O_NONBLOCK)
            if descriptor < 0:
                raise OSError(ctypes.get_errno(), "path event observation unavailable")
            self._descriptor = descriptor
            self.add_paths(paths, directories=directories)
            self.validate()
        except BaseException:
            self.close()
            raise

    def add_paths(
        self, paths: set[Path], *, directories: set[Path] | None = None
    ) -> None:
        """Watch additional inputs before admission without losing earlier events."""
        if self._failure is not None:
            raise ValueError(self._failure)
        if self._descriptor is None:
            raise ValueError("input watch is closed")
        try:
            if directories is None and len(paths) == 1:
                # Admission of one already watched input is the dominant call: keep
                # it to a single membership test and the required event drain.
                (path,) = paths
                if path in self._inputs:
                    self._drain()
                    return
            self._drain()
            directories = set() if directories is None else set(directories)
            inputs = set(paths) | directories
            if not inputs:
                raise ValueError("path watch input count is outside supported bounds")
            # Count and compare against new inputs only: rebuilding the union of the
            # whole watched set on every admission is quadratic in input count.
            pending = inputs - self._inputs
            if len(self._inputs) + len(pending) > 16384:
                raise ValueError("path watch input count is outside supported bounds")
            if not pending and not directories:
                self._drain()
                return
            entries: dict[Path, set[bytes]] = {}
            for path in pending:
                if not path.is_absolute() or ".." in path.parts:
                    raise ValueError("path watch requires absolute inputs")
                if path in directories:
                    entries.setdefault(path, set())
                child = path
                while True:
                    parent = child.parent
                    if child == parent:
                        break
                    entries.setdefault(parent, set()).add(os.fsencode(child.name))
                    if parent in self._directories:
                        break
                    child = parent
            if len(set(entries) | self._directories.keys()) > 32768:
                raise ValueError("path watch ancestor count exceeds limit")
            watch_paths = self._watch_of
            for path in sorted(entries, key=lambda item: (len(item.parts), item)):
                existing = path in self._directories
                if existing:
                    directory = self._directories[path]
                elif path == path.parent:
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
                if not existing:
                    self._directories[path] = directory
                before_status = os.fstat(directory)
                before = capture_directory_identity(before_status)
                if existing:
                    watch = watch_paths[path]
                else:
                    watch = self._add_watch(
                        self._descriptor,
                        os.fsencode(f"/proc/self/fd/{directory}"),
                        _ENTRY_EVENTS | _SELF_EVENTS | _ONLY_DIRECTORY,
                    )
                    if watch < 0:
                        raise OSError(
                            ctypes.get_errno(), f"cannot watch input parent: {path}"
                        )
                    watch_paths[path] = watch
                self._entries.setdefault(watch, set()).update(entries[path])
                self._watch_paths.setdefault(watch, set()).add(path)
                if path in directories:
                    self._memberships.add(watch)
                after_status = os.fstat(directory)
                after = capture_directory_identity(after_status)
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
                    before != after
                    or (
                        path in directories
                        and (
                            before_status.st_mtime_ns,
                            before_status.st_ctime_ns,
                        )
                        != (
                            after_status.st_mtime_ns,
                            after_status.st_ctime_ns,
                        )
                    )
                    or capture_directory_identity(linked) != after
                    or self._identities.get(path, after) != after
                ):
                    self._fail(f"input directory changed during watch setup: {path}")
                self._identities[path] = after
            if directories - self._directories.keys():
                self._fail("input membership directory is missing")
            self._inputs.update(pending)
            self._drain()
        except BaseException:
            if self._failure is None:
                self._failure = "input watch expansion failed"
            raise

    def _fail(self, message: str) -> None:
        if self._failure is None:
            self._failure = message
        raise ValueError(self._failure)

    def _consume(self, content: bytes) -> None:
        cursor = 0
        while cursor < len(content):
            if len(content) - cursor < _EVENT_HEADER.size:
                self._fail("truncated input watch event header")
            watch, mask, cookie, length = _EVENT_HEADER.unpack_from(content, cursor)
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
                paths = tuple(str(path) for path in sorted(self._watch_paths[watch]))
                self._fail(
                    "watched input entry or ancestor changed: "
                    f"directories={paths!r}, entry={name!r}, watch={watch}, "
                    f"mask=0x{mask:08x}, cookie={cookie}"
                )

    def _drain(self) -> None:
        for _attempt in range(64):
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
        if self._failure is not None:
            raise ValueError(self._failure)
        if self._descriptor is None:
            self._fail("input watch is closed")
        self._drain()
        try:
            for path, descriptor in self._directories.items():
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
                    capture_directory_identity(linked) != self._identities[path]
                    or capture_directory_identity(os.fstat(descriptor))
                    != self._identities[path]
                ):
                    self._fail(f"input watch directory detached or changed: {path}")
        except OSError as error:
            self._fail(f"input watch directory became unavailable: {error}")
        self._drain()

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


def observe_file(root: Path, name: str) -> dict[str, Any] | None:
    before = leaf_stat(root, name)
    content = read_file(root, name, missing_ok=True)
    after = leaf_stat(root, name)
    fields = (
        "st_dev",
        "st_ino",
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
        "device": after.st_dev,
        "inode": after.st_ino,
        "links": after.st_nlink,
    }
