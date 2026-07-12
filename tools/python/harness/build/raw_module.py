"""Assemble a raw overlay image from independently compiled function objects.

Only compiled object text is placed in the image.  Callers receive placements
so that uncovered bytes can be reported rather than silently copied from the
original module.
"""

from __future__ import annotations

import re
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path


OBJECT_ADDRESS_RE = re.compile(r"func_([0-9a-fA-F]{8})\.c\.obj$")


@dataclass(frozen=True)
class ObjectPlacement:
    address: int
    size: int
    original_size: int
    truncated: bool
    object_path: Path


def function_address_from_object(path: Path) -> int:
    match = OBJECT_ADDRESS_RE.search(path.name)
    if match is None:
        raise ValueError(f"object name does not include a function address: {path}")
    return int(match.group(1), 16)


def extract_text(objcopy: Path, object_path: Path) -> bytes:
    with tempfile.TemporaryDirectory(prefix="harness-object-") as temporary:
        output = Path(temporary) / "text.bin"
        subprocess.run(
            [
                str(objcopy),
                "-O",
                "binary",
                "-j",
                ".text",
                str(object_path),
                str(output),
            ],
            check=True,
        )
        return output.read_bytes()


def build_raw_image(
    *,
    objcopy: Path,
    objects: list[Path],
    base_address: int,
    output_size: int,
    truncate_overlaps: bool = False,
) -> tuple[bytes, list[ObjectPlacement]]:
    if output_size < 0:
        raise ValueError("output size must not be negative")
    rows = sorted((function_address_from_object(path), path) for path in objects)
    image = bytearray(output_size)
    placements: list[ObjectPlacement] = []
    for index, (address, object_path) in enumerate(rows):
        offset = address - base_address
        if offset < 0 or offset >= output_size:
            raise ValueError(f"object is outside target image: {object_path}")
        text = extract_text(objcopy, object_path)
        original_size = len(text)
        next_offset = output_size
        if index + 1 < len(rows):
            next_offset = rows[index + 1][0] - base_address
        allowed = min(output_size, next_offset) - offset
        truncated = original_size > allowed
        if truncated and not truncate_overlaps:
            raise ValueError(f"object overlaps following function: {object_path}")
        size = min(original_size, allowed)
        image[offset : offset + size] = text[:size]
        placements.append(
            ObjectPlacement(address, size, original_size, truncated, object_path)
        )
    return bytes(image), placements
