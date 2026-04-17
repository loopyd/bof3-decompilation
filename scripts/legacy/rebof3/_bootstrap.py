from __future__ import annotations

import sys
from pathlib import Path


def bootstrap_package(file_path: str, *, repo_depth: int = 3) -> str:
    path = Path(file_path).resolve()
    repo_root = path.parents[repo_depth]
    repo_root_text = str(repo_root)
    if repo_root_text not in sys.path:
        sys.path.insert(0, repo_root_text)
    return ".".join(path.relative_to(repo_root).parts[:-1])


