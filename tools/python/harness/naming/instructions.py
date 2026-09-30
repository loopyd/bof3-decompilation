"""Resolve and verify bounded original-function instruction captures."""

from __future__ import annotations

import hashlib
import json
import re
from contextvars import ContextVar
from pathlib import Path

from harness.analysis.project import prepare_target
from harness.domain.ids import parse_function_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests

INSTRUCTION_CAPTURE: ContextVar[bool] = ContextVar("instruction_capture", default=False)


def resolve_instructions(root: Path, selector: str) -> tuple[dict, bytes]:
    function = parse_function_id(selector)
    manifest = load_target_manifests(root)[function.target.value]
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    boundary = layout.find_boundary_at(function.address)
    if boundary is None or not boundary.is_function or boundary.virtual_end is None:
        raise ValueError(
            f"instruction capture requires a closed reviewed function: {selector}"
        )
    start, end = boundary.virtual_start, boundary.virtual_end
    size = end - start
    # ponytail: bound one capture to 64 KiB; larger functions need reviewed chunking.
    if start % 4 or size <= 0 or size % 4 or size > 65536:
        raise ValueError("instruction capture range is unaligned or exceeds 64 KiB")
    spec = prepare_target(root, function.target.value, manifest=manifest)
    image = spec.binary.read_bytes()
    offset = spec.binary_offset + start - spec.load_address
    if offset < spec.binary_offset or offset + size > len(image):
        raise ValueError("instruction capture escapes original payload")
    command = f"pdj {size // 4} @ 0x{start:08X} @!{size}"
    return {
        "selector": str(function),
        "start": start,
        "end": end,
        "offset": offset,
        "binary_sha256": hashlib.sha256(image).hexdigest(),
        "splat_sha256": layout.sha256,
        "replay_sha256": spec.replay_sha256,
        "command": f"bin/harness analysis rz-project query {function.target.value} -c '{command}'",
        "rizin": command,
    }, image[offset : offset + size]


def validate_instructions(raw: str, binding: dict, original: bytes) -> None:
    records = json.loads(raw)
    if not isinstance(records, list) or len(records) != len(original) // 4:
        raise ValueError("incomplete instruction output")
    for index, record in enumerate(records):
        if (
            not isinstance(record, dict)
            or type(record.get("offset")) is not int
            or record["offset"] != binding["start"] + index * 4
            or type(record.get("size")) is not int
            or record["size"] != 4
            or not isinstance(record.get("bytes"), str)
            or re.fullmatch(r"[0-9a-fA-F]{8}", record["bytes"]) is None
            or bytes.fromhex(record["bytes"]) != original[index * 4 : index * 4 + 4]
        ):
            raise ValueError("instruction output differs from original bytes")


def semantic_identities(semantic: list) -> list[str]:
    identities = []
    for item in semantic:
        command = item.get("command")
        if not isinstance(command, str) or not command.split():
            raise ValueError("semantic command is missing")
        kind = Path(command.split()[0]).name
        identities.append(item.get("instruction_id", f"semantic:{kind}:{command}"))
    return identities


def replay_instructions(root: Path, payload: dict, semantic: list) -> bool:
    """Revalidate instruction bindings and complete receipt output from live bytes."""
    for item in semantic:
        if item.get("instruction_id") is None:
            continue
        if (
            type(item.get("exit")) is not int
            or item["exit"] != 0
            or item.get("killed") is not False
            or item.get("semantic_success") is not True
        ):
            raise ValueError("instruction execution was not successful")
        selector = item.get("selector")
        if not isinstance(selector, str):
            raise ValueError("instruction selector must be a string")
        function = parse_function_id(selector)
        if item["instruction_id"] != f"semantic:instructions-v1:{function}":
            raise ValueError("instruction identity mismatch")
        binding, original = resolve_instructions(root, selector)
        if item.get("binding") != binding or item.get("command") != binding["command"]:
            raise ValueError("instruction binding is stale")
        path = Path(item["raw_file"])
        raw = (path if path.is_absolute() else root / path).read_text()
        validate_instructions(raw, binding, original)
        receipts = [
            x for x in payload["commands"] if x.get("command") == binding["command"]
        ]
        if (
            len(receipts) != 1
            or receipts[0].get("output") != raw
            or receipts[0].get("status") != "passed"
            or receipts[0].get("selector") != selector
        ):
            raise ValueError("instruction receipt does not retain complete output")
        from harness.naming.evidence import _RUNNER_TOKEN, instruction_observations

        kind, name = payload["row"].split(":", 1)
        if kind == "function":
            # The caller has already compared these items to the report-owned plan.
            row = {
                "kind": kind,
                "name": name,
                "outside_payload": True,
                "required_work": [
                    {"id": item["id"], "status": "open"}
                    for item in payload["items"]
                    if item.get("operation") == "owner"
                    and item.get("supplemental") is False
                ],
            }
            capture = {
                "selector": selector,
                "start": binding["start"],
                "bytes": original.hex(),
                "binding": binding,
            }
            expected = instruction_observations(
                _RUNNER_TOKEN, payload["target"], row, capture
            )
            receipt = json.loads((root / receipts[0]["receipt"]).read_text())
            if any(receipt.get(key) != value for key, value in expected.items()):
                raise ValueError(
                    "instruction receipt observations differ from original bytes"
                )
            if receipts[0].get("target") != function.target.value:
                raise ValueError(
                    "instruction receipt target differs from original owner"
                )
    return True


def analyzer_instructions(root: Path, semantic: list) -> tuple[dict, ...]:
    """Re-read live original bytes; never promote authored disassembly semantics."""
    captures = []
    for item in semantic:
        if item.get("instruction_id") is None:
            continue
        selector = item["selector"]
        binding, original = resolve_instructions(root, selector)
        if item.get("binding") != binding or item.get("command") != binding["command"]:
            raise ValueError("selected_call instruction binding is stale")
        if (
            item.get("exit") != 0
            or item.get("killed") is not False
            or item.get("semantic_success") is not True
        ):
            raise ValueError("selected_call instruction execution failed")
        raw = item.get("raw")
        if raw is None:
            raw = (root / item["raw_file"]).read_text()
        validate_instructions(raw, binding, original)
        captures.append(
            {
                "selector": selector,
                "start": binding["start"],
                "bytes": original.hex(),
                "binding": binding,
            }
        )
    return tuple(captures)
