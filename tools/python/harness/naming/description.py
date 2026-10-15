"""Capture original data ranges and validate native naming descriptions."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path
from typing import Any

from harness.analysis.project import prepare_target
from harness.domain.ids import parse_function_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests

_HEX = re.compile(r"0x[0-9A-F]+")
_STORAGE_KINDS = {"data", "bss", "rodata", "unknown"}
_STORAGE_AUTHORITY = {"reviewed_splat", "original_binary"}
_SYMBOL_KINDS = {"data", "function"}


def _initializer_prefix(layout, address: int, start: int, raw: bytes) -> dict:
    """Observe original function-address words without guessing object boundaries."""
    words = []
    stop = "unaligned selector"
    if address % 4 == 0:
        stop = "16-word inspection cap"
        for index in range(16):
            offset = address - start + index * 4
            if offset + 4 > len(raw):
                stop = "containing segment boundary"
                break
            pointer = int.from_bytes(raw[offset : offset + 4], "little")
            matches = [
                b
                for b in layout.boundaries
                if b.virtual_start <= pointer
                and (b.virtual_end is None or pointer < b.virtual_end)
            ]
            if (
                len(matches) != 1
                or not matches[0].is_function
                or matches[0].virtual_start != pointer
            ):
                stop = "word is not one unambiguous reviewed function start"
                break
            boundary = matches[0]
            end = boundary.virtual_end
            if pointer % 4 or end is None or end % 4 or end <= pointer:
                stop = "reviewed function range is incomplete or unaligned"
                break
            words.append(
                {
                    "address": address + index * 4,
                    "value": pointer,
                    "function_end": end,
                    "kind": boundary.kind,
                }
            )
    return {"words": words, "stop": stop}


def capture_data_range(root: Path, selector: str) -> dict:
    """Read the whole containing reviewed segment, without inferring object extent."""
    selected = parse_function_id(selector)
    result = {
        "selector": str(selected),
        "binding": None,
        "bytes": None,
        "initializer": None,
        "gap": None,
    }
    try:
        manifest = load_target_manifests(root)[selected.target.value]
        layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
        boundaries = [
            boundary
            for boundary in layout.boundaries
            if boundary.virtual_start <= selected.address
            and (
                boundary.virtual_end is None or selected.address < boundary.virtual_end
            )
        ]
        if len(boundaries) != 1:
            raise ValueError("data range requires one containing reviewed segment")
        boundary = boundaries[0]
        if boundary.kind not in {"data", "rodata"} or boundary.virtual_end is None:
            raise ValueError("data range requires finite mapped data or rodata")
        start, end = boundary.virtual_start, boundary.virtual_end
        if not 0 < end - start <= 65536:
            raise ValueError("data range is empty or exceeds 64 KiB")
        spec = prepare_target(root, selected.target.value, manifest=manifest)
        image = spec.binary.read_bytes()
        offset = spec.binary_offset + start - spec.load_address
        if offset < spec.binary_offset or offset + end - start > len(image):
            raise ValueError("data range escapes original payload")
        result["binding"] = {
            "scope": "containing_reviewed_segment",
            "kind": boundary.kind,
            "start": start,
            "end": end,
            "offset": offset,
            "image_sha256": hashlib.sha256(image).hexdigest(),
            "splat_sha256": layout.sha256,
            "replay_sha256": spec.replay_sha256,
        }
        raw = image[offset : offset + end - start]
        result["bytes"] = raw.hex()
        result["initializer"] = _initializer_prefix(
            layout, selected.address, start, raw
        )
    except ValueError as error:
        result["gap"] = str(error)
    return result


def data_range_value(operation: dict, target: str, address: int) -> dict:
    """Validate segment ownership and its bounded original function-address prefix."""
    payload = operation.get("payload")
    selected = parse_function_id(str(operation["selector"]))
    if (
        not isinstance(payload, dict)
        or set(payload) != {"selector", "binding", "bytes", "initializer", "gap"}
        or payload["selector"] != str(selected)
        or selected.target.value != target
        or selected.address != address
    ):
        raise ValueError("data range capture identity or shape is invalid")
    if payload["gap"] is not None:
        if (
            not isinstance(payload["gap"], str)
            or not payload["gap"].strip()
            or payload["binding"] is not None
            or payload["bytes"] is not None
            or payload["initializer"] is not None
        ):
            raise ValueError("data range gap shape is invalid")
        raise ValueError(payload["gap"])
    binding = payload["binding"]
    if (
        not isinstance(binding, dict)
        or set(binding)
        != {
            "scope",
            "kind",
            "start",
            "end",
            "offset",
            "image_sha256",
            "splat_sha256",
            "replay_sha256",
        }
        or binding["scope"] != "containing_reviewed_segment"
        or binding["kind"] not in {"data", "rodata"}
        or any(type(binding[key]) is not int for key in ("start", "end", "offset"))
        or not 0 <= binding["start"] <= address < binding["end"] <= 0x100000000
        or not 0 < binding["end"] - binding["start"] <= 65536
        or binding["offset"] < 0
        or any(
            not isinstance(binding[key], str)
            or re.fullmatch(r"[0-9a-f]{64}", binding[key]) is None
            for key in ("image_sha256", "splat_sha256", "replay_sha256")
        )
        or not isinstance(payload["bytes"], str)
        or len(payload["bytes"]) != 2 * (binding["end"] - binding["start"])
        or re.fullmatch(r"[0-9a-f]+", payload["bytes"]) is None
    ):
        raise ValueError("data range binding or original bytes are invalid")
    initializer = payload["initializer"]
    if (
        not isinstance(initializer, dict)
        or set(initializer) != {"words", "stop"}
        or not isinstance(initializer["words"], list)
        or len(initializer["words"]) > 16
        or initializer["stop"]
        not in {
            "unaligned selector",
            "16-word inspection cap",
            "containing segment boundary",
            "word is not one unambiguous reviewed function start",
            "reviewed function range is incomplete or unaligned",
        }
    ):
        raise ValueError("data initializer prefix shape is invalid")
    raw = bytes.fromhex(payload["bytes"])
    for index, word in enumerate(initializer["words"]):
        offset = address - binding["start"] + index * 4
        if (
            not isinstance(word, dict)
            or set(word) != {"address", "value", "function_end", "kind"}
            or any(
                type(word[k]) is not int for k in ("address", "value", "function_end")
            )
            or address % 4
            or word["address"] != address + index * 4
            or offset + 4 > len(raw)
            or word["value"] != int.from_bytes(raw[offset : offset + 4], "little")
            or word["value"] % 4
            or word["function_end"] % 4
            or not 0 <= word["value"] < word["function_end"] <= 0x100000000
            or word["kind"] not in {"c", "asm"}
        ):
            raise ValueError(
                "data initializer word differs from captured bytes or reviewed range"
            )
    return {
        **binding,
        "address": f"0x{address:08X}",
        "byte_count": binding["end"] - binding["start"],
        "original_sha256": hashlib.sha256(bytes.fromhex(payload["bytes"])).hexdigest(),
        "initializer": initializer,
    }


def _hex(value: object) -> int | None:
    return int(value, 16) if isinstance(value, str) and _HEX.fullmatch(value) else None


def _validated_row(operation: dict[str, Any], fields: set[str]) -> list[dict[str, Any]]:
    payload = operation.get("payload")
    if not isinstance(payload, list):
        raise ValueError("analyzer payload is not an array")
    for row in payload:
        if not isinstance(row, dict) or set(row) != fields:
            raise ValueError("analyzer payload row has invalid shape")
    return payload


def _describe(
    operation: dict[str, Any],
    target: str,
    *,
    expected_name: str | None = None,
    expected_kind: str,
) -> tuple[int, int, dict[str, Any] | None]:
    rows = _validated_row(
        operation,
        {"target", "address", "payload", "splat", "symbol", "storage", "references"},
    )
    if len(rows) != 1:
        raise ValueError("describe result is ambiguous")
    row = rows[0]
    try:
        selector = parse_function_id(str(operation["selector"]))
    except ValueError as error:
        raise ValueError("describe selector is invalid") from error
    selected = _hex(row["address"])
    if (
        row["target"] != target
        or selector.target.value != target
        or selected is None
        or selector.address != selected
    ):
        raise ValueError("describe target or selector mismatch")
    symbol = row["symbol"]
    if (
        not isinstance(symbol, dict)
        or set(symbol) != {"name", "kind"}
        or symbol["kind"] not in _SYMBOL_KINDS
        or symbol["kind"] != expected_kind
        or not isinstance(symbol["name"], str)
        or (expected_name is not None and symbol["name"] != expected_name)
    ):
        raise ValueError("describe object identity mismatch")
    payload, splat = row["payload"], row["splat"]
    if not isinstance(payload, dict) or set(payload) != {
        "contained",
        "payload_offset",
        "file_offset",
        "remaining_bytes",
    }:
        raise ValueError("describe payload shape is invalid")
    if not isinstance(splat, dict) or set(splat) != {"kind", "start", "end", "name"}:
        raise ValueError("describe boundary shape is invalid")
    start, end = _hex(splat["start"]), _hex(splat["end"])
    if start is None or end is None or not start < end or not start <= selected < end:
        raise ValueError("describe boundary does not contain selector")
    function_segment = splat["kind"] in {"c", "asm"}
    if expected_kind == "function":
        if not function_segment:
            raise ValueError("describe function boundary is not reviewed code")
    elif function_segment:
        raise ValueError("describe data boundary is a function segment")
    storage = row["storage"]
    if storage is not None:
        allowed = {
            "kind",
            "start",
            "end",
            "file_offset",
            "present_in_binary",
            "authority",
        }
        storage_start = (
            _hex(storage.get("start")) if isinstance(storage, dict) else None
        )
        storage_end = _hex(storage.get("end")) if isinstance(storage, dict) else None
        authority = storage.get("authority") if isinstance(storage, dict) else None
        if (
            not isinstance(storage, dict)
            or set(storage) != allowed
            or storage["kind"] not in _STORAGE_KINDS
            or storage_start is None
            or storage_end is None
            or storage_start >= storage_end
            or storage_start != selected
            or storage_end > end
            or not isinstance(storage["present_in_binary"], bool)
            or not isinstance(authority, list)
            or not authority
            or any(
                not isinstance(value, str) or value not in _STORAGE_AUTHORITY
                for value in authority
            )
            or len(authority) != len(set(authority))
            or ("reviewed_splat" in authority and splat["kind"] != storage["kind"])
        ):
            raise ValueError("describe storage shape is invalid")
        mapped = storage["kind"] != "bss"
        file_offset = _hex(storage["file_offset"])
        payload_offset = _hex(payload["payload_offset"])
        payload_file_offset = _hex(payload["file_offset"])
        remaining = payload["remaining_bytes"]
        extent = storage_end - storage_start
        if mapped:
            invalid_presence = (
                file_offset is None
                or file_offset < 0
                or file_offset != payload_file_offset
                or storage["present_in_binary"] is not True
                or "original_binary" not in authority
                or payload["contained"] is not True
                or payload_offset is None
                or payload_file_offset is None
                or not isinstance(remaining, int)
                or isinstance(remaining, bool)
                or remaining < extent
            )
        else:
            invalid_presence = (
                storage["file_offset"] is not None
                or storage["present_in_binary"] is not False
                or "original_binary" in authority
                or payload["contained"] is not False
                or payload["payload_offset"] is not None
                or payload["file_offset"] is not None
                or type(remaining) is not int
                or remaining != 0
            )
        if invalid_presence:
            raise ValueError("describe storage presence is inconsistent")
    elif expected_kind == "data":
        raise ValueError("describe canonical storage is unavailable")
    return start, end, storage
