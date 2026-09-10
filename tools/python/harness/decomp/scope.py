"""Mission source ownership and output-only policy for local native gates."""

from __future__ import annotations

import os
from pathlib import Path
import stat

from harness.common.deadlines import check_deadline
from harness.common.digests import digest
from harness.common.directory import open_parent_fd, validate_repo_path
from harness.decomp.inventory import observe_path

MODE = "mission-scoped-source-edit"
PROTECTED = (
    ".git",
    ".codex",
    ".agents",
    "out/reviews",
    "out/index",
    "out/binaries",
    "out/reverse",
)
OUTPUTS = ("build", "out/asm-diff", "out/bindings", "out/matching", "out/splat")


def capture_directory(root: Path, name: str) -> dict:
    parent, _leaf = open_parent_fd(root, f"{name}/.directory-probe")
    try:
        opened = os.fstat(parent)
        named = (root / name).lstat()
        if not stat.S_ISDIR(named.st_mode) or (opened.st_dev, opened.st_ino) != (
            named.st_dev,
            named.st_ino,
        ):
            raise ValueError(f"mission directory identity drifted: {name}")
        return {
            "device": opened.st_dev,
            "inode": opened.st_ino,
            "uid": opened.st_uid,
            "gid": opened.st_gid,
        }
    finally:
        os.close(parent)


def capture_members(root: Path, paths: list[str]) -> dict:
    parents = sorted({str(Path(name).parent) for name in paths})
    result = {}
    for name in parents:
        validate_repo_path(name)
        check_deadline()
        before = capture_directory(root, name)
        entries = {}
        for child in sorted((root / name).iterdir()):
            check_deadline()
            relative = child.relative_to(root).as_posix()
            entries[relative] = observe_path(root, relative)
        if before != capture_directory(root, name):
            raise ValueError("mission source directory changed during policy capture")
        result[name] = {"identity": before, "entries": entries}
    return result


def capture_policy(
    root: Path, paths: list[str], inputs: list[str], temporary: str
) -> dict:
    root = root.resolve(strict=True)
    if (
        not isinstance(paths, list)
        or not paths
        or any(not isinstance(name, str) for name in paths)
        or paths != sorted(set(paths))
    ):
        raise ValueError("native lift scope needs sorted unique explicit paths")
    validate_repo_path(temporary)
    if not temporary.startswith("out/dispatch/"):
        raise ValueError(
            "native lift temporary data needs an owned dispatch scratch path"
        )
    for name in paths:
        validate_repo_path(name)
        if any(
            name == protected or name.startswith(protected + "/")
            for protected in PROTECTED
        ):
            raise ValueError("native lift source scope crosses protected evidence")
    members = capture_members(root, paths)
    outputs = (*OUTPUTS, temporary)
    writable = sorted(set(members) | set(outputs))
    readonly = set(PROTECTED)
    for parent in members.values():
        if any(
            state is not None and "symlink" in state
            for state in parent["entries"].values()
        ):
            raise ValueError("native source directory contains unsupported symlinks")
        readonly.update(name for name in parent["entries"] if name not in paths)
    for name in inputs:
        validate_repo_path(name)
        if name not in paths and any(
            name.startswith(parent + "/") for parent in writable
        ):
            readonly.add(name)
    for name in writable:
        if any(
            name == protected or name.startswith(protected + "/")
            for protected in readonly
        ):
            raise ValueError("native writable scope would reopen a protected path")
    readonly = sorted(
        name
        for name in readonly
        if not any(
            name.startswith(parent + "/") for parent in readonly if parent != name
        )
    )
    return {
        "schema": "bof3.mission-directory-policy/v1",
        "mode": MODE,
        "root": str(root),
        "paths": paths,
        "source_parents": members,
        "outputs": {name: capture_directory(root, name) for name in outputs},
        "writable": writable,
        "readonly": readonly,
        "temporary": temporary,
        "network": False,
        "future_names_confined": False,
    }


def verify_policy(
    root: Path, policy: dict, expected_digest: str, *, before_dispatch: bool
) -> None:
    if digest(policy) != expected_digest:
        raise ValueError("mission policy pin drifted")
    if root.resolve().as_posix() != policy["root"]:
        raise ValueError("mission policy root drifted")
    for name, identity in policy["outputs"].items():
        if capture_directory(root, name) != identity:
            raise ValueError("native output directory identity drifted")
    current = capture_members(root, policy["paths"])
    for name, before in policy["source_parents"].items():
        after = current[name]
        if before["identity"] != after["identity"]:
            raise ValueError("native source parent identity drifted")
        changed = {
            entry
            for entry in set(before["entries"]) | set(after["entries"])
            if before["entries"].get(entry) != after["entries"].get(entry)
        }
        if changed - (set() if before_dispatch else set(policy["paths"])):
            raise ValueError(
                "native source-directory membership or unowned state drifted"
            )
