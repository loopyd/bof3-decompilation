from __future__ import annotations

import hashlib
import struct
from pathlib import Path


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def parse_psx_exe(path: Path) -> dict[str, int]:
    data = path.read_bytes()
    if len(data) < 0x20 or data[:8] != b"PS-X EXE":
        raise ValueError(f"not a PS-X EXE: {path}")
    return {
        "pc0": struct.unpack_from("<I", data, 0x10)[0],
        "text_addr": struct.unpack_from("<I", data, 0x18)[0],
        "text_size": struct.unpack_from("<I", data, 0x1C)[0],
    }
