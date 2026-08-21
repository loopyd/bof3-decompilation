"""Reviewed-annotation scope and digest facts for naming transactions."""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

from ..domain.sources import local_include_files


def reviewed_annotations(root: Path, target: str) -> list[str]:
    """Reviewed.rz plus recursively resolved local includes, repo-relative."""
    root = root.resolve()
    path = root / "config" / "targets" / target / "reviewed.rz"
    if not path.is_file():
        return []
    return [
        path.relative_to(root).as_posix(),
        *(
            file.relative_to(root).as_posix()
            for file in local_include_files(root, [path])
        ),
    ]


def _reviewed_payload(
    root: Path, target: str, old_name: str | None = None, new_name: str | None = None
) -> str:
    contents: dict[str, str | None] = {}
    for rel in reviewed_annotations(root, target):
        path = root / rel
        text = (
            path.read_text(encoding="utf-8", errors="replace")
            if path.is_file()
            else None
        )
        if old_name and new_name and text is not None:
            text = re.sub(rf"\b{re.escape(old_name)}\b", new_name, text)
        contents[rel] = text
    return json.dumps(contents, sort_keys=True)


def reviewed_scope_digest(root: Path, target: str) -> str | None:
    """SHA-256 over the current reviewed-annotation scope, or None when absent."""
    if not reviewed_annotations(root, target):
        return None
    return hashlib.sha256(
        _reviewed_payload(root.resolve(), target).encode("utf-8")
    ).hexdigest()


def expected_reviewed_digest(
    root: Path, target: str, old_name: str, new_name: str
) -> str | None:
    """Digest after the approved old-to-new spelling rewrite."""
    if not reviewed_annotations(root, target):
        return None
    return hashlib.sha256(
        _reviewed_payload(root.resolve(), target, old_name, new_name).encode("utf-8")
    ).hexdigest()
