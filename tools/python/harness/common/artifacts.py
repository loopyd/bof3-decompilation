"""Read bounded artifacts, retain file identities and write captured results."""

from __future__ import annotations

import json
from contextlib import contextmanager
from contextvars import ContextVar
from dataclasses import dataclass
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.failures import PolicyFailure, retain


class IncompleteWrite(OSError):
    """An artifact writer did not write every supplied byte."""


@dataclass(frozen=True)
class FileIdentity:
    path: Path
    bytes: int
    sha256: str

    def as_record(self) -> dict:
        return {"path": str(self.path), "bytes": self.bytes, "sha256": self.sha256}


@dataclass(frozen=True)
class ConsumedFile:
    identity: FileIdentity
    content: bytes


_READER = ContextVar("ordinary_artifact_reader", default=None)


def active_reader():
    return _READER.get()


@contextmanager
def using_reader(reader):
    token = _READER.set(reader)
    try:
        yield
    finally:
        _READER.reset(token)


def physical_read_bytes(
    path: Path, bound: int, events=None, *, read_phase="read", close_phase="close"
) -> bytes:
    """Read at most the declared byte limit without invoking an installed reader."""
    try:
        check_deadline()
        if type(bound) is not int or bound < 0:
            raise PolicyFailure("artifact read limit must be a nonnegative integer")
        if not path.is_file():
            raise PolicyFailure(f"artifact is not a regular file: {path}")
        stream = path.open("rb")
        try:
            try:
                content = stream.read(bound + 1)
            except Exception as error:
                if events is not None:
                    retain(
                        events, error, phase=read_phase, path=path, origin="ordinary"
                    )
                raise
        finally:
            try:
                stream.close()
            except Exception as error:
                if events is not None:
                    retain(
                        events, error, phase=close_phase, path=path, origin="ordinary"
                    )
                raise
        if len(content) > bound:
            raise PolicyFailure(f"artifact exceeds its read limit: {path}")
        check_deadline()
        return content
    except Exception as error:
        if events is not None:
            retain(
                events,
                error,
                phase=read_phase,
                path=path,
                origin="policy" if isinstance(error, PolicyFailure) else "ordinary",
            )
        raise


def read_bytes(
    path: Path, bound: int, events=None, *, read_phase="read", close_phase="close"
) -> bytes:
    reader = active_reader()
    if reader is not None:
        local = []
        try:
            return reader(path, bound, local)
        finally:
            if events is not None:
                events.extend(
                    (
                        origin,
                        {"read": read_phase, "close": close_phase}.get(phase, phase),
                        subject,
                        error,
                    )
                    for origin, phase, subject, error in local
                )
    return physical_read_bytes(
        path, bound, events, read_phase=read_phase, close_phase=close_phase
    )


def encode_json(
    document: object, *, path: object, events=None, phase="serialize"
) -> bytes:
    try:
        return (
            json.dumps(document, indent=2, sort_keys=True, allow_nan=False) + "\n"
        ).encode()
    except Exception as error:
        if events is not None:
            retain(events, error, phase=phase, path=path, origin="ordinary")
        raise


def write_bytes(
    path: Path,
    content: bytes,
    *,
    readonly=False,
    create_parents=True,
    events=None,
    write_phase="write",
) -> None:
    """Create one captured artifact without replacing an existing file."""
    try:
        check_deadline()
        if create_parents:
            path.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
        with path.open("xb") as stream:
            if stream.write(content) != len(content):
                raise IncompleteWrite(f"incomplete artifact write: {path}")
        path.chmod(0o400 if readonly else 0o600)
        check_deadline()
    except Exception as error:
        if events is not None:
            retain(events, error, phase=write_phase, path=path, origin="ordinary")
        raise
