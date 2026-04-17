from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path

from .direct_overlay_catalog import DEFAULT_EMI_ROOT, build_catalog
from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import (
    emit_output_summary,
    ensure_output_parents,
    write_json_output,
    write_markdown_output,
)
from ..models.inventory import InventoryArchiveRow, InventoryEmiEntryRow
from .db.connection import connect_inventory_database
from .db.migrations import ensure_inventory_schema
from .layout import INVENTORY_SQLITE
from .repositories.archives import ArchiveRepository


def render_markdown(catalog: dict) -> str:
    family_counts = catalog["family_counts"]
    addr_counts = catalog["load_address_counts"]
    candidates = catalog["candidates"]

    lines = [
        "# Overlay Candidates",
        "",
        "Machine-generated list of code-bearing overlay candidates scanned directly from original EMI archives.",
        "",
        f"- Candidate count: {catalog['candidate_count']}",
        f"- Unique payload hashes: {catalog['unique_payload_hashes']}",
        f"- Source root: `{catalog['generated_from']}`",
        "",
        "## Families",
        "",
    ]

    for family, count in family_counts.items():
        lines.append(f"- {family}: {count}")

    lines.extend(
        [
            "",
            "## Load Address Clusters",
            "",
        ]
    )

    for address, count in list(addr_counts.items())[:20]:
        lines.append(f"- {address}: {count}")

    lines.extend(
        [
            "",
            "## Representative Candidates",
            "",
        ]
    )

    per_family = defaultdict(list)
    for candidate in candidates:
        per_family[candidate["family"]].append(candidate)

    for family in sorted(per_family):
        lines.append(f"### {family}")
        lines.append("")
        for candidate in per_family[family][:8]:
            lines.append(
                "- "
                f"`{candidate['candidate_name']}` "
                f"from `{candidate['payload_path']}` "
                f"at `{candidate['ram_ptr_hex']}` "
                f"(size `{candidate['size']}`, dup group `{candidate['duplicate_group_size']}`)"
            )
        lines.append("")

    return "\n".join(lines)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("inventory", "overlay-catalog"),
        description="Catalog code-bearing overlay candidates directly from original EMI archives.",
    )
    add_logging_args(parser)
    parser.add_argument("--emi-root", type=Path, default=DEFAULT_EMI_ROOT)
    parser.add_argument("--db", type=Path, default=INVENTORY_SQLITE)
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
    logger = logger_from_args(args, "overlay_catalog")
    ensure_output_parents(args.json_out, args.md_out)
    catalog = build_catalog(args.emi_root)
    connection = connect_inventory_database(args.db)
    ensure_inventory_schema(connection)
    archives = ArchiveRepository(connection)
    for candidate in catalog["candidates"]:
        if not {
            "archive_id",
            "archive_name",
            "family",
            "emi_path",
            "entry_index",
            "size",
            "ram_ptr",
            "payload_path",
        }.issubset(candidate):
            continue
        archives.upsert_archive(
            row=InventoryArchiveRow(
                archive_id=str(candidate["archive_id"]),
                archive_name=str(candidate["archive_name"]),
                family=str(candidate["family"]),
                emi_path=str(candidate["emi_path"]),
            )
        )
        archives.upsert_entry(
            row=InventoryEmiEntryRow(
                archive_id=str(candidate["archive_id"]),
                entry_index=int(candidate["entry_index"]),
                size=int(candidate["size"]),
                family=str(candidate["family"]),
                entry_name=(
                    None
                    if candidate.get("entry_name") is None
                    else str(candidate.get("entry_name"))
                ),
                type_id=0,
                load_arg=int(candidate["ram_ptr"]),
                first_word=(
                    None
                    if candidate.get("first4") is None
                    else int(candidate.get("first4"))
                ),
                sha256=str(candidate.get("sha256") or ""),
                payload_path=str(candidate["payload_path"]),
                code_candidate=True,
            )
        )
    connection.close()
    if args.json_out is not None:
        write_json_output(args.json_out, catalog)
    if args.md_out is not None:
        write_markdown_output(args.md_out, render_markdown(catalog))
    emit_output_summary(
        logger,
        summary=f"overlay candidates={catalog['candidate_count']}",
        json_path=args.json_out,
        md_path=args.md_out,
    )
    return 0
