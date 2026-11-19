"""Guarded cleanup execution against temporary repositories only."""

from __future__ import annotations

import fcntl
from pathlib import Path

import pytest

from harness.cleanup import inventory, operations
from harness.cleanup.inventory import Activity, Options, assess
from harness.cleanup.operations import execute
from harness.common.lease import acquire_writer

from conftest import age, git, write


def test_live_cleanup_preserves_source_index_and_untracked_work(repo: Path) -> None:
    write(repo, "ledger/old.jsonl")
    source = write(repo, "untracked.c", b"keep me")
    index = (repo / ".git/index").read_bytes()
    head = git(repo, "rev-parse", "HEAD")
    result = execute(assess(repo))
    assert not result.error and len(result.completed) == 1
    assert not (repo / "ledger").exists()
    assert source.read_bytes() == b"keep me"
    assert (repo / ".git/index").read_bytes() == index
    assert git(repo, "rev-parse", "HEAD") == head


def test_changed_directory_membership_refuses_deletion(repo: Path) -> None:
    write(repo, "ledger/old.jsonl")
    plan = assess(repo)
    write(repo, "ledger/new.c", b"new work")
    result = execute(plan)
    assert "changed" in result.error
    assert not result.completed and (repo / "ledger/new.c").exists()


def test_new_index_claim_refuses_deletion(repo: Path) -> None:
    write(repo, "ledger/source.c")
    plan = assess(repo)
    git(repo, "add", "-f", "ledger/source.c")
    result = execute(plan)
    assert "indexed" in result.error
    assert (repo / "ledger/source.c").exists()


def test_writer_and_cargo_locks_refuse_live_cleanup(repo: Path) -> None:
    write(repo, "ledger/old.jsonl")
    plan = assess(repo)
    with acquire_writer(repo):
        result = execute(plan)
    assert result.error and not result.completed
    path = write(repo, "tools/rust/example/target/debug/incremental/object.o")
    age(path.parent)
    lock = write(repo, "tools/rust/example/target/debug/.cargo-lock", b"")
    plan = assess(repo)
    with lock.open("r+b") as stream:
        fcntl.flock(stream, fcntl.LOCK_EX | fcntl.LOCK_NB)
        result = execute(plan)
    assert "Cargo" in result.error and not result.completed
    assert (repo / "ledger/old.jsonl").exists()


def test_activity_is_rechecked_before_deletion(repo, monkeypatch):
    path = write(repo, "ledger/old.jsonl")
    plan = assess(repo)
    monkeypatch.setattr(inventory, "read_activity", lambda root: Activity((path,)))
    result = execute(plan)
    assert "active" in result.error and path.exists()


def test_new_ignore_policy_refuses_deletion(repo):
    path = write(repo, "ledger/log")
    plan = assess(repo)
    ignore = repo / ".gitignore"
    ignore.write_text(ignore.read_text().replace("ledger/\n", ""))
    result = execute(plan)
    assert "no longer ignored" in result.error and path.exists()


def test_hardlinks_shared_by_selected_operations_can_both_be_removed(repo):
    import os

    first = write(repo, "out/source-refinement/first/result.bin", b"x" * 8192)
    second = repo / "out/source-refinement/second/result.bin"
    second.parent.mkdir()
    os.link(first, second)
    age(first.parent)
    age(second.parent)
    plan = assess(repo, Options(evidence=True))
    result = execute(plan)
    assert not result.error and len(result.completed) == 2
    assert result.estimated_bytes == plan.estimated_bytes


def test_symlinked_ancestor_cannot_redirect_deletion(repo: Path) -> None:
    parent = repo / "tools/rust/example/target"
    path = write(repo, "tools/rust/example/target/debug/incremental/object.o")
    age(path.parent)
    plan = assess(repo)
    outside = repo.parent / "outside-target"
    parent.rename(outside)
    parent.symlink_to(outside, target_is_directory=True)
    result = execute(plan)
    assert result.error
    assert (outside / "debug/incremental/object.o").exists()


@pytest.mark.parametrize("state", ["clean", "dirty", "locked", "unmerged", "active"])
def test_worktree_cleanup_obeys_git_and_process_guards(repo, monkeypatch, state):
    worktree = repo / "out/worktrees/linked"
    git(repo, "worktree", "add", "--detach", str(worktree), "HEAD")
    if state == "dirty":
        write(worktree, "source.c", b"changed")
    if state == "locked":
        git(repo, "worktree", "lock", str(worktree))
    if state == "unmerged":
        write(worktree, "source.c", b"new commit")
        git(worktree, "add", "source.c")
        git(worktree, "commit", "-m", "Unmerged")
    if state == "active":
        monkeypatch.setattr(
            inventory, "read_activity", lambda root: Activity((worktree,))
        )
    age(worktree)
    plan = assess(repo, Options(worktrees=True))
    chosen = [op for op in plan.operations if op.category == "worktree"]
    assert bool(chosen) == (state == "clean")
    result = execute(plan)
    assert not result.error
    assert worktree.exists() == (state != "clean")
    listed = git(repo, "worktree", "list", "--porcelain")
    assert (str(worktree) in listed) == (state != "clean")
    assert repo.exists()


def test_partial_failure_reports_only_completed_removals(repo, monkeypatch):
    write(repo, "ledger/log")
    dump = write(repo, "old.c.rtl", b";; Function old\n(insn 1)\n")
    age(dump)
    plan = assess(repo)
    real_remove = operations.remove_artifact

    def remove(root, operation):
        if operation == plan.operations[1]:
            raise OSError("simulated removal failure")
        real_remove(root, operation)

    monkeypatch.setattr(operations, "remove_artifact", remove)
    result = execute(plan)
    assert len(result.completed) == 1 and "simulated" in result.error
    assert not (repo / plan.operations[0].path).exists()
    assert (repo / plan.operations[1].path).exists()


def test_worktree_locked_after_assessment_refuses_removal(repo):
    worktree = repo / "out/worktrees/linked"
    git(repo, "worktree", "add", "--detach", str(worktree), "HEAD")
    age(worktree)
    plan = assess(repo, Options(worktrees=True))
    assert any(operation.category == "worktree" for operation in plan.operations)
    git(repo, "worktree", "lock", str(worktree))
    result = execute(plan)
    assert "locked" in result.error and not result.completed and worktree.exists()
