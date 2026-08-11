#!/usr/bin/env python3
"""Provision ignored BOF3 prerequisites in the current managed worktree."""

import os
from pathlib import Path
import subprocess

worktree = Path.cwd()
root = Path(subprocess.run(
    ("git", "worktree", "list", "--porcelain"),
    check=True,
    capture_output=True,
    text=True,
).stdout.splitlines()[0].removeprefix("worktree "))

for name in (".venv", "inputs"):
    source = root / name
    target = worktree / name
    if source.exists() and not target.exists():
        os.symlink(source, target, target_is_directory=True)

source_out = root / "out"
target_out = worktree / "out"
if source_out.exists() and not target_out.exists():
    subprocess.run(("cp", "-a", "--reflink=auto", str(source_out), str(target_out)), check=True)

ignored = subprocess.run(
    ("git", "ls-files", "--others", "--ignored", "--exclude-standard", "-z", "--", "toolchains"),
    cwd=root,
    check=True,
    capture_output=True,
).stdout.split(b"\0")
for raw in ignored:
    if not raw:
        continue
    relative = Path(os.fsdecode(raw))
    source = root / relative
    target = worktree / relative
    if source.is_file() and not target.exists():
        target.parent.mkdir(parents=True, exist_ok=True)
        os.symlink(source, target)
