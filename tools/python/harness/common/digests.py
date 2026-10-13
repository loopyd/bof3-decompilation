"""Compute structured digests and streaming file identities."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

from harness.common.artifacts import active_reader


def digest(value: object) -> str:
    encoded = json.dumps(value, sort_keys=True, separators=(",", ":"))
    return "v1:" + hashlib.sha256(encoded.encode()).hexdigest()


def file_record(path: Path, *, reader=None, events=None) -> dict:
    path = Path(path).absolute()
    reader = reader if reader is not None else active_reader()
    if reader is not None:
        data = reader(path, path.stat().st_size, events)
        return {
            "path": str(path),
            "bytes": len(data),
            "sha256": hashlib.sha256(data).hexdigest(),
        }
    digest = hashlib.sha256()
    count = 0
    with path.open("rb") as stream:
        while block := stream.read(1 << 20):
            count += len(block)
            digest.update(block)
    return {"path": str(path), "bytes": count, "sha256": digest.hexdigest()}
