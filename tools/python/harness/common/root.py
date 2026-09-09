"""Open a verified repository root for confined transaction operations."""

from __future__ import annotations

import os
from pathlib import Path


def root_fd(root: Path) -> int:
    if root.is_symlink():
        raise ValueError("transaction root must not be a symlink")
    try:
        resolved = root.resolve(strict=True)
    except OSError as error:
        raise ValueError("transaction root is invalid") from error
    if not resolved.is_dir():
        raise ValueError("transaction root is invalid")
    return os.open(resolved, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
