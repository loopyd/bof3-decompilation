"""Decode one code-owned, target-qualified fixed-width table consumer."""

from __future__ import annotations

import hashlib
import json
import re
import struct
from dataclasses import dataclass
from typing import Any

from harness.domain.ids import parse_function_id


@dataclass(frozen=True)
class TableConsumerSpec:
    """Immutable reviewed identity and original-byte contract."""

    target: str
    row: str
    selected_address: int
    selected_end: int
    consumer_address: int
    consumer_end: int
    image_sha256: str
    table_bytes: bytes
    consumer_words: tuple[int, ...]
    pointer_addresses: tuple[int, ...]
    load_offsets: tuple[int, int, int]


def _digest(value: object) -> str:
    return hashlib.sha256(
        json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    ).hexdigest()


_SHA256 = re.compile(r"[0-9a-f]{64}")


def _source(family: str, target: str, value: object, input_digest: str) -> str:
    return f"{family}:{_digest([target, value, input_digest])}"


def _selector_offset(words: tuple[int, ...]) -> int:
    selectors = [
        (index, word)
        for index, word in enumerate(words)
        if word >> 26 == 0x24 and (word >> 21) & 0x1F and (word >> 16) & 0x1F
    ]
    if len(selectors) != 1:
        raise ValueError("table consumer selector load is ambiguous")
    index, selector = selectors[0]
    base = (selector >> 21) & 0x1F
    destination = (selector >> 16) & 0x1F
    definitions = [
        word
        for word in words[:index]
        if (word >> 16) & 0x1F == base and word >> 26 == 0x23
    ]
    if not definitions or any(
        (word >> 16) & 0x1F == base and word >> 26 != 0x23
        for word in words[index - 1 : index]
    ):
        raise ValueError("table consumer selector base is invalid")
    if index + 3 >= len(words):
        raise ValueError("table consumer selector def-use is incomplete")
    shift = words[index + 2]
    address = words[index + 3]
    if (
        shift >> 26 != 0
        or shift & 0x3F != 0
        or (shift >> 21) & 0x1F
        or (shift >> 16) & 0x1F != destination
        or (shift >> 11) & 0x1F != destination
        or (shift >> 6) & 0x1F != 2
        or address >> 26 != 0
        or address & 0x3F != 0x21
        or (address >> 21) & 0x1F != 29
        or (address >> 16) & 0x1F != destination
        or (address >> 11) & 0x1F != destination
    ):
        raise ValueError("table consumer selector def-use is invalid")
    offset = selector & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    if offset < 0:
        raise ValueError("table consumer selector offset is invalid")
    return offset


def decode_table_consumer(
    operation: dict[str, Any], spec: TableConsumerSpec, input_digest: str
) -> list[dict[str, Any]]:
    """Return facts only when the runner payload exactly matches the reviewed spec."""

    payload = operation.get("payload")
    if not isinstance(payload, dict) or set(payload) != {
        "selected",
        "consumer",
        "selected_range",
        "consumer_range",
        "selected_bytes",
        "consumer_bytes",
        "image_sha256",
        "pointer_descriptions",
    }:
        raise ValueError("table consumer payload shape is invalid")
    selected = parse_function_id(str(payload["selected"]))
    consumer = parse_function_id(str(payload["consumer"]))
    selector = parse_function_id(str(operation.get("selector")))
    if (
        selected.target.value != spec.target
        or selected.address != spec.selected_address
        or consumer.target.value != spec.target
        or consumer.address != spec.consumer_address
        or consumer != selector
    ):
        raise ValueError("table consumer identity mismatch")
    if payload["selected_range"] != [
        spec.selected_address,
        spec.selected_end,
    ] or payload["consumer_range"] != [spec.consumer_address, spec.consumer_end]:
        raise ValueError("table consumer reviewed range mismatch")
    if not isinstance(payload["selected_bytes"], str) or not isinstance(
        payload["consumer_bytes"], str
    ):
        raise ValueError("table consumer original bytes are invalid")
    try:
        table = bytes.fromhex(payload["selected_bytes"])
        code = bytes.fromhex(payload["consumer_bytes"])
    except ValueError as error:
        raise ValueError("table consumer original bytes are invalid") from error
    if (
        table != spec.table_bytes
        or len(table) != spec.selected_end - spec.selected_address
        or len(table) % 4
        or len(code) != spec.consumer_end - spec.consumer_address
        or len(code) % 4
        or tuple(struct.unpack(f"<{len(code) // 4}I", code)) != spec.consumer_words
        or not isinstance(payload["image_sha256"], str)
        or _SHA256.fullmatch(payload["image_sha256"]) is None
        or payload["image_sha256"] != spec.image_sha256
    ):
        raise ValueError("table consumer does not match reviewed original bytes")
    pointers = list(struct.unpack(f"<{len(table) // 4}I", table))
    if tuple(pointers) != spec.pointer_addresses:
        raise ValueError("table consumer pointers do not match reviewed globals")
    descriptions = payload["pointer_descriptions"]
    if not isinstance(descriptions, list) or len(descriptions) != len(pointers):
        raise ValueError("table consumer pointer descriptions are incomplete")
    for pointer, rows in zip(pointers, descriptions, strict=True):
        if (
            not isinstance(rows, list)
            or len(rows) != 1
            or not isinstance(rows[0], dict)
        ):
            raise ValueError("table consumer pointer description is ambiguous")
        row = rows[0]
        splat = row.get("splat")
        try:
            address = int(str(row.get("address")), 16)
            start = int(str(splat.get("start")), 16) if isinstance(splat, dict) else -1
        except ValueError as error:
            raise ValueError("table consumer pointer description is invalid") from error
        if (
            row.get("target") != spec.target
            or address != pointer
            or not isinstance(splat, dict)
            or splat.get("kind") not in {"c", "asm"}
            or start != pointer
        ):
            raise ValueError("table consumer pointer is not a reviewed function start")
    layout = {
        "selected_range": [
            f"0x{spec.selected_address:08X}",
            f"0x{spec.selected_end:08X}",
        ],
        "element_width": 4,
        "element_count": len(pointers),
        "words": [f"0x{pointer:08X}" for pointer in pointers],
        "image_sha256": payload["image_sha256"],
    }
    consumer_value = {
        "consumer_range": [
            f"0x{spec.consumer_address:08X}",
            f"0x{spec.consumer_end:08X}",
        ],
        "load_sites": [
            f"0x{spec.consumer_address + offset * 4:08X}"
            for offset in spec.load_offsets
        ],
        "widths": [4, 4, 4],
        "offsets": [0, 4, 8],
        "selector": {
            "kind": "unsigned_byte",
            "offset": _selector_offset(spec.consumer_words),
            "scale": 4,
        },
        "dispatch": "jalr",
        "delay_slot": "nop",
        "bounds_check": False,
    }
    return [
        {
            "class": "reviewed_layout",
            "polarity": "positive",
            "value": layout,
            "source_id": _source(
                "original-image:reviewed-layout", spec.target, layout, input_digest
            ),
        },
        {
            "class": "selected_access",
            "polarity": "positive",
            "value": {"kind": "original_consumer_loads", **consumer_value},
            "source_id": _source(
                "original-image:independent-consumer",
                spec.target,
                consumer_value,
                input_digest,
            ),
        },
        {
            "class": "one_level_beyond",
            "polarity": "positive",
            "value": {"kind": "independent_consumer", **consumer_value},
            "source_id": _source(
                "original-image:independent-consumer",
                spec.target,
                consumer_value,
                input_digest,
            ),
        },
    ]
