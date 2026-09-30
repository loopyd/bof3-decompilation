"""Bounded Git queries and exact index snapshots for review transactions."""

from __future__ import annotations

import os
import secrets
import stat
import subprocess
from dataclasses import dataclass
from pathlib import Path

from harness.common.rename import describe_locations, publish_file
from harness.common.lease import verify_writer
from harness.common.deadlines import resolve_deadline
from harness.common.process import run_bounded


@dataclass(frozen=True)
class GitIndexSnapshot:
    """Exact path, content, inode identity, and filesystem state of an index."""

    path: Path
    content: bytes | None
    state: tuple[int, int, int, int, int, int, int, int] | None


@dataclass(frozen=True)
class GitState:
    """HEAD and a physical index snapshot observed through one path query."""

    index: GitIndexSnapshot
    head: str | None


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


# Only subcommands whose result is a pure function of the index and refs are cached;
# working-tree queries (status, diff, grep) are never cached.
_CACHEABLE_GIT = frozenset(
    {"rev-parse", "ls-files", "ls-tree", "show-ref", "for-each-ref", "symbolic-ref"}
)
_READ_GIT_CACHE: dict[tuple[str, tuple[str, ...]], tuple[tuple, str]] = {}


def _git_state_token(root: Path) -> tuple | None:
    """Cheap index/ref identity token; None disables caching for this root."""
    git = root / ".git"
    if not git.is_dir():
        return None
    entries: list[tuple] = []
    for name in ("index", "packed-refs"):
        try:
            status = os.stat(git / name, follow_symlinks=False)
        except OSError:
            entries.append((name, None))
        else:
            entries.append(
                (
                    name,
                    status.st_dev,
                    status.st_ino,
                    status.st_size,
                    status.st_mtime_ns,
                )
            )
    try:
        head = (git / "HEAD").read_bytes()
    except OSError:
        return None
    entries.append(("HEAD", head))
    if head.startswith(b"ref: "):
        ref = head[5:].strip().decode("utf-8", "surrogateescape")
        if not ref or ref.startswith("/") or ".." in ref.split("/"):
            return None
        try:
            status = os.stat(git / ref, follow_symlinks=False)
        except OSError:
            entries.append((ref, None))
        else:
            entries.append(
                (
                    ref,
                    status.st_dev,
                    status.st_ino,
                    status.st_size,
                    status.st_mtime_ns,
                )
            )
    return tuple(entries)


def read_git(root: Path, arguments: list[str]) -> str:
    """Run a captured Git query within the owner's cutoff, preserving filename bytes."""
    for key in os.environ:
        if (
            key
            in {
                "GIT_DIR",
                "GIT_WORK_TREE",
                "GIT_INDEX_FILE",
                "GIT_COMMON_DIR",
                "GIT_OBJECT_DIRECTORY",
                "GIT_ALTERNATE_OBJECT_DIRECTORIES",
                "GIT_CONFIG",
                "GIT_CONFIG_COUNT",
                "GIT_CONFIG_PARAMETERS",
                "GIT_CONFIG_SYSTEM",
            }
            and os.environ[key]
        ):
            raise ValueError(f"Git environment override prevents snapshot: {key}")
    if os.environ.get("GIT_CONFIG_GLOBAL") not in {None, "/dev/null"}:
        raise ValueError("custom global Git configuration prevents snapshot")
    key = (str(root), tuple(arguments))
    token = (
        _git_state_token(root) if arguments and arguments[0] in _CACHEABLE_GIT else None
    )
    if token is not None:
        cached = _READ_GIT_CACHE.get(key)
        if cached is not None and cached[0] == token:
            return cached[1]
    command = [
        "git",
        "--no-optional-locks",
        "-c",
        "core.fsmonitor=false",
        "-c",
        "submodule.recurse=false",
        *arguments,
    ]
    result = run_bounded(
        root,
        command,
        timeout=30,
        output_limit=2 * 1024 * 1024,
        deadline=resolve_deadline(),
        errors="surrogateescape",
    )
    if result["failure"] or result["exit_code"] != 0:
        code = (
            (124 if result["failure"] == "timeout" else 125)
            if result["failure"]
            else result["exit_code"]
        )
        cause = subprocess.CalledProcessError(
            code,
            command,
            output=result["stdout"],
            stderr=result["stderr"]
            + (
                f"\nGit query aborted: {result['failure']}" if result["failure"] else ""
            ),
        )
        reason = result["failure"] or f"exit {code}"
        raise RuntimeError(f"Git snapshot query failed: {reason}") from cause
    if token is not None:
        _READ_GIT_CACHE[key] = (token, result["stdout"])
    return result["stdout"]


def _index_path(root: Path) -> Path | None:
    if not (root / ".git").exists():
        return None
    return Path(
        read_git(
            root, ["rev-parse", "--path-format=absolute", "--git-path", "index"]
        ).rstrip("\n")
    )


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
        descriptor = os.open(
            path.name, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK, dir_fd=parent
        )
    except FileNotFoundError:
        return GitIndexSnapshot(path, None, None)
    with os.fdopen(descriptor, "rb") as stream:
        state = os.fstat(stream.fileno())
        if not stat.S_ISREG(state.st_mode) or state.st_size > 128 * 1024 * 1024:
            raise ValueError("Git index is not a regular file")
        content = stream.read(128 * 1024 * 1024 + 1)
        if len(content) > 128 * 1024 * 1024 or _index_state(
            os.fstat(stream.fileno())
        ) != _index_state(state):
            raise ValueError("Git index changed during capture")
        return GitIndexSnapshot(path, content, _index_state(state))


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
    """Move an owned artifact through the checked publication backend."""

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
        publish_file(leaf, quarantine_leaf, src_dir_fd=parent, dst_dir_fd=parent)
    except BaseException as error:
        failure = (
            "collision" if isinstance(error, FileExistsError) else "publication failed"
        )
        raise RuntimeError(
            f"Git cleanup quarantine {failure}; outcome requires verification; "
            f"{describe_locations(parent, leaf, quarantine_leaf)} for manual recovery"
        ) from error
    moved = _capture_index_at(parent, Path(quarantine_leaf))
    moved_identity = moved.state[:2] if moved.state is not None else None
    if moved_identity != identity or moved.content != content:
        try:
            publish_file(quarantine_leaf, leaf, src_dir_fd=parent, dst_dir_fd=parent)
        except BaseException as error:
            raise RuntimeError(
                "Git cleanup quarantine verification failed; restoration outcome "
                f"requires verification; {describe_locations(parent, quarantine_leaf, leaf)} "
                "for manual recovery"
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
            publish_file(recovery, index_leaf, src_dir_fd=parent, dst_dir_fd=parent)
        except BaseException as error:
            raise RuntimeError(
                "Git index changed concurrently during rollback; restoration outcome "
                f"requires verification; {describe_locations(parent, recovery, index_leaf)} "
                "for manual recovery"
            ) from error
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
        try:
            publish_file(
                backup.path.name,
                recovery_leaf,
                src_dir_fd=parent,
                dst_dir_fd=parent,
            )
        except BaseException as error:
            raise RuntimeError(
                "Git index quarantine failed; outcome requires verification; "
                f"{describe_locations(parent, backup.path.name, recovery_leaf, backup_leaf)} "
                "for manual recovery"
            ) from error
        _restore_moved_index(parent, recovery_leaf, backup.path.name, expected)
    if backup.content is not None:
        try:
            publish_file(
                backup_leaf,
                backup.path.name,
                src_dir_fd=parent,
                dst_dir_fd=parent,
            )
        except BaseException as error:
            failure = (
                "appeared concurrently"
                if isinstance(error, FileExistsError)
                else "publication failed"
            )
            raise RuntimeError(
                f"Git index {failure}; outcome requires verification; "
                f"{describe_locations(parent, backup_leaf, backup.path.name, recovery_leaf)} "
                "for manual recovery"
            ) from error
    os.fsync(parent)
    return True


def capture_git_index(root: Path) -> GitIndexSnapshot | None:
    path = _index_path(root)
    return _capture_index_path(path) if path is not None else None


def capture_git_state(root: Path) -> GitState | None:
    if not (root / ".git").exists():
        return None
    unborn = False
    try:
        output = read_git(
            root,
            [
                "rev-parse",
                "--path-format=absolute",
                "--git-path",
                "index",
                "--verify",
                "--quiet",
                "HEAD",
            ],
        )
    except RuntimeError as error:
        cause = error.__cause__
        if (
            not isinstance(cause, subprocess.CalledProcessError)
            or cause.returncode != 1
        ):
            raise
        output = cause.output
        unborn = True
    if not isinstance(output, str) or not output.endswith("\n"):
        raise ValueError("invalid Git index/HEAD observation")
    if unborn:
        name, head = output[:-1], None
    else:
        name, separator, head = output[:-1].rpartition("\n")
        if not separator or not head:
            raise ValueError("invalid Git index/HEAD observation")
    path = Path(name)
    if not path.is_absolute():
        raise ValueError("Git index observation is not absolute")
    return GitState(_capture_index_path(path), head)


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
    verify_writer(root)
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
        verify_writer(root)
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
        verify_writer(root)
        remove_backup = not _restore_git_index_locked(
            parent, backup_leaf, recovery_leaf, backup, expected
        )
        if backup.content is None and not remove_backup:
            remove_backup = True
    finally:
        try:
            verify_writer(root)
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
    return read_git(root, ["ls-files", "--stage", "-z"]).encode(
        "utf-8", "surrogateescape"
    )
