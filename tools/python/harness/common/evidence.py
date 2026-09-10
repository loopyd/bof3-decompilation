"""Confined evidence output helpers for review transactions."""

from __future__ import annotations

import json
import os
from pathlib import Path
from typing import Any

from harness.common.files import atomic_write, read_file
from harness.common.directory import validate_repo_path, open_parent_fd


def evidence_output_path(root: Path, value: object) -> str:
    """Return one canonical, symlink-safe application-proof output path."""

    name = validate_repo_path(value)
    if not name.startswith("out/reviews/evidence/"):
        raise ValueError("application proof output must be under out/reviews/evidence")
    parent, _leaf = open_parent_fd(root, name, create=True)
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


def write_new_evidence_output(root: Path, name: str, value: Any) -> None:
    safe = evidence_output_path(root, name)
    atomic_write(
        root,
        safe,
        (json.dumps(value, indent=2, sort_keys=True) + "\n").encode(),
        expected=None,
        exclusive=True,
    )
