"""Frozen reconstruction pins, scalar constraints and refusal semantics."""

from __future__ import annotations

import hashlib
import json
import re
from collections.abc import Callable
from contextlib import contextmanager
from contextvars import ContextVar
from datetime import datetime, timezone
from pathlib import Path
from typing import Literal

from harness.common.failures import FailureEvent, retain
from harness.common.artifacts import read_bytes as read_bounded_bytes
from harness.common.digests import digest
from harness.io import unique_object


class PreparationRefused(RuntimeError):
    """A frozen input or an output destination failed a preparation predicate."""


_READ_BOUND: ContextVar[int | None] = ContextVar(
    "reconstruction_read_bound", default=None
)
_READ_OBSERVATIONS: ContextVar[list[dict] | None] = ContextVar(
    "reconstruction_read_observations", default=None
)
_READER: ContextVar[Callable[[Path, int, list[FailureEvent] | None], bytes] | None] = (
    ContextVar("reconstruction_snapshot_reader", default=None)
)


@contextmanager
def bounded_reads(
    max_bytes: int | None,
    *,
    observations: list[dict] | None = None,
    reader: Callable[[Path, int, list[FailureEvent] | None], bytes] | None = None,
):
    require(
        max_bytes is None or type(max_bytes) is int and max_bytes > 0,
        "input read bound must be a positive integer",
    )
    inherited = _READ_BOUND.get()
    bounds = [value for value in (inherited, max_bytes) if value is not None]
    token = _READ_BOUND.set(min(bounds) if bounds else None)
    reads = observations if observations is not None else _READ_OBSERVATIONS.get()
    if reads is None and bounds:
        reads = []
    observed = _READ_OBSERVATIONS.set(reads)
    selected_reader = _READER.set(reader if reader is not None else _READER.get())
    try:
        yield reads
    finally:
        _READER.reset(selected_reader)
        _READ_OBSERVATIONS.reset(observed)
        _READ_BOUND.reset(token)


BASELINE_SCHEMA = "retdec118.corpus/v1"
PINNED_BASELINE_SHA256 = (
    "eedcf95bda348eab5d5467d7e8aa165909c35f9a823e6babe1180594c05a1b00"
)
DEFAULT_BASELINE_PATH = "tools/retdec-psx/corpus.json"

BUNDLE_SCHEMA = "retdec118.bundle/v1"
PROBLEM_SCHEMA = "retdec118.problem/v1"
ANSWER_SCHEMA = "retdec118.answer/v1"
PREPARATION_SCHEMA = "retdec118.preparation/v1"
RUNTIME_QUESTION_SCHEMA = "retdec118.runtime-question/v2"
COMMAND_SCHEMA = "retdec118.command/v1"

MILESTONE_ID = "retdec-118"
ADMITTED = 118
EXCLUSIONS = 7
DEFAULT_RUN_ROOT = "out/retdec-118-reconstruction"

RULES_ID = "retdec118-generic"
RULES_PATH = "rules.json"
RULES_BYTES = b"[]"

ROLE_ORIGIN = {
    "problem": "approved",
    "answer": "evaluator",
    "preparation": "control",
}

RUN_ID = re.compile(r"^[a-z0-9][a-z0-9._-]{0,63}$")
IDENT = re.compile(r"^[a-z][a-z0-9-]{0,31}$")
TOKEN = re.compile(r"^[A-Za-z_][A-Za-z0-9_]{0,63}$")
IMAGE_ID = re.compile(r"^img/[0-9a-f]{12}$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")
TOOL_ID = re.compile(r"^[a-z0-9][a-z0-9._-]{0,63}$")
REVISION = re.compile(r"^[0-9A-Za-z][0-9A-Za-z._+-]{0,63}$")
DOC_DIGEST = re.compile(r"^v1:[0-9a-f]{64}$")

_UNSAFE_REL_PARTS = {"", ".", ".."}


def require(condition: object, message: str) -> None:
    """Refuse preparation when a required frozen predicate is false."""

    if not condition:
        raise PreparationRefused(message)


def timestamp() -> str:
    """Whole-second RFC3339 UTC instant used for bundle sealing."""

    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def doc_digest(document: dict) -> str:
    """``DocDigest`` = canonical-JSON digest of the whole parsed document."""

    return digest(document)


def bundle_digest(index: dict) -> str:
    """``BundleDigest`` = canonical-JSON digest of the index minus its own field."""

    return digest(
        {key: value for key, value in index.items() if key != "bundle_digest"}
    )


def read_bytes(
    path: Path,
    subject: str,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> bytes:
    """Read one input snapshot; the returned bytes are the only ones used."""

    require(io_mode in {"normalized", "exact"}, "unsupported input observation mode")
    require(
        max_bytes is None or type(max_bytes) is int and max_bytes > 0,
        "input read bound must be a positive integer",
    )
    limits = [value for value in (max_bytes, _READ_BOUND.get()) if value is not None]
    if limits:
        limit = min(limits)
        reader = _READER.get()
        try:
            data = (
                read_bounded_bytes(path, limit, events)
                if reader is None
                else reader(path, limit, events)
            )
        except OSError as error:
            if io_mode == "exact":
                raise
            raise PreparationRefused(
                f"{subject} is unreadable: {path} ({error})"
            ) from error
        observations = _READ_OBSERVATIONS.get()
        if observations is not None and reader is None:
            observations.append(
                dict(path=str(path), bytes=len(data), sha256=sha256_bytes(data))
            )
        require(len(data) <= limit, f"{subject} exceeds its input read bound")
        return data
    if io_mode == "exact":
        local, stream, data = [], None, None
        try:
            stream = path.open("rb")
            data = stream.read()
        except Exception as error:
            retain(local, error, phase=subject, path=path, origin="ordinary")
        if stream is not None:
            try:
                stream.close()
            except Exception as error:
                retain(
                    local, error, phase=subject + "-close", path=path, origin="ordinary"
                )
        if events is not None:
            events.extend(local)
        if local:
            raise local[0][3]
        return data
    try:
        return path.read_bytes()
    except OSError as error:
        raise PreparationRefused(
            f"{subject} is unreadable: {path} ({error})"
        ) from error


def parse_json(data: bytes, subject: str) -> object:
    """Parse an already-verified snapshot; callers never re-read the path."""

    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError as error:
        raise PreparationRefused(f"{subject} is not UTF-8: {error}") from error
    try:
        return json.loads(text, object_pairs_hook=unique_object)
    except ValueError as error:
        raise PreparationRefused(f"{subject} is not valid JSON: {error}") from error


def validate_run_id(value: object) -> str:
    require(
        isinstance(value, str) and RUN_ID.fullmatch(value) is not None,
        f"run id must match {RUN_ID.pattern}: {value!r}",
    )
    return str(value)


def validate_rel_path(value: object) -> str:
    """One bundle-relative POSIX path: relative, no traversal, no drive colon."""

    require(isinstance(value, str), f"bundle path must be text: {value!r}")
    path = str(value)
    require(
        len(path) <= 255
        and path == path.strip()
        and "\\" not in path
        and "\0" not in path,
        f"bundle path is unsafe: {value!r}",
    )
    require(
        not path.startswith("/") and ":" not in path,
        f"bundle path is absolute: {value!r}",
    )
    parts = path.split("/")
    require(
        all(part not in _UNSAFE_REL_PARTS for part in parts),
        f"bundle path has an unsafe component: {value!r}",
    )
    return path


def validate_ident(value: object, subject: str) -> str:
    require(
        isinstance(value, str) and IDENT.fullmatch(value) is not None,
        f"{subject} must match {IDENT.pattern}: {value!r}",
    )
    return str(value)


def validate_tool_id(value: object, subject: str) -> str:
    require(
        isinstance(value, str) and TOOL_ID.fullmatch(value) is not None,
        f"{subject} must match {TOOL_ID.pattern}: {value!r}",
    )
    return str(value)


def validate_sha256(value: object, subject: str) -> str:
    require(
        isinstance(value, str) and SHA256.fullmatch(value) is not None,
        f"{subject} must be a lowercase sha256: {value!r}",
    )
    return str(value)


def read_verified(path: Path, expected: object, subject: str) -> bytes:
    """Return the exact bytes whose digest equals the recorded pin.

    The caller parses or retains this same snapshot, so a replacement between a
    hash check and a later read can never change what is consumed.
    """

    data = read_bytes(path, subject)
    observed = sha256_bytes(data)
    require(
        observed == expected,
        f"{subject} changed: {path} expected {expected} observed {observed}",
    )
    return data


def file_ref(
    bundle: str, path: str, data: bytes, expected_bundle: str | None = None
) -> dict:

    validate_rel_path(path)
    return {
        "bundle": validate_rel_path(bundle),
        "path": path,
        "sha256": sha256_bytes(data),
        "bytes": len(data),
        "expected_bundle": expected_bundle,
    }


def doc_ref(
    bundle: str, document_digest: str, expected_bundle: str | None = None
) -> dict:
    require(
        isinstance(document_digest, str)
        and DOC_DIGEST.fullmatch(document_digest) is not None,
        f"doc reference digest must be a DocDigest: {document_digest!r}",
    )
    return {
        "bundle": validate_rel_path(bundle),
        "digest": document_digest,
        "expected_bundle": expected_bundle,
    }


__all__ = [
    "ADMITTED",
    "ANSWER_SCHEMA",
    "BASELINE_SCHEMA",
    "BUNDLE_SCHEMA",
    "COMMAND_SCHEMA",
    "DEFAULT_RUN_ROOT",
    "EXCLUSIONS",
    "MILESTONE_ID",
    "PINNED_BASELINE_SHA256",
    "PREPARATION_SCHEMA",
    "PROBLEM_SCHEMA",
    "PreparationRefused",
    "ROLE_ORIGIN",
    "RULES_BYTES",
    "RULES_ID",
    "RULES_PATH",
    "RUNTIME_QUESTION_SCHEMA",
    "bundle_digest",
    "doc_digest",
    "doc_ref",
    "file_ref",
    "parse_json",
    "read_bytes",
    "read_verified",
    "require",
    "sha256_bytes",
    "timestamp",
    "validate_ident",
    "validate_rel_path",
    "validate_run_id",
    "validate_sha256",
    "validate_tool_id",
]
