from __future__ import annotations

import argparse
import json
import shlex
import sys
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import run_command, write_json_output, write_text_output
from ..config import (
    DEFAULT_GHIDRA_DECOMP_ROOT,
    DEFAULT_MATCH_ROOT,
    DEFAULT_PSX_PROFILE,
)
from . import build as build_lib
from . import report as report_lib
from . import source_map
from . import workspace as workspace_lib


def mapping_sort_key(mapping: dict[str, Any]) -> tuple[str, int, str]:
    return (
        str(mapping.get("source_file") or ""),
        int(source_map.parse_hexish(str(mapping.get("entry_hex") or "0"))),
        str(mapping.get("source_function") or ""),
    )


def resolve_row_for_mapping(
    rows: list[dict[str, Any]], mapping: dict[str, Any]
) -> dict[str, Any] | None:
    entry_value = source_map.parse_hexish(str(mapping.get("entry_hex") or "0"))
    matches = [
        row
        for row in rows
        if source_map.parse_hexish(str(row.get("entry") or "0")) == entry_value
    ]
    if not matches:
        return None
    ranked = sorted(
        matches,
        key=lambda row: (
            source_map.mapping_score(
                mapping,
                program_path=str(row.get("program_path") or ""),
                program_name=str(row.get("program_name") or ""),
                source_hint=str(row.get("source_hint") or ""),
            ),
            str(row.get("program_path") or ""),
        ),
        reverse=True,
    )
    return ranked[0]


def collect_lift_targets(
    rows: list[dict[str, Any]],
    *,
    program_rows: list[dict[str, Any]] | None = None,
    artifact_root: Path = workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
    source_root: Path,
    source_glob: str | None,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    targets: list[dict[str, Any]] = []
    unresolved: list[dict[str, Any]] = []
    seen: set[tuple[str, str]] = set()
    for mapping in sorted(
        source_map.collect_source_mappings(source_root), key=mapping_sort_key
    ):
        source_file = str(mapping.get("source_file") or "")
        if source_glob and not Path(source_file).match(source_glob):
            continue
        row = resolve_row_for_mapping(rows, mapping)
        if row is None and program_rows is not None:
            row = workspace_lib.infer_function_row_for_mapping(
                mapping,
                program_rows=program_rows,
                artifact_root=artifact_root,
            )
        if row is None:
            unresolved.append(
                {
                    "source_file": source_file,
                    "source_function": mapping.get("source_function"),
                    "entry_hex": mapping.get("entry_hex"),
                }
            )
            continue
        key = (str(row.get("program_path") or ""), str(row.get("entry_hex") or ""))
        if key in seen:
            continue
        seen.add(key)
        targets.append(
            {
                "program_path": row.get("program_path"),
                "entry_hex": row.get("entry_hex"),
                "source_hint": row.get("source_hint"),
                "source_file": source_file,
                "source_function": mapping.get("source_function"),
            }
        )
    return targets, unresolved


def mapping_module_and_slot(mapping: dict[str, Any]) -> tuple[str | None, str | None]:
    source_file = str(mapping.get("source_file") or "")
    parts = Path(source_file).parts
    try:
        modules_index = parts.index("modules")
    except ValueError:
        return None, None
    module_name = None if len(parts) <= modules_index + 1 else parts[modules_index + 1]
    slot = None
    if len(parts) > modules_index + 2 and parts[modules_index + 2].isdigit():
        slot = str(int(parts[modules_index + 2]))
    return None if module_name is None else module_name.lower(), slot


def seed_program_row_sort_key(
    mapping: dict[str, Any], program_row: dict[str, Any]
) -> tuple[int, int, int, str, str]:
    source_file = str(mapping.get("source_file") or "").lower()
    program_path = str(program_row.get("program_path") or "")
    program_path_lower = program_path.lower()
    source_hint = str(program_row.get("source_hint") or "")
    source_hint_lower = source_hint.lower()
    module_name, slot = mapping_module_and_slot(mapping)
    affinity = 0

    if source_file.startswith("bof3/src/core/"):
        if program_path == "/boot/SLUS_004.22":
            affinity += 1000
        elif program_path_lower.startswith("/boot/"):
            affinity += 500

    if module_name == "logo":
        if program_path == "/boot/LOGO/LOGO.EXE":
            affinity += 1000
        elif "logo" in program_path_lower or "logo" in source_hint_lower:
            affinity += 500
    elif module_name is not None:
        hint_stem = Path(source_hint).stem.lower()
        if hint_stem == module_name:
            affinity += 400
        if f"/{module_name}/" in program_path_lower:
            affinity += 100
        if slot is not None and source_hint_lower.endswith(f"#{slot}"):
            affinity += 200

    score, neg_source_line, source_file_key = source_map.mapping_score(
        mapping,
        program_path=program_path,
        program_name=str(program_row.get("program_name") or ""),
        source_hint=source_hint,
    )
    return affinity, score, neg_source_line, source_file_key, program_path


def select_seed_program_row(
    mapping: dict[str, Any], *, program_rows: list[dict[str, Any]]
) -> dict[str, Any] | None:
    if not program_rows:
        return None
    ranked = sorted(
        program_rows,
        key=lambda row: seed_program_row_sort_key(mapping, row),
        reverse=True,
    )
    return ranked[0] if ranked else None


def seed_ghidra_bundles_for_unresolved(
    unresolved: list[dict[str, Any]],
    *,
    program_rows: list[dict[str, Any]],
    artifact_root: Path,
    logger: Any,
) -> list[dict[str, Any]]:
    attempts: list[dict[str, Any]] = []
    seen: set[tuple[str, str]] = set()
    for mapping in unresolved:
        entry_hex = str(mapping.get("entry_hex") or "")
        if not entry_hex:
            continue
        program_row = select_seed_program_row(mapping, program_rows=program_rows)
        if program_row is None:
            attempts.append(
                {
                    "entry_hex": entry_hex,
                    "source_file": mapping.get("source_file"),
                    "source_function": mapping.get("source_function"),
                    "status": "skipped_missing_program_row",
                }
            )
            continue

        row = workspace_lib.build_synthetic_function_row(
            program_row,
            entry=entry_hex,
            source_function=str(mapping.get("source_function") or "") or None,
            source_signature=str(mapping.get("source_signature") or "") or None,
        )
        key = (str(row.get("program_path") or ""), str(row.get("entry_hex") or ""))
        if key in seen:
            continue
        seen.add(key)

        artifacts_dir = workspace_lib.suggested_artifacts_dir(
            row,
            artifact_root,
            source_override=None,
        )
        if artifacts_dir is None:
            attempts.append(
                {
                    "program_path": row.get("program_path"),
                    "entry_hex": row.get("entry_hex"),
                    "source_file": mapping.get("source_file"),
                    "source_function": mapping.get("source_function"),
                    "status": "skipped_missing_artifacts_dir",
                }
            )
            continue

        bundle_json = artifacts_dir / "func.json"
        if bundle_json.exists():
            attempts.append(
                {
                    "program_path": row.get("program_path"),
                    "entry_hex": row.get("entry_hex"),
                    "source_file": mapping.get("source_file"),
                    "source_function": mapping.get("source_function"),
                    "artifacts_dir": workspace_lib.relative_to_root(artifacts_dir),
                    "bundle_json": workspace_lib.relative_to_root(bundle_json),
                    "status": "skipped_existing_bundle",
                }
            )
            continue

        command_text = workspace_lib.ghidra_decomp_command(
            row,
            artifacts_dir,
            source=None,
        )
        if not command_text:
            attempts.append(
                {
                    "program_path": row.get("program_path"),
                    "entry_hex": row.get("entry_hex"),
                    "source_file": mapping.get("source_file"),
                    "source_function": mapping.get("source_function"),
                    "artifacts_dir": workspace_lib.relative_to_root(artifacts_dir),
                    "status": "skipped_missing_command",
                }
            )
            continue

        logger.detail(
            f"seed ghidra bundle {row['program_path']} {row['entry_hex']} {mapping.get('source_function') or ''}".rstrip()
        )
        result = run_command(shlex.split(command_text))
        log_path = artifacts_dir / "ghidra_decomp.log"
        write_text_output(
            log_path,
            result.stdout + ("" if not result.stderr else "\n" + result.stderr),
        )
        attempts.append(
            {
                "program_path": row.get("program_path"),
                "entry_hex": row.get("entry_hex"),
                "source_file": mapping.get("source_file"),
                "source_function": mapping.get("source_function"),
                "artifacts_dir": workspace_lib.relative_to_root(artifacts_dir),
                "bundle_json": workspace_lib.relative_to_root(bundle_json),
                "command": shlex.split(command_text),
                "log_path": workspace_lib.relative_to_root(log_path),
                "returncode": int(result.returncode),
                "succeeded": result.returncode == 0 and bundle_json.exists(),
                "status": "seeded" if bundle_json.exists() else "failed",
            }
        )
    return attempts


def default_output_paths(workspace_root: Path, profile: str) -> tuple[Path, Path]:
    output_dir = workspace_root / "_reports"
    slug = profile.replace("-", "_")
    return (
        output_dir / f"lifted_match_{slug}.json",
        output_dir / f"lifted_match_{slug}.tsv",
    )


def build_once(
    *,
    profile: str,
    build_command: list[str],
    log_path: Path,
) -> tuple[dict[str, Any], Any]:
    result = run_command(build_command, env=build_lib.build_env(profile))
    write_text_output(
        log_path, result.stdout + ("" if not result.stderr else "\n" + result.stderr)
    )
    status = {
        "psx_profile": profile,
        "command": build_command,
        "command_text": " ".join(build_command),
        "build_root": workspace_lib.relative_to_root(
            build_lib.default_build_root_for_profile(profile)
        ),
        "build_root_exists": build_lib.default_build_root_for_profile(profile).exists(),
        "log_path": workspace_lib.relative_to_root(log_path),
        "returncode": int(result.returncode),
        "succeeded": result.returncode == 0,
    }
    return status, result


def write_workspace_build_statuses(
    targets: list[dict[str, Any]],
    rows: list[dict[str, Any]],
    *,
    program_rows: list[dict[str, Any]] | None,
    inventory_db: Path,
    workspace_root: Path,
    artifact_root: Path,
    profile: str,
    build_command: list[str],
    log_path: Path,
    result: Any,
) -> list[dict[str, Any]]:
    prepared: list[dict[str, Any]] = []
    build_root = build_lib.default_build_root_for_profile(profile)
    for target in targets:
        workspace_json, payload = workspace_lib.refresh_workspace_json(
            rows,
            program=str(target["program_path"]),
            entry=str(target["entry_hex"]),
            inventory_db=inventory_db,
            workspace_root=workspace_root,
            artifact_root=artifact_root,
            source_override=str(target.get("source_hint") or "") or None,
            program_rows=program_rows,
        )
        status = build_lib.build_status_payload(
            payload,
            profile=profile,
            command=build_command,
            log_path=log_path,
            build_root=build_root,
            result=result,
        )
        write_json_output(workspace_json.parent / "build.json", status)
        prepared.append(
            {
                **target,
                "workspace_dir": workspace_lib.relative_to_root(workspace_json.parent),
                "workspace_json": workspace_lib.relative_to_root(workspace_json),
            }
        )
    return prepared


def run_diff_for_target(
    target: dict[str, Any],
    *,
    workspace_root: Path,
    refresh_ghidra_bundle: bool,
) -> dict[str, Any]:
    workspace_dir = Path(target["workspace_dir"])
    if not workspace_dir.is_absolute():
        workspace_dir = workspace_lib.ROOT / workspace_dir
    log_path = workspace_dir / "sweep.diff.log"
    command = [
        sys.executable,
        "-m",
        "scripts.rebof3",
        "match",
        "diff",
        "--program",
        str(target["program_path"]),
        "--entry",
        str(target["entry_hex"]),
        "--workspace-root",
        str(workspace_root),
        "--run-backend",
    ]
    if refresh_ghidra_bundle:
        command.append("--refresh-ghidra-bundle")
    result = run_command(command)
    write_text_output(
        log_path, result.stdout + ("" if not result.stderr else "\n" + result.stderr)
    )
    diff_json = workspace_dir / "diff.json"
    diff_payload = (
        None
        if not diff_json.exists()
        else json.loads(diff_json.read_text(encoding="utf-8"))
    )
    return {
        **target,
        "diff_command": command,
        "diff_log_path": workspace_lib.relative_to_root(log_path),
        "diff_returncode": int(result.returncode),
        "diff_succeeded": result.returncode == 0,
        "status": None if diff_payload is None else diff_payload.get("status"),
        "match_metrics": None
        if diff_payload is None
        else diff_payload.get("match_metrics"),
        "report_path": None
        if diff_payload is None
        else workspace_lib.relative_to_root(diff_json),
        "report_row": None
        if diff_payload is None
        else report_lib.row_from_diff_payload(
            diff_payload, report_path=workspace_lib.relative_to_root(diff_json)
        ),
    }


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "sweep"),
        description="Run the canonical diff loop across all lifted address-mapped functions and emit a global report.",
    )
    add_logging_args(parser)
    parser.add_argument(
        "--inventory-db",
        type=Path,
        default=workspace_lib.DEFAULT_INVENTORY_DB,
    )
    parser.add_argument(
        "--workspace-root",
        type=Path,
        default=DEFAULT_MATCH_ROOT,
    )
    parser.add_argument(
        "--artifact-root",
        type=Path,
        default=DEFAULT_GHIDRA_DECOMP_ROOT,
    )
    parser.add_argument(
        "--source-root",
        type=Path,
        default=source_map.DEFAULT_SOURCE_ROOT,
    )
    parser.add_argument(
        "--source-glob",
        help="Optional source file glob, for example 'bof3/src/modules/*.c'",
    )
    parser.add_argument(
        "--build-command",
        nargs="+",
        default=["make", "build"],
    )
    parser.add_argument("--refresh-ghidra-bundle", action="store_true")
    parser.add_argument("--limit", type=int)
    parser.add_argument("--output-json", type=Path)
    parser.add_argument("--output-tsv", type=Path)
    parser.add_argument("--dry-run", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_sweep")
    if not args.inventory_db.exists():
        logger.error(f"inventory db not found: {args.inventory_db}")
        return 1

    rows = workspace_lib.load_function_rows(args.inventory_db)
    program_rows = workspace_lib.load_program_rows(args.inventory_db)
    targets, unresolved = collect_lift_targets(
        rows,
        program_rows=program_rows,
        artifact_root=args.artifact_root,
        source_root=args.source_root,
        source_glob=args.source_glob,
    )
    seed_attempts: list[dict[str, Any]] = []
    if args.refresh_ghidra_bundle and unresolved:
        seed_attempts = seed_ghidra_bundles_for_unresolved(
            unresolved,
            program_rows=program_rows,
            artifact_root=args.artifact_root,
            logger=logger,
        )
        targets, unresolved = collect_lift_targets(
            rows,
            program_rows=program_rows,
            artifact_root=args.artifact_root,
            source_root=args.source_root,
            source_glob=args.source_glob,
        )
    if args.limit is not None:
        targets = targets[: max(args.limit, 0)]

    profile = DEFAULT_PSX_PROFILE
    output_json, output_tsv = default_output_paths(args.workspace_root, profile)
    if args.output_json is not None:
        output_json = args.output_json
    if args.output_tsv is not None:
        output_tsv = args.output_tsv

    if args.dry_run:
        logger.summary(
            f"profile={profile} lifted_targets={len(targets)} unresolved={len(unresolved)} output_json={output_json}"
        )
        return 0

    build_log = args.workspace_root / "_reports" / f"build_{profile}.log"
    build_status, build_result = build_once(
        profile=profile,
        build_command=list(args.build_command),
        log_path=build_log,
    )

    prepared = write_workspace_build_statuses(
        targets,
        rows,
        program_rows=program_rows,
        inventory_db=args.inventory_db,
        workspace_root=args.workspace_root,
        artifact_root=args.artifact_root,
        profile=profile,
        build_command=list(args.build_command),
        log_path=build_log,
        result=build_result,
    )

    attempts: list[dict[str, Any]] = []
    report_rows: list[dict[str, Any]] = []
    if build_result.returncode == 0:
        for target in prepared:
            logger.detail(
                f"diff {target['program_path']} {target['entry_hex']} {target['source_function']}"
            )
            attempt = run_diff_for_target(
                target,
                workspace_root=args.workspace_root,
                refresh_ghidra_bundle=bool(args.refresh_ghidra_bundle),
            )
            attempts.append(attempt)
            if attempt.get("report_row") is not None:
                report_rows.append(dict(attempt["report_row"]))
    else:
        attempts = [
            {
                **target,
                "diff_succeeded": False,
                "diff_returncode": None,
                "status": "blocked_build_failed",
                "report_row": None,
            }
            for target in prepared
        ]

    report_rows.sort(key=report_lib.score_row, reverse=True)
    payload = {
        "profile": profile,
        "build_status": build_status,
        "lifted_target_count": len(targets),
        "resolved_target_count": len(prepared),
        "unresolved_targets": unresolved,
        "ghidra_bundle_seed_attempt_count": len(seed_attempts),
        "ghidra_bundle_seed_attempts": seed_attempts,
        "attempt_count": len(attempts),
        "report_row_count": len(report_rows),
        "rows": report_rows,
        "attempts": attempts,
    }
    write_json_output(output_json, payload)
    write_text_output(output_tsv, report_lib.render_tsv(report_rows))
    logger.summary(
        "Sweep finished with "
        f"{len(report_rows)} ranked rows from {len(attempts)} attempts "
        f"under profile {profile}."
    )
    logger.item(
        "Outputs: "
        f"json {workspace_lib.relative_to_root(output_json)}, "
        f"tsv {workspace_lib.relative_to_root(output_tsv)}"
    )
    if seed_attempts:
        logger.item(
            f"Seeded {len(seed_attempts)} missing Ghidra bundles before diffing."
        )
    for line in report_lib.render_brief_rows(report_rows, limit=5):
        logger.item(line)
    return 0 if build_result.returncode == 0 else int(build_result.returncode)


if __name__ == "__main__":
    raise SystemExit(main())
