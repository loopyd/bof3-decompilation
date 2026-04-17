from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import (
    hash_text,
    relative_to_root,
    write_json_output,
    write_markdown_output,
)
from ..config import DEFAULT_PSX_PROFILE, ROOT
from . import report_refresh
from . import scoreboard as scoreboard_lib


DEFAULT_BUILD_ARTIFACT_MANIFEST = (
    ROOT / "build" / "bof3-psyq40" / "artifacts" / "metadata" / "artifacts.json"
)


def load_artifact_manifest_summary(path: Path | None) -> dict[str, Any] | None:
    if path is None or not path.exists():
        return None
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError):
        return None
    artifacts = list(payload.get("artifacts") or [])
    kinds: dict[str, int] = {}
    stages: dict[str, int] = {}
    placeholder_count = 0
    for artifact in artifacts:
        kind = str(artifact.get("kind") or "unknown")
        stage = str(artifact.get("build_stage") or "unknown")
        kinds[kind] = int(kinds.get(kind, 0)) + 1
        stages[stage] = int(stages.get(stage, 0)) + 1
        placeholder_count += int(bool(artifact.get("placeholder")))
    return {
        "manifest_path": relative_to_root(path),
        "declared_artifacts": len(artifacts),
        "placeholder_artifacts": placeholder_count,
        "raw_stage_artifacts": int(stages.get("raw", 0)),
        "archive_stage_artifacts": int(stages.get("archive", 0)),
        "kinds": kinds,
    }


def render_summary_markdown(payload: dict[str, Any]) -> str:
    summary = payload.get("summary") or {}
    artifacts = payload.get("artifacts") or {}
    lines = [
        "# Decomp Status",
        "",
        f"- Generated: {payload.get('generated_at') or 'unknown'}",
        f"- Profile: {payload.get('profile') or DEFAULT_PSX_PROFILE}",
        f"- Campaign ready: {summary.get('campaign_ready')}",
        f"- Code-candidate entries: {summary.get('code_candidate_entries') or 0}",
        f"- Imported programs: {summary.get('imported_overlay_programs') or 0}",
        f"- Inventory functions: {summary.get('inventory_functions') or 0}",
        f"- Lifted C functions: {summary.get('lifted_c_functions') or 0}",
        f"- Functions without source: {summary.get('functions_without_source') or 0}",
        f"- Source coverage: {summary.get('source_coverage_percent') or 0}%",
        f"- Diffed functions: {summary.get('diffed_functions') or 0}",
        f"- Exact matches: {summary.get('exact_match_functions') or 0}",
        f"- Asm-differ exact: {summary.get('asm_exact_functions') or 0}",
        f"- Matched functions: {summary.get('matched_function_count') or 0}",
        f"- Attempted functions: {summary.get('attempted_functions') or 0}",
        f"- Stalled functions: {summary.get('stalled_functions') or 0}",
        f"- Best match: {summary.get('highest_objdiff_match_percent') if summary.get('highest_objdiff_match_percent') is not None else 'n/a'}",
        f"- Average match: {summary.get('average_objdiff_match_percent') if summary.get('average_objdiff_match_percent') is not None else 'n/a'}",
        f"- Median match: {summary.get('median_objdiff_match_percent') if summary.get('median_objdiff_match_percent') is not None else 'n/a'}",
        f"- Worst match: {summary.get('lowest_objdiff_match_percent') if summary.get('lowest_objdiff_match_percent') is not None else 'n/a'}",
        "",
        "## Program Coverage",
        "",
        f"- BIN programs: {summary.get('bin_programs') or 0}",
        f"- Boot programs: {summary.get('boot_programs') or 0}",
        f"- Logo programs: {summary.get('logo_programs') or 0}",
        f"- Other programs: {summary.get('other_programs') or 0}",
        "",
        "## Build Artifacts",
        "",
        f"- Declared artifacts: {artifacts.get('declared_artifacts') if artifacts else 'n/a'}",
        f"- Raw-stage artifacts: {artifacts.get('raw_stage_artifacts') if artifacts else 'n/a'}",
        f"- Archive-stage artifacts: {artifacts.get('archive_stage_artifacts') if artifacts else 'n/a'}",
        f"- Placeholder artifacts: {artifacts.get('placeholder_artifacts') if artifacts else 'n/a'}",
        "",
        "## Blocking Issues",
        "",
    ]
    blocking_issues = list(summary.get("blocking_issues") or [])
    if not blocking_issues:
        lines.append("- None")
    else:
        lines.extend(f"- {issue}" for issue in blocking_issues)
    return "\n".join(lines) + "\n"


def build_tracked_snapshot(payload: dict[str, Any], *, profile: str) -> dict[str, Any]:
    summary = payload.get("summary") or {}
    artifacts = payload.get("artifacts") or {}
    family_rows: list[dict[str, Any]] = []
    for row in payload.get("families") or []:
        family_rows.append(
            {
                "family": row.get("family"),
                "code_candidate_entries": row.get("code_candidate_entries"),
                "entries_missing_programs": row.get("entries_missing_programs"),
                "reviewed_non_code_entries": row.get("reviewed_non_code_entries"),
                "programs": row.get("programs"),
                "inventory_functions": row.get("inventory_functions"),
                "exact_match_functions": row.get("exact_match_functions"),
                "asm_exact_functions": row.get("asm_exact_functions"),
                "matched_function_count": row.get("matched_function_count"),
                "highest_match_percent": row.get("highest_objdiff_match_percent"),
                "lowest_match_percent": row.get("lowest_objdiff_match_percent"),
                "average_match_percent": row.get("average_objdiff_match_percent"),
                "median_match_percent": row.get("median_objdiff_match_percent"),
            }
        )
    return {
        "generated_at": payload.get("generated_at"),
        "profile": profile,
        "inventory_db": payload.get("inventory_db"),
        "match_root": payload.get("match_root"),
        "source_root": payload.get("source_root"),
        "artifact_root": payload.get("artifact_root"),
        "campaign_ready": summary.get("campaign_ready"),
        "blocking_issues": list(summary.get("blocking_issues") or []),
        "artifacts": artifacts,
        "coverage": {
            "entries": {
                "code_candidate": summary.get("code_candidate_entries"),
                "missing_programs": summary.get("code_entries_missing_programs"),
                "missing_functions": summary.get("code_entries_missing_functions"),
                "reviewed_non_code": summary.get("reviewed_non_code_entries"),
            },
            "programs": {
                "total": summary.get("programs"),
                "bin": summary.get("bin_programs"),
                "boot": summary.get("boot_programs"),
                "logo": summary.get("logo_programs"),
                "other": summary.get("other_programs"),
                "imported_overlay": summary.get("imported_overlay_programs"),
            },
            "functions": {
                "inventory": summary.get("inventory_functions"),
                "bin": summary.get("bin_functions"),
                "boot": summary.get("boot_functions"),
                "logo": summary.get("logo_functions"),
                "other": summary.get("other_functions"),
                "lifted_c": summary.get("lifted_c_functions"),
                "without_source": summary.get("functions_without_source"),
                "source_coverage_percent": summary.get("source_coverage_percent"),
                "build_ok": summary.get("build_ok_functions"),
                "build_failed": summary.get("build_failed_functions"),
                "diffed": summary.get("diffed_functions"),
                "exact_match": summary.get("exact_match_functions"),
                "asm_exact": summary.get("asm_exact_functions"),
                "matched": summary.get("matched_function_count"),
                "attempted": summary.get("attempted_functions"),
                "stalled": summary.get("stalled_functions"),
                "highest_match_percent": summary.get("highest_objdiff_match_percent"),
                "lowest_match_percent": summary.get("lowest_objdiff_match_percent"),
                "average_match_percent": summary.get("average_objdiff_match_percent"),
                "median_match_percent": summary.get("median_objdiff_match_percent"),
            },
            "duplicates": {
                "groups": summary.get("duplicate_groups"),
                "multi_entry_groups": summary.get("multi_entry_duplicate_groups"),
                "entries_in_multi_groups": summary.get("entries_in_multi_groups"),
                "largest_group": summary.get("largest_duplicate_group"),
            },
            "unresolved_source_mappings": summary.get("unresolved_source_mappings"),
        },
        "families": family_rows,
    }


def build_manifest(
    *,
    payload: dict[str, Any],
    profile: str,
    output_root: Path,
    rendered_outputs: dict[str, str],
) -> dict[str, Any]:
    entries = []
    for name, text in rendered_outputs.items():
        path = output_root / name
        entries.append(
            {
                "name": name,
                "path": relative_to_root(path),
                "sha256": hash_text(text),
                "bytes": len(text.encode("utf-8")),
            }
        )
    return {
        "generated_at": payload.get("generated_at"),
        "profile": profile,
        "output_root": relative_to_root(output_root),
        "summary": {
            "code_candidate_entries": payload.get("summary", {}).get(
                "code_candidate_entries"
            ),
            "programs": payload.get("summary", {}).get("programs"),
            "inventory_functions": payload.get("summary", {}).get(
                "inventory_functions"
            ),
            "exact_match_functions": payload.get("summary", {}).get(
                "exact_match_functions"
            ),
            "asm_exact_functions": payload.get("summary", {}).get(
                "asm_exact_functions"
            ),
            "campaign_ready": payload.get("summary", {}).get("campaign_ready"),
        },
        "files": sorted(entries, key=lambda item: item["name"]),
    }


def write_status_snapshot(
    payload: dict[str, Any],
    *,
    output_root: Path,
    profile: str,
) -> dict[str, Path]:
    summary_payload = {**payload, "profile": profile}
    tracked_snapshot = build_tracked_snapshot(payload, profile=profile)
    status_json = output_root / "status.json"
    legacy_scoreboard_json = output_root / "scoreboard.json"
    status_md = output_root / "status.md"
    functions_tsv = output_root / "functions.tsv"
    programs_tsv = output_root / "programs.tsv"
    families_tsv = output_root / "families.tsv"
    entries_tsv = output_root / "entries.tsv"
    manifest_json = output_root / "manifest.json"

    rendered_outputs = {
        "status.json": scoreboard_lib.render_summary_json(tracked_snapshot),
        "status.md": render_summary_markdown(summary_payload),
        "functions.tsv": scoreboard_lib.render_tsv(payload.get("functions") or []),
        "programs.tsv": scoreboard_lib.render_programs_tsv(
            payload.get("programs") or []
        ),
        "families.tsv": scoreboard_lib.render_families_tsv(
            payload.get("families") or []
        ),
        "entries.tsv": scoreboard_lib.render_entries_tsv(payload.get("entries") or []),
    }

    status_json.parent.mkdir(parents=True, exist_ok=True)
    if legacy_scoreboard_json.exists():
        legacy_scoreboard_json.unlink()
    write_json_output(status_json, tracked_snapshot)
    write_markdown_output(status_md, rendered_outputs["status.md"])
    functions_tsv.write_text(rendered_outputs["functions.tsv"], encoding="utf-8")
    programs_tsv.write_text(rendered_outputs["programs.tsv"], encoding="utf-8")
    families_tsv.write_text(rendered_outputs["families.tsv"], encoding="utf-8")
    entries_tsv.write_text(rendered_outputs["entries.tsv"], encoding="utf-8")

    manifest = build_manifest(
        payload=summary_payload,
        profile=profile,
        output_root=output_root,
        rendered_outputs=rendered_outputs,
    )
    write_json_output(manifest_json, manifest)
    return {
        "status_json": status_json,
        "status_md": status_md,
        "functions_tsv": functions_tsv,
        "programs_tsv": programs_tsv,
        "families_tsv": families_tsv,
        "entries_tsv": entries_tsv,
        "manifest_json": manifest_json,
    }


def refresh_status_snapshot(
    *,
    inventory_db: Path,
    match_root: Path,
    source_root: Path,
    artifact_root: Path,
    profile: str,
    output_root: Path,
    build_artifact_manifest: Path | None = DEFAULT_BUILD_ARTIFACT_MANIFEST,
) -> dict[str, Path]:
    payload = scoreboard_lib.build_scoreboard_payload(
        inventory_db=inventory_db,
        match_root=match_root,
        source_root=source_root,
        artifact_root=artifact_root,
    )
    artifact_summary = load_artifact_manifest_summary(build_artifact_manifest)
    if artifact_summary is not None:
        payload = {**payload, "artifacts": artifact_summary}
    return write_status_snapshot(payload, output_root=output_root, profile=profile)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "status"),
        description="Persist the current decomp status snapshot.",
    )
    add_logging_args(parser)
    parser.add_argument(
        "-i",
        "--inventory-db",
        type=Path,
        default=scoreboard_lib.DEFAULT_INVENTORY_DB,
    )
    parser.add_argument(
        "-m",
        "--match-root",
        type=Path,
        default=scoreboard_lib.DEFAULT_MATCH_ROOT,
    )
    parser.add_argument(
        "-s",
        "--source-root",
        type=Path,
        default=scoreboard_lib.DEFAULT_SOURCE_ROOT,
    )
    parser.add_argument(
        "-a",
        "--artifact-root",
        type=Path,
        default=scoreboard_lib.workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
    )
    parser.add_argument(
        "--build-artifact-manifest",
        type=Path,
        default=DEFAULT_BUILD_ARTIFACT_MANIFEST,
    )
    parser.add_argument("-P", "--profile", default=DEFAULT_PSX_PROFILE)
    parser.add_argument("-t", "--tracked-output", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_status")
    output_root = report_refresh.resolve_status_output_root(
        profile=args.profile,
        tracked_output=bool(args.tracked_output),
    )
    outputs = refresh_status_snapshot(
        inventory_db=args.inventory_db,
        match_root=args.match_root,
        source_root=args.source_root,
        artifact_root=args.artifact_root,
        profile=args.profile,
        output_root=output_root,
        build_artifact_manifest=args.build_artifact_manifest,
    )
    logger.summary(f"status={relative_to_root(output_root)}")
    logger.item(f"json={relative_to_root(outputs['status_json'])}")
    return 0
