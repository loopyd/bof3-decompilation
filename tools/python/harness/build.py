"""CMake build frontend shared by bulk and focused lift workflows."""

from __future__ import annotations

import hashlib
from pathlib import Path
import shutil
import subprocess


def cmake_target_for_source(root: Path, source: Path) -> str:
    relative = source.resolve().relative_to(root.resolve()).as_posix()
    digest = hashlib.sha1(relative.encode()).hexdigest()[:16]
    return f"lift_{digest}"


def cmake_target_for_directory(source_directory: str) -> str:
    digest = hashlib.sha1(source_directory.encode()).hexdigest()[:16]
    return f"target_{digest}"


def configure(root: Path) -> Path:
    build_tree = root / "build" / "cmake"
    command = ["cmake", "-S", str(root), "-B", str(build_tree)]
    if not (build_tree / "CMakeCache.txt").is_file() and shutil.which("ninja"):
        command.extend(["-G", "Ninja"])
    result = subprocess.run(command, cwd=root, text=True, capture_output=True)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    return build_tree


def build(root: Path, target: str = "lifts") -> subprocess.CompletedProcess[str]:
    build_tree = configure(root)
    return subprocess.run(
        ["cmake", "--build", str(build_tree), "--target", target],
        cwd=root,
        text=True,
        capture_output=True,
    )
