from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import (
    format_hex,
    parse_hexish,
    relative_to_root,
    write_json_output,
    write_text_output,
)
from ..inventory.db.connection import connect_inventory_database
from ..inventory.db.migrations import ensure_inventory_schema
from ..inventory.repositories.programs import ProgramRepository
from ..config import DEFAULT_MATCH_ROOT, DEFAULT_PSX_PROFILE
from . import import_backlog as import_backlog_lib
from . import report_refresh
from . import scoreboard as scoreboard_lib

DEFAULT_INVENTORY_DB = scoreboard_lib.DEFAULT_INVENTORY_DB


def default_output_paths(match_root: Path, profile: str) -> tuple[Path, Path]:
    output_dir = match_root / "_reports"
    slug = profile.replace("-", "_")
    return (
        output_dir / f"frontier_backlog_{slug}.json",
        output_dir / f"frontier_backlog_{slug}.tsv",
    )


def resolve_program_selectors(inventory_db: Path) -> dict[str, str]:
    connection = connect_inventory_database(inventory_db)
    ensure_inventory_schema(connection)
    repo = ProgramRepository(connection)
    try:
        rows = connection.execute(
            "SELECT program_path FROM programs ORDER BY program_path"
        ).fetchall()
        return {
            str(row["program_path"]): repo.resolve_program_selector(
                program_path=str(row["program_path"])
            )
            for row in rows
        }
    finally:
        connection.close()


def collect_seed_offset_counters(
    *,
    program_rows: list[dict[str, Any]],
    function_rows: list[dict[str, Any]],
    entry_by_key: dict[tuple[str, int], dict[str, Any]],
) -> tuple[dict[str, Counter[int]], dict[tuple[str, int], Counter[int]]]:
    program_by_path = {
        str(program.get("program_path") or ""): program for program in program_rows
    }
    duplicate_counters: dict[str, Counter[int]] = defaultdict(Counter)
    family_load_counters: dict[tuple[str, int], Counter[int]] = defaultdict(Counter)
    for row in function_rows:
        program_path = str(row.get("program_path") or "")
        program = program_by_path.get(program_path)
        if program is None:
            continue
        archive_id = program.get("archive_id")
        entry_index = program.get("entry_index")
        if archive_id in (None, "") or entry_index in (None, ""):
            continue
        entry = entry_by_key.get((str(archive_id), int(entry_index)))
        if not entry:
            continue
        load_arg = entry.get("load_arg")
        if load_arg in (None, ""):
            continue
        entry_value = parse_hexish(str(row.get("entry_hex") or row.get("entry") or "0"))
        offset = entry_value - int(load_arg)
        if offset < 0:
            continue
        duplicate_group_key = program.get("duplicate_group_key")
        if duplicate_group_key:
            duplicate_counters[str(duplicate_group_key)][offset] += 1
        family = str(program.get("family") or entry.get("family") or "UNKNOWN")
        family_load_counters[(family, int(load_arg))][offset] += 1
    return duplicate_counters, family_load_counters


def build_seed_candidates(
    *,
    family: str,
    load_arg: int | None,
    size: int | None,
    duplicate_group_key: str | None,
    duplicate_counters: dict[str, Counter[int]],
    family_load_counters: dict[tuple[str, int], Counter[int]],
    limit: int = 8,
) -> tuple[str, list[dict[str, Any]]]:
    max_address = None if size in (None, "", 0) else int(load_arg or 0) + int(size)

    def candidates_from_counter(
        counter: Counter[int], *, source: str
    ) -> list[dict[str, Any]]:
        candidates: list[dict[str, Any]] = []
        if load_arg in (None, ""):
            return candidates
        for offset, support_count in counter.most_common():
            address = int(load_arg) + int(offset)
            if max_address is not None and address >= max_address:
                continue
            candidates.append(
                {
                    "address": address,
                    "address_hex": format_hex(address),
                    "offset": int(offset),
                    "offset_hex": format_hex(int(offset)),
                    "support_count": int(support_count),
                    "source": source,
                }
            )
            if len(candidates) >= limit:
                break
        return candidates

    if duplicate_group_key:
        duplicate_candidates = candidates_from_counter(
            duplicate_counters.get(str(duplicate_group_key), Counter()),
            source="duplicate_peer_offsets",
        )
        if duplicate_candidates:
            return "duplicate_peer_offsets", duplicate_candidates

    if load_arg not in (None, ""):
        family_candidates = candidates_from_counter(
            family_load_counters.get((family, int(load_arg)), Counter()),
            source="family_load_peer_offsets",
        )
        if family_candidates:
            return "family_load_peer_offsets", family_candidates

    if load_arg in (None, ""):
        return "no_seed_available", []
    return (
        "load_base_only",
        [
            {
                "address": int(load_arg),
                "address_hex": format_hex(int(load_arg)),
                "offset": 0,
                "offset_hex": format_hex(0),
                "support_count": 1,
                "source": "load_base_only",
            }
        ],
    )


def build_frontier_backlog_payload(
    *,
    inventory_db: Path,
    match_root: Path,
    source_root: Path,
    artifact_root: Path,
) -> dict[str, Any]:
    scoreboard_payload = scoreboard_lib.build_scoreboard_payload(
        inventory_db=inventory_db,
        match_root=match_root,
        source_root=source_root,
        artifact_root=artifact_root,
    )
    entry_by_key = {
        (str(entry.get("archive_id") or ""), int(entry.get("entry_index") or 0)): entry
        for entry in scoreboard_payload.get("entries") or []
    }
    duplicate_counters, family_load_counters = collect_seed_offset_counters(
        program_rows=list(scoreboard_payload.get("programs") or []),
        function_rows=list(scoreboard_payload.get("functions") or []),
        entry_by_key=entry_by_key,
    )
    selectors_by_path = resolve_program_selectors(inventory_db)
    items: list[dict[str, Any]] = []
    for program in scoreboard_payload.get("programs") or []:
        archive_id = program.get("archive_id")
        entry_index = program.get("entry_index")
        if archive_id in (None, "") or entry_index in (None, ""):
            continue
        if int(program.get("function_count") or 0) != 0:
            continue
        key = (str(archive_id), int(entry_index))
        entry = entry_by_key.get(key, {})
        family = str(program.get("family") or entry.get("family") or "UNKNOWN")
        confidence = entry.get("entry_table_confidence")
        promotable = confidence in {"medium", "high"}
        seed_strategy, seed_candidates = build_seed_candidates(
            family=family,
            load_arg=None
            if entry.get("load_arg") in (None, "")
            else int(entry.get("load_arg")),
            size=None if entry.get("size") in (None, "") else int(entry.get("size")),
            duplicate_group_key=None
            if program.get("duplicate_group_key") in (None, "")
            else str(program.get("duplicate_group_key")),
            duplicate_counters=duplicate_counters,
            family_load_counters=family_load_counters,
        )
        items.append(
            {
                "program_path": program.get("program_path"),
                "program_name": program.get("program_name"),
                "family": family,
                "lane": import_backlog_lib.lane_for_family(family),
                "archive_id": archive_id,
                "entry_index": int(entry_index),
                "source_hint": program.get("source_hint"),
                "duplicate_group_key": program.get("duplicate_group_key"),
                "duplicate_group_size": int(program.get("duplicate_group_size") or 0),
                "entry_table_confidence": confidence,
                "ghidra_program_selector": selectors_by_path.get(
                    str(program.get("program_path") or "")
                ),
                "load_arg": entry.get("load_arg"),
                "size": entry.get("size"),
                "frontier_state": (
                    "promotable_entry_labels" if promotable else "manual_frontier"
                ),
                "seed_strategy": seed_strategy,
                "seed_candidates": seed_candidates,
                "seed_count": len(seed_candidates),
                "documented_priority": family
                in import_backlog_lib.DOCUMENTED_PRIORITY_FAMILIES,
                "family_priority": import_backlog_lib.family_priority(family),
            }
        )
    items.sort(
        key=lambda item: (
            0 if item["frontier_state"] == "promotable_entry_labels" else 1,
            0
            if item["seed_strategy"] == "duplicate_peer_offsets"
            else 1
            if item["seed_strategy"] == "family_load_peer_offsets"
            else 2,
            0 if item["entry_table_confidence"] == "high" else 1,
            item["family_priority"],
            -int(item["duplicate_group_size"]),
            str(item["program_path"] or ""),
        )
    )
    for index, item in enumerate(items, start=1):
        item["queue_rank"] = index

    family_rows: dict[str, dict[str, Any]] = {}
    for item in items:
        family = str(item.get("family") or "UNKNOWN")
        row = family_rows.get(family)
        if row is None:
            row = {
                "family": family,
                "lane": import_backlog_lib.lane_for_family(family),
                "zero_function_programs": 0,
                "promotable_programs": 0,
                "manual_frontier_programs": 0,
                "high_confidence_programs": 0,
                "duplicate_peer_seed_programs": 0,
                "family_load_seed_programs": 0,
                "load_base_only_programs": 0,
                "largest_duplicate_group": 0,
            }
            family_rows[family] = row
        row["zero_function_programs"] += 1
        row["promotable_programs"] += int(
            item["frontier_state"] == "promotable_entry_labels"
        )
        row["manual_frontier_programs"] += int(
            item["frontier_state"] == "manual_frontier"
        )
        row["high_confidence_programs"] += int(
            item.get("entry_table_confidence") == "high"
        )
        row["duplicate_peer_seed_programs"] += int(
            item.get("seed_strategy") == "duplicate_peer_offsets"
        )
        row["family_load_seed_programs"] += int(
            item.get("seed_strategy") == "family_load_peer_offsets"
        )
        row["load_base_only_programs"] += int(
            item.get("seed_strategy") == "load_base_only"
        )
        row["largest_duplicate_group"] = max(
            int(row["largest_duplicate_group"]),
            int(item.get("duplicate_group_size") or 0),
        )

    summary = {
        "zero_function_programs": len(items),
        "promotable_programs": sum(
            1 for item in items if item["frontier_state"] == "promotable_entry_labels"
        ),
        "manual_frontier_programs": sum(
            1 for item in items if item["frontier_state"] == "manual_frontier"
        ),
        "high_confidence_programs": sum(
            1 for item in items if item.get("entry_table_confidence") == "high"
        ),
        "duplicate_peer_seed_programs": sum(
            1 for item in items if item.get("seed_strategy") == "duplicate_peer_offsets"
        ),
        "family_load_seed_programs": sum(
            1
            for item in items
            if item.get("seed_strategy") == "family_load_peer_offsets"
        ),
        "load_base_only_programs": sum(
            1 for item in items if item.get("seed_strategy") == "load_base_only"
        ),
    }
    return {
        "generated_at": scoreboard_payload.get("generated_at"),
        "inventory_db": str(inventory_db),
        "match_root": str(match_root),
        "source_root": str(source_root),
        "summary": summary,
        "families": sorted(
            family_rows.values(),
            key=lambda row: (
                row["zero_function_programs"],
                -row["promotable_programs"],
                row["family"],
            ),
            reverse=True,
        ),
        "items": items,
    }


def render_tsv(items: list[dict[str, Any]]) -> str:
    header = [
        "queue_rank",
        "family",
        "lane",
        "program_path",
        "archive_id",
        "entry_index",
        "frontier_state",
        "seed_strategy",
        "seed_count",
        "seed_addresses",
        "entry_table_confidence",
        "duplicate_group_size",
        "ghidra_program_selector",
    ]
    lines = ["\t".join(header)]
    for item in items:
        lines.append(
            "\t".join(
                [
                    str(item.get("queue_rank") or ""),
                    str(item.get("family") or ""),
                    str(item.get("lane") or ""),
                    str(item.get("program_path") or ""),
                    str(item.get("archive_id") or ""),
                    str(item.get("entry_index") or ""),
                    str(item.get("frontier_state") or ""),
                    str(item.get("seed_strategy") or ""),
                    str(item.get("seed_count") or ""),
                    ",".join(
                        str(candidate.get("address_hex") or "")
                        for candidate in item.get("seed_candidates") or []
                    ),
                    str(item.get("entry_table_confidence") or ""),
                    str(item.get("duplicate_group_size") or ""),
                    str(item.get("ghidra_program_selector") or ""),
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_brief_rows(items: list[dict[str, Any]], *, limit: int = 5) -> list[str]:
    lines: list[str] = []
    for item in items[: max(limit, 0)]:
        lines.append(
            f"#{item.get('queue_rank') or '?'} {item.get('program_path') or '<unknown>'}: "
            f"{item.get('frontier_state') or 'unknown'}, "
            f"strategy {item.get('seed_strategy') or 'unknown'}, "
            f"seeds {item.get('seed_count') or 0}"
        )
    return lines


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "frontier-backlog"),
        description="Report imported zero-function overlays and promotable frontier candidates.",
    )
    add_logging_args(parser)
    parser.add_argument("--inventory-db", type=Path, default=DEFAULT_INVENTORY_DB)
    parser.add_argument("--match-root", type=Path, default=DEFAULT_MATCH_ROOT)
    parser.add_argument(
        "--source-root",
        type=Path,
        default=scoreboard_lib.DEFAULT_SOURCE_ROOT,
    )
    parser.add_argument(
        "--artifact-root",
        type=Path,
        default=scoreboard_lib.workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
    )
    parser.add_argument("--output-json", type=Path)
    parser.add_argument("--output-tsv", type=Path)
    parser.add_argument("--refresh-status", action="store_true")
    parser.add_argument("--tracked-output", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_frontier_backlog")
    payload = build_frontier_backlog_payload(
        inventory_db=args.inventory_db,
        match_root=args.match_root,
        source_root=args.source_root,
        artifact_root=args.artifact_root,
    )
    output_json, output_tsv = default_output_paths(args.match_root, DEFAULT_PSX_PROFILE)
    if args.output_json is not None:
        output_json = args.output_json
    if args.output_tsv is not None:
        output_tsv = args.output_tsv
    write_json_output(output_json, payload)
    write_text_output(output_tsv, render_tsv(payload["items"]))
    logger.summary(
        " ".join(
            [
                f"zero_function_programs={payload['summary']['zero_function_programs']}",
                f"promotable={payload['summary']['promotable_programs']}",
                f"json={relative_to_root(output_json)}",
                f"tsv={relative_to_root(output_tsv)}",
            ]
        )
    )
    if args.refresh_status:
        status_root = report_refresh.refresh_status_snapshot(
            profile=DEFAULT_PSX_PROFILE,
            tracked_output=bool(args.tracked_output),
            inventory_db=args.inventory_db,
            match_root=args.match_root,
            source_root=args.source_root,
            artifact_root=args.artifact_root,
        )
        logger.item(f"status {relative_to_root(status_root)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
