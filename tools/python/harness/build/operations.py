"""CMake build frontend shared by bulk and focused lift workflows."""

from __future__ import annotations

import hashlib
from pathlib import Path
import re
import shutil
import subprocess


def cmake_target_for_source(root: Path, source: Path) -> str:
    relative = source.resolve().relative_to(root.resolve()).as_posix()
    digest = hashlib.sha1(relative.encode()).hexdigest()[:16]
    return f"lift_{digest}"


def cmake_target_for_directory(source_directory: str) -> str:
    digest = hashlib.sha1(source_directory.encode()).hexdigest()[:16]
    return f"target_{digest}"


def _has_missing_source(root: Path, generated: Path) -> bool:
    text = generated.read_text(encoding="utf-8", errors="ignore")
    sources = set(re.findall(r"(?:[A-Za-z]:)?[^\s:|]+/src/[^\s:|]+\.(?:c|s|S)", text))
    return any(not Path(source).is_file() for source in sources)


def configure(root: Path) -> Path:
    build_tree = root / "build" / "cmake"
    cache = build_tree / "CMakeCache.txt"
    if cache.is_file():
        generated = next(
            (
                path
                for path in (build_tree / "build.ninja", build_tree / "Makefile")
                if path.is_file()
            ),
            None,
        )
        for line in cache.read_text().splitlines():
            if line.startswith("CMAKE_HOME_DIRECTORY:"):
                cached_home = line.split("=", 1)[1].strip()
                inputs = [root / "CMakeLists.txt"] + list(
                    (root / "config" / "targets").rglob("target.toml")
                )
                stale = generated is not None and (
                    _has_missing_source(root, generated)
                    or any(
                        path.is_file()
                        and path.stat().st_mtime_ns > generated.stat().st_mtime_ns
                        for path in inputs
                    )
                )
                if (
                    generated is not None
                    and not stale
                    and cached_home == str(root.resolve())
                ):
                    return build_tree
                if stale and cached_home == str(root.resolve()):
                    break
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
