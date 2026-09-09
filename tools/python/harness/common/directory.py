"""Confined repository directory traversal and descriptor identity checks."""

from __future__ import annotations

import os
import stat
from pathlib import Path

from harness.common.root import root_fd


def validate_repo_path(value: object) -> str:
    if not isinstance(value, str) or not value or "\\" in value:
        raise ValueError("transaction path must be canonical repo-relative")
    path = Path(value)
    if (
        path.is_absolute()
        or path.as_posix() != value
        or any(part in {"", ".", ".."} for part in path.parts)
    ):
        raise ValueError("transaction path must be canonical repo-relative")
    return value


def open_parent_chain(
    root: Path, name: str, *, create: bool = False
) -> tuple[list[int], str]:
    parts = Path(validate_repo_path(name)).parts
    descriptors = [root_fd(root)]
    try:
        for part in parts[:-1]:
            try:
                child = os.open(
                    part,
                    os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW,
                    dir_fd=descriptors[-1],
                )
            except FileNotFoundError:
                if not create:
                    raise
                os.mkdir(part, 0o755, dir_fd=descriptors[-1])
                os.fsync(descriptors[-1])
                child = os.open(
                    part,
                    os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW,
                    dir_fd=descriptors[-1],
                )
            except OSError as error:
                raise ValueError(
                    f"transaction path has unsafe component: {name}"
                ) from error
            descriptors.append(child)
        return descriptors, parts[-1]
    except BaseException:
        for descriptor in reversed(descriptors):
            os.close(descriptor)
        raise


def open_parent_fd(root: Path, name: str, *, create: bool = False) -> tuple[int, str]:
    descriptors, leaf = open_parent_chain(root, name, create=create)
    for descriptor in descriptors[:-1]:
        os.close(descriptor)
    return descriptors[-1], leaf


def verify_parent_chain(root: Path, name: str, descriptors: list[int]) -> None:
    root_stat = os.stat(root, follow_symlinks=False)
    opened_root = os.fstat(descriptors[0])
    if stat.S_ISLNK(root_stat.st_mode) or (
        root_stat.st_dev,
        root_stat.st_ino,
    ) != (opened_root.st_dev, opened_root.st_ino):
        raise ValueError(f"transaction parent detached from canonical path: {name}")
    for index, part in enumerate(Path(name).parts[:-1]):
        try:
            linked = os.stat(part, dir_fd=descriptors[index], follow_symlinks=False)
        except FileNotFoundError as error:
            raise ValueError(
                f"transaction parent detached from canonical path: {name}"
            ) from error
        opened = os.fstat(descriptors[index + 1])
        if not stat.S_ISDIR(linked.st_mode) or (linked.st_dev, linked.st_ino) != (
            opened.st_dev,
            opened.st_ino,
        ):
            raise ValueError(f"transaction parent detached from canonical path: {name}")


def close_descriptors(descriptors: list[int]) -> None:
    for descriptor in reversed(descriptors):
        os.close(descriptor)
