"""Immutable evidence records and streamed output for native role dispatch."""

from __future__ import annotations

import json
import hashlib
import os
import stat
from pathlib import Path
from typing import Any, BinaryIO

from harness.common.continuation import check_budget
from harness.common.digests import digest
from harness.common.directory import open_parent_fd
from harness.common.files import read_file
from harness.io import unique_object

ARTIFACTS = (
    "request.json",
    "binding.json",
    "prompt.txt",
    "capabilities.json",
    "spawn.json",
    "stdout.jsonl",
    "stderr.log",
    "proposal.md",
)
STREAMS = {"stdout": "stdout.jsonl", "stderr": "stderr.log"}


def seal_record(value: dict[str, Any]) -> dict[str, Any]:
    return {**value, "digest": digest(value)}


def read_record(root: Path, name: str) -> dict[str, Any]:
    value = json.loads(read_file(root, name), object_pairs_hook=unique_object)
    if not isinstance(value, dict):
        raise ValueError("dispatch record must be an object")
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
                raise ValueError("dispatch stream identity drifted")
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
            raise OSError("private dispatch stream write made no progress")
        remaining = remaining[written:]
    os.fsync(stream.fileno())


def write_private(root: Path, name: str, value: bytes) -> None:
    with open_stream(root, name) as stream:
        write_chunk(stream, value)


def hash_artifacts(root: Path, directory: str) -> dict[str, str]:
    return {
        name: hashlib.sha256(read_file(root, f"{directory}/{name}")).hexdigest()
        for name in ARTIFACTS
    }


def debit_dispatch(
    root: Path,
    budget: dict,
    chain: list[dict],
    identity: str,
    expected_result: str | None,
) -> tuple[str, dict]:
    base = f"out/reviews/dispatch/{budget['digest'][3:]}"
    previous = chain[-1]
    before = check_budget(
        root, budget, chain, budget["digest"], previous["digest"], previous["sequence"]
    )
    if before["exhausted"]:
        raise ValueError("original dispatch budget is exhausted")
    sequence = previous["sequence"] + 1
    if previous["sequence"]:
        if not expected_result:
            raise ValueError(
                "prior dispatch completion requires an external result pin"
            )
        status = read_record(root, f"{base}/{previous['sequence']}/result.json")
        if (
            status.get("transport_completed") is not True
            or status.get("digest") != expected_result
            or status.get("checkpoint_digest") != previous["digest"]
            or status.get("digest")
            != digest({key: value for key, value in status.items() if key != "digest"})
        ):
            raise ValueError(
                "previous dispatch is unresolved; parent recovery required"
            )
        if (
            read_file(
                root, f"{base}/{previous['sequence']}/failure.json", missing_ok=True
            )
            is not None
        ):
            raise ValueError("previous dispatch failed; parent recovery required")
        if status.get("artifacts") != hash_artifacts(
            root, f"{base}/{previous['sequence']}"
        ):
            raise ValueError("prior dispatch artifacts drifted")
    elif expected_result is not None:
        raise ValueError("initial dispatch cannot adopt a previous result")
    launches = {**previous["launches"], identity: previous["launches"][identity] + 1}
    charged = seal_record(
        {
            "schema": "bof3.execution-consumption/v1",
            "budget_digest": budget["digest"],
            "sequence": sequence,
            "previous_digest": previous["digest"],
            "launches": launches,
            "repairs": previous["repairs"],
        }
    )
    status = check_budget(
        root, budget, [*chain, charged], budget["digest"], charged["digest"], sequence
    )
    if status["remaining_ns"] <= 0:
        raise ValueError("original dispatch deadline expired before debit")
    directory = f"{base}/{sequence}"
    write_record(root, f"{directory}/consumption.json", charged)
    return directory, charged
