"""PsyQ SDK materialization: staging, normalization, and archive extraction.

Discovery of source trees/archives stays in :mod:`.psyq_discovery`; this
module owns mutating a discovered input into the staged SDK layout.
"""

from __future__ import annotations

import contextlib
import shutil
import tempfile
from pathlib import Path

from .psyq_discovery import PsyqSource
from .releases import extract_archive


TEXT_FILE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp", ".inc", ".inl", ".s", ".txt"}


def create_lowercase_aliases(root: Path) -> None:
    for candidate in sorted(root.rglob("*")):
        alias_name = candidate.name.lower()
        if candidate.name == alias_name:
            continue
        alias_path = candidate.with_name(alias_name)
        if alias_path.exists():
            continue
        if candidate.is_dir():
            shutil.copytree(candidate, alias_path)
        else:
            shutil.copy2(candidate, alias_path)


def should_normalize_text_file(path: Path) -> bool:
    if path.suffix.lower() in TEXT_FILE_SUFFIXES:
        return True
    return path.suffix == "" and path.parent.name.lower() == "include"


def file_uses_crlf(path: Path) -> bool:
    return b"\r\n" in path.read_bytes()


def normalize_text_file_newlines(path: Path) -> bool:
    data = path.read_bytes()
    if b"\0" in data or b"\r" not in data:
        return False
    normalized = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    if normalized == data:
        return False
    path.write_bytes(normalized)
    return True


def list_text_files_with_crlf(root: Path) -> list[Path]:
    offending: list[Path] = []
    for candidate in sorted(root.rglob("*")):
        if not candidate.is_file() or candidate.is_symlink():
            continue
        if not should_normalize_text_file(candidate):
            continue
        if file_uses_crlf(candidate):
            offending.append(candidate)
    return offending


def normalize_text_tree_newlines(root: Path) -> int:
    candidates = list_text_files_with_crlf(root)
    return sum(1 for candidate in candidates if normalize_text_file_newlines(candidate))


def staged_sdk_layout_exists(root: Path) -> bool:
    return (root / "include").exists() and (root / "lib").exists()


def original_sdk_is_ready(root: Path) -> bool:
    libgpu_header = root / "include" / "libgpu.h"
    return (
        staged_sdk_layout_exists(root)
        and libgpu_header.exists()
        and not list_text_files_with_crlf(root)
    )


@contextlib.contextmanager
def materialized_source_root(source_input: PsyqSource):
    if source_input.kind == "tree":
        yield source_input.path
        return
    with tempfile.TemporaryDirectory(prefix="harness-psyq-") as tmp_dir:
        extraction_root = Path(tmp_dir) / "source"
        extraction_root.mkdir(parents=True, exist_ok=True)
        extract_archive(source_input.path, extraction_root)
        yield extraction_root


__all__ = [
    "TEXT_FILE_SUFFIXES",
    "create_lowercase_aliases",
    "file_uses_crlf",
    "list_text_files_with_crlf",
    "materialized_source_root",
    "normalize_text_file_newlines",
    "normalize_text_tree_newlines",
    "original_sdk_is_ready",
    "should_normalize_text_file",
    "staged_sdk_layout_exists",
]
