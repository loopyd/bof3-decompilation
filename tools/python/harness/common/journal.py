"""Exclusive immutable records and streamed evidence for local harness operations."""

from __future__ import annotations

import json
import os
import stat
from pathlib import Path
from typing import Any, BinaryIO

from harness.common.directory import open_parent_fd
from harness.common.files import read_file
from harness.io import unique_object


def read_record(root: Path, name: str) -> dict[str, Any]:
    value = json.loads(read_file(root, name), object_pairs_hook=unique_object)
    if not isinstance(value, dict):
        raise ValueError("evidence record must be an object")
    return value


def write_record(root: Path, name: str, value: dict[str, Any]) -> None:
    write_private(
        root, name, (json.dumps(value, sort_keys=True, indent=2) + "\n").encode()
    )


def open_stream(root: Path, name: str) -> BinaryIO:
    parent, leaf = open_parent_fd(root, name, create=True)
    try:
        descriptor = os.open(
            leaf,
            os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW | os.O_CLOEXEC,
            0o600,
            dir_fd=parent,
        )
        try:
            opened = os.fstat(descriptor)
            if (
                not stat.S_ISREG(opened.st_mode)
                or opened.st_uid != os.geteuid()
                or opened.st_nlink != 1
            ):
                raise ValueError("evidence stream identity drifted")
            os.fsync(descriptor)
            os.fsync(parent)
            return os.fdopen(descriptor, "wb", buffering=0)
        except BaseException:
            os.close(descriptor)
            raise
    finally:
        os.close(parent)


def write_chunk(stream: BinaryIO, value: bytes | memoryview) -> None:
    remaining = memoryview(value)
    while remaining:
        written = stream.write(remaining)
        if written is None or written <= 0:
            raise OSError("evidence stream write made no progress")
        remaining = remaining[written:]
    os.fsync(stream.fileno())


def write_private(root: Path, name: str, value: bytes) -> None:
    with open_stream(root, name) as stream:
        write_chunk(stream, value)
