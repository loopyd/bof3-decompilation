"""Validate explicit authored Markdown scopes without following discovery aliases."""

from __future__ import annotations

from collections.abc import Sequence
import os
from pathlib import Path, PurePosixPath
import stat

from harness.common.directory import open_parent_fd, validate_repo_path
from harness.common.paths import leaf_stat


EXCLUDED_ROOTS = {
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


def is_authored_path(name: str) -> bool:
    path = PurePosixPath(name)
    return (
        bool(path.parts)
        and path.parts[0] not in EXCLUDED_ROOTS
        and (path.parts[0] != ".pi" or name.startswith(".pi/agents/"))
        and (path.parts[0] != ".codex" or name.startswith(".codex/skills/"))
        and not (len(path.parts) == 1 and name.startswith("session-"))
    )


def validate_document_paths(root: Path, values: Sequence[str]) -> tuple[str, ...]:
    if not values or len(values) != len(set(values)):
        raise ValueError("docs requires nonempty, unique explicit Markdown paths")
    paths = tuple(validate_repo_path(value) for value in values)
    for name in paths:
        path = PurePosixPath(name)
        if (
            path.suffix.lower() != ".md"
            or not is_authored_path(name)
            or leaf_stat(root, name) is None
        ):
            raise ValueError("docs requires existing authored repository Markdown")
    return paths


def collect_document_paths(
    root: Path, values: Sequence[str]
) -> tuple[list[str], list[dict]]:
    if not values or len(set(values)) != len(values):
        raise ValueError("refs requires nonempty, unique file or directory scopes")
    documents = []
    skipped = []
    seen = set()

    def visit(name: str, *, explicit: bool) -> None:
        if name in seen:
            return
        seen.add(name)
        if not is_authored_path(name + "/"):
            if explicit:
                raise ValueError(f"refs scope is not authored Markdown: {name}")
            skipped.append({"path": name, "reason": "excluded"})
            return
        parent, leaf = open_parent_fd(root, name)
        try:
            details = os.stat(leaf, dir_fd=parent, follow_symlinks=False)
            if stat.S_ISDIR(details.st_mode):
                descriptor = os.open(
                    leaf, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW, dir_fd=parent
                )
                try:
                    children = sorted(os.listdir(descriptor))
                finally:
                    os.close(descriptor)
            elif stat.S_ISREG(details.st_mode):
                if Path(name).suffix.lower() == ".md":
                    documents.extend(validate_document_paths(root, [name]))
                elif explicit:
                    raise ValueError(f"refs requires Markdown files: {name}")
                return
            else:
                if explicit:
                    raise ValueError(
                        f"refs scope is not a regular file/directory: {name}"
                    )
                skipped.append({"path": name, "reason": "symlink-or-special-file"})
                return
        finally:
            os.close(parent)
        for child in children:
            visit(f"{name}/{child}", explicit=False)

    for value in values:
        visit(validate_repo_path(value), explicit=True)
    return documents, skipped


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
