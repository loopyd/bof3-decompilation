"""Git index and workspace snapshots for review transactions."""

from __future__ import annotations

import os
import secrets
import stat
import subprocess
from dataclasses import dataclass
from pathlib import Path

from .transaction_files import (
    _rename_noreplace,
    atomic_write,
    canonical_repo_path,
    read_file,
    safe_unlink,
)


@dataclass(frozen=True)
class GitIndexSnapshot:
    """Exact path, content, inode identity, and filesystem state of an index."""

    path: Path
    content: bytes | None
    state: tuple[int, int, int, int, int, int, int, int] | None


def _index_state(
    value: os.stat_result,
) -> tuple[int, int, int, int, int, int, int, int]:
    return (
        value.st_dev,
        value.st_ino,
        value.st_mode,
        value.st_uid,
        value.st_gid,
        value.st_size,
        value.st_mtime_ns,
        value.st_ctime_ns,
    )


def _index_path(root: Path) -> Path | None:
    if not (root / ".git").exists():
        return None
    value = subprocess.run(
        ["git", "rev-parse", "--path-format=absolute", "--git-path", "index"],
        cwd=root,
        capture_output=True,
        check=True,
    ).stdout.rstrip(b"\n")
    return Path(os.fsdecode(value))


def _verify_index_parent(path: Path, parent: int) -> None:
    linked = os.stat(path.parent, follow_symlinks=False)
    opened = os.fstat(parent)
    if not stat.S_ISDIR(linked.st_mode) or (linked.st_dev, linked.st_ino) != (
        opened.st_dev,
        opened.st_ino,
    ):
        raise RuntimeError("Git index parent changed concurrently")


def _capture_index_at(parent: int, path: Path) -> GitIndexSnapshot:
    try:
        descriptor = os.open(path.name, os.O_RDONLY | os.O_NOFOLLOW, dir_fd=parent)
    except FileNotFoundError:
        return GitIndexSnapshot(path, None, None)
    with os.fdopen(descriptor, "rb") as stream:
        state = os.fstat(stream.fileno())
        if not stat.S_ISREG(state.st_mode):
            raise ValueError("Git index is not a regular file")
        return GitIndexSnapshot(path, stream.read(), _index_state(state))


def _capture_index_path(path: Path) -> GitIndexSnapshot:
    parent = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
    try:
        _verify_index_parent(path, parent)
        return _capture_index_at(parent, path)
    finally:
        os.close(parent)


def _quarantine_owned_artifact(
    parent: int,
    leaf: str,
    quarantine_leaf: str,
    identity: tuple[int, int],
    content: bytes,
) -> None:
    """Move an owned artifact without clobbering or deleting any inode."""

    try:
        descriptor = os.open(leaf, os.O_RDONLY | os.O_NOFOLLOW, dir_fd=parent)
    except FileNotFoundError as error:
        raise RuntimeError(
            f"Git cleanup source disappeared; {leaf} and reserved "
            f"{quarantine_leaf} require manual recovery"
        ) from error
    with os.fdopen(descriptor, "rb") as stream:
        current = os.fstat(stream.fileno())
        current_content = stream.read()
    if (
        not stat.S_ISREG(current.st_mode)
        or (
            current.st_dev,
            current.st_ino,
        )
        != identity
        or current_content != content
    ):
        raise RuntimeError(
            f"Git cleanup source was concurrently replaced; unexpected {leaf} "
            f"preserved and quarantine target reserved at {quarantine_leaf}"
        )
    try:
        _rename_noreplace(leaf, quarantine_leaf, src_dir_fd=parent, dst_dir_fd=parent)
    except OSError as error:
        raise RuntimeError(
            f"Git cleanup quarantine collision; {leaf} preserved and existing "
            f"{quarantine_leaf} retained for manual recovery"
        ) from error
    moved = _capture_index_at(parent, Path(quarantine_leaf))
    moved_identity = moved.state[:2] if moved.state is not None else None
    if moved_identity != identity or moved.content != content:
        try:
            _rename_noreplace(
                quarantine_leaf, leaf, src_dir_fd=parent, dst_dir_fd=parent
            )
        except OSError as error:
            raise RuntimeError(
                f"Git cleanup quarantine verification failed; unexpected artifact "
                f"retained at {quarantine_leaf} and {leaf} requires manual recovery"
            ) from error
        os.fsync(parent)
        raise RuntimeError(
            f"Git cleanup source was substituted at rename boundary; unexpected "
            f"{leaf} restored unchanged and owned artifact requires manual recovery"
        )
    os.fsync(parent)
    # Quarantine is intentionally durable audit evidence; never unlink it.


_LOCK_CONTENT = b""


def _write_index_artifact(
    parent: int, leaf: str, content: bytes, *, state: tuple[int, ...] | None = None
) -> tuple[int, int]:
    descriptor = os.open(
        leaf,
        os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW,
        stat.S_IMODE(state[2]) if state is not None else 0o600,
        dir_fd=parent,
    )
    with os.fdopen(descriptor, "wb") as stream:
        stream.write(content)
        stream.flush()
        if state is not None:
            os.fchmod(stream.fileno(), stat.S_IMODE(state[2]))
            os.utime(stream.fileno(), ns=(state[6], state[6]))
        os.fsync(stream.fileno())
        identity = os.fstat(stream.fileno())
    os.fsync(parent)
    return identity.st_dev, identity.st_ino


def _restore_moved_index(
    parent: int, recovery: str, index_leaf: str, expected: GitIndexSnapshot
) -> None:
    moved = _capture_index_at(parent, expected.path.with_name(recovery))
    # Rename updates ctime; every other captured field and the inode must persist.
    moved_state = moved.state[:-1] if moved.state is not None else None
    expected_state = expected.state[:-1] if expected.state is not None else None
    if moved.content != expected.content or moved_state != expected_state:
        try:
            _rename_noreplace(
                recovery, index_leaf, src_dir_fd=parent, dst_dir_fd=parent
            )
        except OSError:
            pass
        raise RuntimeError(
            "Git index changed concurrently during rollback; current index restored"
        )


def _restore_git_index_locked(
    parent: int,
    backup_leaf: str,
    recovery_leaf: str,
    backup: GitIndexSnapshot,
    expected: GitIndexSnapshot,
) -> bool:
    _verify_index_parent(backup.path, parent)
    current = _capture_index_at(parent, backup.path)
    if current != expected:
        raise RuntimeError(
            f"Git index changed concurrently; current index preserved and backup "
            f"retained at {backup.path.with_name(backup_leaf)}"
        )
    if current == backup:
        return False
    if expected.content is not None:
        _rename_noreplace(
            backup.path.name,
            recovery_leaf,
            src_dir_fd=parent,
            dst_dir_fd=parent,
        )
        _restore_moved_index(parent, recovery_leaf, backup.path.name, expected)
    if backup.content is not None:
        try:
            _rename_noreplace(
                backup_leaf,
                backup.path.name,
                src_dir_fd=parent,
                dst_dir_fd=parent,
            )
        except OSError as error:
            raise RuntimeError(
                f"Git index appeared concurrently; current index preserved, backup "
                f"retained at {backup.path.with_name(backup_leaf)}, and transaction "
                f"index retained at {backup.path.with_name(recovery_leaf)}"
            ) from error
    os.fsync(parent)
    return True


def capture_git_index(root: Path) -> GitIndexSnapshot | None:
    path = _index_path(root)
    return _capture_index_path(path) if path is not None else None


def git_index_backup(root: Path) -> GitIndexSnapshot | None:
    return capture_git_index(root)


def restore_git_index(
    root: Path,
    backup: GitIndexSnapshot | None,
    expected: GitIndexSnapshot | None,
) -> None:
    """Restore only an unchanged transaction-produced index, never clobbering it."""

    if backup is None:
        return
    if (
        expected is None
        or expected.path != backup.path
        or _index_path(root) != backup.path
    ):
        raise RuntimeError("Git index path changed during transaction rollback")
    parent = os.open(backup.path.parent, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
    _verify_index_parent(backup.path, parent)
    token = secrets.token_hex(12)
    backup_leaf = f"index.transaction-backup-{token}"
    recovery_leaf = f"index.transaction-current-{token}"
    lock_leaf = "index.lock"
    lock_quarantine_leaf = f"index.transaction-quarantine-lock-{token}"
    backup_quarantine_leaf = f"index.transaction-quarantine-backup-{token}"
    lock_identity: tuple[int, int] | None = None
    remove_backup = False
    backup_identity: tuple[int, int] | None = None
    try:
        backup_identity = _write_index_artifact(
            parent, backup_leaf, backup.content or b"", state=backup.state
        )
        lock = os.open(
            lock_leaf,
            os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW,
            0o600,
            dir_fd=parent,
        )
        lock_state = os.fstat(lock)
        lock_identity = (lock_state.st_dev, lock_state.st_ino)
        os.close(lock)
        _verify_index_parent(backup.path, parent)
        remove_backup = not _restore_git_index_locked(
            parent, backup_leaf, recovery_leaf, backup, expected
        )
        if backup.content is None and not remove_backup:
            remove_backup = True
    finally:
        try:
            if lock_identity is not None:
                _quarantine_owned_artifact(
                    parent,
                    lock_leaf,
                    lock_quarantine_leaf,
                    lock_identity,
                    _LOCK_CONTENT,
                )
            if remove_backup and backup_identity is not None:
                _quarantine_owned_artifact(
                    parent,
                    backup_leaf,
                    backup_quarantine_leaf,
                    backup_identity,
                    backup.content or b"",
                )
        finally:
            os.close(parent)


# Semantic staged entries are intentionally separate from the exact index snapshot.


def git_index_state(root: Path) -> bytes | None:
    """Capture semantic staged entries without mutable stat-cache bytes."""

    if not (root / ".git").is_dir():
        return None
    return subprocess.run(
        ["git", "ls-files", "--stage", "-z"],
        cwd=root,
        capture_output=True,
        check=True,
    ).stdout


def workspace_backup(root: Path) -> dict[str, bytes]:
    if not (root / ".git").exists():
        return {}
    result = subprocess.run(
        ["git", "ls-files", "-co", "--exclude-standard", "-z"],
        cwd=root,
        capture_output=True,
        check=True,
    )
    names = result.stdout.decode(errors="surrogateescape").split("\0")
    backup = {}
    for name in names:
        if not name or name.startswith(
            ("out/", "sessions/subagent-artifacts/", ".pi/subagents/")
        ):
            continue
        canonical_repo_path(name)
        content = read_file(root, name, missing_ok=True)
        if content is not None:
            backup[name] = content
    return backup


def rollback_workspace(root: Path, backup: dict[str, bytes]) -> list[str]:
    errors = []
    quarantines = []
    current = workspace_backup(root)
    for name in sorted(set(current) - set(backup)):
        try:
            quarantine = safe_unlink(root, name, expected=current[name])
            if quarantine is not None:
                quarantines.append(quarantine)
        except (OSError, ValueError) as error:
            errors.append(f"{name}: {error}")
    for name, content in backup.items():
        try:
            atomic_write(root, name, content, expected=current.get(name))
        except (OSError, ValueError) as error:
            errors.append(f"{name}: {error}")
    if errors:
        raise RuntimeError("type transaction rollback failed: " + "; ".join(errors))
    return quarantines
