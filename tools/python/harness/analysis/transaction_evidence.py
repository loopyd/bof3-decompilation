"""Confined evidence output helpers for review transactions."""

from __future__ import annotations

import json
import os
from pathlib import Path
from typing import Any

from .transaction_files import (
    atomic_write,
    canonical_repo_path,
    parent_fd,
    read_file,
)


def evidence_output_path(root: Path, value: object) -> str:
    """Return one canonical, symlink-safe application-proof output path."""

    name = canonical_repo_path(value)
    if not name.startswith("out/reviews/evidence/"):
        raise ValueError("application proof output must be under out/reviews/evidence")
    parent, _leaf = parent_fd(root, name, create=True)
    os.close(parent)
    return name


def write_evidence_output(root: Path, name: str, value: Any) -> None:
    safe = evidence_output_path(root, name)
    current = read_file(root, safe, missing_ok=True)
    atomic_write(
        root,
        safe,
        (json.dumps(value, indent=2, sort_keys=True) + "\n").encode(),
        expected=current,
    )
