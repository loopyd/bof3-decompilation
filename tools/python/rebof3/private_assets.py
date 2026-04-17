from __future__ import annotations

from pathlib import Path


PRIVATE_ASSETS_SUBMODULE_PATH = "external/private-assets"
OPTIONAL_SUBMODULE_PATHS = (PRIVATE_ASSETS_SUBMODULE_PATH,)


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
    return [
        path
        for path in list_git_submodule_paths(repo_root)
        if path not in OPTIONAL_SUBMODULE_PATHS
    ]


def list_optional_submodule_paths(repo_root: Path) -> list[str]:
    return [
        path
        for path in list_git_submodule_paths(repo_root)
        if path in OPTIONAL_SUBMODULE_PATHS
    ]
