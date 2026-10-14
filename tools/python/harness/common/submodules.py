"""Gitlink mutation boundaries and legacy dependency record validation."""

from __future__ import annotations

import json
from functools import wraps
from pathlib import Path

from harness.common.directory import validate_repo_path
from harness.common.git import capture_git_state, read_git

SCHEMA = "bof3.submodule-snapshot/v3"
LEGACY_SCHEMA = "bof3.submodule-snapshot/v2"


def _encode(value: dict) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":")).encode()


def _parse_entries(output: str, *, tree: bool = False) -> dict[str, tuple[str, str]]:
    entries = {}
    for record in output.split("\0"):
        if not record:
            continue
        fields, name = record.split("\t", 1)
        mode, middle, last = fields.split()
        checksum = last if tree else middle
        validate_repo_path(name)
        if (not tree and last != "0") or name in entries:
            raise ValueError("gitlink discovery requires an unmerged-free Git index")
        if mode not in {"100644", "100755", "120000", "160000"}:
            raise ValueError("unsupported Git entry kind")
        if len(checksum) not in {40, 64} or any(
            character not in "0123456789abcdef" for character in checksum
        ):
            raise ValueError("invalid Git entry object")
        entries[name] = (mode, checksum)
    return entries


def collect_entries(root: Path) -> tuple[dict, dict]:
    before = capture_git_state(root)
    if before is None:
        return {}, {}
    indexed = _parse_entries(read_git(root, ["ls-files", "--stage", "-z"]))
    committed = (
        _parse_entries(read_git(root, ["ls-tree", "-r", "-z", before.head]), tree=True)
        if before.head is not None
        else {}
    )
    if capture_git_state(root) != before:
        raise ValueError("Git index or HEAD changed during gitlink discovery")
    return indexed, committed


def collect_gitlinks(root: Path) -> dict[str, str]:
    """Protect both staged and HEAD boundaries, including staged removals."""
    indexed, committed = collect_entries(root)
    return {
        name: checksum
        for entries in (committed, indexed)
        for name, (mode, checksum) in entries.items()
        if mode == "160000"
    }


def validate_mutations(root: Path, names: set[str]) -> None:
    """Refuse dependency ownership even when optional transaction guards are absent."""
    for name in names:
        validate_repo_path(name)
    for boundary in collect_gitlinks(root):
        if any(
            name == boundary
            or name.startswith(boundary + "/")
            or boundary.startswith(name + "/")
            for name in names
        ):
            raise ValueError(
                f"transaction mutation overlaps protected submodule: {boundary}"
            )


def protect_artifacts(function):
    """Exclude dependencies from the output namespace before acquiring a writer."""

    @wraps(function)
    def execute(root: Path, *arguments, **options):
        validate_mutations(root, {"out"})
        return function(root, *arguments, **options)

    return execute


def validate_snapshot(name: str, content: bytes) -> None:
    value = json.loads(content)
    fields = {"schema", "path", "gitlink", "state", "identity", "marker"}
    if isinstance(value, dict) and value.get("state") == "initialized":
        fields.update(
            {
                "gitdir",
                "git_identity",
                "head",
                "controls",
                "files",
                "namespace",
                "dirty",
            }
        )
    if (
        not isinstance(value, dict)
        or set(value) != fields
        or value["schema"] not in (SCHEMA, LEGACY_SCHEMA)
        or value["path"] != name
        or value["state"] not in {"absent", "uninitialized", "initialized"}
        or _encode(value) != content
    ):
        raise ValueError("invalid submodule snapshot")
