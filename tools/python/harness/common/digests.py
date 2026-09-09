"""Canonical structured evidence digests."""

from __future__ import annotations

import hashlib
import json


def digest(value: object) -> str:
    encoded = json.dumps(value, sort_keys=True, separators=(",", ":"))
    return "v1:" + hashlib.sha256(encoded.encode()).hexdigest()
