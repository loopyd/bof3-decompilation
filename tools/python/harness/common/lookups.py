"""Observe bounded logical path lookups through held nofollow directory descriptors."""

from __future__ import annotations

import copy
import hashlib
import os
import stat
from collections import deque
from collections.abc import Callable
from pathlib import Path

from harness.common.deadlines import check_deadline

_DIRECTORY_FLAGS = os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW | os.O_CLOEXEC
_FILE_FLAGS = os.O_RDONLY | os.O_NOFOLLOW | os.O_CLOEXEC | os.O_NONBLOCK
_MAX_NODES = 8192
_MAX_DIRECTORIES = 512
_MAX_FILE_BYTES = 64 * 1024 * 1024
_MAX_TOTAL_BYTES = 256 * 1024 * 1024


def _identity(status: os.stat_result) -> list[int]:
    values = [
        status.st_dev,
        status.st_ino,
        status.st_mode,
        status.st_uid,
        status.st_gid,
    ]
    if not stat.S_ISDIR(status.st_mode):
        values.extend(
            [status.st_nlink, status.st_size, status.st_mtime_ns, status.st_ctime_ns]
        )
    return values


def resolve_lookup(
    name: str, *, cwd: Path, observe: Callable[[Path, str], dict]
) -> dict:
    """Walk bounded raw lookup spelling with a caller-owned node observation source."""
    if (
        not isinstance(name, str)
        or not name
        or "\0" in name
        or len(os.fsencode(name)) > 16384
        or not cwd.is_absolute()
        or ".." in cwd.parts
    ):
        raise ValueError("lookup requires a bounded name and canonical CWD")
    raw = name if name.startswith("/") else str(cwd) + "/" + name
    pending = deque(raw.split("/")[1:])
    parent = Path("/")
    trail = [str(parent)]
    expansions = 0
    steps = 0
    terminal = parent
    status = "directory"
    while pending:
        check_deadline()
        steps += 1
        if steps > 256:
            raise ValueError("lookup path-step budget exceeded")
        component = pending.popleft()
        if component in {"", "."}:
            terminal = parent
            status = "directory"
            continue
        if component == "..":
            parent = parent.parent
            terminal = parent
            status = "directory"
            trail.append(str(parent))
            continue
        terminal = parent / component
        trail.append(str(terminal))
        node = observe(parent, component)
        status = node["kind"]
        if status == "symlink":
            expansions += 1
            if expansions > 40:
                raise ValueError("lookup alias expansion budget exceeded")
            target = node["target"]
            if target.startswith("/"):
                parent = Path("/")
            pending.extendleft(reversed(target.split("/")))
        elif status == "directory":
            parent = terminal
        elif status == "missing":
            break
        elif pending:
            status = "not-directory"
            break
    check_deadline()
    return {
        "spelling": name,
        "cwd": str(cwd),
        "trail": trail,
        "terminal": str(terminal),
        "status": status,
    }


class LookupBatch:
    """Capture requested lookup chains only; values remain provisional until exit."""

    def __init__(self) -> None:
        self._directories: dict[Path, int] = {}
        self._nodes: dict[str, dict] = {}
        self._bytes = 0
        self._active = False
        self._closed = False
        self._failed = False

    def __enter__(self) -> LookupBatch:
        try:
            check_deadline()
            if self._active or self._closed:
                raise ValueError("lookup batch cannot be reused")
            self._active = True
            root = Path("/")
            descriptor = os.open(root, _DIRECTORY_FLAGS)
            self._directories[root] = descriptor
            status = os.fstat(descriptor)
            self._nodes[str(root)] = {
                "kind": "directory",
                "identity": _identity(status),
            }
            return self
        except BaseException:
            self.close()
            raise

    def _require_active(self) -> None:
        check_deadline()
        if not self._active or self._closed or self._failed:
            raise ValueError("lookup batch is inactive or failed")

    def _read_file(self, parent: Path, name: str, before: os.stat_result) -> str:
        if before.st_nlink != 1:
            raise ValueError("lookup file exceeds link bounds")
        if before.st_size > _MAX_FILE_BYTES:
            # An oversized unrelated program on PATH (e.g. a version-manager
            # binary) is bound by identity rather than read into memory.
            return hashlib.sha256(
                f"{before.st_dev}:{before.st_ino}:{before.st_size}:{before.st_mtime_ns}".encode()
            ).hexdigest()
        descriptor = os.open(name, _FILE_FLAGS, dir_fd=self._directories[parent])
        try:
            if _identity(os.fstat(descriptor)) != _identity(before):
                raise ValueError("lookup file changed before reading")
            checksum = hashlib.sha256()
            size = 0
            while True:
                check_deadline()
                allowance = min(
                    1024 * 1024,
                    _MAX_FILE_BYTES - size,
                    _MAX_TOTAL_BYTES - self._bytes,
                )
                content = os.read(descriptor, allowance + 1)
                size += len(content)
                self._bytes += len(content)
                if size > _MAX_FILE_BYTES or self._bytes > _MAX_TOTAL_BYTES:
                    raise ValueError("lookup content exceeds its byte budget")
                if not content:
                    break
                checksum.update(content)
            if (
                size != before.st_size
                or _identity(os.fstat(descriptor)) != _identity(before)
                or _identity(
                    os.stat(
                        name, dir_fd=self._directories[parent], follow_symlinks=False
                    )
                )
                != _identity(before)
            ):
                raise ValueError("lookup file changed while reading")
            return checksum.hexdigest()
        finally:
            os.close(descriptor)

    def _observe(self, parent: Path, name: str) -> dict:
        self._require_active()
        path = parent / name
        key = str(path)
        if key in self._nodes:
            return self._nodes[key]
        if len(self._nodes) >= _MAX_NODES:
            raise ValueError("lookup node budget exceeded")
        descriptor = self._directories[parent]
        try:
            before = os.stat(name, dir_fd=descriptor, follow_symlinks=False)
        except FileNotFoundError:
            node = {"kind": "missing"}
        else:
            node = {"kind": "nonregular", "identity": _identity(before)}
            if stat.S_ISDIR(before.st_mode):
                if len(self._directories) >= _MAX_DIRECTORIES:
                    raise ValueError("lookup directory budget exceeded")
                opened = os.open(name, _DIRECTORY_FLAGS, dir_fd=descriptor)
                self._directories[path] = opened
                if _identity(os.fstat(opened)) != _identity(before):
                    raise ValueError("lookup directory changed during capture")
                node["kind"] = "directory"
            elif stat.S_ISLNK(before.st_mode):
                target = os.readlink(name, dir_fd=descriptor)
                if not target or len(os.fsencode(target)) > 16384:
                    raise ValueError("lookup alias target exceeds its bound")
                node.update(kind="symlink", target=target)
            elif stat.S_ISREG(before.st_mode):
                node.update(kind="file", sha256=self._read_file(parent, name, before))
            if _identity(
                os.stat(name, dir_fd=descriptor, follow_symlinks=False)
            ) != _identity(before):
                raise ValueError("lookup entry changed during capture")
        self._nodes[key] = node
        return node

    def observe(self, name: str, *, cwd: Path) -> dict:
        """Resolve raw spelling without lexical normalization across symlink edges."""
        try:
            self._require_active()
            if cwd.resolve() != cwd:
                raise ValueError("lookup requires a bounded name and canonical CWD")
            return resolve_lookup(name, cwd=cwd, observe=self._observe)
        except BaseException:
            self._failed = True
            raise

    def describe(self) -> dict[str, dict]:
        """Return provisional node copies; successful exit remains mandatory."""
        try:
            self._require_active()
            return copy.deepcopy(self._nodes)
        except BaseException:
            self._failed = True
            raise

    def _verify(self) -> None:
        self._require_active()
        for name, node in self._nodes.items():
            check_deadline()
            path = Path(name)
            try:
                status = (
                    os.stat(path, follow_symlinks=False)
                    if path == path.parent
                    else os.stat(
                        path.name,
                        dir_fd=self._directories[path.parent],
                        follow_symlinks=False,
                    )
                )
            except FileNotFoundError:
                if node["kind"] == "missing":
                    continue
                raise ValueError("lookup entry disappeared") from None
            if node["kind"] == "missing" or _identity(status) != node["identity"]:
                raise ValueError("lookup entry changed before batch exit")
            if (
                node["kind"] == "directory"
                and _identity(os.fstat(self._directories[path])) != node["identity"]
            ):
                raise ValueError("lookup directory became detached")
            if (
                node["kind"] == "symlink"
                and os.readlink(path.name, dir_fd=self._directories[path.parent])
                != node["target"]
            ):
                raise ValueError("lookup alias changed before batch exit")
        check_deadline()

    def close(self) -> None:
        """Release owned descriptors even after expiry or partial setup failure."""
        descriptors = list(self._directories.values())
        self._directories.clear()
        self._active = False
        self._closed = True
        error = None
        for descriptor in reversed(descriptors):
            try:
                os.close(descriptor)
            except OSError as caught:
                error = caught
        if error is not None:
            raise error

    def __exit__(self, error_type, error, traceback) -> None:
        try:
            if error_type is None:
                self._verify()
        finally:
            self.close()
