from __future__ import annotations

import argparse
import json
from collections import defaultdict
from pathlib import Path
from typing import Any

from ..config import ROOT
from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import (
    emit_output_summary,
    ensure_output_parents,
    write_json_output,
    write_markdown_output,
)
from .db.connection import connect_inventory_database
from .db.migrations import ensure_inventory_schema
from .layout import (
    INVENTORY_SQLITE,
)
from .repositories.overlays import OverlayRepository


def load_candidates(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def build_clusters(catalog: dict[str, Any]) -> dict[str, Any]:
    by_hash: dict[str, list[dict[str, Any]]] = defaultdict(list)
    by_region: dict[tuple[str, int], list[dict[str, Any]]] = defaultdict(list)

    for candidate in catalog["candidates"]:
        by_hash[candidate["sha256"]].append(candidate)
        by_region[(candidate["ram_ptr_hex"], candidate["size"])].append(candidate)

    exact_groups = []
    for sha256, members in by_hash.items():
        if len(members) < 2:
            continue

        members = sorted(
            members,
            key=lambda item: (item["family"], item["archive_id"], item["entry_index"]),
        )
        exact_groups.append(
            {
                "sha256": sha256,
                "group_size": len(members),
                "representative": members[0]["candidate_name"],
                "load_addresses": sorted({member["ram_ptr_hex"] for member in members}),
                "families": sorted({member["family"] for member in members}),
                "members": [
                    {
                        "candidate_name": member["candidate_name"],
                        "archive_id": member["archive_id"],
                        "entry_index": member["entry_index"],
                        "payload_path": member["payload_path"],
                        "ram_ptr_hex": member["ram_ptr_hex"],
                        "size": member["size"],
                    }
                    for member in members
                ],
            }
        )

    exact_groups.sort(key=lambda group: (-group["group_size"], group["representative"]))

    region_groups = []
    for (ram_ptr_hex, size), members in by_region.items():
        if len(members) < 2:
            continue

        distinct_hashes = sorted({member["sha256"] for member in members})
        region_groups.append(
            {
                "ram_ptr_hex": ram_ptr_hex,
                "size": size,
                "member_count": len(members),
                "distinct_hash_count": len(distinct_hashes),
                "families": sorted({member["family"] for member in members}),
                "representative_candidates": sorted(
                    {member["candidate_name"] for member in members}
                )[:16],
            }
        )

    region_groups.sort(
        key=lambda group: (-group["member_count"], group["ram_ptr_hex"], group["size"])
    )

    return {
        "generated_from": "processed/inventory/inventory.sqlite",
        "exact_duplicate_group_count": len(exact_groups),
        "region_cluster_count": len(region_groups),
        "exact_duplicate_groups": exact_groups,
        "region_clusters": region_groups,
    }


def render_markdown(clusters: dict[str, Any]) -> str:
    lines = [
        "# Overlay Clusters",
        "",
        "Machine-generated duplicate and region clustering for code-bearing EMI overlay candidates.",
        "",
        f"- Exact duplicate groups: {clusters['exact_duplicate_group_count']}",
        f"- Region clusters: {clusters['region_cluster_count']}",
        "",
        "## Largest Exact Duplicate Groups",
        "",
    ]

    for group in clusters["exact_duplicate_groups"][:20]:
        families = ", ".join(group["families"])
        loads = ", ".join(group["load_addresses"])
        lines.append(
            f"- `{group['representative']}`: {group['group_size']} copies across [{families}] at [{loads}]"
        )

    lines.extend(
        [
            "",
            "## Largest Region Clusters",
            "",
        ]
    )

    for group in clusters["region_clusters"][:20]:
        families = ", ".join(group["families"])
        lines.append(
            f"- `{group['ram_ptr_hex']}` size `{group['size']}`: {group['member_count']} members, "
            f"{group['distinct_hash_count']} distinct payload hashes, families [{families}]"
        )

    return "\n".join(lines) + "\n"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("inventory", "overlay-clusters"),
        description="Group duplicate overlay candidates by exact payload hash and load region.",
    )
    add_logging_args(parser)
    parser.add_argument("--catalog", type=Path, default=INVENTORY_SQLITE)
    parser.add_argument(
        "--json-out", type=Path, default=None, help="optional JSON output"
    )
    parser.add_argument(
        "--md-out", type=Path, default=None, help="optional Markdown output"
    )
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "overlay_clusters")
    ensure_output_parents(args.json_out, args.md_out)
    if not args.catalog.exists():
        logger.error(f"catalog not found: {args.catalog}")
        return 1
    if args.catalog.name == "inventory.sqlite" or args.catalog.suffix == ".sqlite":
        connection = connect_inventory_database(args.catalog)
        ensure_inventory_schema(connection)
        clusters = OverlayRepository(connection).build_clusters()
        connection.close()
    else:
        clusters = build_clusters(load_candidates(args.catalog))
    if args.json_out is not None:
        write_json_output(args.json_out, clusters)
    if args.md_out is not None:
        write_markdown_output(args.md_out, render_markdown(clusters))
    emit_output_summary(
        logger,
        summary=(
            f"overlay exact_groups={clusters['exact_duplicate_group_count']} "
            f"region_clusters={clusters['region_cluster_count']}"
        ),
        json_path=args.json_out,
        md_path=args.md_out,
    )
    return 0
