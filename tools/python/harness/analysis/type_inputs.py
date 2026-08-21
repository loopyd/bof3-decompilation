"""Owned type-input discovery, validation, and deterministic fingerprints."""

from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Any

from ..discovery import file_sha256
from ..domain.claims import manifest_header_paths, manifest_source_paths

SCALAR_HEADER = Path("include/base/types.h")


def authored_type_headers(
    root: Path, manifest: Any, *, include_shared: bool = True
) -> list[tuple[Path, str]]:
    """Return explicit shared and target-private declaration owners."""

    rows = [(root / SCALAR_HEADER, "shared_base")] if include_shared else []
    rows.extend(
        (path, "header_claim") for path in manifest_header_paths(root, manifest)
    )
    for path, _provenance in rows:
        if not path.is_file():
            raise ValueError(f"missing claimed type input: {path.relative_to(root)}")
    return sorted(set(rows), key=lambda row: row[0].as_posix())


def type_input_rows(root: Path, manifest: Any) -> list[tuple[str, str, str]]:
    """Return every required source of indexed type facts or fail closed."""

    paths: list[tuple[Path, str]] = [
        (root / manifest.splat, "splat"),
        (root / f"config/targets/{manifest.id.value}/target.toml", "manifest"),
        (root / SCALAR_HEADER, "shared_base"),
    ]
    paths.extend((path, "header") for path in manifest_header_paths(root, manifest))
    if manifest.has_explicit_sources:
        paths.extend((path, "source") for path in manifest_source_paths(root, manifest))
    missing = sorted(
        path.relative_to(root).as_posix() for path, _kind in paths if not path.is_file()
    )
    if missing:
        raise ValueError(
            f"missing claimed type inputs for {manifest.id.value}: {missing}"
        )
    return [
        (path.relative_to(root).as_posix(), file_sha256(path), kind)
        for path, kind in sorted(set(paths), key=lambda item: item[0].as_posix())
    ]


def type_input_digest(inputs: list[tuple[str, str, str]]) -> str:
    payload = "\n".join("\0".join(row) for row in inputs).encode()
    return hashlib.sha256(payload).hexdigest()


__all__ = [
    "SCALAR_HEADER",
    "authored_type_headers",
    "type_input_digest",
    "type_input_rows",
]
