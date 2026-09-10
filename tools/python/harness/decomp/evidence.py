"""Exclusive lift evidence directories, native streams and failure retention."""

from __future__ import annotations

from contextlib import contextmanager, ExitStack
import hashlib
import os
from pathlib import Path
from typing import Iterator

from harness.common.directory import open_parent_fd, validate_repo_path
from harness.common.files import read_file
from harness.common.process import ProcessCleanupError
from harness.context.journal import open_stream, write_chunk, write_record


def hash_files(root: Path, directory: str, names: list[str]) -> dict[str, str]:
    if len(set(names)) != len(names):
        raise ValueError("lift evidence contains duplicate filenames")
    for name in names:
        validate_repo_path(name)
        if len(Path(name).parts) != 1:
            raise ValueError("lift evidence filenames must be direct children")
    return {
        name: hashlib.sha256(read_file(root, f"{directory}/{name}")).hexdigest()
        for name in sorted(names)
    }


def reserve_directory(root: Path, output: str, *, kind: str) -> None:
    validate_repo_path(output)
    if kind not in {"diagnosis", "audit"} or (
        not output.startswith(f"out/reviews/lift-{kind}/")
        or len(Path(output).parts) != 4
    ):
        raise ValueError(
            "lift output must be a fresh out/reviews/lift-KIND/NAME directory"
        )
    parent, leaf = open_parent_fd(root, output, create=True)
    try:
        try:
            os.mkdir(leaf, dir_fd=parent)
        except FileExistsError as error:
            raise ValueError(
                "lift output already exists; retain prior evidence"
            ) from error
        os.fsync(parent)
    finally:
        os.close(parent)


@contextmanager
def capture_streams(root: Path, output: str) -> Iterator[tuple]:
    with ExitStack() as stack:
        streams = {}
        artifacts = []

        def publish(name: str, value: dict) -> None:
            if name.endswith("-started"):
                label = name.removesuffix("-started")
                for stream in ("stdout", "stderr"):
                    streams[label, stream] = stack.enter_context(
                        open_stream(root, f"{output}/{label}-{stream}.log")
                    )
                    artifacts.append(f"{label}-{stream}.log")
            elif name.endswith("-terminal"):
                label = name.removesuffix("-terminal")
                for stream in ("stdout", "stderr"):
                    if (
                        read_file(root, f"{output}/{label}-{stream}.log")
                        != value[stream].encode()
                    ):
                        raise ValueError(
                            "retained lift stream differs from native output"
                        )
            write_record(root, f"{output}/{name}.json", value)
            artifacts.append(f"{name}.json")

        def stream_gate(label: str, stream: str, chunk: bytes) -> None:
            write_chunk(streams[label, stream], chunk)

        yield publish, stream_gate, artifacts


def retain_failure(root: Path, output: str, error: BaseException) -> None:
    try:
        write_record(
            root,
            output + "/failure.json",
            {
                "error_type": type(error).__name__,
                "cleanup_unconfirmed": isinstance(error, ProcessCleanupError),
                "source_accepted": False,
                "restoration_authorized": False,
                "retry_authorized": False,
            },
        )
    except BaseException:
        if isinstance(error, ProcessCleanupError):
            raise error
        raise
