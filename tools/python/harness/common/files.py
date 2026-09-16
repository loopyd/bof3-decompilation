"""Confined, symlink-safe repository file operations for transactions."""

from __future__ import annotations

import os
import secrets
import stat
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.quarantine import (
    matches_identity,
    reserve_quarantine,
    validate_quarantine,
)
from harness.common.rename import describe_locations, publish_file
from harness.common.directory import (
    close_descriptors,
    open_parent_chain,
    open_parent_fd,
    validate_repo_path,
    verify_parent_chain,
)

_MISSING = object()


def read_leaf_state(
    parent: int,
    leaf: str,
    name: str,
    *,
    missing_ok: bool,
    max_bytes: int | None = None,
    expected: os.stat_result | None = None,
) -> tuple[bytes | None, os.stat_result | None]:
    """Read through a held parent, optionally binding pre-read file metadata."""
    validate_repo_path(leaf)
    if len(Path(leaf).parts) != 1:
        raise ValueError("file leaf must be one path component")
    if max_bytes is not None and (type(max_bytes) is not int or max_bytes < 0):
        raise ValueError("invalid transaction file capture limit")
    if expected is not None and not isinstance(expected, os.stat_result):
        raise ValueError("invalid expected file metadata")
    try:
        descriptor = os.open(
            leaf, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK, dir_fd=parent
        )
    except FileNotFoundError:
        if missing_ok:
            return None, None
        raise
    except OSError as error:
        raise ValueError(f"transaction path is unsafe: {name}") from error
    try:
        with os.fdopen(descriptor, "rb", closefd=False) as stream:
            leaf_state = os.fstat(stream.fileno())
            if not stat.S_ISREG(leaf_state.st_mode):
                raise ValueError(f"transaction path is not a regular file: {name}")
            if expected is not None and any(
                getattr(expected, field) != getattr(leaf_state, field)
                for field in (
                    "st_dev",
                    "st_ino",
                    "st_mode",
                    "st_nlink",
                    "st_uid",
                    "st_gid",
                    "st_size",
                    "st_mtime_ns",
                    "st_ctime_ns",
                )
            ):
                raise ValueError(f"file changed before descriptor read: {name}")
            if max_bytes is not None and leaf_state.st_size > max_bytes:
                raise ValueError(f"transaction file exceeds capture limit: {name}")
            content = stream.read() if max_bytes is None else stream.read(max_bytes + 1)
            if max_bytes is not None and len(content) > max_bytes:
                raise ValueError(f"transaction file exceeds capture limit: {name}")
            return content, leaf_state
    finally:
        os.close(descriptor)


def _read_leaf(parent: int, leaf: str, name: str, *, missing_ok: bool) -> bytes | None:
    return read_leaf_state(parent, leaf, name, missing_ok=missing_ok)[0]


def read_file(
    root: Path, name: str, *, missing_ok: bool = False, max_bytes: int | None = None
) -> bytes | None:
    check_deadline()
    if max_bytes is not None and (type(max_bytes) is not int or max_bytes < 0):
        raise ValueError("invalid transaction file capture limit")
    try:
        parent, leaf = open_parent_fd(root, name)
    except FileNotFoundError:
        if missing_ok:
            check_deadline()
            return None
        raise
    try:
        check_deadline()
        if max_bytes is not None:
            content = read_leaf_state(
                parent, leaf, name, missing_ok=missing_ok, max_bytes=max_bytes
            )[0]
        else:
            content = _read_leaf(parent, leaf, name, missing_ok=missing_ok)
    finally:
        os.close(parent)
    check_deadline()
    return content


def preflight_existing_replacements(root: Path, names: set[str]) -> None:
    """Validate existing leaves without claiming native move support."""
    for name in sorted(names):
        try:
            parent, leaf = open_parent_fd(root, name)
        except FileNotFoundError:
            continue
        try:
            _, current = read_leaf_state(parent, leaf, name, missing_ok=True)
            if current is not None and current.st_nlink != 1:
                raise ValueError(f"unsafe transaction path: {name}")
        finally:
            os.close(parent)


def atomic_write(
    root: Path,
    name: str,
    content: bytes,
    *,
    expected: bytes | None | object = _MISSING,
    exclusive: bool = False,
    mode: int | None = None,
    creation_mode: int = 0o644,
) -> str | None:
    """Publish verified content through the native or cooperative move backend."""

    if mode is not None and (type(mode) is not int or not 0 <= mode <= 0o7777):
        raise ValueError("invalid transaction file mode")
    if type(creation_mode) is not int or not 0 <= creation_mode <= 0o7777:
        raise ValueError("invalid transaction creation mode")
    explicit_mode = mode
    descriptors, leaf = open_parent_chain(root, name, create=True)
    parent = descriptors[-1]
    temporary = f".{leaf}.transaction-{secrets.token_hex(12)}"
    temporary_name = str(Path(name).with_name(temporary))
    descriptor = -1
    quarantine: str | None = None
    try:
        current, current_stat = read_leaf_state(parent, leaf, name, missing_ok=True)
        if expected is not _MISSING and current != expected:
            raise ValueError(
                f"transaction path drifted immediately before write: {name}"
            )
        if exclusive and current is not None:
            raise FileExistsError(name)
        if mode is None:
            mode = (
                (current_stat.st_mode & 0o777)
                if current_stat is not None
                else creation_mode
            )
        descriptor = os.open(
            temporary,
            os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW,
            mode,
            dir_fd=parent,
        )
        with os.fdopen(descriptor, "wb") as stream:
            descriptor = -1
            stream.write(content)
            stream.flush()
            if explicit_mode is not None:
                os.fchmod(stream.fileno(), explicit_mode)
            os.fsync(stream.fileno())
        verify_parent_chain(root, name, descriptors)
        if current_stat is not None:
            quarantine = safe_unlink(
                root,
                name,
                expected=current,
                expected_identity=(current_stat.st_dev, current_stat.st_ino),
            )
            assert quarantine is not None
        elif _read_leaf(parent, leaf, name, missing_ok=True) is not None:
            raise ValueError(
                f"transaction path drifted immediately before write: {name}; "
                f"temporary retained as {temporary_name} for manual recovery"
            )
        verify_parent_chain(root, name, descriptors)
        try:
            publish_file(temporary, leaf, src_dir_fd=parent, dst_dir_fd=parent)
        except BaseException as error:
            recovery = describe_locations(parent, temporary, leaf)
            if quarantine is not None:
                recovery += f"; quarantine retained as {quarantine}"
            raise RuntimeError(
                f"transaction publication failed for {name}; outcome requires verification; "
                f"{recovery} for manual recovery"
            ) from error
        verify_parent_chain(root, name, descriptors)
        os.fsync(parent)
        return quarantine
    finally:
        if descriptor >= 0:
            os.close(descriptor)
        close_descriptors(descriptors)


def _restore_quarantine(
    root: Path,
    name: str,
    descriptors: list[int],
    quarantine: str,
    quarantine_descriptors: list[int],
    source_stat: os.stat_result,
    content: bytes,
) -> None:
    quarantine_leaf = Path(quarantine).name
    leaf = Path(name).name
    verify_parent_chain(root, quarantine, quarantine_descriptors)
    verify_parent_chain(root, name, descriptors)
    restored = _read_leaf(
        quarantine_descriptors[-1], quarantine_leaf, quarantine, missing_ok=False
    )
    source_now = os.stat(
        quarantine_leaf,
        dir_fd=quarantine_descriptors[-1],
        follow_symlinks=False,
    )
    if (
        restored != content
        or not matches_identity(source_now, source_stat)
        or source_now.st_nlink != source_stat.st_nlink
    ):
        raise ValueError(
            f"transaction quarantine drifted during rollback: {quarantine}"
        )
    try:
        publish_file(
            quarantine_leaf,
            leaf,
            src_dir_fd=quarantine_descriptors[-1],
            dst_dir_fd=descriptors[-1],
        )
    except FileExistsError as error:
        raise ValueError(
            f"transaction path drifted during rollback: {name}; "
            f"quarantine retained as {quarantine} for manual recovery"
        ) from error
    verify_parent_chain(root, quarantine, quarantine_descriptors)
    verify_parent_chain(root, name, descriptors)
    moved = _read_leaf(descriptors[-1], leaf, name, missing_ok=False)
    moved_stat = os.stat(leaf, dir_fd=descriptors[-1], follow_symlinks=False)
    if (
        moved != content
        or not matches_identity(moved_stat, source_stat)
        or moved_stat.st_nlink != source_stat.st_nlink
    ):
        raise ValueError(f"transaction path drifted during rollback: {name}")
    os.fsync(quarantine_descriptors[-1])
    os.fsync(descriptors[-1])


def restore_quarantined(
    root: Path,
    name: str,
    quarantine: str,
    *,
    expected: bytes,
    expected_identity: tuple[int, int] | None = None,
    expected_mode: int | None = None,
    create: bool = False,
) -> None:
    """Restore one recorded quarantine only when its destination is absent."""

    descriptors, _leaf = open_parent_chain(root, name, create=create)
    quarantine_descriptors = []
    try:
        quarantine_descriptors, quarantine_leaf = open_parent_chain(root, quarantine)
        content = _read_leaf(
            quarantine_descriptors[-1], quarantine_leaf, quarantine, missing_ok=False
        )
        source_stat = os.stat(
            quarantine_leaf,
            dir_fd=quarantine_descriptors[-1],
            follow_symlinks=False,
        )
        if content is None or content != expected:
            raise ValueError(f"transaction quarantine drifted: {quarantine}")
        if (
            (
                expected_identity is not None
                and (source_stat.st_dev, source_stat.st_ino) != expected_identity
            )
            or (
                expected_mode is not None
                and stat.S_IMODE(source_stat.st_mode) != expected_mode
            )
            or source_stat.st_nlink != 1
        ):
            raise ValueError(f"transaction quarantine identity drifted: {quarantine}")
        _restore_quarantine(
            root,
            name,
            descriptors,
            quarantine,
            quarantine_descriptors,
            source_stat,
            content,
        )
    finally:
        close_descriptors(quarantine_descriptors)
        close_descriptors(descriptors)


def safe_unlink(
    root: Path,
    name: str,
    *,
    expected: bytes | None | object = _MISSING,
    expected_identity: tuple[int, int] | None = None,
    quarantine: str | None = None,
    expected_mode: int | None = None,
) -> str | None:
    """Move a regular file to durable quarantine without deleting any inode."""

    quarantine = (
        reserve_quarantine(name)
        if quarantine is None
        else validate_quarantine(name, quarantine)
    )
    descriptors, leaf = open_parent_chain(root, name)
    quarantine_descriptors: list[int] = []
    moved = False
    source_stat: os.stat_result | None = None
    current = b""
    try:
        verify_parent_chain(root, name, descriptors)
        try:
            source = os.open(leaf, os.O_RDONLY | os.O_NOFOLLOW, dir_fd=descriptors[-1])
        except FileNotFoundError:
            if expected not in {_MISSING, None}:
                raise ValueError(
                    f"transaction path drifted immediately before rollback: {name}"
                )
            return None
        except OSError as error:
            raise ValueError(f"transaction path is unsafe: {name}") from error
        with os.fdopen(source, "rb") as stream:
            source_stat = os.fstat(stream.fileno())
            if not stat.S_ISREG(source_stat.st_mode):
                raise ValueError(f"transaction path is not a regular file: {name}")
            current = stream.read()
        if expected is not _MISSING and current != expected:
            raise ValueError(
                f"transaction path drifted immediately before rollback: {name}"
            )
        if (
            expected_identity is not None
            and (
                source_stat.st_dev,
                source_stat.st_ino,
            )
            != expected_identity
        ):
            raise ValueError(
                f"transaction path inode drifted immediately before rollback: {name}"
            )

        if (
            expected_mode is not None
            and stat.S_IMODE(source_stat.st_mode) != expected_mode
        ):
            raise ValueError(f"transaction path mode drifted before quarantine: {name}")
        quarantine_descriptors, quarantine_leaf = open_parent_chain(
            root, quarantine, create=True
        )
        if (
            os.fstat(descriptors[-1]).st_dev
            != os.fstat(quarantine_descriptors[-1]).st_dev
        ):
            raise ValueError("transaction quarantine must share repository filesystem")
        verify_parent_chain(root, name, descriptors)
        verify_parent_chain(root, quarantine, quarantine_descriptors)
        try:
            publish_file(
                leaf,
                quarantine_leaf,
                src_dir_fd=descriptors[-1],
                dst_dir_fd=quarantine_descriptors[-1],
            )
        except BaseException:
            try:
                quarantine_state = os.stat(
                    quarantine_leaf,
                    dir_fd=quarantine_descriptors[-1],
                    follow_symlinks=False,
                )
                moved = (
                    quarantine_state.st_dev,
                    quarantine_state.st_ino,
                    quarantine_state.st_mode,
                    quarantine_state.st_nlink,
                    quarantine_state.st_size,
                    quarantine_state.st_mtime_ns,
                ) == (
                    source_stat.st_dev,
                    source_stat.st_ino,
                    source_stat.st_mode,
                    source_stat.st_nlink,
                    source_stat.st_size,
                    source_stat.st_mtime_ns,
                )
            except FileNotFoundError:
                moved = False
            raise
        moved = True

        moved_content = _read_leaf(
            quarantine_descriptors[-1], quarantine_leaf, quarantine, missing_ok=False
        )
        moved_stat = os.stat(
            quarantine_leaf,
            dir_fd=quarantine_descriptors[-1],
            follow_symlinks=False,
        )
        if moved_content != current or not matches_identity(moved_stat, source_stat):
            _restore_quarantine(
                root,
                name,
                descriptors,
                quarantine,
                quarantine_descriptors,
                moved_stat,
                current if moved_content is None else moved_content,
            )
            moved = False
            raise ValueError(f"transaction path drifted during rollback: {name}")
        verify_parent_chain(root, name, descriptors)
        verify_parent_chain(root, quarantine, quarantine_descriptors)
        os.fsync(descriptors[-1])
        os.fsync(quarantine_descriptors[-1])
        return quarantine
    except BaseException:
        if moved and source_stat is not None:
            _restore_quarantine(
                root,
                name,
                descriptors,
                quarantine,
                quarantine_descriptors,
                source_stat,
                current,
            )
        raise
    finally:
        close_descriptors(quarantine_descriptors)
        close_descriptors(descriptors)
