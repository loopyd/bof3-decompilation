"""Confined worktree mirrors for isolated Git status and dependency evidence."""

from __future__ import annotations

import base64
import hashlib
import stat
from dataclasses import asdict
from pathlib import Path

from harness.common.directory import validate_repo_path
from harness.common.inventory import capture_file, scan_tree
from harness.common.inventory import CaptureBudget
from harness.common.links import read_symlink
from harness.common.repositories import RepositoryCopy


def parse_entries(output: str, *, tree: bool = False) -> dict[str, tuple[str, str]]:
    entries = {}
    for record in output.split("\0"):
        if not record:
            continue
        fields, name = record.split("\t", 1)
        mode, middle, last = fields.split()
        checksum = last if tree else middle
        validate_repo_path(name)
        if (not tree and last != "0") or name in entries:
            raise ValueError("workspace capture requires an unmerged-free Git index")
        if mode not in {"100644", "100755", "120000", "160000"}:
            raise ValueError("unsupported Git entry kind")
        if len(checksum) not in {40, 64} or any(
            character not in "0123456789abcdef" for character in checksum
        ):
            raise ValueError("invalid Git entry object")
        entries[name] = (mode, checksum)
    return entries


def capture_worktree(
    root: Path,
    name: str | None,
    copy: RepositoryCopy,
    entries: dict,
    boundaries: set[str],
    exclusions: frozenset[str] = frozenset(),
) -> tuple[dict, dict]:
    excluded = frozenset({".git", *boundaries, *exclusions})
    namespace = scan_tree(
        root, name, copy.budget, excluded=excluded, opaque=frozenset({".git"})
    )
    files = {}
    for child, identity in namespace.items():
        if not child:
            continue
        if ".git" in Path(child).parts:
            continue
        path = copy.worktree / child
        mode = identity["mode"]
        if stat.S_ISDIR(mode):
            path.mkdir(parents=True, exist_ok=True)
            continue
        if not (stat.S_ISREG(mode) or stat.S_ISLNK(mode)):
            raise ValueError(f"special worktree node prevents capture: {name}/{child}")
        path.parent.mkdir(parents=True, exist_ok=True)
        if stat.S_ISLNK(mode) and path.name in {".gitignore", ".gitattributes"}:
            raise ValueError("linked Git worktree controls require explicit support")
        path.write_bytes(b"")
        if path.name in {".gitignore", ".gitattributes"}:
            record, content = capture_file(
                root, f"{name}/{child}" if name else child, copy.budget
            )
            files[child] = {"kind": "file", **record}
            path.write_bytes(content)
    for boundary in boundaries:
        (copy.worktree / boundary).mkdir(parents=True, exist_ok=True)
    untracked = copy.query(["ls-files", "--others", "--exclude-standard", "-z"])
    selected = set(entries) | {
        validate_repo_path(child) for child in untracked.split("\0") if child
    }
    selected = {
        child
        for child in selected
        if not any(
            child == prefix or child.startswith(prefix + "/") for prefix in exclusions
        )
    }
    validate_embedded(copy, namespace, selected)
    inventory = sorted(selected - boundaries)
    selected.update(files)
    for child in sorted(selected - boundaries):
        path = f"{name}/{child}" if name else child
        identity = namespace.get(child)
        if identity is None:
            files[child] = None
            continue
        destination = copy.worktree / child
        if stat.S_ISLNK(identity["mode"]):
            link = read_symlink(root, path)
            if link is None:
                raise ValueError("worktree link changed during capture")
            copy.budget.charge(len(link.content))
            record = asdict(link)
            record["content_base64"] = base64.b64encode(record.pop("content")).decode(
                "ascii"
            )
            files[child] = {"kind": "symlink", **record}
            if destination.name not in {".gitignore", ".gitattributes"}:
                destination.write_bytes(link.content)
        else:
            record, content = capture_file(root, path, copy.budget)
            if {key: record[key] for key in identity} != identity:
                raise ValueError("worktree file changed during capture")
            files[child] = {"kind": "file", **record}
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(content)
            destination.chmod(0o755 if identity["mode"] & 0o111 else 0o644)
    return {
        "namespace": namespace,
        "files": files,
        "inventory": inventory,
        "exclusions": sorted(exclusions),
    }, entries


def validate_embedded(
    copy: RepositoryCopy, namespace: dict, selected: set[str]
) -> None:
    parents = {
        Path(child).parent.as_posix() + "/"
        for child in namespace
        if Path(child).name == ".git"
    }
    if not parents:
        return
    output = copy.query(
        ["check-ignore", "--no-index", "--stdin", "-z"],
        input_data="".join(parent + "\0" for parent in sorted(parents)).encode(
            "utf-8", "surrogateescape"
        ),
        accepted_codes=(0, 1),
    )
    ignored = {parent for parent in output.split("\0") if parent}
    if ignored != parents or any(
        item == parent[:-1] or item.startswith(parent)
        for parent in parents
        for item in selected
    ):
        raise ValueError("unignored undeclared nested Git worktree prevents capture")


def verify_worktree(
    root: Path,
    name: str | None,
    budget: CaptureBudget,
    snapshot: dict,
    boundaries: set[str],
) -> None:
    if (
        scan_tree(
            root,
            name,
            budget,
            excluded=frozenset({".git", *boundaries, *snapshot.get("exclusions", [])}),
            opaque=frozenset({".git"}),
        )
        != snapshot["namespace"]
    ):
        raise ValueError("submodule worktree namespace changed during query")
    for child, expected in snapshot["files"].items():
        path = f"{name}/{child}" if name else child
        if expected is not None and expected["kind"] == "symlink":
            link = read_symlink(root, path)
            if link is None:
                raise ValueError("submodule worktree link disappeared")
            actual = asdict(link)
            actual["content_base64"] = base64.b64encode(actual.pop("content")).decode(
                "ascii"
            )
            actual = {"kind": "symlink", **actual}
        else:
            actual = capture_file(root, path, budget, missing_ok=True)[0]
            if actual is not None:
                actual = {"kind": "file", **actual}
        if actual != expected:
            raise ValueError(f"snapshot worktree input changed during query: {path}")


def is_worktree_dirty(
    copy: RepositoryCopy, snapshot: dict, entries: dict, committed: dict
) -> bool:
    return entries != committed or bool(describe_changes(copy, snapshot, entries))


def describe_changes(
    copy: RepositoryCopy, snapshot: dict, entries: dict
) -> dict[str, str]:
    output = copy.query(
        [
            "status",
            "--porcelain=v1",
            "-z",
            "--untracked-files=all",
            "--ignore-submodules=all",
            "--no-renames",
        ]
    )
    changes = {record[3:]: record[:2] for record in output.split("\0") if record}
    links = {
        name
        for name, value in snapshot["files"].items()
        if value is not None and value["kind"] == "symlink"
    }
    for name in links:
        content = base64.b64decode(snapshot["files"][name]["content_base64"])
        entry = entries.get(name)
        if entry is None:
            continue
        algorithm = "sha1" if len(entry[1]) == 40 else "sha256"
        checksum = hashlib.new(
            algorithm, f"blob {len(content)}\0".encode() + content
        ).hexdigest()
        staged = changes.get(name, "  ")[0]
        working = "T" if entry[0] != "120000" else "M" if checksum != entry[1] else " "
        if staged + working == "  ":
            changes.pop(name, None)
        else:
            changes[name] = staged + working
    return changes
