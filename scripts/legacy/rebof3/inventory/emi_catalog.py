from __future__ import annotations

import argparse
import hashlib
from collections import Counter
from pathlib import Path
from typing import Any

from ..assets.emi_archive import EmiArchive
from ..config import ROOT
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


EXTRACT_BIN_DIR = ROOT / "build" / "extracted" / "BIN"

def file_sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def family_from_emi_path(emi_path: Path) -> str:
    rel_parts = emi_path.relative_to(EXTRACT_BIN_DIR).parts
    if not rel_parts:
        return "unknown"
    return rel_parts[0]


def entry_record(emi_path: Path, entry: Any, payload: bytes) -> dict[str, Any]:
    archive_id = (
        emi_path.relative_to(ROOT / "build" / "extracted").with_suffix("").as_posix()
    )
    archive_name = emi_path.stem
    family = family_from_emi_path(emi_path)
    type_id = entry.type_id
    ram_ptr = entry.load_arg
    size = entry.size
    first4 = entry.first_word
    entry_index = entry.index

    audio_bundle_id: int | None = None
    if type_id in {6, 7, 8, 10} and 0 <= ram_ptr < 0x100:
        audio_bundle_id = ram_ptr

    return {
        "archive_id": archive_id,
        "archive_name": archive_name,
        "entry_index": entry_index,
        "emi_path": emi_path.relative_to(ROOT).as_posix(),
        "entry_name": entry.default_name,
        "family": family,
        "first4": first4,
        "image_candidate": type_id == 3,
        "audio_bundle_id": audio_bundle_id,
        "code_candidate": type_id in {0, 1} and ram_ptr >= 0x80000000,
        "palette_candidate": type_id == 0
        and 0x80033000 <= ram_ptr <= 0x8003AFFF
        and size in {0x40, 0x200, 0x400, 0x1000},
        "payload_path": f"{emi_path.relative_to(ROOT).as_posix()}#{entry_index}",
        "ram_ptr": ram_ptr,
        "ram_ptr_hex": f"0x{ram_ptr:08x}",
        "sector_count": (size + 0x7FF) >> 11,
        "sha256": file_sha256_bytes(payload),
        "size": size,
        "type": type_id,
    }


def build_catalog() -> dict[str, Any]:
    entries: list[dict[str, Any]] = []
    type_counts: Counter[int] = Counter()
    family_counts: Counter[str] = Counter()
    code_candidates = 0

    for emi_path in sorted(EXTRACT_BIN_DIR.rglob("*.EMI")):
        archive = EmiArchive(emi_path)
        for entry in archive.entries:
            record = entry_record(emi_path, entry, archive.payload(entry.index))
            entries.append(record)
            type_counts[record["type"]] += 1
            family_counts[record["family"]] += 1
            if record["code_candidate"]:
                code_candidates += 1

    return {
        "generated_from": EXTRACT_BIN_DIR.relative_to(ROOT).as_posix(),
        "entry_count": len(entries),
        "code_candidate_count": code_candidates,
        "type_counts": dict(sorted(type_counts.items())),
        "family_counts": dict(sorted(family_counts.items())),
        "entries": entries,
    }


def render_markdown(catalog: dict[str, Any]) -> str:
    lines = [
        "# EMI Catalog",
        "",
        "Machine-generated inventory of every EMI entry scanned directly from original archives under `build/extracted/BIN/`.",
        "",
        f"- Entry count: {catalog['entry_count']}",
        f"- Code candidates: {catalog['code_candidate_count']}",
        "",
        "## Entry Types",
        "",
    ]

    for type_id, count in catalog["type_counts"].items():
        lines.append(f"- type `{type_id}`: {count}")

    lines.extend(
        [
            "",
            "## Families",
            "",
        ]
    )

    for family, count in catalog["family_counts"].items():
        lines.append(f"- {family}: {count}")

    lines.extend(
        [
            "",
            "## Representative Code Candidates",
            "",
        ]
    )

    shown = 0
    for entry in catalog["entries"]:
        if not entry["code_candidate"]:
            continue
        lines.append(
            "- "
            f"`{entry['archive_id']}/{entry['entry_name']}` "
            f"-> `{entry['ram_ptr_hex']}` "
            f"(size `{entry['size']}`, type `{entry['type']}`)"
        )
        shown += 1
        if shown == 30:
            break

    return "\n".join(lines) + "\n"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("inventory", "emi-catalog"),
        description="Catalog EMI archives and per-entry payload metadata from `build/extracted/BIN`.",
    )
    add_logging_args(parser)
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
    logger = logger_from_args(args, "emi_catalog")
    ensure_output_parents(args.json_out, args.md_out)
    catalog = build_catalog()
    connection = connect_inventory_database(args.db)
    ensure_inventory_schema(connection)
    archives = ArchiveRepository(connection)
    for entry in catalog["entries"]:
        archives.upsert_archive(
            InventoryArchiveRow(
                archive_id=str(entry["archive_id"]),
                archive_name=str(entry["archive_name"]),
                family=str(entry["family"]),
                emi_path=str(entry["emi_path"]),
            )
        )
        archives.upsert_entry(
            InventoryEmiEntryRow(
                archive_id=str(entry["archive_id"]),
                entry_index=int(entry["entry_index"]),
                size=int(entry["size"]),
                family=str(entry["family"]),
                entry_name=str(entry["entry_name"]),
                type_id=int(entry["type"]),
                load_arg=int(entry["ram_ptr"]),
                first_word=int(entry["first4"]),
                sha256=str(entry["sha256"]),
                payload_path=str(entry["payload_path"]),
                code_candidate=bool(entry["code_candidate"]),
                palette_candidate=bool(entry["palette_candidate"]),
            )
        )
    connection.close()
    if args.json_out is not None:
        write_json_output(args.json_out, catalog)
    if args.md_out is not None:
        write_markdown_output(args.md_out, render_markdown(catalog))
    emit_output_summary(
        logger,
        summary=(
            f"emi entries={catalog['entry_count']} "
            f"code_candidates={catalog['code_candidate_count']}"
        ),
        json_path=args.json_out,
        md_path=args.md_out,
    )
    return 0
