"""Validate declared capture provenance before continuing a saved machine."""

from __future__ import annotations

import json
from pathlib import Path


def decode_object(pairs: list[tuple[str, object]]) -> dict:
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate origin receipt key: {key}")
        result[key] = value
    return result


def verify_origin(path: Path, inputs: dict, emulator: dict) -> dict:
    """Bind state bytes and tool/media identities, not receipt authenticity."""
    if "file:STATE" not in inputs:
        raise ValueError("--origin requires --input state=PATH")
    if "executable" in inputs:
        raise ValueError("--origin continuation cannot also load an executable")
    with path.open("rb") as source:
        raw = source.read(2 * 1024 * 1024 + 1)
    if len(raw) > 2 * 1024 * 1024:
        raise ValueError("origin receipt exceeds 2 MiB")
    try:
        origin = json.loads(raw, object_pairs_hook=decode_object)
        if (
            origin["schema"] != "psx.runtime-session/v1"
            or origin["status"] != "passed"
            or origin["completion"]["schema"] != "psx.runtime-completion/v1"
        ):
            raise ValueError("origin must be a successful runtime capture receipt")
        state = inputs["file:STATE"]
        matches = [
            row
            for row in origin["completion"]["captures"]
            if row["sha256"] == state["sha256"] and row["bytes"] == state["bytes"]
        ]
        if not matches:
            raise ValueError("state bytes do not match an origin capture")
        capture_names = [row["path"] for row in matches]
        if any(not isinstance(name, str) or not name for name in capture_names):
            raise ValueError("malformed origin capture name")
        for key in ("revision", "binary_sha256", "sdl_sha256"):
            if origin["emulator"][key] != emulator[key]:
                raise ValueError(f"origin emulator identity mismatch: {key}")
        expected = {
            key: (value["sha256"], value["bytes"])
            for key, value in origin["inputs"].items()
            if key == "bios" or key.startswith("disc:")
        }
        supplied = {
            key: (value["sha256"], value["bytes"])
            for key, value in inputs.items()
            if key == "bios" or key.startswith("disc:")
        }
        if expected != supplied:
            raise ValueError("origin BIOS/media identities differ from supplied inputs")
    except (KeyError, TypeError, AttributeError, RecursionError, UnicodeError) as error:
        raise ValueError("malformed origin receipt") from error
    return {
        "schema": "psx.runtime-origin/v1",
        "state_sha256": state["sha256"],
        "captures": capture_names,
        "verified": "capture bytes, emulator, SDL, bootstrap BIOS and declared media",
        "limitations": "User-supplied provenance is not authenticated; guest ROM and hidden host state are not inferred.",
    }
