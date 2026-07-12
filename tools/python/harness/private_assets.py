from __future__ import annotations

from pathlib import Path


def list_git_submodule_paths(repo_root: Path) -> list[str]:
    gitmodules_path = repo_root / ".gitmodules"
    if not gitmodules_path.exists():
        return []

    paths: list[str] = []
    for line in gitmodules_path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if not stripped.startswith("path = "):
            continue
        paths.append(stripped.removeprefix("path = ").strip())
    return paths


def list_required_submodule_paths(repo_root: Path) -> list[str]:
    return list_git_submodule_paths(repo_root)
