"""Exercise the public cleanup command without deleting repository evidence."""

from __future__ import annotations

import hashlib
import os
import subprocess
from pathlib import Path

from conftest import age, git, write

ROOT = Path(__file__).resolve().parents[4]


def invoke(repo: Path, *args: str, **environment: str):
    return subprocess.run(
        [str(ROOT / "bin/harness"), "cleanup", "--root", str(repo), *args],
        cwd=repo,
        env={**os.environ, **environment},
        capture_output=True,
        text=True,
    )


def fingerprint(root: Path):
    return {
        str(path.relative_to(root)): (
            path.lstat().st_ino,
            path.lstat().st_mtime_ns,
            hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else "",
        )
        for path in root.rglob("*")
    }


def test_public_all_dry_run_preserves_files_index_worktrees_and_import_cache(repo):
    for name in [
        "ledger/log",
        "out/source-refinement/old/receipt.json",
        ".pi/sessions/old/log.jsonl",
    ]:
        path = write(repo, name)
        age(path.parent)
    worktree = repo / "out/worktrees/linked"
    git(repo, "worktree", "add", "--detach", str(worktree), "HEAD")
    age(worktree)
    before = fingerprint(repo)
    caches = repo.parent / "import-caches"
    result = invoke(repo, "--all", "--dry-run", PYTHONPYCACHEPREFIX=str(caches))
    assert result.returncode == 0, result.stderr
    for category in ("ledger", "evidence", "session-history", "worktree"):
        assert f"[{category}]" in result.stdout
    assert "Estimated savings:" in result.stdout and "bytes" in result.stdout
    assert "DRY RUN" in result.stdout
    assert fingerprint(repo) == before
    assert not caches.exists()
    explicit = invoke(
        repo,
        "--with-worktrees",
        "--with-session-history",
        "--with-evidence",
        "--dry-run",
    )
    assert explicit.returncode == 0 and explicit.stdout == result.stdout


def test_public_live_cleanup_reports_savings_and_preserves_default_exclusions(repo):
    write(repo, "ledger/log", b"x" * 8192)
    evidence = write(repo, "out/source-refinement/old/receipt.json")
    age(evidence.parent)
    result = invoke(repo)
    assert result.returncode == 0, result.stderr
    assert "Removed 1 operation" in result.stdout
    assert "Estimated savings:" in result.stdout
    assert "Observed free-space change:" in result.stdout
    assert not (repo / "ledger").exists() and evidence.exists()


def test_public_help_example_and_invalid_root(repo):
    help_result = invoke(repo, "--help")
    assert help_result.returncode == 0
    for flag in (
        "--dry-run",
        "--with-worktrees",
        "--with-session-history",
        "--with-evidence",
        "--all",
    ):
        assert flag in help_result.stdout
    example = invoke(repo, "--example")
    assert example.returncode == 0
    assert example.stdout.strip() == "bin/harness cleanup --dry-run"
    invalid = invoke(repo.parent, "--dry-run")
    assert invalid.returncode != 0 and "Git" in invalid.stderr
