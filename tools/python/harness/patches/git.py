"""Bounded Git subprocess transport without index or repository mutation."""

from __future__ import annotations

import os
from pathlib import Path
import subprocess


def run_git(directory: Path, *arguments: str, data: bytes | None = None) -> bytes:
    environment = {
        key: value for key, value in os.environ.items() if not key.startswith("GIT_")
    }
    result = subprocess.run(
        ["git", "-C", str(directory), *arguments],
        input=data,
        capture_output=True,
        check=False,
        timeout=60,
        env=environment,
    )
    if result.returncode:
        raise ValueError(
            f"patch Git operation failed: {result.stderr.decode(errors='replace').strip()}"
        )
    return result.stdout


def git_text(directory: Path, *arguments: str) -> str:
    return run_git(directory, *arguments).decode().strip()
