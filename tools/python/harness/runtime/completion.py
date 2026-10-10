from __future__ import annotations

import json
from pathlib import Path

from harness.io import file_sha256

COMPLETION_SCHEMA = "psx.runtime-completion/v1"
MAX_MARKER_BYTES = 65536
MAX_CAPTURE_FILES = 128
MAX_CAPTURE_BYTES = 64 * 1024 * 1024
MAX_TOTAL_CAPTURE_BYTES = 128 * 1024 * 1024
RESERVED_CAPTURE_NAMES = frozenset(
    {"completion.json", "receipt.json", "mission.lua", "stdout.log", "stderr.log"}
)


def validate_completion(output: Path, *, reader=None) -> dict:

    marker = output / "completion.json"
    if (
        marker.is_symlink()
        or not marker.is_file()
        or marker.stat().st_size > MAX_MARKER_BYTES
    ):
        raise ValueError(
            "runtime requires a regular completion.json (at most 65536 bytes)"
        )
    if reader is None:
        completion = json.loads(marker.read_text(encoding="utf-8"))
    else:
        from harness.runtime.origin import decode_object

        completion = json.loads(
            reader(marker, MAX_MARKER_BYTES, None), object_pairs_hook=decode_object
        )
    if (
        not isinstance(completion, dict)
        or set(completion) != {"schema", "captures"}
        or completion["schema"] != COMPLETION_SCHEMA
    ):
        raise ValueError("invalid runtime completion schema")
    captures = completion["captures"]
    if not isinstance(captures, list) or len(captures) > MAX_CAPTURE_FILES:
        raise ValueError("runtime captures must be a list of at most 128 files")
    seen = set(RESERVED_CAPTURE_NAMES)
    verified = []
    total = 0
    for item in captures:
        if not isinstance(item, dict) or set(item) != {"path", "bytes"}:
            raise ValueError("invalid runtime capture descriptor")
        name, size = item["path"], item["bytes"]
        if (
            not isinstance(name, str)
            or not name
            or Path(name).name != name
            or name in seen
            or name in {".", ".."}
        ):
            raise ValueError(
                "runtime capture paths must be unique, unreserved leaf names"
            )
        if type(size) is not int or not 0 <= size <= MAX_CAPTURE_BYTES:
            raise ValueError("runtime capture size must be 0..67108864")
        total += size
        if total > MAX_TOTAL_CAPTURE_BYTES:
            raise ValueError("runtime captures exceed 128 MiB")
        path = output / name
        if path.is_symlink() or not path.is_file() or path.stat().st_size != size:
            raise ValueError(f"missing or invalid runtime capture: {name}")
        seen.add(name)
        if reader is None:
            sha256 = file_sha256(path)
        else:
            import hashlib

            content = reader(path, size, None)
            if len(content) != size:
                raise ValueError("runtime capture changed while consumed")
            sha256 = hashlib.sha256(content).hexdigest()
        verified.append({"path": name, "bytes": size, "sha256": sha256})
    return {"schema": completion["schema"], "captures": verified}


__all__ = [
    "COMPLETION_SCHEMA",
    "MAX_CAPTURE_BYTES",
    "MAX_CAPTURE_FILES",
    "MAX_MARKER_BYTES",
    "MAX_TOTAL_CAPTURE_BYTES",
    "RESERVED_CAPTURE_NAMES",
    "validate_completion",
]
