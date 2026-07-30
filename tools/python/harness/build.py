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
    cache = build_tree / "CMakeCache.txt"
    if cache.is_file():
        complete = (build_tree / "build.ninja").is_file() or (
            build_tree / "Makefile"
        ).is_file()
        for line in cache.read_text().splitlines():
            if line.startswith("CMAKE_HOME_DIRECTORY:"):
                cached_home = line.split("=", 1)[1].strip()
                if complete and cached_home == str(root.resolve()):
                    return build_tree
                break
        # CMake cannot overwrite either a foreign cache or an incomplete cache
        # from another generator, so start this disposable tree afresh.
        shutil.rmtree(build_tree)
    command = ["cmake", "-S", str(root), "-B", str(build_tree)]
    if shutil.which("ninja"):
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


def batch_build(root: Path, targets: list[str]) -> subprocess.CompletedProcess[str]:
    """Build multiple CMake targets in one CMake build invocation."""
    if not targets:
        raise ValueError("batch_build requires at least one target")
    build_tree = configure(root)
    return subprocess.run(
        ["cmake", "--build", str(build_tree), "--target", *targets],
        cwd=root,
        text=True,
        capture_output=True,
    )
