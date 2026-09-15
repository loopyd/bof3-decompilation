"""Owned type-input discovery, validation, and deterministic fingerprints."""

from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Any

from harness.domain.claims import manifest_header_paths, manifest_source_paths
from harness.io import file_sha256

SCALAR_HEADER = Path("include/base/types.h")


def authored_type_headers(
    root: Path,
    manifest: Any,
    *,
    include_shared: bool = True,
    validate_paths: bool = True,
) -> list[tuple[Path, str]]:
    """Return explicit shared and target-private declaration owners."""

    rows = [(root / SCALAR_HEADER, "shared_base")] if include_shared else []
    rows.extend(
        (path, "header_claim") for path in manifest_header_paths(root, manifest)
    )
    for path, _provenance in rows:
        if validate_paths and not path.is_file():
            raise ValueError(f"missing claimed type input: {path.relative_to(root)}")
    return sorted(set(rows), key=lambda row: row[0].as_posix())


def type_input_rows(
    root: Path,
    manifest: Any,
    *,
    digest_file=file_sha256,
    source_paths: tuple[Path, ...] | None = None,
    header_paths: tuple[Path, ...] | None = None,
    validate_paths: bool = True,
) -> list[tuple[str, str, str]]:
    """Return every required source of indexed type facts or fail closed."""

    paths: list[tuple[Path, str]] = [
        (root / manifest.splat, "splat"),
        (root / f"config/targets/{manifest.id.value}/target.toml", "manifest"),
        (root / SCALAR_HEADER, "shared_base"),
    ]
    headers = (
        manifest_header_paths(root, manifest) if header_paths is None else header_paths
    )
    paths.extend((path, "header") for path in headers)
    if manifest.has_explicit_sources:
        sources = (
            manifest_source_paths(root, manifest)
            if source_paths is None
            else source_paths
        )
        paths.extend((path, "source") for path in sources)
    missing = (
        sorted(
            path.relative_to(root).as_posix()
            for path, _kind in paths
            if not path.is_file()
        )
        if validate_paths
        else []
    )
    if missing:
        raise ValueError(
            f"missing claimed type inputs for {manifest.id.value}: {missing}"
        )
    return [
        (path.relative_to(root).as_posix(), digest_file(path), kind)
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
