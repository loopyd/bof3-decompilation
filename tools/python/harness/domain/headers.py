"""Validate planned header claims without admitting missing live files."""

from __future__ import annotations

import tomllib
from pathlib import PurePosixPath

from harness.common.directory import validate_repo_path
from .ids import normalize_target_id


def validate_header_transition(
    target: str, header: str, before: str, after: str
) -> str:
    """Allow one canonical header appended to an otherwise unchanged manifest."""
    target_path = PurePosixPath(target)
    if normalize_target_id(target).value != target:
        raise ValueError("header creation needs a canonical target")
    if (
        target_path.is_absolute()
        or str(target_path) != target
        or ".." in target_path.parts
    ):
        raise ValueError("header creation needs a canonical target")
    validate_repo_path(header)
    if not header.startswith("include/") or not header.endswith("_internal.h"):
        raise ValueError("creation requires a target-private internal header")
    try:
        original = tomllib.loads(before)
        proposed = tomllib.loads(after)
    except (TypeError, tomllib.TOMLDecodeError) as error:
        raise ValueError(f"invalid proposed header manifest: {error}") from error
    if original.get("id") != target:
        raise ValueError("header creation target differs from manifest identity")
    headers = original.get("headers", [])
    if not isinstance(headers, list) or any(
        not isinstance(name, str) for name in headers
    ):
        raise ValueError("invalid original header claims")
    if header in headers:
        raise ValueError("new header is already claimed")
    original["headers"] = [*headers, header]
    if original != proposed:
        raise ValueError(
            "creation may only append its header claim; other manifest facts are frozen"
        )
    return f"config/targets/{target}/target.toml"
