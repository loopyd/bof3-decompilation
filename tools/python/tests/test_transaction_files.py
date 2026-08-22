"""Adversarial race tests for confined transaction file writes and quarantine."""

from __future__ import annotations

import errno
import os
import subprocess
from pathlib import Path

import pytest

from harness.analysis import rename_noreplace, transaction_files, transaction_git


def _git_repo(tmp_path: Path) -> Path:
    subprocess.run(["git", "init", "-q"], cwd=tmp_path, check=True)
    subprocess.run(
        ["git", "config", "user.email", "test@example.invalid"],
        cwd=tmp_path,
        check=True,
    )
    subprocess.run(["git", "config", "user.name", "Test"], cwd=tmp_path, check=True)
    tracked = tmp_path / "tracked"
    tracked.write_text("before\n")
    subprocess.run(["git", "add", tracked.name], cwd=tmp_path, check=True)
    subprocess.run(["git", "commit", "-qm", "base"], cwd=tmp_path, check=True)
    return tracked


def _tree(tmp_path: Path) -> tuple[Path, Path]:
    parent = tmp_path / "include"
    parent.mkdir()
    (parent / "test.h").write_bytes(b"before\n")
    replacement = tmp_path / "replacement"
    replacement.mkdir()
    (replacement / "test.h").write_bytes(b"untouched\n")
    return parent, replacement


def _swap(tmp_path: Path, parent: Path, replacement: Path) -> None:
    parent.rename(tmp_path / "detached")
    replacement.rename(parent)


def _assert_swap_rejected(tmp_path: Path) -> None:
    assert (tmp_path / "include/test.h").read_bytes() == b"untouched\n"
    assert not list((tmp_path / "include").glob(".*.transaction-*"))


def test_atomic_write_rejects_parent_swap_during_expected_read(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent, replacement = _tree(tmp_path)
    original = transaction_files._read_leaf
    swapped = False

    def swap_before_read(parent_fd, leaf, name, *, missing_ok):
        nonlocal swapped
        if not swapped:
            swapped = True
            _swap(tmp_path, parent, replacement)
        return original(parent_fd, leaf, name, missing_ok=missing_ok)

    monkeypatch.setattr(transaction_files, "_read_leaf", swap_before_read)
    with pytest.raises(ValueError, match="parent detached"):
        transaction_files.atomic_write(
            tmp_path, "include/test.h", b"after\n", expected=b"before\n"
        )
    _assert_swap_rejected(tmp_path)


def test_atomic_write_rejects_parent_swap_during_temp_write(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent, replacement = _tree(tmp_path)
    original = transaction_files.os.fsync
    swapped = False

    def swap_during_temp_fsync(descriptor):
        nonlocal swapped
        if not swapped and not os.path.isdir(f"/proc/self/fd/{descriptor}"):
            swapped = True
            _swap(tmp_path, parent, replacement)
        return original(descriptor)

    monkeypatch.setattr(transaction_files.os, "fsync", swap_during_temp_fsync)
    with pytest.raises(ValueError, match="parent detached"):
        transaction_files.atomic_write(
            tmp_path, "include/test.h", b"after\n", expected=b"before\n"
        )
    _assert_swap_rejected(tmp_path)


def test_atomic_write_existing_leaf_commit_substitution_preserves_every_inode(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent = tmp_path / "include"
    parent.mkdir()
    leaf = parent / "test.h"
    leaf.write_bytes(b"before\n")
    original_inode = leaf.stat().st_ino
    original = transaction_files._rename_noreplace
    calls = 0

    def substitute_at_commit(source, destination, *, src_dir_fd, dst_dir_fd):
        nonlocal calls
        calls += 1
        if calls == 2:
            leaf.write_bytes(b"unexpected\n")
        return original(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )

    monkeypatch.setattr(transaction_files, "_rename_noreplace", substitute_at_commit)
    with pytest.raises(RuntimeError, match="temporary retained.*quarantine retained"):
        transaction_files.atomic_write(
            tmp_path, "include/test.h", b"after\n", expected=b"before\n"
        )

    assert leaf.read_bytes() == b"unexpected\n"
    temporaries = list(parent.glob(".*.transaction-*"))
    assert len(temporaries) == 1
    assert temporaries[0].read_bytes() == b"after\n"
    quarantines = list((tmp_path / transaction_files.QUARANTINE_DIRECTORY).iterdir())
    assert len(quarantines) == 1
    assert quarantines[0].read_bytes() == b"before\n"
    assert quarantines[0].stat().st_ino == original_inode


def test_atomic_write_new_leaf_commit_substitution_preserves_both_files(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent = tmp_path / "include"
    parent.mkdir()
    leaf = parent / "test.h"
    original = transaction_files._rename_noreplace

    def substitute_at_commit(source, destination, *, src_dir_fd, dst_dir_fd):
        leaf.write_bytes(b"unexpected\n")
        return original(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )

    monkeypatch.setattr(transaction_files, "_rename_noreplace", substitute_at_commit)
    with pytest.raises(RuntimeError, match="temporary retained"):
        transaction_files.atomic_write(tmp_path, "include/test.h", b"after\n")

    assert leaf.read_bytes() == b"unexpected\n"
    temporaries = list(parent.glob(".*.transaction-*"))
    assert len(temporaries) == 1
    assert temporaries[0].read_bytes() == b"after\n"


@pytest.mark.parametrize("unsupported_errno", (None, errno.ENOSYS, errno.EINVAL))
def test_rename_noreplace_unsupported_preserves_both_paths(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, unsupported_errno: int | None
) -> None:
    source = tmp_path / "source"
    destination = tmp_path / "destination"
    source.write_bytes(b"source")
    destination.write_bytes(b"destination")
    directory = os.open(tmp_path, os.O_RDONLY | os.O_DIRECTORY)

    class Libc:
        if unsupported_errno is not None:

            def renameat2(self, *_args):
                assert unsupported_errno is not None
                rename_noreplace.ctypes.set_errno(unsupported_errno)
                return -1

    monkeypatch.setattr(rename_noreplace.ctypes, "CDLL", lambda *_a, **_k: Libc())
    try:
        with pytest.raises(
            rename_noreplace.UnsupportedRenameNoReplaceError,
            match=r"renameat2\(RENAME_NOREPLACE\).*(unavailable|unsupported)",
        ):
            transaction_files._rename_noreplace(
                "source", "destination", src_dir_fd=directory, dst_dir_fd=directory
            )
    finally:
        os.close(directory)

    assert source.read_bytes() == b"source"
    assert destination.read_bytes() == b"destination"


def test_existing_replacement_preflight_rejects_fuseblk_without_state(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    existing = tmp_path / "include/test.h"
    existing.parent.mkdir()
    existing.write_bytes(b"before\n")
    before = sorted(path.relative_to(tmp_path) for path in tmp_path.rglob("*"))

    def unsupported(*_args, **_kwargs):
        raise rename_noreplace.UnsupportedRenameNoReplaceError(
            "renameat2(RENAME_NOREPLACE) is unsupported by fuseblk"
        )

    monkeypatch.setattr(rename_noreplace, "rename_noreplace", unsupported)
    with pytest.raises(
        rename_noreplace.UnsupportedRenameNoReplaceError,
        match="unsupported filesystem.*existing-file transactions",
    ):
        transaction_files.preflight_existing_replacements(tmp_path, {"include/test.h"})

    assert existing.read_bytes() == b"before\n"
    assert sorted(path.relative_to(tmp_path) for path in tmp_path.rglob("*")) == before


def test_new_file_preflight_does_not_require_native_rename_noreplace(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    called = False

    def unsupported(*_args, **_kwargs):
        nonlocal called
        called = True
        raise rename_noreplace.UnsupportedRenameNoReplaceError()

    monkeypatch.setattr(rename_noreplace, "rename_noreplace", unsupported)
    transaction_files.preflight_existing_replacements(tmp_path, {"new.h"})
    assert called is False and not any(tmp_path.iterdir())


def test_rename_noreplace_linux_native_path(tmp_path: Path) -> None:
    source = tmp_path / "source"
    destination = tmp_path / "destination"
    source.write_bytes(b"source")
    directory = os.open(tmp_path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        publication = rename_noreplace.publish_noreplace(
            "source", "destination", src_dir_fd=directory, dst_dir_fd=directory
        )
    finally:
        os.close(directory)

    assert publication == rename_noreplace.Publication("renameat2", None)
    assert not source.exists()
    assert destination.read_bytes() == b"source"


def test_publish_noreplace_fallback_retains_source_and_refuses_collision(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    source = tmp_path / "source"
    destination = tmp_path / "destination"
    source.write_bytes(b"source")
    monkeypatch.setattr(
        rename_noreplace,
        "rename_noreplace",
        lambda *_a, **_k: (_ for _ in ()).throw(
            rename_noreplace.UnsupportedRenameNoReplaceError()
        ),
    )
    directory = os.open(tmp_path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        publication = rename_noreplace.publish_noreplace(
            "source", "destination", src_dir_fd=directory, dst_dir_fd=directory
        )
        with pytest.raises(FileExistsError):
            rename_noreplace.publish_noreplace(
                "source", "destination", src_dir_fd=directory, dst_dir_fd=directory
            )
    finally:
        os.close(directory)

    assert publication == rename_noreplace.Publication("hard-link", "source")
    assert source.read_bytes() == destination.read_bytes() == b"source"


def test_safe_unlink_moves_matching_leaf_to_durable_quarantine(tmp_path: Path) -> None:
    parent = tmp_path / "include"
    parent.mkdir()
    source = parent / "test.h"
    source.write_bytes(b"before\n")
    source_inode = source.stat().st_ino

    quarantine = transaction_files.safe_unlink(
        tmp_path, "include/test.h", expected=b"before\n"
    )

    assert quarantine is not None
    assert not source.exists()
    moved = tmp_path / quarantine
    assert moved.read_bytes() == b"before\n"
    assert moved.stat().st_ino == source_inode
    assert moved.is_relative_to(tmp_path / "out/reviews/evidence/quarantine")


def test_safe_unlink_rejects_parent_swap_during_expected_read(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent, replacement = _tree(tmp_path)
    original = transaction_files.os.fdopen
    swapped = False

    def swap_before_read(descriptor, *args, **kwargs):
        nonlocal swapped
        if not swapped:
            swapped = True
            _swap(tmp_path, parent, replacement)
        return original(descriptor, *args, **kwargs)

    monkeypatch.setattr(transaction_files.os, "fdopen", swap_before_read)
    with pytest.raises(ValueError, match="parent detached"):
        transaction_files.safe_unlink(tmp_path, "include/test.h", expected=b"before\n")
    _assert_swap_rejected(tmp_path)
    assert (tmp_path / "detached/test.h").read_bytes() == b"before\n"


def test_safe_unlink_rejects_parent_swap_at_quarantine_rename(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent, replacement = _tree(tmp_path)
    original = transaction_files._rename_noreplace
    moved = False

    def swap_before_move(source, destination, *, src_dir_fd, dst_dir_fd):
        nonlocal moved
        if not moved:
            moved = True
            _swap(tmp_path, parent, replacement)
        return original(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )

    monkeypatch.setattr(transaction_files, "_rename_noreplace", swap_before_move)
    with pytest.raises(ValueError, match="parent detached"):
        transaction_files.safe_unlink(tmp_path, "include/test.h", expected=b"before\n")
    _assert_swap_rejected(tmp_path)
    quarantines = list((tmp_path / transaction_files.QUARANTINE_DIRECTORY).iterdir())
    assert len(quarantines) == 1
    assert quarantines[0].read_bytes() == b"before\n"


def test_safe_unlink_quarantine_collision_preserves_both_inodes(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent = tmp_path / "include"
    parent.mkdir()
    leaf = parent / "test.h"
    leaf.write_bytes(b"before\n")
    source_inode = leaf.stat().st_ino
    monkeypatch.setattr(transaction_files, "_quarantine_name", lambda _name: "fixed")
    quarantine = tmp_path / transaction_files.QUARANTINE_DIRECTORY / "fixed"
    quarantine.parent.mkdir(parents=True)
    quarantine.write_bytes(b"occupied\n")
    occupied_inode = quarantine.stat().st_ino

    with pytest.raises(FileExistsError):
        transaction_files.safe_unlink(tmp_path, "include/test.h", expected=b"before\n")

    assert leaf.stat().st_ino == source_inode
    assert leaf.read_bytes() == b"before\n"
    assert quarantine.stat().st_ino == occupied_inode
    assert quarantine.read_bytes() == b"occupied\n"


def test_safe_unlink_leaf_substitution_deletes_no_unexpected_inode(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent = tmp_path / "include"
    parent.mkdir()
    original_leaf = parent / "test.h"
    original_leaf.write_bytes(b"before\n")
    original_inode = original_leaf.stat().st_ino
    replacement = parent / "replacement"
    replacement.write_bytes(b"unexpected\n")
    replacement_inode = replacement.stat().st_ino
    original = transaction_files._rename_noreplace
    substituted = False

    def substitute_before_move(source, destination, *, src_dir_fd, dst_dir_fd):
        nonlocal substituted
        if not substituted:
            substituted = True
            original_leaf.rename(parent / "detached-original")
            replacement.rename(original_leaf)
        return original(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )

    monkeypatch.setattr(transaction_files, "_rename_noreplace", substitute_before_move)
    with pytest.raises(ValueError, match="drifted during rollback"):
        transaction_files.safe_unlink(tmp_path, "include/test.h", expected=b"before\n")

    assert original_leaf.read_bytes() == b"unexpected\n"
    assert original_leaf.stat().st_ino == replacement_inode
    assert (parent / "detached-original").stat().st_ino == original_inode


def test_safe_unlink_restore_boundary_substitution_retains_both_inodes(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent = tmp_path / "include"
    parent.mkdir()
    leaf = parent / "test.h"
    leaf.write_bytes(b"before\n")
    original_inode = leaf.stat().st_ino
    original = transaction_files._rename_noreplace
    calls = 0

    def substitute_before_restore(source, destination, *, src_dir_fd, dst_dir_fd):
        nonlocal calls
        calls += 1
        result = original(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )
        if calls == 1:
            leaf.write_bytes(b"unexpected\n")
            raise ValueError("force rollback")
        return result

    monkeypatch.setattr(
        transaction_files, "_rename_noreplace", substitute_before_restore
    )
    with pytest.raises(ValueError, match="quarantine retained as"):
        transaction_files.safe_unlink(tmp_path, "include/test.h", expected=b"before\n")

    assert leaf.read_bytes() == b"unexpected\n"
    quarantines = list((tmp_path / transaction_files.QUARANTINE_DIRECTORY).iterdir())
    assert len(quarantines) == 1
    assert quarantines[0].read_bytes() == b"before\n"
    assert quarantines[0].stat().st_ino == original_inode


def test_git_index_restore_reinstalls_exact_backup_and_quarantines_expected(
    tmp_path: Path,
) -> None:
    tracked = _git_repo(tmp_path)
    backup = transaction_git.git_index_backup(tmp_path)
    assert backup is not None and backup.content is not None
    tracked.write_text("transaction\n")
    subprocess.run(["git", "add", tracked.name], cwd=tmp_path, check=True)
    expected = transaction_git.git_index_backup(tmp_path)
    assert expected is not None and expected != backup

    transaction_git.restore_git_index(tmp_path, backup, expected)

    restored = transaction_git.git_index_backup(tmp_path)
    assert restored is not None
    assert restored.content == backup.content
    assert (
        subprocess.run(["git", "diff", "--cached", "--quiet"], cwd=tmp_path).returncode
        == 0
    )
    recovery = list((tmp_path / ".git").glob("index.transaction-current-*"))
    assert len(recovery) == 1
    assert recovery[0].read_bytes() == expected.content


def test_git_index_cleanup_quarantines_owned_lock_and_unused_backup(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _git_repo(tmp_path)
    snapshot = transaction_git.git_index_backup(tmp_path)
    assert snapshot is not None and snapshot.content is not None
    monkeypatch.setattr(transaction_git.secrets, "token_hex", lambda _size: "audit")

    transaction_git.restore_git_index(tmp_path, snapshot, snapshot)

    git_dir = tmp_path / ".git"
    lock_quarantine = git_dir / "index.transaction-quarantine-lock-audit"
    backup_quarantine = git_dir / "index.transaction-quarantine-backup-audit"
    assert lock_quarantine.read_bytes() == b""
    assert backup_quarantine.read_bytes() == snapshot.content
    assert not (git_dir / "index.lock").exists()
    assert not (git_dir / "index.transaction-backup-audit").exists()


def test_git_index_cleanup_substitution_at_rename_preserves_replacement(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _git_repo(tmp_path)
    snapshot = transaction_git.git_index_backup(tmp_path)
    assert snapshot is not None
    lock = tmp_path / ".git/index.lock"
    detached = tmp_path / ".git/owned-lock-recovery"
    original = transaction_git._rename_noreplace
    replacement_inode = 0

    def substitute(source, destination, *, src_dir_fd, dst_dir_fd):
        nonlocal replacement_inode
        if source == "index.lock" and replacement_inode == 0:
            lock.rename(detached)
            lock.write_bytes(b"concurrent lock\n")
            replacement_inode = lock.stat().st_ino
        return original(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )

    monkeypatch.setattr(transaction_git, "_rename_noreplace", substitute)
    with pytest.raises(RuntimeError, match="substituted at rename boundary"):
        transaction_git.restore_git_index(tmp_path, snapshot, snapshot)

    assert lock.read_bytes() == b"concurrent lock\n"
    assert lock.stat().st_ino == replacement_inode
    assert detached.read_bytes() == b""


def test_git_index_cleanup_collision_preserves_source_and_destination(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _git_repo(tmp_path)
    snapshot = transaction_git.git_index_backup(tmp_path)
    assert snapshot is not None
    monkeypatch.setattr(transaction_git.secrets, "token_hex", lambda _size: "collision")
    collision = tmp_path / ".git/index.transaction-quarantine-lock-collision"
    collision.write_bytes(b"existing audit\n")
    collision_inode = collision.stat().st_ino

    with pytest.raises(RuntimeError, match="quarantine collision"):
        transaction_git.restore_git_index(tmp_path, snapshot, snapshot)

    lock = tmp_path / ".git/index.lock"
    assert lock.read_bytes() == b""
    assert collision.read_bytes() == b"existing audit\n"
    assert collision.stat().st_ino == collision_inode


def test_git_index_restore_rejects_concurrent_substitution_without_clobber(
    tmp_path: Path,
) -> None:
    tracked = _git_repo(tmp_path)
    backup = transaction_git.git_index_backup(tmp_path)
    assert backup is not None and backup.content is not None
    tracked.write_text("transaction\n")
    subprocess.run(["git", "add", tracked.name], cwd=tmp_path, check=True)
    expected = transaction_git.git_index_backup(tmp_path)
    assert expected is not None
    index = tmp_path / ".git/index"
    replacement = tmp_path / ".git/concurrent-index"
    replacement.write_bytes(b"concurrent index\n")
    replacement_inode = replacement.stat().st_ino
    replacement.rename(index)

    with pytest.raises(RuntimeError, match="changed concurrently.*backup retained"):
        transaction_git.restore_git_index(tmp_path, backup, expected)

    assert index.read_bytes() == b"concurrent index\n"
    assert index.stat().st_ino == replacement_inode
    artifacts = list((tmp_path / ".git").glob("index.transaction-backup-*"))
    assert len(artifacts) == 1
    assert artifacts[0].read_bytes() == backup.content


def test_git_index_restore_commit_substitution_restores_unexpected_inode(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    tracked = _git_repo(tmp_path)
    backup = transaction_git.git_index_backup(tmp_path)
    assert backup is not None and backup.content is not None
    tracked.write_text("transaction\n")
    subprocess.run(["git", "add", tracked.name], cwd=tmp_path, check=True)
    expected = transaction_git.git_index_backup(tmp_path)
    assert expected is not None
    index = tmp_path / ".git/index"
    original = transaction_git._rename_noreplace
    substituted = False
    concurrent_inode = 0

    def substitute(source, destination, *, src_dir_fd, dst_dir_fd):
        nonlocal substituted, concurrent_inode
        if not substituted and source == index.name:
            substituted = True
            index.rename(tmp_path / ".git/detached-expected-index")
            index.write_bytes(b"concurrent index\n")
            concurrent_inode = index.stat().st_ino
        return original(
            source, destination, src_dir_fd=src_dir_fd, dst_dir_fd=dst_dir_fd
        )

    monkeypatch.setattr(transaction_git, "_rename_noreplace", substitute)
    with pytest.raises(RuntimeError, match="changed concurrently during rollback"):
        transaction_git.restore_git_index(tmp_path, backup, expected)

    assert index.read_bytes() == b"concurrent index\n"
    assert index.stat().st_ino == concurrent_inode
    backups = list((tmp_path / ".git").glob("index.transaction-backup-*"))
    assert len(backups) == 1
    assert backups[0].read_bytes() == backup.content


def test_git_index_restore_removes_only_expected_new_index(tmp_path: Path) -> None:
    subprocess.run(["git", "init", "-q"], cwd=tmp_path, check=True)
    backup = transaction_git.git_index_backup(tmp_path)
    assert backup is not None and backup.content is None
    tracked = tmp_path / "tracked"
    tracked.write_text("transaction\n")
    subprocess.run(["git", "add", tracked.name], cwd=tmp_path, check=True)
    expected = transaction_git.git_index_backup(tmp_path)
    assert expected is not None and expected.content is not None

    transaction_git.restore_git_index(tmp_path, backup, expected)

    assert not (tmp_path / ".git/index").exists()
    recovery = list((tmp_path / ".git").glob("index.transaction-current-*"))
    assert len(recovery) == 1
    assert recovery[0].read_bytes() == expected.content


def test_git_index_restore_preserves_concurrent_new_index_when_expected_missing(
    tmp_path: Path,
) -> None:
    subprocess.run(["git", "init", "-q"], cwd=tmp_path, check=True)
    backup = transaction_git.git_index_backup(tmp_path)
    assert backup is not None and backup.content is None
    concurrent = tmp_path / ".git/index"
    concurrent.write_bytes(b"concurrent index\n")
    inode = concurrent.stat().st_ino

    with pytest.raises(RuntimeError, match="changed concurrently.*backup retained"):
        transaction_git.restore_git_index(tmp_path, backup, backup)

    assert concurrent.read_bytes() == b"concurrent index\n"
    assert concurrent.stat().st_ino == inode
    assert list((tmp_path / ".git").glob("index.transaction-backup-*"))


def test_rollback_workspace_records_retained_quarantine(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    parent = tmp_path / "include"
    parent.mkdir()
    leaf = parent / "generated.h"
    leaf.write_bytes(b"generated\n")
    monkeypatch.setattr(
        transaction_git,
        "workspace_backup",
        lambda _root: {"include/generated.h": b"generated\n"},
    )
    quarantines = transaction_git.rollback_workspace(tmp_path, {})

    assert not leaf.exists()
    assert len(quarantines) == 1
    assert (tmp_path / quarantines[0]).read_bytes() == b"generated\n"
