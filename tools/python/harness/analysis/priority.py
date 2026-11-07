"""Candidate context, exclusion classification, and priority row assembly."""

from __future__ import annotations

from pathlib import Path
import sys
from typing import Any

from ..domain.sources import CompiledSymbolError
from ..domain.manifests import load_target_manifests
from ..domain.layout import parse_splat_layout
from ..domain.sources import reviewed_function_name
from ..domain.symbols import load_map, sdk_map_path
from .graph import dominates, enrich_graph, function_metrics


RANK_FIELDS = {
    "minimal": {
        "duplicates": (
            "representative",
            "members",
            "unlifted_members",
            "remaining_effort_instructions",
        ),
        "default": (
            "id",
            "instruction_count",
            "cyclomatic_complexity",
            "unique_callers",
            "duplicate_leverage",
            "leaf_status",
            "lifted",
        ),
    },
    "normal": {
        "duplicates": (
            "representative",
            "size",
            "members",
            "unlifted_members",
            "targets",
            "remaining_effort_instructions",
            "functions",
        ),
        "default": (
            "id",
            "size",
            "instruction_count",
            "basic_blocks",
            "cyclomatic_complexity",
            "loops",
            "unique_callers",
            "unique_callees",
            "unresolved_calls",
            "duplicate_leverage",
            "leaf_status",
            "lifted",
            "metric_missing",
        ),
    },
}


def load_candidate_context(
    root: Path,
    target: str,
    *,
    manifests: dict[str, Any] | None = None,
    sdk_cache: dict[str, frozenset[int]] | None = None,
):
    """Load one target's exclusion context, reusing caller-owned shared state.

    ``load_target_manifests`` revalidates every target claim on each call, so a
    whole-corpus ranking must pass one manifest load down to every target. The
    returned context carries the binary path and size instead of the image bytes
    so each boundary read touches only its own slice.
    """

    if manifests is None:
        manifests = load_target_manifests(root)
    manifest = manifests[target]
    binary = root / manifest.binary
    if not binary.is_file():
        return manifest, None, None, frozenset(), None
    if sdk_cache is None:
        sdk_cache = {}
    sdk_addresses = sdk_cache.get(manifest.psyq_space)
    if sdk_addresses is None:
        sdk_addresses = frozenset(
            symbol.address
            for symbol in load_map(sdk_map_path(root, manifest.psyq_space))
            if not symbol.is_raw
        )
        sdk_cache[manifest.psyq_space] = sdk_addresses
    return (
        manifest,
        binary.stat().st_size,
        parse_splat_layout(root / manifest.splat, manifest.load_address),
        sdk_addresses,
        binary,
    )


def candidate_exclusion(root: Path, row: dict[str, Any], *, context=None) -> str | None:
    """Reject analyzer-only roots that lack canonical code evidence.

    Reviewed Splat labels and Rizin's function finder are hypotheses. Ranking
    must not offer a raw-data label or an SDK body as a lift candidate.
    """

    address = int(str(row["address"]), 0)
    if context is None:
        context = load_candidate_context(root, row["target"])
    manifest, image_size, layout, sdk_addresses, binary = context
    if manifest.id.value != row["target"]:
        raise ValueError("candidate context belongs to a different target")
    if image_size is None:
        return "missing_binary"
    assert layout is not None
    assert binary is not None
    boundary = layout.find_boundary_at(address)
    if boundary is None or not boundary.is_function:
        return "not_reviewed_code_boundary"
    offset = address - manifest.load_address
    size = row["size"]
    with binary.open("rb") as handle:
        handle.seek(offset)
        payload = handle.read(size)
    if len(payload) != size:
        return "boundary_outside_binary"
    printable = sum(byte == 0 or 0x20 <= byte < 0x7F for byte in payload)
    if len(payload) >= 8 and printable * 4 >= len(payload) * 3:
        return "ascii_or_nul_data"
    words = [
        int.from_bytes(payload[index : index + 4], "little")
        for index in range(0, len(payload) - 3, 4)
    ]
    binary_end = manifest.load_address + image_size
    if len(words) >= 2 and all(
        manifest.load_address <= word < binary_end for word in words
    ):
        return "in_image_pointer_table"
    if address in sdk_addresses:
        return "shared_sdk_symbol"
    # Canonical raw boundaries need no map entry; semantic boundaries must be
    # agreed by the target-local map (reviewed_function_name).
    if boundary.name == f"func_{address:08X}":
        return None
    try:
        reviewed_function_name(
            root, row["target"], address, layout=layout, manifest=manifest
        )
    except CompiledSymbolError:
        return "noncanonical_boundary_name"
    return None


def priority_rows(
    connection,
    *,
    target: str | None,
    command: str,
    limit: int,
    exclusions: bool = False,
    include_trivial: bool = False,
    unlifted: bool = False,
    function: str | None = None,
    root: Path | None = None,
) -> list[dict[str, Any]]:
    def cost(row: dict[str, Any], name: str) -> int:
        value = row[name]
        return value if value is not None else sys.maxsize

    payload = function_metrics(connection, target)
    if root is not None:
        manifests = load_target_manifests(root)
        sdk_cache: dict[str, frozenset[int]] = {}
        contexts = {
            owner: load_candidate_context(
                root, owner, manifests=manifests, sdk_cache=sdk_cache
            )
            for owner in dict.fromkeys(row["target"] for row in payload)
        }
        exclusion_rows = [
            (row, candidate_exclusion(root, row, context=contexts[row["target"]]))
            for row in payload
        ]
        if exclusions:
            payload = [
                {**row, "candidate_exclusion": reason}
                for row, reason in exclusion_rows
                if reason is not None
            ]
        else:
            payload = [row for row, reason in exclusion_rows if reason is None]
    enrich_graph(connection, payload)
    if function:
        payload = [row for row in payload if row["id"] == function]
    if exclusions:
        payload = [
            {
                key: row[key]
                for key in ("id", "target", "address", "candidate_exclusion")
            }
            for row in payload
        ]
        payload.sort(key=lambda row: (row["id"], row["candidate_exclusion"]))
        return payload[:limit] if limit else payload
    if command != "metrics":
        payload = [row for row in payload if row["reviewed"] and row["size"] >= 8]
        if not include_trivial:
            payload = [row for row in payload if row["trivial_kind"] is None]
    if unlifted:
        payload = [row for row in payload if not row["lifted"]]
    if command == "leafs":
        payload = [row for row in payload if row["leaf_status"] != "non_leaf"]
        payload.sort(
            key=lambda row: (
                row["leaf_status"] != "analyzer_no_edge",
                row["instruction_count"],
                row["id"],
            )
        )
    elif command == "quick-wins":
        payload.sort(
            key=lambda row: (
                row["leaf_status"] != "analyzer_no_edge",
                row["metric_missing"],
                row["unresolved_calls"],
                cost(row, "cyclomatic_complexity"),
                cost(row, "loops"),
                row["instruction_count"],
                -row["duplicate_leverage"],
                -row["unique_callers"],
                row["id"],
            )
        )
    elif command == "hotspots":
        payload.sort(
            key=lambda row: (
                -row["unique_callers"],
                -row["caller_callsites"],
                -row["duplicate_leverage"],
                row["metric_missing"],
                row["unresolved_calls"],
                cost(row, "cyclomatic_complexity"),
                row["instruction_count"],
                row["id"],
            )
        )
    elif command == "pareto":
        payload = [row for row in payload if not row["metric_missing"]]
        payload = [
            row
            for row in payload
            if not any(dominates(other, row) for other in payload if other is not row)
        ]
        payload.sort(
            key=lambda row: (
                -row["unique_callers"],
                -row["duplicate_leverage"],
                row["instruction_count"],
                row["id"],
            )
        )
    if command != "metrics":
        unique: list[dict[str, Any]] = []
        seen: set[tuple[str, int]] = set()
        for row in payload:
            identity = (row["exact_sha256"], row["size"])
            if identity in seen:
                continue
            seen.add(identity)
            unique.append(row)
        payload = unique
    return payload[:limit] if limit else payload
