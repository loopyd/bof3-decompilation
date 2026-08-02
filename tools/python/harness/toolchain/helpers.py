"""Small shared helpers for toolchain-local paths and downloads."""

from __future__ import annotations

import shutil
import urllib.request
from collections.abc import Callable, Iterable
from pathlib import Path


def unique_paths(paths: Iterable[Path]) -> list[Path]:
    """Expand and preserve the first occurrence of each path."""
    result: list[Path] = []
    seen: set[Path] = set()
    for path in paths:
        expanded = path.expanduser()
        if expanded not in seen:
            seen.add(expanded)
            result.append(expanded)
    return result


def paths_under(paths: Iterable[Path], root: Path) -> list[Path]:
    """Return unique paths whose resolved location remains inside *root*."""
    root_resolved = root.resolve()
    return [
        path
        for path in unique_paths(paths)
        if (resolved := path.resolve(strict=False)) == root_resolved
        or root_resolved in resolved.parents
    ]


def require_path_under(path: Path, root: Path, *, label: str) -> Path:
    """Return a user path only when it remains inside the allowed root."""
    expanded = path.expanduser()
    resolved = expanded.resolve(strict=False)
    root_resolved = root.resolve()
    if resolved != root_resolved and root_resolved not in resolved.parents:
    return expanded


def find_matching_files(path: Path, matches: Callable[[Path], bool]) -> list[Path]:
    """Return a file itself or matching files below a directory, in order."""
    if matches(path):
        return [path]
    if not path.is_dir():
        return []
    return [candidate for candidate in sorted(path.rglob("*")) if matches(candidate)]


def download_file(url: str, dest: Path, *, force: bool = False) -> Path:
    """Download a URL unless *dest* already exists or *force* is set."""
