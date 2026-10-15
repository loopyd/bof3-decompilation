"""Capture and decode complete original data dispatch consumers."""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

from harness.analysis.project import prepare_target
from harness.domain.ids import parse_function_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.naming.dispatch import decode_dispatch_body


def capture_dispatch_consumer(root: Path, selected: str, consumer: str) -> dict:
    """Capture only a complete reviewed function, independent of indexed sizes."""
    selected_id, consumer_id = parse_function_id(selected), parse_function_id(consumer)
    result = {
        "selected": str(selected_id),
        "consumer": str(consumer_id),
        "binding": None,
        "bytes": None,
        "gap": None,
    }
    try:
        if selected_id.target != consumer_id.target:
            raise ValueError("dispatch consumer target differs from selected data")
        manifest = load_target_manifests(root)[consumer_id.target.value]
        layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
        matches = [
            b
            for b in layout.boundaries
            if b.virtual_start <= consumer_id.address
            and (b.virtual_end is None or consumer_id.address < b.virtual_end)
        ]
        if (
            len(matches) != 1
            or not matches[0].is_function
            or matches[0].virtual_start != consumer_id.address
        ):
            raise ValueError("dispatch consumer requires one reviewed function start")
        boundary = matches[0]
        start, end = boundary.virtual_start, boundary.virtual_end
        if end is None or start % 4 or end % 4 or not 0 < end - start <= 65536:
            raise ValueError(
                "dispatch consumer requires a finite aligned body up to 64 KiB"
            )
        spec = prepare_target(root, consumer_id.target.value, manifest=manifest)
        image = spec.binary.read_bytes()
        offset = spec.binary_offset + start - spec.load_address
        if offset < spec.binary_offset or offset + end - start > len(image):
            raise ValueError("dispatch consumer range escapes original payload")
        result["binding"] = {
            "start": start,
            "end": end,
            "kind": boundary.kind,
            "image_sha256": hashlib.sha256(image).hexdigest(),
            "splat_sha256": layout.sha256,
            "replay_sha256": spec.replay_sha256,
        }
        result["bytes"] = image[offset : offset + end - start].hex()
    except ValueError as error:
        result["gap"] = str(error)
    return result


def decode_dispatch_consumer(
    operation: dict, target: str, selected_range: dict, input_digest: str
) -> list[dict]:
    """Join original initializer words to complete decoded use, without an allowlist."""
    payload = operation.get("payload")
    if not isinstance(payload, dict) or set(payload) != {
        "selected",
        "consumer",
        "binding",
        "bytes",
        "gap",
    }:
        raise ValueError("dispatch consumer capture shape is invalid; recapture")
    selected = parse_function_id(str(payload["selected"]))
    consumer = parse_function_id(str(payload["consumer"]))
    if (
        selected.target.value != target
        or consumer.target.value != target
        or consumer != parse_function_id(str(operation["selector"]))
        or selected_range.get("address") != f"0x{selected.address:08X}"
    ):
        raise ValueError("dispatch consumer identity mismatch")
    if payload["gap"] is not None:
        if (
            not isinstance(payload["gap"], str)
            or not payload["gap"].strip()
            or payload["binding"] is not None
            or payload["bytes"] is not None
        ):
            raise ValueError("dispatch consumer gap shape is invalid")
        raise ValueError(payload["gap"])
    binding = payload["binding"]
    if (
        not isinstance(binding, dict)
        or set(binding)
        != {"start", "end", "kind", "image_sha256", "splat_sha256", "replay_sha256"}
        or any(type(binding[k]) is not int for k in ("start", "end"))
        or binding["start"] != consumer.address
        or binding["start"] % 4
        or binding["end"] % 4
        or not 0 <= binding["start"] < binding["end"] <= 0x100000000
        or binding["end"] - binding["start"] > 65536
        or binding["kind"] not in {"c", "asm"}
        or any(
            not isinstance(binding[k], str)
            or re.fullmatch(r"[0-9a-f]{64}", binding[k]) is None
            or binding[k] != selected_range.get(k)
            for k in ("image_sha256", "splat_sha256", "replay_sha256")
        )
        or not isinstance(payload["bytes"], str)
        or re.fullmatch(r"[0-9a-f]+", payload["bytes"]) is None
        or len(payload["bytes"]) != 2 * (binding["end"] - binding["start"])
    ):
        raise ValueError("dispatch consumer bytes or reviewed binding is invalid")
    body = decode_dispatch_body(
        bytes.fromhex(payload["bytes"]), consumer.address, selected.address
    )
    initializer = selected_range["initializer"]
    words = initializer["words"]
    read_end = body["observed_read_span"][1]
    if not words or read_end > selected.address + len(words) * 4:
        raise ValueError(
            "dispatch consumer reads beyond the reviewed function-address prefix"
        )
    value = {
        "consumer": str(consumer),
        "consumer_range": [binding["start"], binding["end"]],
        "body": body,
        "load_sites": [read["instruction"] for read in body["selected_reads"]],
        "initializer_words_read": words[: len(body["selected_reads"])],
        "shared_premises": [
            "original image identity",
            "reviewed layout and target load mapping",
        ],
        "scope": "Original function-address content and conditional dispatch use only. No object extent, total count, live pointer contents, safe selector bounds, callee meaning, lifetime or C declaration is established.",
    }
    source_id = (
        "original-image:dispatch-consumer:"
        + hashlib.sha256(
            json.dumps(
                [target, value, input_digest], sort_keys=True, separators=(",", ":")
            ).encode()
        ).hexdigest()
    )
    return [
        {"class": name, "polarity": "positive", "value": value, "source_id": source_id}
        for name in ("selected_access", "one_level_beyond")
    ]
