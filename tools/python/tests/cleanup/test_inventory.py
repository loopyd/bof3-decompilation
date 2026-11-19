"""Cleanup selection and physical-storage estimates in disposable trees."""

from __future__ import annotations

import os
from pathlib import Path

import pytest

from harness.cleanup import inventory
from harness.cleanup.inventory import Activity, Options, assess

from conftest import age, git, write


def selected(plan) -> set[str]:
    return {operation.path for operation in plan.operations}


def test_default_selects_stale_caches_and_keeps_newest_binary(repo: Path) -> None:
    cache = "tools/rust/example/target/debug/incremental"
    write(repo, cache + "/session/object.o")
    age(repo / cache)
    fresh = write(repo, "build/tools/rust/example/debug/incremental/fresh/object.o")
    prefix = "tools/rust/example/target/debug/deps/player-"
    old = write(repo, prefix + "1111111111111111", b"\x7fELFold")
    newest = write(repo, prefix + "2222222222222222", b"\x7fELFnew")
    for path in [old, newest]:
        path.chmod(0o755)
    age(old, 12)
    age(newest, 10)
    dump = write(repo, "example.c.rtl", b"\n;; Function example\n(insn 1)\n")
    age(dump)
    write(repo, "ledger/main.jsonl")
    plan = assess(repo)
    assert selected(plan) == {cache, str(old.relative_to(repo)), dump.name, "ledger"}
    assert fresh.exists() and newest.exists()


@pytest.mark.parametrize(
    ("options", "expected"),
    [
        (Options(), set()),
        (Options(evidence=True), {"out/source-refinement/old"}),
        (Options(session_history=True), {".pi/sessions/old"}),
        (
            Options(evidence=True, session_history=True, worktrees=True),
            {"out/source-refinement/old", ".pi/sessions/old"},
        ),
    ],
)
def test_optional_categories_are_independent(repo, options, expected) -> None:
    for name in [
        "out/source-refinement/old/receipt.json",
        ".pi/sessions/old/log.jsonl",
    ]:
        path = write(repo, name)
        age(path.parent)
    write(repo, "out/source-refinement/current/receipt.json")
    write(repo, "out/extracted/original.bin")
    write(repo, "out/unknown/deliverable.json")
    recovery = write(repo, "out/source-refinement/retained/PRE/source.c")
    age(recovery.parent.parent)
    plan = assess(repo, options)
    assert selected(plan) == expected
    assert any("recent" in skip.reason for skip in plan.skipped) == options.evidence


def test_hardlinks_and_sparse_files_count_only_reclaimable_blocks(repo: Path) -> None:
    inside = write(repo, "ledger/first", b"x" * 8192)
    os.link(inside, repo / "ledger/second")
    sparse = write(repo, "ledger/sparse", b"x")
    with sparse.open("r+b") as stream:
        stream.truncate(16 * 1024 * 1024)
    plan = assess(repo)
    allocated = sum(
        p.lstat().st_blocks * 512 for p in [repo / "ledger", inside, sparse]
    )
    assert plan.estimated_bytes == allocated
    os.link(inside, repo / "retained")
    assert assess(repo).estimated_bytes == allocated - inside.stat().st_blocks * 512


def test_tracked_committed_symlink_and_nested_repository_paths_are_protected(repo):
    write(repo, "ledger/source.c")
    git(repo, "add", "-f", "ledger/source.c")
    git(repo, "commit", "-m", "Tracked ledger exception")
    git(repo, "rm", "--cached", "ledger/source.c")
    assert "ledger" not in selected(assess(repo))
    git(repo, "reset", "--hard", "HEAD~1")
    (repo / "ledger/source.c").unlink(missing_ok=True)
    (repo / "ledger").rmdir()
    outside = repo.parent / "outside"
    outside.mkdir()
    write(outside, "important")
    (repo / "ledger").symlink_to(outside, target_is_directory=True)
    assert "ledger" not in selected(assess(repo))
    (repo / "ledger").unlink()
    write(repo, "ledger/.git/config")
    assert "ledger" not in selected(assess(repo))


def test_activity_protects_open_artifacts_and_busy_rust_caches(repo, monkeypatch):
    write(repo, "ledger/log.jsonl")
    cache = "tools/rust/example/target/debug/incremental"
    write(repo, cache + "/object.o")
    age(repo / cache)
    monkeypatch.setattr(
        inventory,
        "read_activity",
        lambda root: Activity(paths=(root / "ledger/log.jsonl",), rust_busy=True),
    )
    assert not assess(repo).operations


def test_invalid_root_is_not_treated_as_an_empty_success(tmp_path):
    with pytest.raises(ValueError, match="Git"):
        assess(tmp_path)


def test_history_scope_preserves_unknown_state_and_saved_diffs(repo):
    for name in [
        ".pi-subagents/chain-runs/old/transcript.jsonl",
        ".pi-subagents/artifacts/outputs/old.txt",
        ".pi-subagents/artifacts/worktree-diffs/saved.patch",
        ".pi-subagents/missions/current.json",
        ".pi-subagents/settings.json",
    ]:
        path = write(repo, name)
        age(path.parent)
    assert selected(assess(repo, Options(session_history=True))) == {
        ".pi-subagents/chain-runs/old",
        ".pi-subagents/artifacts/outputs/old.txt",
    }
