"""Recursive immutable dependency evidence and unconditional gitlink exclusion."""

from __future__ import annotations

import json
import os
import stat
from contextlib import contextmanager
from dataclasses import dataclass
from functools import wraps
from pathlib import Path

from harness.common.directory import (
    close_descriptors,
    open_parent_chain,
    validate_repo_path,
    verify_parent_chain,
)
from harness.common.git import capture_git_state, read_git
from harness.common.inventory import CaptureBudget, capture_file, describe_identity
from harness.common.repositories import isolate_repository, verify_metadata
from harness.common.trees import (
    capture_worktree,
    is_worktree_dirty,
    parse_entries,
    verify_worktree,
)

SCHEMA = "bof3.submodule-snapshot/v2"


@dataclass(frozen=True)
class SubmoduleSnapshot:
    """Canonical dependency evidence, never source restoration images."""

    content: bytes


def _encode(value: dict) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":")).encode()


def collect_entries(root: Path) -> tuple[dict, dict]:
    before = capture_git_state(root)
    if before is None:
        return {}, {}
    indexed = parse_entries(read_git(root, ["ls-files", "--stage", "-z"]))
    committed = (
        parse_entries(read_git(root, ["ls-tree", "-r", "-z", before.head]), tree=True)
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


@contextmanager
def _directory(root: Path, name: str):
    sentinel = f"{validate_repo_path(name)}/snapshot"
    descriptors, _ = open_parent_chain(root, sentinel)
    try:
        verify_parent_chain(root, sentinel, descriptors)
        yield descriptors[-1], describe_identity(os.fstat(descriptors[-1]))
        verify_parent_chain(root, sentinel, descriptors)
    finally:
        close_descriptors(descriptors)


def _select_gitdir(
    root: Path, name: str, descriptor: int, budget: CaptureBudget
) -> tuple[str | None, dict | None]:
    try:
        marker = os.stat(".git", dir_fd=descriptor, follow_symlinks=False)
    except FileNotFoundError:
        with os.scandir(descriptor) as entries:
            if next(entries, None) is not None:
                raise ValueError(
                    f"uninitialized submodule has unknown working files: {name}"
                )
        return None, None
    if stat.S_ISDIR(marker.st_mode):
        return f"{name}/.git", describe_identity(marker)
    state, content = capture_file(root, f"{name}/.git", budget)
    if len(content) > 4096:
        raise ValueError("submodule Git marker exceeds limit")
    text = content.decode("utf-8").rstrip("\r\n")
    if not text.startswith("gitdir: ") or "\n" in text or "\r" in text:
        raise ValueError("invalid submodule Git marker")
    location = Path(os.path.normpath(root / name / text[8:]))
    try:
        relative = location.relative_to(root).as_posix()
    except ValueError as error:
        raise ValueError("submodule Git directory escapes repository") from error
    return validate_repo_path(relative), state


def _capture_initialized(
    root: Path, name: str, gitdir: str, ancestors: frozenset, budget: CaptureBudget
) -> dict:
    with _directory(root, gitdir) as (_, identity):
        node = (identity["dev"], identity["ino"])
        if node in ancestors or len(ancestors) >= 32:
            raise ValueError("recursive or over-deep submodule Git directory")
        with isolate_repository(root, gitdir, budget) as (copy, controls):
            head = copy.query(["rev-parse", "--verify", "HEAD"]).strip()
            if len(head) not in {40, 64} or any(
                character not in "0123456789abcdef" for character in head
            ):
                raise ValueError("invalid submodule HEAD")
            indexed = parse_entries(copy.query(["ls-files", "--stage", "-z"]))
            flagged = copy.query(["ls-files", "-v", "-z"])
            if any(
                record and not record.startswith("H ") for record in flagged.split("\0")
            ):
                raise ValueError(
                    "sparse or assume-unchanged submodule indexes are unsupported"
                )
            committed = parse_entries(
                copy.query(["ls-tree", "-r", "-z", head]), tree=True
            )
            links = {
                child: checksum
                for entries in (committed, indexed)
                for child, (mode, checksum) in entries.items()
                if mode == "160000"
            }
            snapshot, _ = capture_worktree(root, name, copy, indexed, set(links))
            dirty = is_worktree_dirty(copy, snapshot, indexed, committed)
            for child, checksum in links.items():
                nested = json.loads(
                    read_submodule(
                        root, f"{name}/{child}", checksum, ancestors | {node}, budget
                    ).content
                )
                snapshot["files"][child] = {"kind": "gitlink", "state": nested}
                dirty = dirty or is_submodule_dirty(nested)
            verification = {
                **snapshot,
                "files": {
                    child: value
                    for child, value in snapshot["files"].items()
                    if child not in links
                },
            }
            verify_worktree(root, name, budget, verification, set(links))
            return {
                "gitdir": gitdir,
                "git_identity": identity,
                "head": head,
                "controls": controls,
                "files": snapshot["files"],
                "namespace": snapshot["namespace"],
                "dirty": dirty,
            }


def _capture_absence(root: Path, name: str) -> dict:
    try:
        descriptors, leaf = open_parent_chain(root, name)
    except FileNotFoundError:
        parent = Path(name).parent.as_posix()
        return {"missing_parent": parent, "parent": _capture_absence(root, parent)}
    try:
        try:
            os.stat(leaf, dir_fd=descriptors[-1], follow_symlinks=False)
        except FileNotFoundError:
            verify_parent_chain(root, name, descriptors)
            metadata = os.fstat(descriptors[-1])
            return {"parent_device": metadata.st_dev, "parent_inode": metadata.st_ino}
        raise ValueError(
            f"submodule disappeared or changed type during capture: {name}"
        )
    finally:
        close_descriptors(descriptors)


def read_submodule(
    root: Path,
    name: str,
    gitlink: str,
    ancestors: frozenset = frozenset(),
    budget: CaptureBudget | None = None,
) -> SubmoduleSnapshot:
    """Capture and revalidate dependencies; never query their mutable Git metadata."""
    validate_repo_path(name)
    budget = budget if budget is not None else CaptureBudget()
    budget.charge()
    facts = {"schema": SCHEMA, "path": name, "gitlink": gitlink}
    try:
        with _directory(root, name) as (descriptor, identity):
            selection = _select_gitdir(root, name, descriptor, budget)
            gitdir, marker = selection
            facts.update(
                state="uninitialized" if gitdir is None else "initialized",
                identity=identity,
            )
            if gitdir is not None:
                facts.update(
                    _capture_initialized(root, name, gitdir, ancestors, budget)
                )
            if _select_gitdir(root, name, descriptor, budget) != selection:
                raise ValueError("submodule Git marker changed during capture")
            if describe_identity(os.fstat(descriptor)) != identity:
                raise ValueError("submodule directory changed during capture")
            facts["marker"] = marker
    except FileNotFoundError:
        absence = _capture_absence(root, name)
        if _capture_absence(root, name) != absence:
            raise ValueError("absent submodule namespace changed during capture")
        facts = {**facts, "state": "absent", "identity": absence, "marker": None}
    verify_observations(root, facts, budget)
    return SubmoduleSnapshot(_encode(facts))


def verify_observations(root: Path, facts: dict, budget: CaptureBudget) -> None:
    """Recheck the full aggregate after queries, without interpreting live Git data."""
    name = facts["path"]
    if facts["state"] == "absent":
        if _capture_absence(root, name) != facts["identity"]:
            raise ValueError("absent submodule changed during verification")
        return
    with _directory(root, name) as (descriptor, identity):
        if identity != facts["identity"]:
            raise ValueError("submodule directory changed during verification")
        gitdir, marker = _select_gitdir(root, name, descriptor, budget)
        if marker != facts["marker"] or gitdir != facts.get("gitdir"):
            raise ValueError("submodule Git marker changed during verification")
        if facts["state"] == "initialized":
            with _directory(root, gitdir) as (_, current):
                if current != facts["git_identity"]:
                    raise ValueError(
                        "submodule Git directory changed during verification"
                    )
                verify_metadata(root, gitdir, budget, facts["controls"]["metadata"])
            for path, expected in facts["controls"]["host"].items():
                if (
                    capture_file(
                        Path("/"),
                        Path(path).relative_to("/").as_posix(),
                        budget,
                        missing_ok=True,
                    )[0]
                    != expected
                ):
                    raise ValueError("host Git input changed during verification")
            nested = {
                child: value["state"]
                for child, value in facts["files"].items()
                if value is not None and value["kind"] == "gitlink"
            }
            snapshot = {
                "namespace": facts["namespace"],
                "files": {
                    child: value
                    for child, value in facts["files"].items()
                    if child not in nested
                },
            }
            verify_worktree(root, name, budget, snapshot, set(nested))
            for child in nested.values():
                verify_observations(root, child, budget)
        if _select_gitdir(root, name, descriptor, budget) != (gitdir, marker):
            raise ValueError("submodule marker changed during aggregate verification")


def is_submodule_dirty(facts: dict) -> bool:
    return facts["state"] == "initialized" and (
        facts["head"] != facts["gitlink"] or facts["dirty"]
    )


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
        or value["schema"] != SCHEMA
        or value["path"] != name
        or value["state"] not in {"absent", "uninitialized", "initialized"}
        or _encode(value) != content
    ):
        raise ValueError("invalid submodule snapshot")
