from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path
from typing import Any

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


def candidate_sort_key(candidate: dict[str, Any]) -> tuple[Any, ...]:
    return (
        candidate["family"],
        candidate["archive_id"],
        candidate["entry_index"],
        candidate["candidate_name"],
    )


def member_record(candidate: dict[str, Any]) -> dict[str, Any]:
    record = {
        "candidate_name": candidate["candidate_name"],
        "archive_id": candidate["archive_id"],
        "entry_index": candidate["entry_index"],
        "family": candidate["family"],
        "payload_path": candidate["payload_path"],
        "ram_ptr_hex": candidate["ram_ptr_hex"],
        "sha256": candidate["sha256"],
        "size": candidate["size"],
    }
    if "emi_path" in candidate:
        record["emi_path"] = candidate["emi_path"]
    return record


def build_unique_overlay_map(
    candidates_catalog: dict[str, Any], clusters_catalog: dict[str, Any]
) -> dict[str, Any]:
    candidates = candidates_catalog["candidates"]
    candidates_by_name = {
        candidate["candidate_name"]: candidate for candidate in candidates
    }
    by_sha: dict[str, list[dict[str, Any]]] = defaultdict(list)

    for candidate in candidates:
        by_sha[candidate["sha256"]].append(candidate)

    representatives: list[dict[str, Any]] = []
    alias_map: dict[str, str] = {}

    seen_sha = set()
    for group in clusters_catalog["exact_duplicate_groups"]:
        sha256 = group["sha256"]
        group_candidates = sorted(by_sha[sha256], key=candidate_sort_key)
        representative = group_candidates[0]
        seen_sha.add(sha256)

        representatives.append(
            {
                "representative_name": representative["candidate_name"],
                "representative": member_record(representative),
                "sha256": sha256,
                "group_size": len(group_candidates),
                "families": sorted(
                    {candidate["family"] for candidate in group_candidates}
                ),
                "load_addresses": sorted(
                    {candidate["ram_ptr_hex"] for candidate in group_candidates}
                ),
                "members": [member_record(candidate) for candidate in group_candidates],
            }
        )

        for candidate in group_candidates:
            alias_map[candidate["candidate_name"]] = representative["candidate_name"]

    for sha256, group_candidates in by_sha.items():
        if sha256 in seen_sha:
            continue

        group_candidates = sorted(group_candidates, key=candidate_sort_key)
        representative = group_candidates[0]
        representatives.append(
            {
                "representative_name": representative["candidate_name"],
                "representative": member_record(representative),
                "sha256": sha256,
                "group_size": 1,
                "families": [representative["family"]],
                "load_addresses": [representative["ram_ptr_hex"]],
                "members": [member_record(representative)],
            }
        )
        alias_map[representative["candidate_name"]] = representative["candidate_name"]

    representatives.sort(
        key=lambda group: (
            -group["group_size"],
            group["representative"]["family"],
            group["representative"]["archive_id"],
            group["representative"]["entry_index"],
        )
    )

    return {
        "generated_from": {"inventory": "processed/inventory/inventory.sqlite"},
        "candidate_count": len(candidates),
        "representative_count": len(representatives),
        "exact_duplicate_representative_count": sum(
            1 for group in representatives if group["group_size"] > 1
        ),
        "singleton_representative_count": sum(
            1 for group in representatives if group["group_size"] == 1
        ),
        "representatives": representatives,
        "alias_to_representative": alias_map,
        "unmapped_candidate_count": sum(
            1 for name in candidates_by_name if name not in alias_map
        ),
    }


def render_markdown(unique_map: dict[str, Any]) -> str:
    lines = [
        "# Unique Overlay Map",
        "",
        "Machine-generated mapping from overlay candidates to one representative payload per exact-duplicate group.",
        "",
        f"- Candidate count: {unique_map['candidate_count']}",
        f"- Representative count: {unique_map['representative_count']}",
        f"- Exact duplicate representatives: {unique_map['exact_duplicate_representative_count']}",
        f"- Singleton representatives: {unique_map['singleton_representative_count']}",
        f"- Unmapped candidates: {unique_map['unmapped_candidate_count']}",
        "",
        "## Largest Representative Groups",
        "",
    ]

    for group in unique_map["representatives"][:30]:
        representative = group["representative"]
        families = ", ".join(group["families"])
        loads = ", ".join(group["load_addresses"])
        lines.append(
            f"- `{group['representative_name']}`: {group['group_size']} members, "
            f"families [{families}], loads [{loads}], source `{representative['archive_id']}`"
        )

    return "\n".join(lines) + "\n"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("inventory", "unique-overlay-map"),
        description="Choose representative overlays from the candidate and cluster inventories.",
    )
    add_logging_args(parser)
    parser.add_argument("--candidates", type=Path, default=INVENTORY_SQLITE)
    parser.add_argument("--clusters", type=Path, default=INVENTORY_SQLITE)
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
    logger = logger_from_args(args, "unique_overlay_map")
    ensure_output_parents(args.json_out, args.md_out)
    if not args.candidates.exists():
        logger.error(f"candidates file not found: {args.candidates}")
        return 1
    if not args.clusters.exists():
        logger.error(f"clusters file not found: {args.clusters}")
        return 1
    connection = connect_inventory_database(args.candidates)
    ensure_inventory_schema(connection)
    unique_map = OverlayRepository(connection).build_unique_overlay_map()
    connection.close()
    if args.json_out is not None:
        write_json_output(args.json_out, unique_map)
    if args.md_out is not None:
        write_markdown_output(args.md_out, render_markdown(unique_map))
    emit_output_summary(
        logger,
        summary=(
            f"overlay representatives={unique_map['representative_count']} "
            f"candidates={unique_map['candidate_count']}"
        ),
        json_path=args.json_out,
        md_path=args.md_out,
    )
    return 0
