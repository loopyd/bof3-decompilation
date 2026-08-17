"""Unlabeled in-image data region clustering from the reverse index."""

from __future__ import annotations

from collections import defaultdict
from pathlib import Path

from ..domain import load_target_manifests
from .index import connect

_CLUSTER_GAP = 32


def collect_unlabeled_regions(
    root: Path, target_ids: list[str], *, lifted_only: bool
) -> dict[str, list[dict]]:
    """Cluster unlabeled referenced addresses into BSS/FILE regions."""

    manifests = load_target_manifests(root)
    selected = target_ids or sorted(manifests)
    connection = connect(root)
    try:
        rows = connection.execute(
            "SELECT d.target_id, d.address, COUNT(*) AS refs FROM data_references d "
            + (
                "JOIN functions f ON f.id = d.function_id AND f.lifted "
                if lifted_only
                else ""
            )
            + "WHERE d.symbol IS NULL GROUP BY d.target_id, d.address"
        ).fetchall()
        labeled = defaultdict(set)
        for target_id, address in connection.execute(
            "SELECT target_id, address FROM symbols"
        ):
            labeled[target_id].add(address)
        code_ranges = defaultdict(list)
        for target_id, address, size in connection.execute(
            "SELECT target_id, address, size FROM functions"
        ):
            code_ranges[target_id].append((int(address), int(address) + int(size)))
    finally:
        connection.close()
    report: dict[str, list[dict]] = {}
    for target in selected:
        manifest = manifests.get(target)
        if manifest is None:
            continue
        binary = (root / manifest.binary).read_bytes()
        load = manifest.load_address
        points = sorted(
            (int(row[1]), row[2])
            for row in rows
            if row[0] == target
            and load <= int(row[1]) < load + len(binary)
            and int(row[1]) not in labeled[target]
            and not any(
                start <= int(row[1]) < end for start, end in code_ranges[target]
            )
        )
        regions: list[dict] = []
        for address, refs in points:
            if regions and address - regions[-1]["end"] <= _CLUSTER_GAP:
                regions[-1]["end"] = address
                regions[-1]["refs"] += refs
            else:
                regions.append({"start": address, "end": address, "refs": refs})
        for region in regions:
            chunk = binary[region["start"] - load : region["end"] - load + 16]
            region["class"] = "BSS" if all(byte == 0 for byte in chunk) else "FILE"
            region["start"] = f"0x{region['start']:08X}"
            region["end"] = f"0x{region['end']:08X}"
        report[target] = regions
    return report
