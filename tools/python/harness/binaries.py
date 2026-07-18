"""Executable-image normalization and identity checks."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path
from typing import Any

from .discovery import file_sha256, parse_psx_exe
from .io import write_json


PSX_EXE_HEADER_SIZE = 0x800


def parse_number(value: str) -> int:
    return int(value, 0)


def normalize_executable(source: Path, destination: Path) -> dict[str, Any]:
    """Extract the PS-X EXE load image using its reviewed header identity."""

    header = parse_psx_exe(source)
    data = source.read_bytes()
    image_size = header["text_size"]
    image = data[PSX_EXE_HEADER_SIZE : PSX_EXE_HEADER_SIZE + image_size]
    if len(image) != image_size:
        raise ValueError(f"truncated PS-X EXE load image: {source}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(image)
    metadata = {
        "schema": "harness.normalized-exe/v1",
        "source": str(source),
        "source_sha256": file_sha256(source),
        "image": str(destination),
        "image_sha256": hashlib.sha256(image).hexdigest(),
        "header_size": PSX_EXE_HEADER_SIZE,
        "pc0": header["pc0"],
        "load_address": header["text_addr"],
        "image_size": image_size,
    }
    write_json(destination.with_suffix(destination.suffix + ".json"), metadata)
    return metadata


def verify_splat_hash(config_path: Path, image_path: Path) -> None:
    """Reject a normalized image that differs from its reviewed Splat identity."""

    digest = hashlib.sha1(image_path.read_bytes()).hexdigest()
    match = re.search(
        r"^sha1:\s*([0-9a-f]{40})$",
        config_path.read_text(encoding="utf-8"),
        flags=re.M,
    )
    if match is None:
        raise ValueError(f"missing sha1 field in Splat config: {config_path}")
    if match.group(1) != digest:
        raise ValueError(
            f"hash mismatch in {config_path}: tracked={match.group(1)} image={digest}"
        )


__all__ = ["normalize_executable", "parse_number", "verify_splat_hash"]
