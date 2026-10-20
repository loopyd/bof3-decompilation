"""Gitlink boundaries for transaction mutations and evidence outputs."""

from __future__ import annotations

from functools import wraps
from pathlib import Path

from harness.common.directory import validate_repo_path
from harness.common.git import capture_git_state, read_git


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


# Whole-index and whole-tree scans are expensive; a gitlink set is reused while the
# captured HEAD and index snapshot are unchanged. Any index or HEAD change misses.
_GITLINK_CACHE: dict[tuple, dict[str, str]] = {}
_GITLINK_CACHE_LIMIT = 8


def _gitlink_key(root: Path, state) -> tuple:
    # GitState is a frozen snapshot of HEAD plus the index path, content and
    # filesystem state, so it is directly usable as a cache key.
    return (str(root), state)


def collect_gitlinks(root: Path) -> dict[str, str]:
    """Protect both staged and HEAD boundaries, including staged removals."""
    state = capture_git_state(root)
    if state is not None:
        cached = _GITLINK_CACHE.get(_gitlink_key(root, state))
        if cached is not None:
            return dict(cached)
    indexed, committed = collect_entries(root)
    result = {
        name: checksum
        for entries in (committed, indexed)
        for name, (mode, checksum) in entries.items()
        if mode == "160000"
    }
    state = capture_git_state(root)
    if state is not None:
        if len(_GITLINK_CACHE) >= _GITLINK_CACHE_LIMIT:
            _GITLINK_CACHE.clear()
        _GITLINK_CACHE[_gitlink_key(root, state)] = result
    return dict(result)


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
