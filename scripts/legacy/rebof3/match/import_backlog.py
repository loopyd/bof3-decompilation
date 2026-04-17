from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import relative_to_root, write_json_output, write_text_output
from ..config import DEFAULT_MATCH_ROOT, DEFAULT_PSX_PROFILE
from . import report_refresh
from . import scoreboard as scoreboard_lib

MANUAL_REVIEW_ARCHIVES = {"AFLDKWA", "DEMO", "FIRST"}
FAMILY_PRIORITIES = {
    "ETC": 0,
    "BATTLE": 1,
    "SCENARIO": 2,
    "BMAGIC": 3,
    "BPLCHAR": 4,
    "BOSS": 5,
    "WORLD00": 6,
    "WORLD01": 7,
    "WORLD02": 8,
    "WORLD03": 9,
    "WORLD04": 10,
    "PLCHAR": 11,
}
DOCUMENTED_PRIORITY_FAMILIES = {"ETC", "BATTLE", "SCENARIO"}


def default_output_paths(match_root: Path, profile: str) -> tuple[Path, Path]:
    output_dir = match_root / "_reports"
    slug = profile.replace("-", "_")
    return (
        output_dir / f"import_backlog_{slug}.json",
        output_dir / f"import_backlog_{slug}.tsv",
    )


def parse_duplicate_group_key(group_key: str | None) -> tuple[str | None, int | None]:
    if not group_key:
        return None, None
    archive_id, _, entry_text = str(group_key).rpartition("#")
    if not archive_id or not entry_text:
        return None, None
    try:
        return archive_id, int(entry_text, 0)
    except ValueError:
        return None, None


def archive_stem(archive_id: str | None) -> str:
    if not archive_id:
        return ""
    return str(archive_id).split("/")[-1].upper()


def requires_manual_review(entry_row: dict[str, Any]) -> bool:
    return (
        archive_stem(str(entry_row.get("archive_id") or "")) in MANUAL_REVIEW_ARCHIVES
    )


def lane_for_family(family: str) -> str:
    family_text = str(family or "UNKNOWN")
    if family_text in {"ETC", "SCENARIO"}:
        return "system_script"
    if family_text in {"BATTLE", "BOSS", "BMAGIC"}:
        return "battle_runtime"
    if family_text.startswith("WORLD"):
        return "world_runtime"
    if family_text in {"BPLCHAR", "PLCHAR"}:
        return "character_runtime"
    return "shared_reporting"


def family_priority(family: str) -> int:
    return FAMILY_PRIORITIES.get(str(family or "UNKNOWN"), 99)


def suggested_folder(archive_id: str | None) -> str | None:
    if not archive_id:
        return None
    return f"bins/{archive_id}"


def build_group_index(entry_rows: list[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    grouped: dict[str, dict[str, Any]] = {}
    for row in entry_rows:
        group_key = str(row.get("duplicate_group_key") or "")
        if not group_key:
            archive_id = str(row.get("archive_id") or "")
            entry_index = int(row.get("entry_index") or 0)
            group_key = f"{archive_id}#{entry_index}"
        summary = grouped.get(group_key)
        if summary is None:
            rep_archive_id, rep_entry_index = parse_duplicate_group_key(group_key)
            summary = {
                "duplicate_group_key": group_key,
                "representative_archive_id": rep_archive_id,
                "representative_entry_index": rep_entry_index,
                "duplicate_group_size": int(row.get("duplicate_group_size") or 0),
                "families": set(),
                "missing_entries": 0,
                "imported_program_count": 0,
                "missing_archives": [],
            }
            grouped[group_key] = summary
        summary["duplicate_group_size"] = max(
            int(summary["duplicate_group_size"]),
            int(row.get("duplicate_group_size") or 0),
        )
        summary["families"].add(str(row.get("family") or "UNKNOWN"))
        summary["imported_program_count"] += int(row.get("imported_program_count") or 0)
        if row.get("entry_state") == "candidate_missing_program":
            summary["missing_entries"] += 1
            summary["missing_archives"].append(
                (str(row.get("archive_id") or ""), int(row.get("entry_index") or 0))
            )
    for summary in grouped.values():
        summary["group_seeded_anywhere"] = int(summary["imported_program_count"]) > 0
        families = sorted(summary["families"])
        summary["families"] = families
        representative_family = families[0] if families else "UNKNOWN"
        if representative_family == "BOSS" and "BATTLE" in families:
            representative_family = "BATTLE"
        if representative_family.startswith("WORLD") and "WORLD00" in families:
            representative_family = "WORLD00"
        summary["representative_family"] = representative_family
        summary["lane"] = lane_for_family(representative_family)
    return grouped


def build_reason_tags(
    entry_row: dict[str, Any],
    group_summary: dict[str, Any],
    *,
    is_representative: bool,
    manual_review: bool,
) -> list[str]:
    tags: list[str] = []
    if str(entry_row.get("family") or "UNKNOWN") in DOCUMENTED_PRIORITY_FAMILIES:
        tags.append("documented_family")
    if int(group_summary.get("duplicate_group_size") or 0) > 1:
        tags.append("duplicate_group")
    if int(group_summary.get("duplicate_group_size") or 0) > 1 and is_representative:
        tags.append("representative_entry")
    if bool(group_summary.get("group_seeded_anywhere")):
        tags.append("group_seeded_anywhere")
    if manual_review:
        tags.append("manual_review")
    confidence = entry_row.get("entry_table_confidence")
    if confidence not in (None, ""):
        tags.append(f"entry_table_{confidence}")
    return tags


def build_item_sort_key(
    item: dict[str, Any],
) -> tuple[int, int, int, int, int, str, int]:
    item_state = str(item.get("item_state") or "")
    state_rank = {
        "queued": 0,
        "deferred_group_member": 1,
        "manual_review": 2,
    }.get(item_state, 9)
    return (
        state_rank,
        0 if bool(item.get("documented_priority")) else 1,
        0 if not bool(item.get("group_seeded_anywhere")) else 1,
        0 if bool(item.get("is_representative")) else 1,
        -int(item.get("duplicate_group_size") or 0),
        str(item.get("archive_id") or ""),
        int(item.get("entry_index") or 0),
    )


def build_import_backlog_payload(scoreboard_payload: dict[str, Any]) -> dict[str, Any]:
    entry_rows = list(scoreboard_payload.get("entries") or [])
    missing_entries = [
        row
        for row in entry_rows
        if row.get("entry_state") == "candidate_missing_program"
    ]
    group_index = build_group_index(entry_rows)
    queued_items: list[dict[str, Any]] = []
    deferred_items: list[dict[str, Any]] = []
    group_rows: dict[str, dict[str, Any]] = {}

    for entry_row in missing_entries:
        archive_id = str(entry_row.get("archive_id") or "")
        entry_index = int(entry_row.get("entry_index") or 0)
        group_key = str(
            entry_row.get("duplicate_group_key") or f"{archive_id}#{entry_index}"
        )
        group_summary = group_index[group_key]
        rep_archive_id, rep_entry_index = parse_duplicate_group_key(group_key)
        is_representative = rep_archive_id is None or (
            archive_id == rep_archive_id and entry_index == rep_entry_index
        )
        manual_review = requires_manual_review(entry_row)
        if manual_review:
            item_state = "manual_review"
            recommended_action = "manual_review"
        elif (
            not bool(group_summary.get("group_seeded_anywhere"))
            and not is_representative
        ):
            item_state = "deferred_group_member"
            recommended_action = "defer_until_representative_imported"
        elif bool(group_summary.get("group_seeded_anywhere")):
            item_state = "queued"
            recommended_action = "import_member"
        else:
            item_state = "queued"
            recommended_action = "import_representative"
        family = str(entry_row.get("family") or "UNKNOWN")
        representative_family = str(
            group_summary.get("representative_family") or family
        )
        lane = str(group_summary.get("lane") or lane_for_family(representative_family))
        reason_tags = build_reason_tags(
            entry_row,
            group_summary,
            is_representative=is_representative,
            manual_review=manual_review,
        )
        item = {
            "archive_id": archive_id,
            "entry_index": entry_index,
            "family": family,
            "lane": lane,
            "payload_path": entry_row.get("payload_path"),
            "load_arg": entry_row.get("load_arg"),
            "sha256": entry_row.get("sha256"),
            "duplicate_group_key": group_key,
            "duplicate_group_size": int(group_summary.get("duplicate_group_size") or 0),
            "representative_archive_id": rep_archive_id,
            "representative_entry_index": rep_entry_index,
            "representative_family": representative_family,
            "group_seeded_anywhere": bool(group_summary.get("group_seeded_anywhere")),
            "group_imported_program_count": int(
                group_summary.get("imported_program_count") or 0
            ),
            "group_missing_entries": int(group_summary.get("missing_entries") or 0),
            "item_state": item_state,
            "recommended_action": recommended_action,
            "is_representative": bool(is_representative),
            "manual_review": bool(manual_review),
            "documented_priority": family in DOCUMENTED_PRIORITY_FAMILIES,
            "family_priority": family_priority(representative_family),
            "entry_table_confidence": entry_row.get("entry_table_confidence"),
            "suggested_folder": suggested_folder(archive_id),
            "reason_tags": reason_tags,
        }
        if item_state == "queued":
            queued_items.append(item)
        else:
            deferred_items.append(item)
        group_row = group_rows.get(group_key)
        if group_row is None:
            group_row = {
                "duplicate_group_key": group_key,
                "representative_archive_id": rep_archive_id,
                "representative_entry_index": rep_entry_index,
                "representative_family": representative_family,
                "lane": lane,
                "duplicate_group_size": int(
                    group_summary.get("duplicate_group_size") or 0
                ),
                "missing_entries": int(group_summary.get("missing_entries") or 0),
                "group_seeded_anywhere": bool(
                    group_summary.get("group_seeded_anywhere")
                ),
                "imported_program_count": int(
                    group_summary.get("imported_program_count") or 0
                ),
                "queued_entries": 0,
                "deferred_entries": 0,
                "manual_review_entries": 0,
            }
            group_rows[group_key] = group_row
        if item_state == "queued":
            group_row["queued_entries"] += 1
        elif item_state == "manual_review":
            group_row["manual_review_entries"] += 1
        else:
            group_row["deferred_entries"] += 1

    queued_items.sort(key=build_item_sort_key)
    deferred_items.sort(key=build_item_sort_key)
    for index, item in enumerate(queued_items, start=1):
        item["queue_rank"] = index

    family_rows_map: dict[str, dict[str, Any]] = defaultdict(
        lambda: {
            "missing_program_entries": 0,
            "queued_entries": 0,
            "deferred_group_members": 0,
            "manual_review_entries": 0,
            "unseeded_groups": set(),
            "unseeded_multi_groups": set(),
            "seeded_followup_groups": set(),
            "largest_duplicate_group": 0,
        }
    )
    for item in queued_items + deferred_items:
        family = str(item.get("family") or "UNKNOWN")
        summary = family_rows_map[family]
        summary["missing_program_entries"] += 1
        summary["queued_entries"] += int(item.get("item_state") == "queued")
        summary["deferred_group_members"] += int(
            item.get("item_state") == "deferred_group_member"
        )
        summary["manual_review_entries"] += int(
            item.get("item_state") == "manual_review"
        )
        summary["largest_duplicate_group"] = max(
            int(summary["largest_duplicate_group"]),
            int(item.get("duplicate_group_size") or 0),
        )
        group_key = str(item.get("duplicate_group_key") or "")
        if not bool(item.get("group_seeded_anywhere")):
            summary["unseeded_groups"].add(group_key)
            if int(item.get("duplicate_group_size") or 0) > 1:
                summary["unseeded_multi_groups"].add(group_key)
        else:
            summary["seeded_followup_groups"].add(group_key)

    family_rows = []
    for family, summary in sorted(
        family_rows_map.items(), key=lambda item: (family_priority(item[0]), item[0])
    ):
        family_rows.append(
            {
                "family": family,
                "lane": lane_for_family(family),
                "missing_program_entries": int(summary["missing_program_entries"]),
                "queued_entries": int(summary["queued_entries"]),
                "deferred_group_members": int(summary["deferred_group_members"]),
                "manual_review_entries": int(summary["manual_review_entries"]),
                "unseeded_groups": len(summary["unseeded_groups"]),
                "unseeded_multi_groups": len(summary["unseeded_multi_groups"]),
                "seeded_followup_groups": len(summary["seeded_followup_groups"]),
                "largest_duplicate_group": int(summary["largest_duplicate_group"]),
            }
        )

    lane_heads: dict[str, dict[str, Any]] = {}
    for item in queued_items:
        lane = str(item.get("lane") or "shared_reporting")
        lane_heads.setdefault(
            lane,
            {
                "archive_id": item.get("archive_id"),
                "entry_index": item.get("entry_index"),
                "family": item.get("family"),
                "recommended_action": item.get("recommended_action"),
                "queue_rank": item.get("queue_rank"),
                "duplicate_group_key": item.get("duplicate_group_key"),
            },
        )

    summary = {
        "scoreboard_generated_at": scoreboard_payload.get("generated_at"),
        "missing_program_entries": len(missing_entries),
        "queued_items": len(queued_items),
        "deferred_group_members": sum(
            1
            for item in deferred_items
            if item.get("item_state") == "deferred_group_member"
        ),
        "manual_review_items": sum(
            1 for item in deferred_items if item.get("item_state") == "manual_review"
        ),
        "unseeded_groups": sum(
            1
            for group in group_rows.values()
            if not bool(group.get("group_seeded_anywhere"))
        ),
        "unseeded_multi_groups": sum(
            1
            for group in group_rows.values()
            if not bool(group.get("group_seeded_anywhere"))
            and int(group.get("duplicate_group_size") or 0) > 1
        ),
        "seeded_followup_groups": sum(
            1
            for group in group_rows.values()
            if bool(group.get("group_seeded_anywhere"))
        ),
        "lane_heads": lane_heads,
        "blocking_issues": list(
            scoreboard_payload.get("summary", {}).get("blocking_issues") or []
        ),
    }

    return {
        "generated_at": scoreboard_payload.get("generated_at"),
        "inventory_db": scoreboard_payload.get("inventory_db"),
        "match_root": scoreboard_payload.get("match_root"),
        "source_root": scoreboard_payload.get("source_root"),
        "summary": summary,
        "families": family_rows,
        "groups": sorted(
            group_rows.values(),
            key=lambda item: (
                0 if not bool(item.get("group_seeded_anywhere")) else 1,
                -int(item.get("duplicate_group_size") or 0),
                family_priority(str(item.get("representative_family") or "UNKNOWN")),
                str(item.get("representative_archive_id") or ""),
                int(item.get("representative_entry_index") or 0),
            ),
        ),
        "items": queued_items,
        "deferred_items": deferred_items,
    }


def render_tsv(items: list[dict[str, Any]]) -> str:
    header = [
        "queue_rank",
        "lane",
        "family",
        "archive_id",
        "entry_index",
        "payload_path",
        "recommended_action",
        "duplicate_group_key",
        "duplicate_group_size",
        "group_seeded_anywhere",
        "entry_table_confidence",
        "suggested_folder",
    ]
    lines = ["\t".join(header)]
    for item in items:
        lines.append(
            "\t".join(
                [
                    str(item.get("queue_rank") or ""),
                    str(item.get("lane") or ""),
                    str(item.get("family") or ""),
                    str(item.get("archive_id") or ""),
                    str(item.get("entry_index") or ""),
                    str(item.get("payload_path") or ""),
                    str(item.get("recommended_action") or ""),
                    str(item.get("duplicate_group_key") or ""),
                    str(item.get("duplicate_group_size") or ""),
                    str(item.get("group_seeded_anywhere") or ""),
                    str(item.get("entry_table_confidence") or ""),
                    str(item.get("suggested_folder") or ""),
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_brief_rows(items: list[dict[str, Any]], *, limit: int = 5) -> list[str]:
    lines: list[str] = []
    for item in items[: max(limit, 0)]:
        lines.append(
            f"#{item.get('queue_rank') or '?'} {item.get('family') or 'UNKNOWN'} "
            f"{item.get('archive_id') or ''}#{item.get('entry_index') or ''}: "
            f"lane {item.get('lane') or 'unknown'}, "
            f"action {item.get('recommended_action') or 'unknown'}"
        )
    return lines


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "import-backlog"),
        description="Build a representative-aware import backlog for missing EMI program rows.",
    )
    add_logging_args(parser)
    parser.add_argument(
        "--inventory-db",
        type=Path,
        default=scoreboard_lib.DEFAULT_INVENTORY_DB,
    )
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
    logger = logger_from_args(args, "match_import_backlog")
    if not args.inventory_db.exists():
        logger.error(f"inventory db not found: {args.inventory_db}")
        return 1
    scoreboard_payload = scoreboard_lib.build_scoreboard_payload(
        inventory_db=args.inventory_db,
        match_root=args.match_root,
        source_root=args.source_root,
        artifact_root=args.artifact_root,
    )
    payload = build_import_backlog_payload(scoreboard_payload)
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
                f"queued={len(payload['items'])}",
                f"deferred={len(payload['deferred_items'])}",
                f"manual_review={payload['summary']['manual_review_items']}",
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
