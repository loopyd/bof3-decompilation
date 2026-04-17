from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import relative_to_root, run_command, write_json_output, write_text_output
from ..config import DEFAULT_MATCH_ROOT, DEFAULT_PSX_PROFILE, GHIDRA_MAIN_MODULE
from ..re.services.bootstrap.constants import DEFAULT_GHIDRA_HOME, DEFAULT_PROJECT_NAME
from ..re.services.bootstrap.fallback import ghidra_env
from ..re.services.bootstrap.project import (
    default_project_dir,
    ensure_project_marker,
    project_busy_message,
)
from ..re.services.metadata.capture import capture_into_inventory
from . import frontier_backlog as frontier_backlog_lib
from . import import_backlog as import_backlog_lib
from . import refresh as refresh_lib
from . import scoreboard as scoreboard_lib

DEFAULT_INVENTORY_DB = scoreboard_lib.DEFAULT_INVENTORY_DB


def default_output_paths(match_root: Path, profile: str) -> tuple[Path, Path]:
    output_dir = match_root / "_reports"
    slug = profile.replace("-", "_")
    return (
        output_dir / f"promote_wave_{slug}.json",
        output_dir / f"promote_wave_{slug}.log",
    )


def select_items(
    backlog_payload: dict[str, Any],
    *,
    families: list[str] | None,
    lanes: list[str] | None,
    limit: int | None,
    rank_min: int | None,
    rank_max: int | None,
    min_confidence: str,
) -> list[dict[str, Any]]:
    confidence_order = {"low": 0, "medium": 1, "high": 2}
    minimum_rank = confidence_order[min_confidence]
    family_filters = set(families or ())
    lane_filters = set(lanes or ())
    selected: list[dict[str, Any]] = []
    for item in backlog_payload.get("items") or []:
        if item.get("frontier_state") != "promotable_entry_labels":
            continue
        if family_filters and str(item.get("family") or "") not in family_filters:
            continue
        if lane_filters and str(item.get("lane") or "") not in lane_filters:
            continue
        rank = int(item.get("queue_rank") or 0)
        if rank_min is not None and rank < rank_min:
            continue
        if rank_max is not None and rank > rank_max:
            continue
        item_confidence = str(item.get("entry_table_confidence") or "low")
        if confidence_order.get(item_confidence, -1) < minimum_rank:
            continue
        selected.append(item)
        if limit is not None and len(selected) >= limit:
            break
    return selected


def build_promote_command(
    *,
    ghidra_home: Path,
    project_dir: Path,
    project_name: str,
    config_mode: str,
    min_confidence: str,
    noanalysis: bool,
    output_path: Path,
    selectors: list[str],
) -> list[str]:
    command = [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "plan",
        "labels",
        "--ghidra-home",
        str(ghidra_home),
        "--project-dir",
        str(project_dir),
        "--project-name",
        project_name,
        "--config-mode",
        config_mode,
        "--min-confidence",
        min_confidence,
        "--output",
        str(output_path),
        "--promote",
    ]
    if noanalysis:
        command.append("--noanalysis")
    else:
        command.append("--with-analysis")
    for selector in selectors:
        command.extend(["--program", selector])
    return command


def refresh_reports(
    *,
    inventory_db: Path,
    match_root: Path,
    source_root: Path,
    artifact_root: Path,
    refresh_status: bool = False,
    tracked_output: bool = False,
) -> dict[str, str]:
    refreshed = refresh_lib.refresh_outputs(
        inventory_db=inventory_db,
        match_root=match_root,
        source_root=source_root,
        artifact_root=artifact_root,
        profile=DEFAULT_PSX_PROFILE,
        tracked_output=tracked_output,
        refresh_reports=True,
        refresh_status=refresh_status,
    )
    return {name: relative_to_root(path) for name, path in refreshed.items()}


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "promote-wave"),
        description="Promote entry-label candidates into functions for zero-function imported overlays.",
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
    parser.add_argument("--project-dir", type=Path, default=default_project_dir())
    parser.add_argument("--project-name", default=DEFAULT_PROJECT_NAME)
    parser.add_argument(
        "--ghidra-home",
        type=Path,
        default=Path(os.environ.get("GHIDRA_HOME") or DEFAULT_GHIDRA_HOME),
    )
    parser.add_argument(
        "--config-mode", choices=("isolated", "user"), default="isolated"
    )
    parser.add_argument("--family", action="append")
    parser.add_argument("--lane", action="append")
    parser.add_argument("--limit", type=int, default=8)
    parser.add_argument("--rank-min", type=int)
    parser.add_argument("--rank-max", type=int)
    parser.add_argument(
        "--min-confidence", choices=("medium", "high"), default="medium"
    )
    parser.add_argument("--noanalysis", action="store_true")
    parser.add_argument("--output-json", type=Path)
    parser.add_argument("--log-path", type=Path)
    parser.add_argument("--refresh-reports", action="store_true")
    parser.add_argument("--refresh-status", action="store_true")
    parser.add_argument("--tracked-output", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_promote_wave")
    frontier_payload = frontier_backlog_lib.build_frontier_backlog_payload(
        inventory_db=args.inventory_db,
        match_root=args.match_root,
        source_root=args.source_root,
        artifact_root=args.artifact_root,
    )
    selected_items = select_items(
        frontier_payload,
        families=args.family,
        lanes=args.lane,
        limit=args.limit,
        rank_min=args.rank_min,
        rank_max=args.rank_max,
        min_confidence=args.min_confidence,
    )

    output_json, default_log_path = default_output_paths(
        args.match_root, DEFAULT_PSX_PROFILE
    )
    if args.output_json is not None:
        output_json = args.output_json
    log_path = args.log_path or default_log_path
    promote_output = args.project_dir / "promote_wave_result.json"
    selectors = [
        str(item.get("ghidra_program_selector") or "") for item in selected_items
    ]
    command = build_promote_command(
        ghidra_home=args.ghidra_home,
        project_dir=args.project_dir,
        project_name=args.project_name,
        config_mode=args.config_mode,
        min_confidence=args.min_confidence,
        noanalysis=bool(args.noanalysis),
        output_path=promote_output,
        selectors=selectors,
    )
    report: dict[str, Any] = {
        "generated_at": frontier_payload.get("generated_at"),
        "status": "planned" if args.dry_run else "pending",
        "selected_count": len(selected_items),
        "selected_items": selected_items,
        "selectors": selectors,
        "min_confidence": args.min_confidence,
        "command": command,
        "project_dir": str(args.project_dir),
        "project_name": args.project_name,
        "ghidra_home": str(args.ghidra_home),
        "log_path": relative_to_root(log_path),
        "output_path": relative_to_root(promote_output),
        "refresh_reports": bool(args.refresh_reports),
        "refresh_status": bool(args.refresh_status),
        "tracked_output": bool(args.tracked_output),
    }
    if not selected_items:
        report["status"] = "no_items_selected"
        write_json_output(output_json, report)
        logger.summary(f"selected=0 json={relative_to_root(output_json)}")
        return 0
    if args.dry_run:
        write_json_output(output_json, report)
        logger.summary(
            f"selected={len(selected_items)} json={relative_to_root(output_json)}"
        )
        return 0
    busy_message = project_busy_message(args.project_dir)
    if busy_message is not None:
        logger.error(busy_message)
        return 1
    if ensure_project_marker(args.project_dir, args.project_name) is None:
        logger.error(
            f"ghidra project not found under {args.project_dir}; run make ghidra_bootstrap first"
        )
        return 1
    result = run_command(
        command,
        cwd=scoreboard_lib.workspace_lib.ROOT,
        env=ghidra_env(args.ghidra_home),
        timeout=None,
    )
    write_text_output(
        log_path,
        (result.stdout or "") + ("" if not result.stderr else "\n" + result.stderr),
    )
    report["returncode"] = int(result.returncode)
    if result.returncode != 0:
        report["status"] = "promotion_failed"
        write_json_output(output_json, report)
        logger.error(f"promotion wave failed; see {relative_to_root(log_path)}")
        return result.returncode
    capture_report = capture_into_inventory(
        db_path=args.inventory_db,
        selectors=tuple(selectors),
        kind="function",
        project_dir=args.project_dir,
        project_name=args.project_name,
    )
    report["metadata_capture"] = {
        "canonical_program_count": capture_report.get("canonical_program_count"),
        "row_count": capture_report.get("row_count"),
        "persisted": capture_report.get("persisted"),
    }
    report["refreshed_reports"] = (
        refresh_reports(
            inventory_db=args.inventory_db,
            match_root=args.match_root,
            source_root=args.source_root,
            artifact_root=args.artifact_root,
            refresh_status=bool(args.refresh_status),
            tracked_output=bool(args.tracked_output),
        )
        if args.refresh_reports or args.refresh_status
        else None
    )
    report["status"] = "promoted"
    write_json_output(output_json, report)
    logger.summary(
        " ".join(
            [
                f"selected={len(selected_items)}",
                f"captured_programs={report['metadata_capture']['canonical_program_count']}",
                f"json={relative_to_root(output_json)}",
                f"log={relative_to_root(log_path)}",
            ]
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
