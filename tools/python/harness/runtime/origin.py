"""Validate declared capture provenance before continuing a saved machine."""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path


def decode_object(pairs: list[tuple[str, object]]) -> dict:
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate origin receipt key: {key}")
        result[key] = value
    return result


def validate_native_bundle(bundle: dict) -> dict:
    if (
        not isinstance(bundle, dict)
        or set(bundle) != {"schema", "resources", "bundle_sha256", "state"}
        or bundle.get("schema") != "psx.mission-bundle/v1"
    ):
        raise ValueError(
            "Builtin runtime resource bundle differs from emulator identity"
        )
    rows = bundle["resources"]
    if not isinstance(rows, list) or len(rows) != 54:
        raise ValueError("Builtin runtime resource census differs")
    names = []
    for row in rows:
        if not isinstance(row, dict) or set(row) != {
            "name",
            "role",
            "path",
            "source_sha256",
            "source_bytes",
            "chunk_sha256",
            "chunk_bytes",
        }:
            raise ValueError("Invalid builtin resource identity row")
        name = row["name"]
        if (
            not isinstance(name, str)
            or re.fullmatch(r"[a-z][a-z0-9_]{0,63}", name) is None
            or row["role"] not in {"mission", "module"}
            or row["path"] != f"src/lua/missions/{name}.lua"
        ):
            raise ValueError("Invalid builtin resource name, role or path")
        for key in ("source_sha256", "chunk_sha256"):
            if (
                not isinstance(row[key], str)
                or re.fullmatch(r"[0-9a-f]{64}", row[key]) is None
            ):
                raise ValueError("Invalid builtin resource hash")
        for key in ("source_bytes", "chunk_bytes"):
            if type(row[key]) is not int or not 0 < row[key] <= 1024 * 1024 + 64:
                raise ValueError("Invalid builtin resource byte bound")
        names.append(name)
    if (
        names != sorted(set(names))
        or sum(row["role"] == "mission" for row in rows) != 19
    ):
        raise ValueError("Builtin resource ordering or roles differ")
    data = b"".join(
        (
            "\0".join(
                row[key]
                for key in ("name", "role", "path", "source_sha256", "chunk_sha256")
            )
            + "\n"
        ).encode()
        for row in rows
    )
    if hashlib.sha256(data).hexdigest() != bundle["bundle_sha256"]:
        raise ValueError("Builtin bundle digest differs")
    state = bundle["state"]
    if (
        not isinstance(state, dict)
        or set(state) != {"version", "version_string", "schema_sha256", "schema_bytes"}
        or state["version"] != 6
        or state["version_string"] != "PCSX-Redux Agentic SaveState v6"
        or not isinstance(state["schema_sha256"], str)
        or re.fullmatch(r"[0-9a-f]{64}", state["schema_sha256"]) is None
        or type(state["schema_bytes"]) is not int
        or not 0 < state["schema_bytes"] <= 1024 * 1024
    ):
        raise ValueError("Builtin native state identity differs")
    return bundle


def validate_selection(receipt: dict) -> dict:
    schema = receipt.get("schema")
    inputs = receipt.get("inputs")
    if not isinstance(inputs, dict):
        raise ValueError(
            "malformed origin receipt: runtime selection has no owning input mapping"
        )
    if schema == "psx.runtime-session/v1":
        if "mission" in receipt or "script" not in inputs:
            raise ValueError("Historical runtime receipt shape differs")
        return {
            "kind": "external",
            "script": inputs["script"],
            "modules": {
                key: row for key, row in inputs.items() if key.startswith("module:")
            },
        }
    if schema != "psx.runtime-session/v2":
        raise ValueError("Unsupported runtime session schema")
    bundle = validate_native_bundle(receipt.get("emulator", {}).get("resources"))
    mission = receipt.get("mission")
    if not isinstance(mission, dict):
        raise ValueError("Runtime mission discriminant is missing")
    if mission.get("kind") == "external":
        expected = {
            "kind": "external",
            "script": inputs.get("script"),
            "modules": {
                key: row for key, row in inputs.items() if key.startswith("module:")
            },
        }
        if "script" not in inputs or mission != expected:
            raise ValueError("External runtime mission identities differ")
        canonical = {
            row["name"] for row in bundle["resources"] if row["role"] == "module"
        }
        if any(key.removeprefix("module:") in canonical for key in mission["modules"]):
            raise ValueError(
                "External runtime module shadows a canonical native module"
            )
        return mission
    if (
        mission.get("kind") != "builtin"
        or set(mission) != {"kind", "name", "resources"}
        or "script" in inputs
        or any(key.startswith("module:") for key in inputs)
    ):
        raise ValueError("Builtin runtime mission shape differs")
    if mission["resources"] != bundle:
        raise ValueError("Builtin runtime bundle differs from emulator identity")
    if not any(
        row["name"] == mission["name"] and row["role"] == "mission"
        for row in bundle["resources"]
    ):
        raise ValueError("Builtin entry is not a unique mission")
    return mission


def verify_origin(path: Path, inputs: dict, emulator: dict, *, reader=None) -> dict:
    """Bind state bytes and tool/media identities, not receipt authenticity."""
    if "file:STATE" not in inputs:
        raise ValueError("--origin requires --input state=PATH")
    if "executable" in inputs:
        raise ValueError("--origin continuation cannot also load an executable")
    if reader is None:
        with path.open("rb") as source:
            raw = source.read(2 * 1024 * 1024 + 1)
    else:
        raw = reader(path, 2 * 1024 * 1024, None)
    if len(raw) > 2 * 1024 * 1024:
        raise ValueError("origin receipt exceeds 2 MiB")
    try:
        origin = json.loads(raw, object_pairs_hook=decode_object)
        if (
            origin["schema"] not in {"psx.runtime-session/v1", "psx.runtime-session/v2"}
            or origin["status"] != "passed"
            or origin["completion"]["schema"] != "psx.runtime-completion/v1"
        ):
            raise ValueError("origin must be a successful runtime capture receipt")
        validate_selection(origin)
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
