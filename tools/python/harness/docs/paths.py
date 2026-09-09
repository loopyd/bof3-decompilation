"""Validate explicit authored Markdown scopes without following discovery aliases."""

from __future__ import annotations

from collections.abc import Sequence
from pathlib import Path, PurePosixPath

from harness.common.directory import validate_repo_path
from harness.common.paths import leaf_stat


def validate_document_paths(root: Path, values: Sequence[str]) -> tuple[str, ...]:
    if not values or len(values) != len(set(values)):
        raise ValueError("docs requires nonempty, unique explicit Markdown paths")
    paths = tuple(validate_repo_path(value) for value in values)
    for name in paths:
        path = PurePosixPath(name)
        if (
            path.suffix.lower() != ".md"
            or path.parts[0]
            in {
                "out",
                "build",
                "toolchains",
                "inputs",
                "tmp",
                ".git",
                ".venv",
                ".agents",
                "sessions",
                "ledger",
                ".pi-subagents",
                ".cache",
                ".uv-cache",
                "dist",
                "assets",
            }
            or (path.parts[0] == ".pi" and not name.startswith(".pi/agents/"))
            or (path.parts[0] == ".codex" and not name.startswith(".codex/skills/"))
            or (len(path.parts) == 1 and name.startswith("session-"))
            or leaf_stat(root, name) is None
        ):
            raise ValueError("docs requires existing authored repository Markdown")
    return paths


def validate_cleanup_paths(root: Path, values: Sequence[str]) -> tuple[str, ...]:
    """Preserve the older docs cleanup route's narrower explicit repair scope."""
    if not values:
        raise ValueError("docs requires at least one documentation path")
    if any("\\" in value for value in values):
        raise ValueError(
            "documentation cleanup paths must use canonical forward slashes"
        )
    for value in values:
        path = PurePosixPath(value)
        if value not in {"README.md", "AGENTS.md"} and not (
            len(path.parts) >= 2 and path.parts[0] == "docs"
        ):
            raise ValueError(
                "documentation cleanup paths must be existing regular repository Markdown"
            )
        try:
            validate_document_paths(root, (value,))
        except (OSError, ValueError) as error:
            raise ValueError(
                "documentation cleanup paths must be existing regular repository Markdown"
            ) from error
    return tuple(values)
