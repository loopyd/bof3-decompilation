"""Four-use, largest-first assembly block leads from original reviewed functions."""

from __future__ import annotations

import hashlib
import sqlite3
from collections import defaultdict
from pathlib import Path
from typing import Any

from harness.analysis.index import connect
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.mips import normalized_instruction_stream
from harness.domain.psx import (
    is_psx_exe,
    payload_for,
    reviewed_range_digest,
    validate_psx_header,
)
from harness.domain.symbols import load_map, sdk_map_path
from harness.macros.opportunities import _GENERATED, _guards
from harness.macros.repeats import repeated_intervals

SCHEMA = "bof3.macro-block-opportunities/v1"
MINIMUM_USES = 4
HUMAN_CRITERIA = (
    "coherent_operation",
    "meaningful_parameters",
    "clear_call_sites",
    "reduced_repetition",
    "maintainability",
)


def validate_minimum(value: object) -> int:
    if type(value) is not int or value < 1:
        raise ValueError("block min_instructions must be a positive integer")
    return value


def _corpus(connection: sqlite3.Connection, root: Path, minimum: int):
    manifests = load_target_manifests(root)
    contexts = {}
    functions = []
    words: list[int] = []
    positions: list[tuple[int, int]] = []
    for raw in connection.execute(
        "SELECT id,target_id,address,size,reviewed_sha256,source,lift_status "
        "FROM functions WHERE reviewed=1 AND reviewed_size=size "
        "AND analyzer_sha256=reviewed_sha256 AND trivial_kind IS NULL "
        "AND contains_data=0 ORDER BY target_id,address,id",
    ):
        identity, target, address, size, expected, source, status = raw
        if size < minimum * 4 or size % 4 or address % 4:
            continue
        if target not in contexts:
            manifest = manifests[target]
            binary = (root / manifest.binary).read_bytes()
            if is_psx_exe(binary):
                validate_psx_header(
                    binary, manifest.load_address, binary_name=manifest.binary
                )
            contexts[target] = (
                binary,
                payload_for(binary, manifest.load_address, binary_name=manifest.binary),
                parse_splat_layout(root / manifest.splat, manifest.load_address),
                {
                    symbol.address
                    for symbol in load_map(sdk_map_path(root, manifest.psyq_space))
                    if not symbol.is_raw
                },
            )
        binary, payload, layout, sdk = contexts[target]
        boundary = layout.find_boundary_at(address)
        if (
            address in sdk
            or boundary is None
            or not boundary.is_function
            or boundary.virtual_size != size
        ):
            continue
        if reviewed_range_digest(payload, address, address + size, binary=binary) != (
            expected,
            size,
        ):
            raise ValueError(f"stale reviewed assembly block input: {identity}")
        source_hash = None
        if source:
            path = (root / source).resolve()
            if not path.is_relative_to(root.resolve()) or not path.is_file():
                raise ValueError(f"stale assembly block source: {identity}")
            content = path.read_bytes()
            if _GENERATED.search(content.decode("utf-8", errors="replace")):
                continue
            source = path.relative_to(root.resolve()).as_posix()
            source_hash = hashlib.sha256(content).hexdigest()
        offset = payload.binary_offset + address - payload.load_address
        data = binary[offset : offset + size]
        stream = normalized_instruction_stream(data)
        if not any(stream.words):
            continue
        function_index = len(functions)
        functions.append(
            {
                "function": identity,
                "target": target,
                "address": address,
                "source_path": source,
                "source_sha256": source_hash,
                "lift_status": status,
                "reviewed_sha256": expected,
                "data": data,
                "stream": stream,
            }
        )
        words.extend(stream.words)
        positions.extend((function_index, index) for index in range(len(stream.words)))
        words.append(-function_index - 1)
        positions.append((-1, -1))
    return functions, words, positions


def _nonoverlapping(functions, positions, starts, size):
    ordered = sorted(
        {positions[start] for start in starts},
        key=lambda item: (
            functions[item[0]]["target"],
            functions[item[0]]["address"] + item[1] * 4,
            item[0],
        ),
    )
    ends: dict[str, int] = {}
    selected = []
    for function_index, offset in ordered:
        function = functions[function_index]
        address = function["address"] + offset * 4
        if address < ends.get(function["target"], -1):
            continue
        ends[function["target"]] = address + size * 4
        selected.append((function_index, offset))
    return selected


def block_opportunities_payload(
    connection: sqlite3.Connection, root: Path, *, min_instructions: int
) -> list[dict[str, Any]]:
    """Find repeated sub-function intervals; all output remains evidence-blocked."""
    minimum = validate_minimum(min_instructions)
    functions, words, positions = _corpus(connection, root, minimum)
    groups = []
    for size, parent_size, starts in repeated_intervals(words, minimum):
        selected = _nonoverlapping(functions, positions, starts, size)
        if len(selected) < MINIMUM_USES:
            low, high = max(minimum, parent_size + 1), size - 1
            best = None
            while low <= high:
                middle = (low + high) // 2
                eligible = _nonoverlapping(functions, positions, starts, middle)
                if len(eligible) >= MINIMUM_USES:
                    best = middle, eligible
                    low = middle + 1
                else:
                    high = middle - 1
            if best is None:
                continue
            size, selected = best
        function_index, offset = selected[0]
        shape = functions[function_index]["stream"].words[offset : offset + size]
        if not any(shape):
            continue
        groups.append((size, selected, shape, len(starts)))
    groups.sort(key=lambda group: (-group[0], -len(group[1]), group[2]))
    retained = defaultdict(list)
    candidates = []
    for size, selected, shape, raw_count in groups:
        identity_set = tuple(index for index, _offset in selected)
        if any(
            all(
                parent_offset <= offset and offset + size <= parent_offset + parent_size
                for (_index, offset), (_parent_index, parent_offset) in zip(
                    selected, parent
                )
            )
            for parent_size, parent in retained[identity_set]
        ):
            continue
        retained[identity_set].append((size, selected))
        members = []
        for function_index, offset in selected:
            function = functions[function_index]
            start = function["address"] + offset * 4
            members.append(
                {
                    key: value
                    for key, value in function.items()
                    if key not in {"data", "stream", "address"}
                }
                | {
                    "start_address": f"0x{start:08X}",
                    "end_address": f"0x{start + size * 4:08X}",
                    "function_offset": offset * 4,
                    "instruction_count": size,
                    "block_sha256": hashlib.sha256(
                        function["data"][offset * 4 : (offset + size) * 4]
                    ).hexdigest(),
                    "parameters": [
                        {"instruction": index - offset, "field": field, "value": value}
                        for index, field, value in function["stream"].parameters
                        if offset <= index < offset + size
                    ],
                }
            )
        pattern = hashlib.sha256(
            b"".join(word.to_bytes(4, "little") for word in shape)
        ).hexdigest()
        candidates.append(
            {
                "id": "assembly_block:" + pattern[:16],
                "kind": "assembly_block",
                "status": "blocked",
                "rank": size * len(members),
                "instruction_count": size,
                "pattern": pattern,
                "target_scope": "cross_target"
                if len({row["target"] for row in members}) > 1
                else members[0]["target"],
                "members": members,
                "evidence": {
                    "support": len(members),
                    "raw_occurrences": raw_count,
                    "minimum_uses": MINIMUM_USES,
                    "normalized_words": [f"0x{word:08X}" for word in shape],
                },
                "semantic_guards": _guards(),
                "human_review": {
                    "status": "unreviewed",
                    "question": "would a human make this macro?",
                    "criteria": list(HUMAN_CRITERIA),
                },
                "blockers": [
                    "source_block_mapping_unproven",
                    "entry_exit_and_delay_slot_context_unproven",
                    "live_registers_and_memory_effects_unproven",
                    "human_value_review_required",
                    "read_only_analysis_only",
                ],
            }
        )
    candidates.sort(
        key=lambda row: (-row["instruction_count"], -len(row["members"]), row["id"])
    )
    return candidates


def block_report(root: Path, *, min_instructions: int, target: str | None, limit: int):
    minimum = validate_minimum(min_instructions)
    if type(limit) is not int or limit < 0:
        raise ValueError("block limit must be a nonnegative integer")
    connection = connect(root)
    try:
        if (
            target is not None
            and connection.execute(
                "SELECT 1 FROM targets WHERE id=?", (target,)
            ).fetchone()
            is None
        ):
            raise ValueError(f"unknown assembly block target: {target}")
        candidates = block_opportunities_payload(
            connection, root, min_instructions=minimum
        )
    finally:
        connection.close()
    visible = [
        row
        for row in candidates
        if target is None
        or any(member["target"] == target for member in row["members"])
    ]
    return {
        "schema": SCHEMA,
        "min_instructions": minimum,
        "minimum_uses": MINIMUM_USES,
        "view_target": target,
        "candidate_count": len(visible),
        "safe_application_count": 0,
        "ordering": "instruction_count_desc,support_desc,id",
        "attempt_budget": None,
        "candidates": visible if limit == 0 else visible[:limit],
    }
