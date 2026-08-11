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

# This repository tracks shell/Python entrypoints without Git's executable bit;
# the primary checkout carries the reviewed local modes. Reproduce them in the
# temporary checkout before CMake invokes the tools directly.
for source in (root / "bin").iterdir():
    target = worktree / "bin" / source.name
    if source.is_file() and target.is_file() and os.access(source, os.X_OK):
        target.chmod(source.stat().st_mode)

for name in (".venv", "inputs"):
    source = root / name
    target = worktree / name
    if source.exists() and not target.exists():
        if name == ".venv":
            target.mkdir()
            subprocess.run(("cp", "-as", str(source) + "/.", str(target)), check=True)
        else:
            os.symlink(source, target, target_is_directory=True)

# Managed worktrees do not initialize submodules. Populate their existing
# gitlink directories with reflinked files; replacing gitlinks with symlinks
# creates tracked type changes that can leak into native handoff patches.
for name in ("maspsx", "asm-differ", "decomp-permuter", "m2c"):
    source = root / "third_party" / name
    target = worktree / "third_party" / name
    if source.is_dir() and any(source.iterdir()) and (not target.exists() or not any(target.iterdir())):
        target.mkdir(parents=True, exist_ok=True)
        subprocess.run(
            ("cp", "-a", "--reflink=auto", str(source) + "/.", str(target)),
            check=True,
        )
        git_admin = target / ".git"
        if git_admin.is_file() or git_admin.is_symlink():
            git_admin.unlink()

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
        # compiler-variants deliberately rejects symlinks; reflink/copy preserves
        # executable modes while keeping each managed lane self-contained.
        subprocess.run(("cp", "-a", "--reflink=auto", str(source), str(target)), check=True)
