from __future__ import annotations

from contextlib import contextmanager
from contextvars import ContextVar
from dataclasses import dataclass
from pathlib import Path


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
