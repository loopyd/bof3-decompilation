"""Conservative type candidate services derived from target-qualified index facts."""

from __future__ import annotations

import json
import sqlite3
from collections import defaultdict
from typing import Any

_OPCODE_WIDTH = {
    "lb": (1, "signed"),
    "lbu": (1, "unsigned"),
    "sb": (1, "unknown"),
    "lh": (2, "signed"),
    "lhu": (2, "unsigned"),
    "sh": (2, "unknown"),
    "lw": (4, "unknown"),
    "sw": (4, "unknown"),
}
_REGION_GAP = 0x20


def _candidate(
    connection: sqlite3.Connection,
    target: str,
    address: int,
    end: int | None,
    kind: str,
    evidence_class: str,
    width: int | None,
    signedness: str,
    evidence: list[dict[str, Any]],
    blocker: str,
) -> None:
    connection.execute(
        "INSERT OR REPLACE INTO type_candidates VALUES (?, ?, ?, ?, ?, ?, ?, ?, "
        "'blocked', 'lead', 'unresolved', ?, ?)",
        (
            f"{target}@{address:08X}:{kind}",
            target,
            address,
            end,
            kind,
            evidence_class,
            width,
            signedness,
            json.dumps(evidence, sort_keys=True),
            blocker,
        ),
    )


def _accesses(
    connection: sqlite3.Connection, target: str
) -> dict[int, list[sqlite3.Row]]:
    grouped: dict[int, list[sqlite3.Row]] = defaultdict(list)
    for row in connection.execute(
        "SELECT address, function_id, source, access_kind, opcode FROM data_references "
        "WHERE target_id = ? ORDER BY address, function_id, source",
        (target,),
    ):
        grouped[int(row[0])].append(row)
    return grouped


def _regions(addresses: list[int]) -> list[tuple[int, int, list[int]]]:
    result: list[tuple[int, int, list[int]]] = []
    members: list[int] = []
    for address in sorted(addresses):
        if members and address - members[-1] > _REGION_GAP:
            result.append((members[0], members[-1], members))
            members = []
        members.append(address)
    if members:
        result.append((members[0], members[-1], members))
    return result


def _storage_candidates(
    connection: sqlite3.Connection,
    target: str,
    grouped: dict[int, list[sqlite3.Row]],
) -> None:
    for address, accesses in grouped.items():
        width_facts = {
            _OPCODE_WIDTH[row[4]] for row in accesses if row[4] in _OPCODE_WIDTH
        }
        widths = {fact[0] for fact in width_facts}
        signs = {fact[1] for fact in width_facts if fact[1] != "unknown"}
        width = next(iter(widths)) if len(widths) == 1 else None
        signedness = next(iter(signs)) if len(signs) == 1 else "unknown"
        evidence = [
            {
                "function": row[1],
                "source": f"0x{int(row[2]):08X}",
                "access": row[3],
                "opcode": row[4],
                "evidence_class": "representation",
            }
            for row in accesses
        ]
        reason = (
            "conflicting access widths"
            if len(widths) != 1
            else "aggregate base, extent, and semantic role are not independently proven"
        )
        if len(widths) != 1:
            connection.execute(
                "INSERT OR IGNORE INTO type_conflicts VALUES (?, ?, ?, ?, ?, 'access_width')",
                (
                    target,
                    f"0x{address:08X}",
                    json.dumps(sorted(widths)),
                    "",
                    "reverse_index",
                ),
            )
        _candidate(
            connection,
            target,
            address,
            address + width if width else None,
            "storage",
            "representation",
            width,
            signedness,
            evidence,
            reason,
        )


def _aggregate_and_field_candidates(
    connection: sqlite3.Connection,
    target: str,
    grouped: dict[int, list[sqlite3.Row]],
) -> None:
    symbol_starts = {
        int(address)
        for (address,) in connection.execute(
            "SELECT address FROM symbols WHERE target_id = ? AND kind = 'data'",
            (target,),
        )
    }
    for base, last, members in _regions(list(grouped)):
        consumers = sorted(
            {row[1] for address in members for row in grouped[address] if row[1]}
        )
        region_evidence = [
            {
                "base": f"0x{base:08X}",
                "end": f"0x{last:08X}",
                "members": [f"0x{address:08X}" for address in members],
                "independent_consumers": consumers,
                "evidence_class": "representation",
            }
        ]
        _candidate(
            connection,
            target,
            base,
            last + 1,
            "aggregate_region",
            "representation",
            None,
            "unknown",
            region_evidence,
            "reviewed aggregate extent, alignment, padding, and semantic role are not proven",
        )
        if base not in symbol_starts or len(members) < 2:
            continue
        for address in members:
            facts = {
                _OPCODE_WIDTH[row[4]]
                for row in grouped[address]
                if row[4] in _OPCODE_WIDTH
            }
            widths = {fact[0] for fact in facts}
            width = next(iter(widths)) if len(widths) == 1 else None
            _candidate(
                connection,
                target,
                address,
                address + width if width else None,
                f"field_offset_{address - base:X}",
                "representation",
                width,
                "unknown",
                [{"aggregate_base": f"0x{base:08X}", "field_offset": address - base}],
                "base+offset access is a layout lead; field name and aggregate extent are unresolved",
            )
        offsets = [address - base for address in members]
        strides = {
            right - left for left, right in zip(offsets, offsets[1:]) if right > left
        }
        if len(strides) == 1 and len(members) >= 3:
            stride = next(iter(strides))
            _candidate(
                connection,
                target,
                base,
                last + 1,
                "array_stride",
                "representation",
                stride,
                "unknown",
                [
                    {
                        "stride": stride,
                        "elements_observed": len(members),
                        "nested": False,
                    }
                ],
                "repeated stride is a lead; array extent and element semantics are unresolved",
            )


def _prototype_candidates(connection: sqlite3.Connection, target: str) -> None:
    for row in connection.execute(
        "SELECT id, address, argument_count, stack_frame FROM functions WHERE target_id = ? "
        "ORDER BY address",
        (target,),
    ):
        callers = [
            caller
            for (caller,) in connection.execute(
                "SELECT DISTINCT caller FROM calls WHERE callee = ? ORDER BY caller",
                (row[0],),
            )
        ]
        if not callers and row[2] is None:
            continue
        _candidate(
            connection,
            target,
            int(row[1]),
            None,
            "prototype",
            "abi",
            None,
            "unknown",
            [
                {
                    "function": row[0],
                    "analyzer_argument_count": row[2],
                    "stack_frame": row[3],
                    "callers": callers,
                    "evidence_class": "representation",
                }
            ],
            "caller register/stack use, result use, and independently reviewed prototype are incomplete",
        )


def _class_like_candidates(connection: sqlite3.Connection, target: str) -> None:
    per_base: dict[int, set[str]] = defaultdict(set)
    for address, function_id in connection.execute(
        "SELECT address, function_id FROM data_references WHERE target_id = ? "
        "AND function_id IS NOT NULL ORDER BY address, function_id",
        (target,),
    ):
        per_base[int(address)].add(function_id)
    for address, consumers in per_base.items():
        dispatch = connection.execute(
            "SELECT COUNT(*) FROM xrefs WHERE target_id = ? AND source >= ? AND source < ? "
            "AND kind IN ('call', 'jump')",
            (target, address, address + 0x40),
        ).fetchone()[0]
        if len(consumers) < 2 or not dispatch:
            continue
        _candidate(
            connection,
            target,
            address,
            None,
            "class_like_receiver_dispatch",
            "semantic_lead",
            None,
            "unknown",
            [{"receiver_consumers": sorted(consumers), "dispatch_sites": dispatch}],
            "receiver/dispatch correlation is a lead; object identity and semantic contract are unproven",
        )


def infer_type_candidates(connection: sqlite3.Connection, target: str) -> None:
    """Index all safe inference rungs as blocked candidates, never promotions."""

    grouped = _accesses(connection, target)
    _storage_candidates(connection, target, grouped)
    _aggregate_and_field_candidates(connection, target, grouped)
    _prototype_candidates(connection, target)
    _class_like_candidates(connection, target)


__all__ = ["infer_type_candidates"]
