"""PS-X EXE executable-byte facts: loader header, payload offset, reviewed ranges.

The PS-X EXE loader header is 0x800 bytes: the payload that maps at
``load_address`` is the on-disk image minus that offset.  Raw (non-PS-X)
binaries map their full size.  This module is the single authority for the
magic, the 0x800 offset, header validation, and reviewed-range byte
extraction/hashing so analysis and command layers never re-derive them.
"""

from __future__ import annotations

import hashlib
import struct
from dataclasses import dataclass

PSX_EXE_MAGIC = b"PS-X EXE"
PSX_EXE_HEADER_SIZE = 0x800


def is_psx_exe(data: bytes) -> bool:
    """True when the image carries a PS-X EXE loader header."""

    return data[:8] == PSX_EXE_MAGIC


def binary_offset_for(data: bytes) -> int:
    """Payload offset within the on-disk image (0x800 for PS-X EXE, else 0)."""

    return PSX_EXE_HEADER_SIZE if is_psx_exe(data) else 0


def validate_psx_header(data: bytes, load_address: int, *, binary_name: str) -> None:
    """Reject a truncated or manifest-inconsistent PS-X EXE header."""

    if len(data) < PSX_EXE_HEADER_SIZE:
        raise ValueError(f"truncated PS-X EXE header: {binary_name}")
    header_address, header_size = struct.unpack_from("<II", data, 0x18)
    if header_address != load_address:
        raise ValueError(
            f"PS-X EXE t_addr 0x{header_address:08X} != manifest load_address "
            f"0x{load_address:08X}: {binary_name}"
        )
    if header_size != len(data) - PSX_EXE_HEADER_SIZE:
        raise ValueError(
            f"PS-X EXE t_size 0x{header_size:X} != payload size "
            f"0x{len(data) - PSX_EXE_HEADER_SIZE:X}: {binary_name}"
        )


@dataclass(frozen=True)
class PsxPayload:
    """Payload bounds for one target image, accounting for the 0x800 offset."""

    load_address: int
    binary_offset: int
    payload_size: int

    @property
    def payload_end(self) -> int:
        return self.load_address + self.payload_size


def payload_for(data: bytes, load_address: int, *, binary_name: str) -> PsxPayload:
    """Return payload bounds for one image; a zero/empty payload fails."""

    offset = binary_offset_for(data)
    payload_size = len(data) - offset
    if payload_size <= 0:
        raise ValueError(f"target payload is empty: {binary_name}")
    return PsxPayload(load_address, offset, payload_size)


def reviewed_range_digest(
    payload: PsxPayload, start: int, end: int | None, *, binary: bytes
) -> tuple[str, int] | None:
    """Reviewed (sha256, size) over a finite, payload-contained Splat range.

    Returns ``None`` when the range cannot be hashed from the image: an open
    end (``end is None``), or an end that escapes the payload.  Accounts for
    the PS-X 0x800 loader offset when slicing the on-disk image.
    """

    if end is None:
        return None
    if not (payload.load_address <= start < end <= payload.payload_end):
        return None
    lo = payload.binary_offset + (start - payload.load_address)
    hi = payload.binary_offset + (end - payload.load_address)
    return hashlib.sha256(binary[lo:hi]).hexdigest(), end - start


__all__ = [
    "PSX_EXE_HEADER_SIZE",
    "PSX_EXE_MAGIC",
    "PsxPayload",
    "binary_offset_for",
    "is_psx_exe",
    "payload_for",
    "reviewed_range_digest",
    "validate_psx_header",
]
