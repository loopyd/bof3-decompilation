from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path, PurePosixPath
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
from . import frontier_backlog as frontier_backlog_lib
from . import refresh as refresh_lib
from . import scoreboard as scoreboard_lib

DEFAULT_INVENTORY_DB = scoreboard_lib.DEFAULT_INVENTORY_DB


def default_output_paths(match_root: Path, profile: str) -> tuple[Path, Path]:
    output_dir = match_root / "_reports"
    slug = profile.replace("-", "_")
    return (
        output_dir / f"repair_wave_{slug}.json",
        output_dir / f"repair_wave_{slug}.log",
    )


def select_items(
    backlog_payload: dict[str, Any],
    *,
    families: list[str] | None,
    lanes: list[str] | None,
    seed_strategies: list[str] | None,
    limit: int | None,
    rank_min: int | None,
    rank_max: int | None,
) -> list[dict[str, Any]]:
    family_filters = set(families or ())
    lane_filters = set(lanes or ())
    strategy_filters = set(seed_strategies or ())
    selected: list[dict[str, Any]] = []
    for item in backlog_payload.get("items") or []:
        if family_filters and str(item.get("family") or "") not in family_filters:
            continue
        if lane_filters and str(item.get("lane") or "") not in lane_filters:
            continue
        if (
            strategy_filters
            and str(item.get("seed_strategy") or "") not in strategy_filters
        ):
            continue
        rank = int(item.get("queue_rank") or 0)
        if rank_min is not None and rank < rank_min:
            continue
        if rank_max is not None and rank > rank_max:
            continue
        if not str(item.get("source_hint") or ""):
            continue
        selected.append(item)
        if limit is not None and len(selected) >= limit:
            break
    return selected


def repair_folder(item: dict[str, Any]) -> str:
    program_path = str(item.get("program_path") or "")
    return str(PurePosixPath(program_path).parent).lstrip("/")


def build_import_command(
    *,
    ghidra_home: Path,
    project_dir: Path,
    project_name: str,
    config_mode: str,
    noanalysis: bool,
    source_hint: str,
    folder: str,
) -> list[str]:
    command = [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "binary",
        "import",
        source_hint,
        "--ghidra-home",
        str(ghidra_home),
        "--project-dir",
        str(project_dir),
        "--project-name",
        project_name,
        "--folder",
        folder,
        "--config-mode",
        config_mode,
    ]
    command.append("--noanalysis" if noanalysis else "--with-analysis")
    return command


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "repair-wave"),
        description="Re-import missing Ghidra programs for canonical overlay rows.",
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
    parser.add_argument(
        "--seed-strategy",
        action="append",
        choices=(
            "duplicate_peer_offsets",
            "family_load_peer_offsets",
            "load_base_only",
        ),
    )
    parser.add_argument("--limit", type=int, default=8)
    parser.add_argument("--rank-min", type=int)
    parser.add_argument("--rank-max", type=int)
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
    logger = logger_from_args(args, "match_repair_wave")
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
        seed_strategies=args.seed_strategy,
        limit=args.limit,
        rank_min=args.rank_min,
        rank_max=args.rank_max,
    )

    output_json, default_log_path = default_output_paths(
        args.match_root, DEFAULT_PSX_PROFILE
    )
    if args.output_json is not None:
        output_json = args.output_json
    log_path = args.log_path or default_log_path
    report: dict[str, Any] = {
        "generated_at": frontier_payload.get("generated_at"),
        "status": "planned" if args.dry_run else "pending",
        "selected_count": len(selected_items),
        "selected_items": selected_items,
        "families": list(args.family or []),
        "lanes": list(args.lane or []),
        "seed_strategies": list(args.seed_strategy or []),
        "project_dir": str(args.project_dir),
        "project_name": args.project_name,
        "ghidra_home": str(args.ghidra_home),
        "log_path": relative_to_root(log_path),
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

    item_results: list[dict[str, Any]] = []
    log_chunks: list[str] = []
    imported_count = 0
    for item in selected_items:
        source_hint = str(item.get("source_hint") or "")
        folder = repair_folder(item)
        command = build_import_command(
            ghidra_home=args.ghidra_home,
            project_dir=args.project_dir,
            project_name=args.project_name,
            config_mode=args.config_mode,
            noanalysis=bool(args.noanalysis),
            source_hint=source_hint,
            folder=folder,
        )
        result = run_command(
            command,
            cwd=scoreboard_lib.workspace_lib.ROOT,
            env=ghidra_env(args.ghidra_home),
            timeout=None,
        )
        log_chunks.append(
            "\n".join(
                [
                    f"## {item.get('program_path')} {source_hint}",
                    "$ " + " ".join(command),
                    result.stdout or "",
                    result.stderr or "",
                ]
            ).strip()
        )
        item_result = {
            "program_path": item.get("program_path"),
            "ghidra_program_selector": item.get("ghidra_program_selector"),
            "source_hint": source_hint,
            "folder": folder,
            "import_command": command,
            "returncode": int(result.returncode),
        }
        if result.returncode == 0:
            item_result["status"] = "imported"
            imported_count += 1
        else:
            item_result["status"] = "import_failed"
        item_results.append(item_result)

    write_text_output(
        log_path, "\n\n".join(chunk for chunk in log_chunks if chunk).strip() + "\n"
    )
    report["item_results"] = item_results
    report["imported_count"] = imported_count
    report["refreshed_reports"] = (
        {
            name: relative_to_root(path)
            for name, path in refresh_lib.refresh_outputs(
                inventory_db=args.inventory_db,
                match_root=args.match_root,
                source_root=args.source_root,
                artifact_root=args.artifact_root,
                profile=DEFAULT_PSX_PROFILE,
                tracked_output=bool(args.tracked_output),
                refresh_reports=True,
                refresh_status=bool(args.refresh_status),
            ).items()
        }
        if args.refresh_reports or args.refresh_status
        else None
    )
    failed_count = sum(1 for item in item_results if item.get("status") != "imported")
    report["status"] = "completed" if failed_count == 0 else "completed_with_failures"
    write_json_output(output_json, report)
    logger.summary(
        " ".join(
            [
                f"selected={len(selected_items)}",
                f"imported={imported_count}",
                f"json={relative_to_root(output_json)}",
                f"log={relative_to_root(log_path)}",
            ]
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
