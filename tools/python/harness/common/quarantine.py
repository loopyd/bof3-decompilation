"""Reserved destinations for retaining displaced transaction files."""

from __future__ import annotations

import hashlib
import os
import re
import secrets

QUARANTINE_DIRECTORY = "out/reviews/evidence/quarantine"


def matches_identity(current: os.stat_result, expected: os.stat_result) -> bool:
    return (current.st_dev, current.st_ino, current.st_mode) == (
        expected.st_dev,
        expected.st_ino,
        expected.st_mode,
    )


def reserve_quarantine(name: str) -> str:
    fingerprint = hashlib.sha256(name.encode("utf-8")).hexdigest()[:16]
    return f"{QUARANTINE_DIRECTORY}/{secrets.token_hex(16)}-{fingerprint}"


def validate_quarantine(name: str, destination: str) -> str:
    fingerprint = hashlib.sha256(name.encode("utf-8")).hexdigest()[:16]
    pattern = rf"{QUARANTINE_DIRECTORY}/[0-9a-f]{{32}}-{fingerprint}"
    if not isinstance(destination, str) or re.fullmatch(pattern, destination) is None:
        raise ValueError("invalid reserved quarantine destination")
    return destination
