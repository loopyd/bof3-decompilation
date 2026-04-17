from __future__ import annotations

import argparse
import importlib
import json
import os
import sys
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import relative_to_root, run_command, write_json_output, write_text_output
from ..config import (
    DEFAULT_MATCH_ROOT,
    DEFAULT_PSX_PROFILE,
    GHIDRA_MAIN_MODULE,
    GHIDRA_SRC_DIR,
)
from ..inventory.db.connection import connect_inventory_database
from ..inventory.db.migrations import ensure_inventory_schema
from ..inventory.repositories.programs import ProgramRepository
from ..models.inventory import InventoryProgramRow
from ..re.services.bootstrap.constants import DEFAULT_GHIDRA_HOME, DEFAULT_PROJECT_NAME
from ..re.services.bootstrap.fallback import ghidra_env
from ..re.services.bootstrap.project import (
    default_project_dir,
    ensure_project_marker,
    project_busy_message,
)
from ..re.services.metadata.capture import capture_into_inventory
from . import frontier_backlog as frontier_backlog_lib
from . import import_backlog as backlog_lib
from . import refresh as refresh_lib
from . import scoreboard as scoreboard_lib

DEFAULT_INVENTORY_DB = scoreboard_lib.DEFAULT_INVENTORY_DB


def default_output_paths(match_root: Path, profile: str) -> tuple[Path, Path]:
    output_dir = match_root / "_reports"
    slug = profile.replace("-", "_")
    return (
        output_dir / f"import_wave_{slug}.json",
        output_dir / f"import_wave_{slug}.log",
    )


def canonical_program_path(item: dict[str, Any]) -> str:
    archive_id = str(item.get("archive_id") or "").strip("/")
    entry_index = int(item.get("entry_index") or 0)
    return f"/bins/{archive_id}/{entry_index}.bin"


def ghidra_program_selector(import_row: dict[str, Any]) -> str:
    folder = "/" + str(import_row.get("project_folder_path") or "").strip("/")
    program_name = str(import_row.get("program_name") or "").strip()
    return f"{folder}/{program_name}".replace("//", "/")


def persist_imported_program_rows(
    *, db_path: Path, items: list[dict[str, Any]]
) -> list[str]:
    connection = connect_inventory_database(db_path)
    ensure_inventory_schema(connection)
    repo = ProgramRepository(connection)
    persisted_paths: list[str] = []
    try:
        for item in items:
            archive_id = str(item.get("archive_id") or "").strip("/")
            entry_index = int(item.get("entry_index") or 0)
            program_path = canonical_program_path(item)
            repo.upsert_program(
                InventoryProgramRow(
                    program_slug=scoreboard_lib.workspace_lib.slugify(program_path),
                    program_name=f"{entry_index}.bin",
                    program_path=program_path,
                    folder=f"/bins/{archive_id}",
                    source_hint=str(item.get("payload_path") or "") or None,
                )
            )
            persisted_paths.append(program_path)
    finally:
        connection.close()
    return persisted_paths


def select_items(
    backlog_payload: dict[str, Any],
    *,
    families: list[str] | None,
    lanes: list[str] | None,
    recommended_actions: list[str] | None,
    limit: int | None,
    rank_min: int | None,
    rank_max: int | None,
) -> list[dict[str, Any]]:
    selected: list[dict[str, Any]] = []
    family_filters = set(families or ())
    lane_filters = set(lanes or ())
    action_filters = set(recommended_actions or ())
    for item in backlog_payload.get("items") or []:
        if family_filters and str(item.get("family") or "") not in family_filters:
            continue
        if lane_filters and str(item.get("lane") or "") not in lane_filters:
            continue
        if (
            action_filters
            and str(item.get("recommended_action") or "") not in action_filters
        ):
            continue
        rank = int(item.get("queue_rank") or 0)
        if rank_min is not None and rank < rank_min:
            continue
        if rank_max is not None and rank > rank_max:
            continue
        selected.append(item)
        if limit is not None and len(selected) >= limit:
            break
    return selected


def import_bootstrap_types() -> tuple[Any, Any]:
    ghidra_src = str(GHIDRA_SRC_DIR)
    if ghidra_src not in sys.path:
        sys.path.insert(0, ghidra_src)
    binary_module = importlib.import_module("bof3_ghidra.commands.binary")
    models_module = importlib.import_module("bof3_ghidra.models")
    return binary_module, models_module


def build_manifest_payload(
    items: list[dict[str, Any]],
    *,
    project_dir: Path,
    project_name: str,
    noanalysis: bool,
    max_cpu: int | None,
    staging_dir: Path,
) -> dict[str, Any]:
    binary_module, models_module = import_bootstrap_types()
    requests = [
        models_module.BootstrapImportRequest(
            source=str(item.get("payload_path") or ""),
            display=str(item.get("payload_path") or ""),
            folder=str(item.get("suggested_folder") or ""),
            base_addr=None,
        )
        for item in items
    ]
    manifest = binary_module.BinaryImportSupport.build_bootstrap_manifest(
        requests,
        project_dir=project_dir,
        project_name=project_name,
        noanalysis=noanalysis,
        max_cpu=max_cpu,
        staging_dir=staging_dir,
    )
    return manifest.to_dict()


def build_import_command(
    *,
    manifest_path: Path,
    ghidra_home: Path,
    project_dir: Path,
    project_name: str,
    config_mode: str,
    max_cpu: int | None,
    noanalysis: bool,
    restore_metadata: bool,
    restore_metadata_from: Path,
    strict_restore: bool,
) -> list[str]:
    command = [
        sys.executable,
        "-m",
        GHIDRA_MAIN_MODULE,
        "binary",
        "manifest",
        str(manifest_path),
        "--ghidra-home",
        str(ghidra_home),
        "--project-dir",
        str(project_dir),
        "--project-name",
        project_name,
        "--config-mode",
        config_mode,
    ]
    if max_cpu is not None:
        command.extend(["--max-cpu", str(max_cpu)])
    command.append("--noanalysis" if noanalysis else "--with-analysis")
    if restore_metadata:
        command.extend(["--restore-metadata-from", str(restore_metadata_from)])
    else:
        command.append("--no-restore-metadata")
    if restore_metadata and strict_restore:
        command.append("--strict-restore")
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
        prog=package_prog("match", "import-wave"),
        description=(
            "Execute one representative-aware Ghidra import wave for queued EMI backlog items."
        ),
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
        "--recommended-action",
        action="append",
        choices=("import_representative", "import_member"),
    )
    parser.add_argument("--limit", type=int, default=8)
    parser.add_argument("--rank-min", type=int)
    parser.add_argument("--rank-max", type=int)
    parser.add_argument("--max-cpu", type=int)
    parser.add_argument("--noanalysis", action="store_true")
    parser.add_argument(
        "--restore-metadata",
        action=argparse.BooleanOptionalAction,
        default=False,
    )
    parser.add_argument("--strict-restore", action="store_true")
    parser.add_argument("--manifest-out", type=Path)
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
    logger = logger_from_args(args, "match_import_wave")
    if not args.inventory_db.exists():
        logger.error(f"inventory db not found: {args.inventory_db}")
        return 1

    scoreboard_payload = scoreboard_lib.build_scoreboard_payload(
        inventory_db=args.inventory_db,
        match_root=args.match_root,
        source_root=args.source_root,
        artifact_root=args.artifact_root,
    )
    backlog_payload = backlog_lib.build_import_backlog_payload(scoreboard_payload)
    selected_items = select_items(
        backlog_payload,
        families=args.family,
        lanes=args.lane,
        recommended_actions=args.recommended_action,
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
    manifest_path = args.manifest_out or (
        args.project_dir / "import_wave_manifest.json"
    )
    restore_metadata_from = args.inventory_db

    if not selected_items:
        report = {
            "generated_at": scoreboard_payload.get("generated_at"),
            "status": "no_items_selected",
            "selected_count": 0,
            "families": list(args.family or []),
            "lanes": list(args.lane or []),
            "report_inputs": {
                "inventory_db": str(args.inventory_db),
                "match_root": str(args.match_root),
                "source_root": str(args.source_root),
            },
        }
        write_json_output(output_json, report)
        logger.summary(f"selected=0 json={relative_to_root(output_json)}")
        return 0

    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    staging_dir = args.project_dir / "import_wave_staging"
    manifest_payload = build_manifest_payload(
        selected_items,
        project_dir=args.project_dir,
        project_name=args.project_name,
        noanalysis=bool(args.noanalysis),
        max_cpu=args.max_cpu,
        staging_dir=staging_dir,
    )
    write_json_output(manifest_path, manifest_payload)

    import_command = build_import_command(
        manifest_path=manifest_path,
        ghidra_home=args.ghidra_home,
        project_dir=args.project_dir,
        project_name=args.project_name,
        config_mode=args.config_mode,
        max_cpu=args.max_cpu,
        noanalysis=bool(args.noanalysis),
        restore_metadata=bool(args.restore_metadata),
        restore_metadata_from=restore_metadata_from,
        strict_restore=bool(args.strict_restore),
    )

    report: dict[str, Any] = {
        "generated_at": scoreboard_payload.get("generated_at"),
        "status": "planned" if args.dry_run else "pending",
        "selected_count": len(selected_items),
        "selected_items": selected_items,
        "selected_canonical_program_paths": [
            canonical_program_path(item) for item in selected_items
        ],
        "selected_ghidra_programs": [
            ghidra_program_selector(import_row)
            for import_row in manifest_payload.get("imports") or []
        ],
        "manifest_path": relative_to_root(manifest_path),
        "log_path": relative_to_root(log_path),
        "import_command": import_command,
        "project_dir": str(args.project_dir),
        "project_name": args.project_name,
        "ghidra_home": str(args.ghidra_home),
        "families": list(args.family or []),
        "lanes": list(args.lane or []),
        "recommended_actions": list(args.recommended_action or []),
        "limit": args.limit,
        "rank_min": args.rank_min,
        "rank_max": args.rank_max,
        "noanalysis": bool(args.noanalysis),
        "restore_metadata": bool(args.restore_metadata),
        "strict_restore": bool(args.strict_restore),
        "refresh_reports": bool(args.refresh_reports),
        "refresh_status": bool(args.refresh_status),
        "tracked_output": bool(args.tracked_output),
    }

    if args.dry_run:
        write_json_output(output_json, report)
        logger.summary(
            " ".join(
                [
                    f"selected={len(selected_items)}",
                    f"manifest={relative_to_root(manifest_path)}",
                    f"json={relative_to_root(output_json)}",
                ]
            )
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
        import_command,
        cwd=scoreboard_lib.workspace_lib.ROOT,
        env=ghidra_env(args.ghidra_home),
        timeout=None,
    )
    write_text_output(
        log_path,
        (result.stdout or "") + ("" if not result.stderr else "\n" + result.stderr),
    )
    report["import_returncode"] = int(result.returncode)
    if result.returncode != 0:
        report["status"] = "import_failed"
        write_json_output(output_json, report)
        logger.error(f"import wave failed; see {relative_to_root(log_path)}")
        return result.returncode

    report["persisted_program_rows"] = persist_imported_program_rows(
        db_path=args.inventory_db,
        items=selected_items,
    )

    capture_error = None
    try:
        capture_report = capture_into_inventory(
            db_path=args.inventory_db,
            selectors=tuple(report["selected_ghidra_programs"]),
            kind="all",
            project_dir=args.project_dir,
            project_name=args.project_name,
        )
    except Exception as exc:  # noqa: BLE001
        capture_report = None
        capture_error = str(exc)
    report["metadata_capture"] = {
        "canonical_program_count": None
        if capture_report is None
        else capture_report.get("canonical_program_count"),
        "row_count": None
        if capture_report is None
        else capture_report.get("row_count"),
        "persisted": None
        if capture_report is None
        else capture_report.get("persisted"),
        "error": capture_error,
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
    report["status"] = (
        "imported" if capture_error is None else "imported_capture_failed"
    )
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
