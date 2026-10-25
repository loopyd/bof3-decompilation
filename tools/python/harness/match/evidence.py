"""Path-independent identities for complete native function comparison payloads."""

from __future__ import annotations

import os
import re
from pathlib import Path

from harness.common.digests import digest

_SHA256 = re.compile(r"[0-9a-f]{64}\Z")


def comparison_identity(payload: dict, root: Path) -> dict:
    """Normalize output locations only; preserve all semantic comparison fields."""
    image = payload.get("function_image")
    if (
        not isinstance(image, dict)
        or set(image) != {"original_sha256", "current_sha256"}
        or any(
            not isinstance(v, str) or not _SHA256.fullmatch(v) for v in image.values()
        )
    ):
        raise ValueError(
            "comparison requires original and current function-image hashes"
        )
    result = dict(payload)
    outputs = result.pop("outputs")
    if not isinstance(outputs, dict) or any(
        not isinstance(v, str) or not v for v in outputs.values()
    ):
        raise ValueError("invalid comparison output paths")
    # Output membership is part of the schema even though locations are not.
    result["output_kinds"] = sorted(outputs)
    for key in ("source", "original_binary", "current_object"):
        path = Path(result[key])
        if not path.is_absolute():
            path = root / path
        if path != Path(os.path.normpath(path)):
            raise ValueError("noncanonical comparison identity path")
        result[key] = path.relative_to(root).as_posix()
    return result


def comparison_digest(payload: dict, root: Path) -> str:
    return digest(comparison_identity(payload, root))
