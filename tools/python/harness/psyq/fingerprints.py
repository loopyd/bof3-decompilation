"""Relocation-aware MIPS fingerprints used for PsyQ and duplicate scans."""

from __future__ import annotations

import hashlib
from pathlib import Path
import struct
from typing import Iterable


def _masked(data: bytes, relocation_ranges: Iterable[tuple[int, int]]) -> bytes:
    result = bytearray(data)
    for start, end in relocation_ranges:
        if start < 0 or end < start:
            raise ValueError("invalid relocation range")
        for index in range(start, min(end, len(result))):
            result[index] = 0
    return bytes(result)


def relocation_masked_hash(
    data: bytes, relocation_ranges: Iterable[tuple[int, int]] = ()
) -> str:
    return hashlib.sha256(_masked(data, relocation_ranges)).hexdigest()


def function_fingerprint(
    data: bytes, relocation_ranges: Iterable[tuple[int, int]] = ()
) -> dict[str, object]:
    masked = _masked(data, relocation_ranges)
    words = [
        struct.unpack_from("<I", masked, offset)[0]
        for offset in range(0, len(masked) - 3, 4)
    ]
    return {
        "size": len(data),
        "alignment": len(data) % 4 == 0,
        "relocation_masked": hashlib.sha256(masked).hexdigest(),
        "normalized_instructions": hashlib.sha256(
            b"".join(word.to_bytes(4, "little") for word in words)
        ).hexdigest(),
        "words": len(words),
    }


def scan_payload(
    payload: Path, *, min_size: int = 8, max_windows: int | None = 2048
) -> list[dict[str, object]]:
    """Return aligned word windows for a bounded, review-only scan.

    The extractor can produce thousands of image and audio payloads.  A
    bounded stride keeps the index operation predictable while preserving all
    windows for the small function fixtures used by focused analysis.
    """

    data = payload.read_bytes()
    if len(data) < min_size:
        return []
    window_count = (len(data) - min_size) // 4 + 1
    stride = 4
    if max_windows is not None and max_windows > 0 and window_count > max_windows:
        stride *= (window_count + max_windows - 1) // max_windows
    results = []
    for offset in range(0, len(data) - min_size + 1, stride):
        window = data[offset : offset + min_size]
        if any(window):
            results.append(
                {"offset": offset, "fingerprint": relocation_masked_hash(window)}
            )
    return results
