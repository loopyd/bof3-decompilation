"""CMake build frontend shared by bulk and focused lift workflows."""

from __future__ import annotations

import fcntl
import hashlib
import re
import shutil
import subprocess
import time
from pathlib import Path

from harness.build.inventory import has_current_inventory
from harness.domain.cache import collect_manifest_paths


def cmake_target_for_source(root: Path, source: Path) -> str:
    relative = source.resolve().relative_to(root.resolve()).as_posix()
    digest = hashlib.sha1(relative.encode()).hexdigest()[:16]
    return f"lift_{digest}"


def cmake_target_for_directory(source_directory: str) -> str:
    digest = hashlib.sha1(source_directory.encode()).hexdigest()[:16]
    return f"target_{digest}"


def _has_missing_source(root: Path, generated: Path) -> bool:
    text = generated.read_text(encoding="utf-8", errors="ignore")
    sources = {
        source
        for token in text.split()
        if "/src/" in token
        for source in re.findall(
            r"(?:[A-Za-z]:)?[^\s:|]+/src/[^\s:|]+\.(?:c|s|S)", token
        )
    }
    return any(not Path(source).is_file() for source in sources)


def configure(root: Path) -> Path:
    """Configure the shared CMake tree under a cross-process lock.

    Concurrent lift lanes reconfigure the same ``build/cmake`` tree; without a
    lock they race on ``shutil.rmtree`` and the generated ninja files, which is
    the dominant wall-clock stall under fan-out. The lock serializes configure
    while leaving already-current trees fast (the cached fast path still runs
    inside the lock and returns immediately).

    A second bounded retry absorbs the remaining shared-tree race observed in
    30/30 fan-out runs (a sibling lane rewriting ``CMakeFiles`` between the
    inventory check and the build).
    """
    lock_path = root / "build" / ".configure.lock"
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    attempts = 3
    with open(lock_path, "w", encoding="utf-8") as lock_stream:
        fcntl.flock(lock_stream.fileno(), fcntl.LOCK_EX)
        try:
            for attempt in range(attempts):
                try:
                    return _configure_locked(root)
                except (OSError, RuntimeError) as error:
                    if attempt + 1 >= attempts or not _is_shared_tree_race(str(error)):
                        raise
                    time.sleep(2)
            raise AssertionError("unreachable")
        finally:
            fcntl.flock(lock_stream.fileno(), fcntl.LOCK_UN)


def _is_shared_tree_race(message: str) -> bool:
    """Recognize the shared build-tree races fan-out lanes hit constantly."""
    markers = (
        "Directory not empty",
        "build.ninja",
        "CMakeFiles",
        "stale configured build inventory",
        "Build inventory changed during configure",
        "GLOB mismatch",
        "reconfigure the build tree",
    )
    return any(marker in message for marker in markers)


def _configure_locked(root: Path) -> Path:
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
                inputs = [root / "CMakeLists.txt"] + [
                    root / name for name in collect_manifest_paths(root)
                ]
                stale = generated is not None and (
                    _has_missing_source(root, generated)
                    or not has_current_inventory(
                        root, build_tree / "bof3-inventory.json"
                    )
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
    """Build multiple CMake targets in one CMake build invocation.

    Ninja is asked to keep going (``-k 0``) so a single failing translation
    unit cannot hide the state of every later target; callers that need
    per-target attribution read the ``FAILED`` objects the build reports.
    """
    if not targets:
        raise ValueError("batch_build requires at least one target")
    build_tree = configure(root)
    command = ["cmake", "--build", str(build_tree), "--target", *targets]
    if (build_tree / "build.ninja").is_file():
        command.extend(["--", "-k", "0"])
    return subprocess.run(
        command,
        cwd=root,
        text=True,
        capture_output=True,
    )
