"""Validate exact native describe payloads for naming evidence."""

from __future__ import annotations

import re
from typing import Any

from harness.domain.ids import parse_function_id

_HEX = re.compile(r"0x[0-9A-F]+")
_STORAGE_KINDS = {"data", "bss", "rodata", "unknown"}
_STORAGE_AUTHORITY = {"reviewed_splat", "original_binary"}
_SYMBOL_KINDS = {"data", "function"}


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
