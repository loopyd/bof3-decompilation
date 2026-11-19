"""Disposable Git repositories for cleanup tests."""

from __future__ import annotations

import os
import subprocess
import time
from pathlib import Path

import pytest


def git(root: Path, *args: str) -> str:
    return subprocess.check_output(
        ["git", "-C", str(root), *args], stderr=subprocess.PIPE, text=True
    ).strip()


def write(root: Path, name: str, content: bytes = b"disposable") -> Path:
    path = root / name
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(content)
    return path


def age(path: Path, days: int = 10) -> None:
    cutoff = time.time() - days * 86400
    paths = [path, *path.rglob("*")] if path.is_dir() else [path]
    for entry in paths:
        os.utime(entry, (cutoff, cutoff), follow_symlinks=False)


@pytest.fixture
def repo(tmp_path: Path) -> Path:
    root = tmp_path / "repo"
    root.mkdir()
    git(root, "init", "-b", "main")
    git(root, "config", "user.email", "cleanup@example.invalid")
    git(root, "config", "user.name", "Cleanup test")
    write(
        root,
        ".gitignore",
        b"build/\nout/\nledger/\n.pi/sessions/\n.pi-subagents/\ntools/rust/**/target/\n",
    )
    write(root, "source.c", b"int source;\n")
    git(root, "add", ".gitignore", "source.c")
    git(root, "commit", "-m", "Fixture")
    return root
