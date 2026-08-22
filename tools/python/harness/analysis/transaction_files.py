"""Confined, symlink-safe repository file operations for transactions."""

from __future__ import annotations

import hashlib
import os
import secrets
import stat
from pathlib import Path

from .rename_noreplace import (
    UnsupportedRenameNoReplaceError,
    publish_noreplace,
    rename_noreplace as _rename_noreplace,
    require_native_noreplace,
)

QUARANTINE_DIRECTORY = "out/reviews/evidence/quarantine"
_MISSING = object()


def canonical_repo_path(value: object) -> str:
    if not isinstance(value, str) or not value or "\\" in value:
        raise ValueError("transaction path must be canonical repo-relative")
    path = Path(value)
    if (
        path.is_absolute()
        or path.as_posix() != value
        or any(part in {"", ".", ".."} for part in path.parts)
    ):
        raise ValueError("transaction path must be canonical repo-relative")
    return value


def _root_fd(root: Path) -> int:
    if root.is_symlink():
        raise ValueError("transaction root must not be a symlink")
    try:
        resolved = root.resolve(strict=True)
    except OSError as error:
        raise ValueError("transaction root is invalid") from error
    if not resolved.is_dir():
        raise ValueError("transaction root is invalid")
    return os.open(resolved, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)


def _parent_chain(
    root: Path, name: str, *, create: bool = False
) -> tuple[list[int], str]:
    parts = Path(canonical_repo_path(name)).parts
    descriptors = [_root_fd(root)]
    try:
        for part in parts[:-1]:
            try:
                child = os.open(
                    part,
                    os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW,
                    dir_fd=descriptors[-1],
                )
            except FileNotFoundError:
                if not create:
                    raise
                os.mkdir(part, 0o755, dir_fd=descriptors[-1])
                child = os.open(
                    part,
                    os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW,
                    dir_fd=descriptors[-1],
                )
            except OSError as error:
                raise ValueError(
                    f"transaction path has unsafe component: {name}"
                ) from error
            descriptors.append(child)
        return descriptors, parts[-1]
    except BaseException:
        for descriptor in reversed(descriptors):
            os.close(descriptor)
        raise


def parent_fd(root: Path, name: str, *, create: bool = False) -> tuple[int, str]:
    descriptors, leaf = _parent_chain(root, name, create=create)
    for descriptor in descriptors[:-1]:
        os.close(descriptor)
    return descriptors[-1], leaf


def _verify_parent_chain(root: Path, name: str, descriptors: list[int]) -> None:
    root_stat = os.stat(root, follow_symlinks=False)
    opened_root = os.fstat(descriptors[0])
    if stat.S_ISLNK(root_stat.st_mode) or (
        root_stat.st_dev,
        root_stat.st_ino,
    ) != (opened_root.st_dev, opened_root.st_ino):
        raise ValueError(f"transaction parent detached from canonical path: {name}")
    for index, part in enumerate(Path(name).parts[:-1]):
        try:
            linked = os.stat(part, dir_fd=descriptors[index], follow_symlinks=False)
        except FileNotFoundError as error:
            raise ValueError(
                f"transaction parent detached from canonical path: {name}"
            ) from error
        opened = os.fstat(descriptors[index + 1])
        if not stat.S_ISDIR(linked.st_mode) or (linked.st_dev, linked.st_ino) != (
            opened.st_dev,
            opened.st_ino,
        ):
            raise ValueError(f"transaction parent detached from canonical path: {name}")


def _read_leaf_state(
    parent: int, leaf: str, name: str, *, missing_ok: bool
) -> tuple[bytes | None, os.stat_result | None]:
    try:
        descriptor = os.open(leaf, os.O_RDONLY | os.O_NOFOLLOW, dir_fd=parent)
    except FileNotFoundError:
        if missing_ok:
            return None, None
        raise
    except OSError as error:
        raise ValueError(f"transaction path is unsafe: {name}") from error
    with os.fdopen(descriptor, "rb") as stream:
        leaf_state = os.fstat(stream.fileno())
        if not stat.S_ISREG(leaf_state.st_mode):
            raise ValueError(f"transaction path is not a regular file: {name}")
        return stream.read(), leaf_state


def _read_leaf(parent: int, leaf: str, name: str, *, missing_ok: bool) -> bytes | None:
    return _read_leaf_state(parent, leaf, name, missing_ok=missing_ok)[0]


def _close_descriptors(descriptors: list[int]) -> None:
    for descriptor in reversed(descriptors):
        os.close(descriptor)


def read_file(root: Path, name: str, *, missing_ok: bool = False) -> bytes | None:
    try:
        parent, leaf = parent_fd(root, name)
    except FileNotFoundError:
        if missing_ok:
            return None
        raise
    try:
        return _read_leaf(parent, leaf, name, missing_ok=missing_ok)
    finally:
        os.close(parent)


def preflight_existing_replacements(root: Path, names: set[str]) -> None:
    """Reject unsupported filesystems before replacement transactions create state."""
    for name in sorted(names):
        try:
            parent, leaf = parent_fd(root, name)
        except FileNotFoundError:
            continue
        try:
            if _read_leaf(parent, leaf, name, missing_ok=True) is not None:
                require_native_noreplace(parent, leaf)
        finally:
            os.close(parent)


def atomic_write(
    root: Path,
    name: str,
    content: bytes,
    *,
    expected: bytes | None | object = _MISSING,
    exclusive: bool = False,
) -> str | None:
    """Install content without replacing or deleting an unverified inode."""

    descriptors, leaf = _parent_chain(root, name, create=True)
    parent = descriptors[-1]
    temporary = f".{leaf}.transaction-{secrets.token_hex(12)}"
    temporary_name = str(Path(name).with_name(temporary))
    descriptor = -1
    quarantine: str | None = None
    try:
        current, current_stat = _read_leaf_state(parent, leaf, name, missing_ok=True)
        if expected is not _MISSING and current != expected:
            raise ValueError(
                f"transaction path drifted immediately before write: {name}"
            )
        if exclusive and current is not None:
            raise FileExistsError(name)
        mode = (current_stat.st_mode & 0o777) if current_stat is not None else 0o644
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
            os.fsync(stream.fileno())
        _verify_parent_chain(root, name, descriptors)
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
        _verify_parent_chain(root, name, descriptors)
        try:
            try:
                _rename_noreplace(temporary, leaf, src_dir_fd=parent, dst_dir_fd=parent)
            except UnsupportedRenameNoReplaceError:
                publish_noreplace(temporary, leaf, src_dir_fd=parent, dst_dir_fd=parent)
        except BaseException as error:
            recovery = f"temporary retained as {temporary_name}"
            if quarantine is not None:
                recovery += f"; quarantine retained as {quarantine}"
            raise RuntimeError(
                f"transaction commit failed without replacing {name}; "
                f"{recovery} for manual recovery"
            ) from error
        _verify_parent_chain(root, name, descriptors)
        os.fsync(parent)
        return quarantine
    finally:
        if descriptor >= 0:
            os.close(descriptor)
        _close_descriptors(descriptors)


def _quarantine_name(name: str) -> str:
    fingerprint = hashlib.sha256(name.encode()).hexdigest()[:16]
    return f"{secrets.token_hex(16)}-{fingerprint}"


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
    _verify_parent_chain(root, quarantine, quarantine_descriptors)
    _verify_parent_chain(root, name, descriptors)
    restored = _read_leaf(
        quarantine_descriptors[-1], quarantine_leaf, quarantine, missing_ok=False
    )
    source_now = os.stat(
        quarantine_leaf,
        dir_fd=quarantine_descriptors[-1],
        follow_symlinks=False,
    )
    if restored != content or (source_now.st_dev, source_now.st_ino) != (
        source_stat.st_dev,
        source_stat.st_ino,
    ):
        raise ValueError(
            f"transaction quarantine drifted during rollback: {quarantine}"
        )
    try:
        _rename_noreplace(
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
    moved = _read_leaf(descriptors[-1], leaf, name, missing_ok=False)
    moved_stat = os.stat(leaf, dir_fd=descriptors[-1], follow_symlinks=False)
    if moved != content or (moved_stat.st_dev, moved_stat.st_ino) != (
        source_stat.st_dev,
        source_stat.st_ino,
    ):
        raise ValueError(f"transaction path drifted during rollback: {name}")
    os.fsync(quarantine_descriptors[-1])
    os.fsync(descriptors[-1])


def restore_quarantined(
    root: Path, name: str, quarantine: str, *, expected: bytes
) -> None:
    """Restore one recorded quarantine only when its destination is absent."""

    descriptors, _leaf = _parent_chain(root, name)
    quarantine_descriptors, quarantine_leaf = _parent_chain(root, quarantine)
    try:
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
        _close_descriptors(quarantine_descriptors)
        _close_descriptors(descriptors)


def safe_unlink(
    root: Path,
    name: str,
    *,
    expected: bytes | None | object = _MISSING,
    expected_identity: tuple[int, int] | None = None,
) -> str | None:
    """Move a regular file to durable quarantine without deleting any inode."""

    descriptors, leaf = _parent_chain(root, name)
    quarantine = f"{QUARANTINE_DIRECTORY}/{_quarantine_name(name)}"
    quarantine_descriptors: list[int] = []
    moved = False
    source_stat: os.stat_result | None = None
    current = b""
    try:
        _verify_parent_chain(root, name, descriptors)
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

        quarantine_descriptors, quarantine_leaf = _parent_chain(
            root, quarantine, create=True
        )
        if (
            os.fstat(descriptors[-1]).st_dev
            != os.fstat(quarantine_descriptors[-1]).st_dev
        ):
            raise ValueError("transaction quarantine must share repository filesystem")
        _verify_parent_chain(root, name, descriptors)
        _verify_parent_chain(root, quarantine, quarantine_descriptors)
        try:
            _rename_noreplace(
                leaf,
                quarantine_leaf,
                src_dir_fd=descriptors[-1],
                dst_dir_fd=quarantine_descriptors[-1],
            )
        except BaseException:
            try:
                moved = (
                    os.stat(
                        quarantine_leaf,
                        dir_fd=quarantine_descriptors[-1],
                        follow_symlinks=False,
                    ).st_ino
                    == source_stat.st_ino
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
        if moved_content != current or (moved_stat.st_dev, moved_stat.st_ino) != (
            source_stat.st_dev,
            source_stat.st_ino,
        ):
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
        _verify_parent_chain(root, name, descriptors)
        _verify_parent_chain(root, quarantine, quarantine_descriptors)
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
        _close_descriptors(quarantine_descriptors)
        _close_descriptors(descriptors)
