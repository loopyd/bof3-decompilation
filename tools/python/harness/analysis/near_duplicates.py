"""Conservative read-only parameterized function near-duplicate analysis."""

from __future__ import annotations

import hashlib
import re
import sqlite3
from collections import defaultdict
from pathlib import Path
from typing import Any

from ..domain.mips import normalized_instruction_stream
from ..domain.psx import payload_for

_GENERATED = re.compile(
    r"(?:generated\s+by|auto[- ]?generated|do\s+not\s+edit|@generated)", re.IGNORECASE
)


def _candidate_id(key: str) -> str:
    return "near_duplicate:" + hashlib.sha256(key.encode()).hexdigest()[:16]


def _calls(
    connection: sqlite3.Connection, function_id: str
) -> tuple[tuple[int, str, int], ...]:
    function = connection.execute(
        "SELECT address FROM functions WHERE id = ?", (function_id,)
    ).fetchone()
    base = function[0]
    resolved = [
        (callsite - base, "resolved", int(callee.rsplit("@", 1)[1], 16))
        for callee, callsite in connection.execute(
            "SELECT callee, callsite FROM calls WHERE caller = ?", (function_id,)
        )
    ]
    unresolved = [
        (callsite - base, kind, target_address)
        for target_address, callsite, kind in connection.execute(
            "SELECT target_address, callsite, kind FROM unresolved_calls WHERE caller = ?",
            (function_id,),
        )
    ]
    return tuple(sorted(resolved + unresolved))


def _data_shape(
    connection: sqlite3.Connection, function_id: str
) -> tuple[tuple[int, str, str, int], ...]:
    row = connection.execute(
        "SELECT address FROM functions WHERE id = ?", (function_id,)
    ).fetchone()
    base = row[0]
    return tuple(
        (source - base, access_kind, opcode, address)
        for source, access_kind, opcode, address in connection.execute(
            "SELECT source, access_kind, opcode, address FROM data_references "
            "WHERE function_id = ? ORDER BY source, access_kind, opcode, address",
            (function_id,),
        )
    )


def _function_bytes(
    root: Path,
    target_rows: dict[str, tuple[Path, int]],
    target: str,
    address: int,
    size: int,
) -> bytes:
    binary, load_address = target_rows[target]
    binary_bytes = binary.read_bytes()
    payload = payload_for(binary_bytes, load_address, binary_name=binary.as_posix())
    start = payload.binary_offset + address - payload.load_address
    end = start + size
    if start < payload.binary_offset or end > len(binary_bytes):
        raise ValueError(
            f"function bytes outside target payload: {target}@0x{address:08X}"
        )
    return binary_bytes[start:end]


def near_duplicates_payload(
    connection: sqlite3.Connection,
    root: Path,
    *,
    target: str | None,
    limit: int,
) -> list[dict[str, Any]]:
    """Report only immediate/address-delta leads satisfying structural guards."""

    target_rows = {
        target_id: (root / binary, load_address)
        for target_id, binary, load_address in connection.execute(
            "SELECT id, binary, load_address FROM targets"
        )
    }
    clauses = [
        "reviewed = 1",
        "reviewed_size = size",
        "reviewed_sha256 IS NOT NULL",
        "analyzer_sha256 = reviewed_sha256",
        "trivial_kind IS NULL",
        "contains_data = 0",
        "instruction_count >= 3",
        "basic_blocks IS NOT NULL",
        "cfg_edges IS NOT NULL",
        "cyclomatic_complexity IS NOT NULL",
    ]
    params: list[object] = []
    if target:
        clauses.append("target_id = ?")
        params.append(target)
    rows = connection.execute(
        "SELECT id, target_id, address, size, reviewed_sha256, instruction_count, "
        "basic_blocks, cfg_edges, cyclomatic_complexity, loops, source FROM functions "
        f"WHERE {' AND '.join(clauses)} ORDER BY target_id, address",
        params,
    ).fetchall()
    groups: dict[tuple[Any, ...], list[dict[str, Any]]] = defaultdict(list)
    for row in rows:
        function_id, target_id, address, size, reviewed_digest, *metrics, source = row
        if source:
            source_path = root / source
            if source_path.is_file() and _GENERATED.search(
                source_path.read_text(encoding="utf-8", errors="replace")
            ):
                continue
        data = _function_bytes(root, target_rows, target_id, address, size)
        reviewed_actual = hashlib.sha256(data).hexdigest()
        if reviewed_actual != reviewed_digest:
            raise ValueError(f"stale reviewed function bytes: {function_id}")
        normalized = normalized_instruction_stream(data)
        call_shape = _calls(connection, function_id)
        data_shape = _data_shape(connection, function_id)
        key = (
            metrics[0],
            *metrics[1:],
            call_shape,
            data_shape,
            normalized.words,
        )
        groups[key].append(
            {
                "function": function_id,
                "target": target_id,
                "address": f"0x{address:08X}",
                "reviewed_sha256": reviewed_digest,
                "parameters": [
                    {"instruction": index, "field": field, "value": value}
                    for index, field, value in normalized.parameters
                ],
            }
        )
    payload: list[dict[str, Any]] = []
    for key, members in groups.items():
        if (
            len(members) < 2
            or len({member["reviewed_sha256"] for member in members}) < 2
        ):
            continue
        vectors = [member["parameters"] for member in members]
        if not vectors or any(len(vector) != len(vectors[0]) for vector in vectors):
            continue
        positions = [
            [(parameter["instruction"], parameter["field"]) for parameter in vector]
            for vector in vectors
        ]
        if any(position != positions[0] for position in positions[1:]):
            continue
        deltas = []
        for index, position in enumerate(positions[0]):
            values = [vector[index]["value"] for vector in vectors]
            if len(set(values)) > 1:
                deltas.append(
                    {"instruction": position[0], "field": position[1], "values": values}
                )
        if not deltas:
            continue
        (
            instruction_count,
            basic_blocks,
            cfg_edges,
            complexity,
            loops,
            calls,
            data_shape,
            _words,
        ) = key
        member_ids = [member["function"] for member in members]
        candidate_key = "\0".join(member_ids)
        payload.append(
            {
                "id": _candidate_id(candidate_key),
                "kind": "parameterized_near_duplicate",
                "status": "blocked",
                "rank": instruction_count * len(members),
                "target_scope": target
                or (
                    members[0]["target"]
                    if len({m["target"] for m in members}) == 1
                    else "cross_target_report_only"
                ),
                "members": members,
                "parameter_vectors": vectors,
                "immediate_deltas": deltas,
                "evidence": {
                    "instruction_count": instruction_count,
                    "cfg": {
                        "basic_blocks": basic_blocks,
                        "edges": cfg_edges,
                        "cyclomatic_complexity": complexity,
                        "loops": loops,
                    },
                    "call_shape": [list(item) for item in calls],
                    "data_reference_shape": [list(item) for item in data_shape],
                    "reviewed_boundaries": True,
                    "analyzer_boundary_agreement": True,
                    "normalization": "register/opcode/funct exact; only non-branch immediate/address fields erased",
                },
                "counterexamples": [],
                "semantic_guards": {
                    name: {
                        "status": "unproven",
                        "evidence": "instruction shape does not prove source-level semantics",
                    }
                    for name in (
                        "evaluation_count",
                        "side_effects",
                        "integer_promotions",
                        "precedence",
                        "lvalue",
                        "volatile",
                        "aliasing",
                        "control_flow",
                    )
                },
                "blockers": [
                    "parameter_semantics_unreviewed",
                    "source_c_shape_equivalence_unproven",
                    "compiler_profile_equivalence_unproven",
                    "cross_target_report_only"
                    if len({m["target"] for m in members}) > 1
                    else "target_local_review_required",
                    "read_only_analysis_only",
                ],
            }
        )
    payload.sort(key=lambda row: (-row["rank"], row["id"]))
    return payload if limit == 0 else payload[:limit]


__all__ = ["near_duplicates_payload"]
