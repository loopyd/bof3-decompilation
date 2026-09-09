"""Prepare identity-bound POST images before reviewed source mutation."""

from __future__ import annotations

import hashlib
import os
import re
import secrets
import stat
from pathlib import Path
from typing import Any

from harness.common.directory import open_parent_fd
from harness.common.files import atomic_write, read_file, restore_quarantined
from harness.common.lease import verify_writer
from harness.common.paths import leaf_stat
from harness.common.observation import observe_file
from harness.common.quarantine import reserve_quarantine, validate_quarantine
from harness.common.rename import require_native_noreplace

IMAGE_DIRECTORY = "out/reviews/evidence/images"


def _verify_image_filesystem(root: Path, name: str, path: str) -> None:
    candidate = Path(name)
    while True:
        try:
            destination, _leaf = open_parent_fd(root, candidate.as_posix())
            break
        except FileNotFoundError:
            candidate = candidate.parent
    try:
        prepared, leaf = open_parent_fd(root, path)
        try:
            if os.fstat(destination).st_dev != os.fstat(prepared).st_dev:
                raise ValueError("prepared POST must share the destination filesystem")
            require_native_noreplace(prepared, leaf)
        finally:
            os.close(prepared)
    finally:
        os.close(destination)


def validate_image_path(name: str, path: object) -> str:
    fingerprint = hashlib.sha256(name.encode()).hexdigest()[:16]
    pattern = rf"{IMAGE_DIRECTORY}/[0-9a-f]{{32}}-{fingerprint}/post"
    if not isinstance(path, str) or re.fullmatch(pattern, path) is None:
        raise ValueError("invalid prepared image path")
    return path


def prepare_image(
    root: Path, name: str, content: bytes, mode: int | None
) -> dict[str, Any]:
    fingerprint = hashlib.sha256(name.encode()).hexdigest()[:16]
    directory = f"{IMAGE_DIRECTORY}/{secrets.token_hex(16)}-{fingerprint}"
    parent, leaf = open_parent_fd(root, directory, create=True)
    try:
        os.mkdir(leaf, 0o700, dir_fd=parent)
        os.fsync(parent)
    finally:
        os.close(parent)
    path = f"{directory}/post"
    verify_writer(root)
    atomic_write(root, path, content, expected=None, mode=mode)
    _verify_image_filesystem(root, name, path)
    metadata = leaf_stat(root, path)
    if metadata is None or metadata.st_nlink != 1 or read_file(root, path) != content:
        raise ValueError(f"prepared POST changed: {name}")
    return {
        "sha256": hashlib.sha256(content).hexdigest(),
        "mode": stat.S_IMODE(metadata.st_mode),
        "device": metadata.st_dev,
        "inode": metadata.st_ino,
        "staging": path,
        "quarantine": reserve_quarantine(name),
    }


def install_image(
    root: Path, name: str, content: bytes, *, image: dict[str, Any]
) -> None:
    path = validate_image_path(name, image["staging"])
    if hashlib.sha256(content).hexdigest() != image["sha256"]:
        raise ValueError(
            f"prepared POST content differs from the requested edit: {name}"
        )
    verify_writer(root)
    restore_quarantined(
        root,
        name,
        path,
        expected=content,
        expected_identity=(image["device"], image["inode"]),
        expected_mode=image["mode"],
        create=True,
    )
    verify_writer(root)


def classify_restoration(
    root: Path, name: str, content: bytes | None, image: dict[str, Any]
) -> str:
    pre, post = image["pre"], image["post"]
    before_hash = hashlib.sha256(content).hexdigest() if content is not None else None
    if (
        pre["sha256"] != before_hash
        or hashlib.sha256(image["installed"]).hexdigest() != post["sha256"]
    ):
        raise ValueError(f"transaction recovery content drifted: {name}")
    keys = ("sha256", "mode", "device", "inode")
    expected_pre = (
        {**{key: pre[key] for key in keys}, "links": 1} if content is not None else None
    )
    expected_post = {**{key: post[key] for key in keys}, "links": 1}
    quarantine = image["quarantine"]
    if (quarantine is None) != (content is None):
        raise ValueError(f"transaction PRE quarantine binding drifted: {name}")
    if quarantine is not None:
        validate_quarantine(name, quarantine)
    staging = validate_image_path(name, post["staging"])
    displaced_path = validate_quarantine(name, post["quarantine"])
    if displaced_path == quarantine:
        raise ValueError(f"transaction quarantines overlap: {name}")
    current = observe_file(root, name)
    displaced_pre = observe_file(root, quarantine) if quarantine is not None else None
    staged = observe_file(root, staging)
    displaced_post = observe_file(root, displaced_path)
    if (
        any(
            value is not None and value != expected_post
            for value in (staged, displaced_post)
        )
        or sum(value == expected_post for value in (current, staged, displaced_post))
        != 1
    ):
        raise ValueError(f"transaction POST locations drifted during rollback: {name}")
    if current == expected_pre and displaced_pre is None:
        return "pre"
    if displaced_pre == expected_pre:
        if current == expected_post:
            return "post"
        if current is None:
            return "missing"
    raise ValueError(f"transaction path drifted during rollback: {name}")
