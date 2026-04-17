from __future__ import annotations

import argparse
import json
import sqlite3
from collections import defaultdict
from pathlib import Path
from statistics import median
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import (
    format_hex,
    parse_hexish,
    relative_to_root,
    utc_now,
    write_json_output,
    write_text_output,
)
from ..config import DEFAULT_MATCH_ROOT, DEFAULT_PSX_PROFILE
from ..inventory.layout import INVENTORY_SQLITE
from . import history as history_lib
from . import report as report_lib
from . import report_refresh
from . import review_overrides
from . import source_map
from . import workspace as workspace_lib

DEFAULT_INVENTORY_DB = INVENTORY_SQLITE
DEFAULT_SOURCE_ROOT = source_map.DEFAULT_SOURCE_ROOT


def canonical_program_path(program_path: str) -> str:
    text = str(program_path or "")
    if text == "/SLUS_004.22":
        return "/boot/SLUS_004.22"
    if text == "/LOGO/LOGO.EXE":
        return "/boot/LOGO/LOGO.EXE"
    if text.endswith(".bin.0"):
        return text[:-2]
    if text.endswith(".EXE.0") or text.endswith(".22.0"):
        return text[:-2]
    return text


def normalize_entry_hex(value: Any) -> str:
    if value in (None, ""):
        return ""
    try:
        return format_hex(parse_hexish(str(value)))
    except ValueError:
        return str(value)


def function_key(program_path: str, entry_hex: str) -> tuple[str, str]:
    return canonical_program_path(str(program_path or "")), normalize_entry_hex(
        entry_hex
    )


def row_canonical_preference(row: dict[str, Any]) -> tuple[int, int, int, int, str]:
    program_path = str(row.get("program_path") or "")
    canonical = canonical_program_path(program_path)
    return (
        int(program_path == canonical),
        int(program_path.startswith("/boot/")),
        int(not program_path.endswith(".0")),
        int(bool(row.get("source_hint"))),
        program_path,
    )


def dedupe_program_rows(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    deduped: dict[str, dict[str, Any]] = {}
    for row in rows:
        canonical = canonical_program_path(str(row.get("program_path") or ""))
        candidate = {**row, "program_path": canonical}
        current = deduped.get(canonical)
        if current is None or row_canonical_preference(
            candidate
        ) > row_canonical_preference(current):
            deduped[canonical] = candidate
    return sorted(
        deduped.values(), key=lambda item: str(item.get("program_path") or "")
    )


def dedupe_function_inventory_rows(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    deduped: dict[tuple[str, str], dict[str, Any]] = {}
    for row in rows:
        canonical = canonical_program_path(str(row.get("program_path") or ""))
        entry_hex = normalize_entry_hex(row.get("entry_hex") or row.get("entry"))
        key = (canonical, entry_hex)
        candidate = {
            **row,
            "program_path": canonical,
            "entry_hex": entry_hex,
        }
        current = deduped.get(key)
        if current is None or row_canonical_preference(
            candidate
        ) > row_canonical_preference(current):
            deduped[key] = candidate
    return sorted(
        deduped.values(),
        key=lambda item: (
            str(item.get("program_path") or ""),
            parse_hexish(str(item.get("entry") or item.get("entry_hex") or "0")),
        ),
    )


def classify_program_path(program_path: str) -> str:
    normalized = canonical_program_path(str(program_path or ""))
    if normalized.startswith("/bins/"):
        return "bin"
    if normalized == "/boot/LOGO/LOGO.EXE":
        return "logo"
    if normalized.startswith("/boot/"):
        return "boot"
    return "other"


def default_output_paths(match_root: Path, profile: str) -> tuple[Path, Path]:
    output_dir = match_root / "_reports"
    slug = profile.replace("-", "_")
    return (
        output_dir / f"scoreboard_{slug}.json",
        output_dir / f"scoreboard_{slug}.tsv",
    )


def load_inventory_health(inventory_db: Path) -> dict[str, Any]:
    connection = sqlite3.connect(inventory_db)
    connection.row_factory = sqlite3.Row
    try:
        schema_version_row = connection.execute(
            "SELECT MAX(version) AS version FROM schema_migrations"
        ).fetchone()
        schema_version = (
            0
            if schema_version_row is None or schema_version_row["version"] is None
            else int(schema_version_row["version"])
        )
        counts = {
            "archives": 0,
            "emi_entries": 0,
            "code_candidate_entries": 0,
            "overlay_aliases": 0,
            "overlay_entry_tables": 0,
            "programs": 0,
            "functions": 0,
            "slot_map": 0,
            "disc_lba_entries": 0,
        }
        for label, query in (
            ("archives", "SELECT COUNT(*) AS count FROM archives"),
            ("emi_entries", "SELECT COUNT(*) AS count FROM emi_entries"),
            (
                "code_candidate_entries",
                "SELECT COUNT(*) AS count FROM emi_entries WHERE code_candidate = 1",
            ),
            ("overlay_aliases", "SELECT COUNT(*) AS count FROM overlay_aliases"),
            (
                "overlay_entry_tables",
                "SELECT COUNT(*) AS count FROM overlay_entry_tables",
            ),
            ("programs", "SELECT COUNT(*) AS count FROM programs"),
            ("functions", "SELECT COUNT(*) AS count FROM functions"),
            ("slot_map", "SELECT COUNT(*) AS count FROM slot_map"),
            (
                "disc_lba_entries",
                "SELECT COUNT(*) AS count FROM disc_lba_entries",
            ),
        ):
            row = connection.execute(query).fetchone()
            counts[label] = 0 if row is None else int(row["count"])
    finally:
        connection.close()
    return {
        "inventory_db": str(inventory_db),
        "schema_version": schema_version,
        **counts,
    }


def load_code_entry_rows(inventory_db: Path) -> list[dict[str, Any]]:
    connection = sqlite3.connect(inventory_db)
    connection.row_factory = sqlite3.Row
    try:
        rows = connection.execute(
            """
            WITH rep_sizes AS (
                SELECT
                    representative_archive_id,
                    representative_entry_index,
                    COUNT(*) AS group_size
                FROM overlay_aliases
                GROUP BY representative_archive_id, representative_entry_index
            )
            SELECT
                entries.archive_id,
                entries.entry_index,
                entries.family,
                entries.load_arg,
                entries.size,
                entries.payload_path,
                entries.sha256,
                entries.code_candidate,
                alias.representative_archive_id,
                alias.representative_entry_index,
                COALESCE(rep_sizes.group_size, 0) AS duplicate_group_size,
                tables.confidence AS entry_table_confidence,
                programs.program_path,
                programs.program_name,
                programs.program_slug,
                programs.source_hint
            FROM emi_entries AS entries
            LEFT JOIN overlay_aliases AS alias
                ON alias.archive_id = entries.archive_id
               AND alias.entry_index = entries.entry_index
            LEFT JOIN rep_sizes
                ON rep_sizes.representative_archive_id = alias.representative_archive_id
               AND rep_sizes.representative_entry_index = alias.representative_entry_index
            LEFT JOIN overlay_entry_tables AS tables
                ON tables.archive_id = entries.archive_id
               AND tables.entry_index = entries.entry_index
            LEFT JOIN programs
                ON programs.source_hint = entries.payload_path
            WHERE entries.code_candidate = 1
            ORDER BY entries.family, entries.archive_id, entries.entry_index, programs.program_path
            """
        ).fetchall()
    finally:
        connection.close()
    return [dict(row) for row in rows]


def load_program_overlay_rows(inventory_db: Path) -> list[dict[str, Any]]:
    connection = sqlite3.connect(inventory_db)
    connection.row_factory = sqlite3.Row
    try:
        rows = connection.execute(
            """
            WITH rep_sizes AS (
                SELECT
                    representative_archive_id,
                    representative_entry_index,
                    COUNT(*) AS group_size
                FROM overlay_aliases
                GROUP BY representative_archive_id, representative_entry_index
            )
            SELECT
                programs.program_path,
                programs.program_name,
                programs.program_slug,
                programs.folder,
                programs.source_hint,
                entries.archive_id,
                entries.entry_index,
                entries.family,
                entries.load_arg,
                entries.payload_path,
                alias.representative_archive_id,
                alias.representative_entry_index,
                COALESCE(rep_sizes.group_size, 0) AS duplicate_group_size
            FROM programs
            LEFT JOIN emi_entries AS entries
                ON entries.payload_path = programs.source_hint
            LEFT JOIN overlay_aliases AS alias
                ON alias.archive_id = entries.archive_id
               AND alias.entry_index = entries.entry_index
            LEFT JOIN rep_sizes
                ON rep_sizes.representative_archive_id = alias.representative_archive_id
               AND rep_sizes.representative_entry_index = alias.representative_entry_index
            ORDER BY programs.program_path
            """
        ).fetchall()
    finally:
        connection.close()
    return [dict(row) for row in rows]


def duplicate_group_key_from_row(row: dict[str, Any]) -> str | None:
    rep_archive = row.get("representative_archive_id")
    rep_entry = row.get("representative_entry_index")
    if rep_archive not in (None, "") and rep_entry not in (None, ""):
        return f"{rep_archive}#{int(rep_entry)}"
    archive_id = row.get("archive_id")
    entry_index = row.get("entry_index")
    if archive_id not in (None, "") and entry_index not in (None, ""):
        return f"{archive_id}#{int(entry_index)}"
    return None


def infer_family(
    *, program_path: str, source_hint: str | None, overlay_family: str | None
) -> str:
    if overlay_family:
        return str(overlay_family)
    canonical_path = canonical_program_path(str(program_path or ""))
    program_path_lower = canonical_path.lower()
    source_hint_lower = str(source_hint or "").lower()
    if canonical_path == "/boot/SLUS_004.22":
        return "SLUS"
    if canonical_path == "/boot/LOGO/LOGO.EXE" or "logo" in source_hint_lower:
        return "LOGO"
    if program_path_lower.startswith("/boot/"):
        return "BOOT"
    return "UNKNOWN"


def summarize_program_kinds(rows: list[dict[str, Any]], *, key: str) -> dict[str, int]:
    counts = {"bin": 0, "boot": 0, "logo": 0, "other": 0}
    for row in rows:
        counts[classify_program_path(str(row.get(key) or ""))] += 1
    return counts


def collect_latest_workspace_payloads(
    match_root: Path, file_name: str
) -> dict[tuple[str, str], dict[str, Any]]:
    payloads: dict[tuple[str, str], dict[str, Any]] = {}
    for path in sorted(match_root.glob(f"**/{file_name}")):
        try:
            payload = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, UnicodeDecodeError, json.JSONDecodeError):
            continue
        key = function_key(
            str(payload.get("program_path") or ""),
            str(payload.get("entry_hex") or payload.get("entry") or ""),
        )
        if not key[0] or not key[1]:
            continue
        mtime_ns = path.stat().st_mtime_ns
        current = payloads.get(key)
        if current is not None and int(current["_mtime_ns"]) > mtime_ns:
            continue
        payloads[key] = {
            **payload,
            "_path": relative_to_root(path),
            "_mtime_ns": mtime_ns,
        }
    return payloads


def infer_missing_function_rows(
    function_rows: list[dict[str, Any]],
    *,
    program_rows: list[dict[str, Any]],
    source_mappings: dict[tuple[str, str], dict[str, Any]],
    build_statuses: dict[tuple[str, str], dict[str, Any]],
    diff_payloads: dict[tuple[str, str], dict[str, Any]],
    history_summaries: dict[tuple[str, str], dict[str, Any]],
    artifact_root: Path,
) -> list[dict[str, Any]]:
    existing_keys = {
        function_key(
            str(row.get("program_path") or ""),
            str(row.get("entry_hex") or row.get("entry") or ""),
        )
        for row in function_rows
    }
    candidate_keys = set(existing_keys)
    candidate_keys.update(source_mappings.keys())
    candidate_keys.update(build_statuses.keys())
    candidate_keys.update(diff_payloads.keys())
    candidate_keys.update(history_summaries.keys())
    inferred: list[dict[str, Any]] = []
    for program_path, entry_hex in sorted(candidate_keys):
        key = (program_path, entry_hex)
        if key in existing_keys or not program_path or not entry_hex:
            continue
        mapping = source_mappings.get(key)
        row = None
        if mapping is not None:
            row = infer_function_row_from_mapping(
                mapping,
                program_rows=program_rows,
                artifact_root=artifact_root,
            )
        if row is None:
            program_row = next(
                (
                    candidate
                    for candidate in program_rows
                    if canonical_program_path(str(candidate.get("program_path") or ""))
                    == program_path
                ),
                None,
            )
            if program_row is None:
                continue
            row = workspace_lib.build_synthetic_function_row(
                program_row,
                entry=entry_hex,
                source_function=str(mapping.get("source_function") or "") or None
                if mapping is not None
                else None,
                source_signature=str(mapping.get("source_signature") or "") or None
                if mapping is not None
                else None,
            )
        inferred.append({**row, "program_path": canonical_program_path(program_path)})
    return dedupe_function_inventory_rows(inferred)


def collect_workspace_history_summaries(
    match_root: Path,
) -> dict[tuple[str, str], dict[str, Any]]:
    summaries: dict[tuple[str, str], dict[str, Any]] = {}
    for path in sorted(match_root.glob(f"**/{history_lib.HISTORY_BASENAME}")):
        summary = history_lib.summarize_entries(
            history_lib.load_entries_from_path(path)
        )
        key = function_key(
            str(summary.get("program_path") or ""),
            str(summary.get("entry_hex") or ""),
        )
        if not key[0] or not key[1]:
            continue
        summaries[key] = {
            **summary,
            "_path": relative_to_root(path),
        }
    return summaries


def index_function_rows_by_entry(
    rows: list[dict[str, Any]],
) -> dict[int, list[dict[str, Any]]]:
    indexed: dict[int, list[dict[str, Any]]] = defaultdict(list)
    for row in rows:
        entry_value = parse_hexish(str(row.get("entry") or row.get("entry_hex") or "0"))
        indexed[entry_value].append(row)
    return indexed


def resolve_row_for_mapping(
    rows_by_entry: dict[int, list[dict[str, Any]]], mapping: dict[str, Any]
) -> dict[str, Any] | None:
    entry_value = parse_hexish(str(mapping.get("entry_hex") or "0"))
    matches = rows_by_entry.get(entry_value, [])
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


def infer_function_row_from_mapping(
    mapping: dict[str, Any],
    *,
    program_rows: list[dict[str, Any]],
    artifact_root: Path,
) -> dict[str, Any] | None:
    entry_hex = str(mapping.get("entry_hex") or "")
    if not entry_hex:
        return None
    entry_value = parse_hexish(entry_hex)
    candidates: list[tuple[tuple[int, int, str], dict[str, Any]]] = []
    for program_row in program_rows:
        row = workspace_lib.build_synthetic_function_row(
            program_row,
            entry=entry_hex,
            source_function=str(mapping.get("source_function") or "") or None,
            source_signature=str(mapping.get("source_signature") or "") or None,
        )
        artifacts_dir = workspace_lib.suggested_artifacts_dir(
            row,
            artifact_root,
            source_override=None,
        )
        if artifacts_dir is None:
            continue
        bundle_json = artifacts_dir / "func.json"
        if not bundle_json.exists():
            continue
        if not workspace_lib.bundle_supports_entry(
            bundle_json, entry_value=entry_value
        ):
            continue
        candidates.append(
            (
                source_map.mapping_score(
                    mapping,
                    program_path=str(row.get("program_path") or ""),
                    program_name=str(row.get("program_name") or ""),
                    source_hint=str(row.get("source_hint") or ""),
                ),
                row,
            )
        )
    if not candidates:
        return None
    candidates.sort(
        key=lambda item: (item[0], str(item[1].get("program_path") or "")),
        reverse=True,
    )
    return candidates[0][1]


def collect_resolved_source_mappings(
    function_rows: list[dict[str, Any]],
    *,
    program_rows: list[dict[str, Any]],
    artifact_root: Path,
    source_root: Path,
) -> tuple[dict[tuple[str, str], dict[str, Any]], list[dict[str, Any]]]:
    resolved: dict[tuple[str, str], dict[str, Any]] = {}
    unresolved: list[dict[str, Any]] = []
    rows_by_entry = index_function_rows_by_entry(function_rows)
    for mapping in source_map.collect_source_mappings(source_root):
        row = resolve_row_for_mapping(rows_by_entry, mapping)
        if row is None:
            row = infer_function_row_from_mapping(
                mapping,
                program_rows=program_rows,
                artifact_root=artifact_root,
            )
        if row is None:
            unresolved.append(
                {
                    "entry_hex": str(mapping.get("entry_hex") or ""),
                    "source_file": str(mapping.get("source_file") or ""),
                    "source_function": str(mapping.get("source_function") or ""),
                }
            )
            continue
        key = function_key(
            str(row.get("program_path") or ""),
            str(row.get("entry_hex") or row.get("entry") or ""),
        )
        current = resolved.get(key)
        if current is None:
            resolved[key] = mapping
            continue
        current_score = source_map.mapping_score(
            current,
            program_path=str(row.get("program_path") or ""),
            program_name=str(row.get("program_name") or ""),
            source_hint=str(row.get("source_hint") or ""),
        )
        candidate_score = source_map.mapping_score(
            mapping,
            program_path=str(row.get("program_path") or ""),
            program_name=str(row.get("program_name") or ""),
            source_hint=str(row.get("source_hint") or ""),
        )
        if candidate_score > current_score:
            resolved[key] = mapping
    unresolved.sort(
        key=lambda item: (
            str(item.get("source_file") or ""),
            str(item.get("entry_hex") or ""),
            str(item.get("source_function") or ""),
        )
    )
    return resolved, unresolved


def derive_function_state(
    *,
    source_mapping: dict[str, Any] | None,
    build_status: dict[str, Any] | None,
    diff_payload: dict[str, Any] | None,
) -> str:
    if diff_payload is not None:
        metrics = diff_payload.get("match_metrics") or {}
        if float(metrics.get("objdiff_match_percent") or 0.0) >= 100.0:
            return "exact_match"
        status = str(diff_payload.get("status") or "")
        return status or "diffed"
    if build_status is not None:
        return "build_ok" if bool(build_status.get("succeeded")) else "build_failed"
    if source_mapping is not None:
        return "lifted_c"
    return "inventory_only"


def build_function_rows(
    function_rows: list[dict[str, Any]],
    *,
    program_meta_by_path: dict[str, dict[str, Any]],
    source_mappings: dict[tuple[str, str], dict[str, Any]],
    build_statuses: dict[tuple[str, str], dict[str, Any]],
    diff_payloads: dict[tuple[str, str], dict[str, Any]],
    history_summaries: dict[tuple[str, str], dict[str, Any]],
) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for row in function_rows:
        program_path = str(row.get("program_path") or "")
        entry_hex = normalize_entry_hex(row.get("entry_hex") or row.get("entry"))
        key = function_key(program_path, entry_hex)
        meta = program_meta_by_path.get(program_path, {})
        source_mapping = source_mappings.get(key)
        build_status = build_statuses.get(key)
        diff_payload = diff_payloads.get(key)
        history_summary = history_summaries.get(key, {})
        metrics = (
            {}
            if diff_payload is None
            else dict(diff_payload.get("match_metrics") or {})
        )
        match_bucket = "" if not metrics else report_lib.match_bucket(metrics)
        rows.append(
            {
                "program_path": program_path,
                "program_name": row.get("program_name"),
                "entry_hex": entry_hex,
                "name": row.get("name"),
                "signature": row.get("signature"),
                "family": infer_family(
                    program_path=program_path,
                    source_hint=str(
                        meta.get("source_hint") or row.get("source_hint") or ""
                    ),
                    overlay_family=None
                    if meta.get("family") is None
                    else str(meta.get("family")),
                ),
                "archive_id": meta.get("archive_id"),
                "entry_index": meta.get("entry_index"),
                "program_kind": classify_program_path(program_path),
                "source_hint": meta.get("source_hint") or row.get("source_hint"),
                "duplicate_group_key": duplicate_group_key_from_row(meta),
                "duplicate_group_size": int(meta.get("duplicate_group_size") or 0),
                "source_file": None
                if source_mapping is None
                else source_mapping.get("source_file"),
                "source_function": None
                if source_mapping is None
                else source_mapping.get("source_function"),
                "has_source_mapping": source_mapping is not None,
                "build_status_present": build_status is not None,
                "build_succeeded": bool(build_status and build_status.get("succeeded")),
                "build_status": None
                if build_status is None
                else build_status.get("_path"),
                "diff_report_present": diff_payload is not None,
                "diff_status": None
                if diff_payload is None
                else diff_payload.get("status"),
                "diff_report": None
                if diff_payload is None
                else diff_payload.get("_path"),
                "workspace_dir": (
                    None
                    if diff_payload is None and build_status is None
                    else (diff_payload or build_status).get("workspace_dir")
                ),
                "history_path": history_summary.get("_path"),
                "attempt_count": int(history_summary.get("attempt_count") or 0),
                "build_attempt_count": int(
                    history_summary.get("build_attempt_count") or 0
                ),
                "diff_attempt_count": int(
                    history_summary.get("diff_attempt_count") or 0
                ),
                "permuter_attempt_count": int(
                    history_summary.get("permuter_attempt_count") or 0
                ),
                "scored_attempt_count": int(
                    history_summary.get("scored_attempt_count") or 0
                ),
                "non_improving_scored_attempts": int(
                    history_summary.get("non_improving_scored_attempts") or 0
                ),
                "best_objdiff_match_percent": history_summary.get(
                    "best_objdiff_match_percent"
                ),
                "best_asm_score": history_summary.get("best_asm_score"),
                "stalled": bool(history_summary.get("stalled")),
                "match_bucket": match_bucket,
                "objdiff_match_percent": metrics.get("objdiff_match_percent"),
                "asm_score": metrics.get("asm_score")
                if metrics.get("asm_score") not in (None, "")
                else history_summary.get("best_asm_score"),
                "asm_exact": (
                    metrics.get("asm_score") not in (None, "")
                    and float(metrics.get("asm_score") or 0.0) == 0.0
                )
                or (
                    metrics.get("asm_score") in (None, "")
                    and history_summary.get("best_asm_score") not in (None, "")
                    and float(history_summary.get("best_asm_score") or 0.0) == 0.0
                ),
                "semantic_status": metrics.get("semantic_status"),
                "function_state": derive_function_state(
                    source_mapping=source_mapping,
                    build_status=build_status,
                    diff_payload=diff_payload,
                ),
            }
        )
    rows.sort(
        key=lambda item: (
            item["family"],
            item["program_path"],
            parse_hexish(item["entry_hex"]),
        )
    )
    return rows


def collect_match_values(rows: list[dict[str, Any]]) -> list[float]:
    values: list[float] = []
    for row in rows:
        value = row.get("objdiff_match_percent")
        if value in (None, ""):
            continue
        values.append(float(value))
    return values


def build_match_stats(rows: list[dict[str, Any]]) -> dict[str, Any]:
    values = collect_match_values(rows)
    positive_values = [value for value in values if value > 0.0]
    if not values:
        return {
            "matched_function_count": 0,
            "highest_objdiff_match_percent": None,
            "lowest_objdiff_match_percent": None,
            "average_objdiff_match_percent": None,
            "median_objdiff_match_percent": None,
        }
    return {
        "matched_function_count": len(positive_values),
        "highest_objdiff_match_percent": max(values),
        "lowest_objdiff_match_percent": min(values),
        "average_objdiff_match_percent": sum(values) / len(values),
        "median_objdiff_match_percent": float(median(values)),
    }


def count_asm_exact(rows: list[dict[str, Any]]) -> int:
    count = 0
    for row in rows:
        value = row.get("asm_score")
        if value in (None, ""):
            continue
        if float(value) == 0.0:
            count += 1
    return count


def build_program_rows(
    program_meta_rows: list[dict[str, Any]],
    *,
    function_summaries: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    stats_by_program: dict[str, dict[str, int]] = defaultdict(
        lambda: {
            "function_count": 0,
            "lifted_c_functions": 0,
            "build_ok_functions": 0,
            "build_failed_functions": 0,
            "diffed_functions": 0,
            "exact_match_functions": 0,
            "asm_exact_functions": 0,
            "attempted_functions": 0,
            "stalled_functions": 0,
        }
    )
    for row in function_summaries:
        stats = stats_by_program[str(row.get("program_path") or "")]
        stats["function_count"] += 1
        stats["lifted_c_functions"] += int(bool(row.get("has_source_mapping")))
        stats["build_ok_functions"] += int(bool(row.get("build_succeeded")))
        stats["build_failed_functions"] += int(
            bool(row.get("build_status_present"))
            and not bool(row.get("build_succeeded"))
        )
        stats["diffed_functions"] += int(bool(row.get("diff_report_present")))
        stats["exact_match_functions"] += int(
            float(row.get("objdiff_match_percent") or 0.0) >= 100.0
        )
        stats["asm_exact_functions"] += int(bool(row.get("asm_exact")))
        stats["attempted_functions"] += int(int(row.get("attempt_count") or 0) > 0)
        stats["stalled_functions"] += int(bool(row.get("stalled")))
        stats.setdefault("rows", []).append(row)

    rows: list[dict[str, Any]] = []
    for meta in program_meta_rows:
        program_path = str(meta.get("program_path") or "")
        stats = stats_by_program[program_path]
        function_count = int(stats["function_count"])
        exact_match_count = int(stats["exact_match_functions"])
        diffed_count = int(stats["diffed_functions"])
        lifted_count = int(stats["lifted_c_functions"])
        source_missing_count = max(function_count - lifted_count, 0)
        match_stats = build_match_stats(list(stats.get("rows") or []))
        if function_count == 0:
            program_state = "import_pending"
        elif exact_match_count and exact_match_count == function_count:
            program_state = "match_mature"
        elif diffed_count:
            program_state = "match_partial"
        elif lifted_count:
            program_state = "partial_coverage"
        else:
            program_state = "frontier_pending"
        rows.append(
            {
                "program_path": program_path,
                "program_name": meta.get("program_name"),
                "program_slug": meta.get("program_slug"),
                "source_hint": meta.get("source_hint"),
                "program_kind": classify_program_path(program_path),
                "family": infer_family(
                    program_path=program_path,
                    source_hint=str(meta.get("source_hint") or ""),
                    overlay_family=None
                    if meta.get("family") is None
                    else str(meta.get("family")),
                ),
                "archive_id": meta.get("archive_id"),
                "entry_index": meta.get("entry_index"),
                "duplicate_group_key": duplicate_group_key_from_row(meta),
                "duplicate_group_size": int(meta.get("duplicate_group_size") or 0),
                "function_count": function_count,
                "lifted_c_functions": lifted_count,
                "functions_without_source": source_missing_count,
                "source_coverage_percent": 0.0
                if function_count == 0
                else round((float(lifted_count) / float(function_count)) * 100.0, 3),
                "build_ok_functions": int(stats["build_ok_functions"]),
                "build_failed_functions": int(stats["build_failed_functions"]),
                "diffed_functions": diffed_count,
                "exact_match_functions": exact_match_count,
                "asm_exact_functions": int(stats["asm_exact_functions"]),
                "attempted_functions": int(stats["attempted_functions"]),
                "stalled_functions": int(stats["stalled_functions"]),
                **match_stats,
                "program_state": program_state,
            }
        )
    rows.sort(key=lambda item: (item["family"], item["program_path"]))
    return rows


def build_entry_rows(
    code_entry_rows: list[dict[str, Any]],
    *,
    program_rows: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    program_by_path = {str(row.get("program_path") or ""): row for row in program_rows}
    grouped: dict[tuple[str, int], dict[str, Any]] = {}
    for row in code_entry_rows:
        archive_id = str(row.get("archive_id") or "")
        entry_index = int(row.get("entry_index") or 0)
        key = (archive_id, entry_index)
        summary = grouped.get(key)
        if summary is None:
            summary = {
                "archive_id": archive_id,
                "entry_index": entry_index,
                "family": row.get("family"),
                "load_arg": row.get("load_arg"),
                "size": row.get("size"),
                "payload_path": row.get("payload_path"),
                "sha256": row.get("sha256"),
                "duplicate_group_key": duplicate_group_key_from_row(row),
                "duplicate_group_size": int(row.get("duplicate_group_size") or 0),
                "entry_table_confidence": row.get("entry_table_confidence"),
                "program_paths": [],
                "program_states": [],
                "function_count": 0,
            }
            grouped[key] = summary
        program_path = row.get("program_path")
        if program_path in (None, ""):
            continue
        program_path_text = str(program_path)
        if program_path_text in summary["program_paths"]:
            continue
        program_row = program_by_path.get(program_path_text, {})
        summary["program_paths"].append(program_path_text)
        summary["program_states"].append(program_row.get("program_state"))
        summary["function_count"] += int(program_row.get("function_count") or 0)
    rows: list[dict[str, Any]] = []
    for item in grouped.values():
        imported_program_count = len(item["program_paths"])
        review_reason = review_overrides.likely_noncode_reason(item.get("payload_path"))
        if review_reason is not None:
            entry_state = "reviewed_non_code"
        elif imported_program_count == 0:
            entry_state = "candidate_missing_program"
        elif int(item["function_count"] or 0) == 0:
            entry_state = "candidate_missing_functions"
        else:
            entry_state = "candidate_imported"
        rows.append(
            {
                **item,
                "imported_program_count": imported_program_count,
                "entry_state": entry_state,
                "review_reason": review_reason,
            }
        )
    rows.sort(
        key=lambda item: (
            str(item.get("family") or ""),
            str(item.get("archive_id") or ""),
            int(item.get("entry_index") or 0),
        )
    )
    return rows


def build_family_rows(
    entry_rows: list[dict[str, Any]],
    *,
    program_rows: list[dict[str, Any]],
    function_rows: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    families: dict[str, dict[str, Any]] = defaultdict(
        lambda: {
            "code_candidate_entries": 0,
            "entries_missing_programs": 0,
            "entries_missing_functions": 0,
            "reviewed_non_code_entries": 0,
            "imported_programs": set(),
            "programs": 0,
            "inventory_functions": 0,
            "lifted_c_functions": 0,
            "build_ok_functions": 0,
            "build_failed_functions": 0,
            "diffed_functions": 0,
            "exact_match_functions": 0,
            "asm_exact_functions": 0,
            "attempted_functions": 0,
            "stalled_functions": 0,
            "duplicate_groups": set(),
            "multi_entry_duplicate_groups": set(),
        }
    )
    for row in entry_rows:
        family = str(row.get("family") or "UNKNOWN")
        summary = families[family]
        summary["code_candidate_entries"] += 1
        summary["entries_missing_programs"] += int(
            row.get("entry_state") == "candidate_missing_program"
        )
        summary["entries_missing_functions"] += int(
            row.get("entry_state") == "candidate_missing_functions"
        )
        summary["reviewed_non_code_entries"] += int(
            row.get("entry_state") == "reviewed_non_code"
        )
        for program_path in row.get("program_paths") or []:
            summary["imported_programs"].add(str(program_path))
        duplicate_group_key = row.get("duplicate_group_key")
        if duplicate_group_key:
            summary["duplicate_groups"].add(str(duplicate_group_key))
            if int(row.get("duplicate_group_size") or 0) > 1:
                summary["multi_entry_duplicate_groups"].add(str(duplicate_group_key))
    for row in program_rows:
        family = str(row.get("family") or "UNKNOWN")
        summary = families[family]
        summary["programs"] += 1
    for row in function_rows:
        family = str(row.get("family") or "UNKNOWN")
        summary = families[family]
        summary["inventory_functions"] += 1
        summary["lifted_c_functions"] += int(bool(row.get("has_source_mapping")))
        summary["build_ok_functions"] += int(bool(row.get("build_succeeded")))
        summary["build_failed_functions"] += int(
            bool(row.get("build_status_present"))
            and not bool(row.get("build_succeeded"))
        )
        summary["diffed_functions"] += int(bool(row.get("diff_report_present")))
        summary["exact_match_functions"] += int(
            float(row.get("objdiff_match_percent") or 0.0) >= 100.0
        )
        summary["asm_exact_functions"] += int(bool(row.get("asm_exact")))
        summary["attempted_functions"] += int(int(row.get("attempt_count") or 0) > 0)
        summary["stalled_functions"] += int(bool(row.get("stalled")))
        summary.setdefault("rows", []).append(row)

    rows: list[dict[str, Any]] = []
    for family, summary in sorted(families.items()):
        match_stats = build_match_stats(list(summary.get("rows") or []))
        rows.append(
            {
                "family": family,
                "code_candidate_entries": int(summary["code_candidate_entries"]),
                "entries_missing_programs": int(summary["entries_missing_programs"]),
                "entries_missing_functions": int(summary["entries_missing_functions"]),
                "reviewed_non_code_entries": int(summary["reviewed_non_code_entries"]),
                "imported_programs": len(summary["imported_programs"]),
                "programs": int(summary["programs"]),
                "inventory_functions": int(summary["inventory_functions"]),
                "lifted_c_functions": int(summary["lifted_c_functions"]),
                "build_ok_functions": int(summary["build_ok_functions"]),
                "build_failed_functions": int(summary["build_failed_functions"]),
                "diffed_functions": int(summary["diffed_functions"]),
                "exact_match_functions": int(summary["exact_match_functions"]),
                "asm_exact_functions": int(summary["asm_exact_functions"]),
                "attempted_functions": int(summary["attempted_functions"]),
                "stalled_functions": int(summary["stalled_functions"]),
                "duplicate_groups": len(summary["duplicate_groups"]),
                "multi_entry_duplicate_groups": len(
                    summary["multi_entry_duplicate_groups"]
                ),
                **match_stats,
            }
        )
    return rows


def build_summary(
    inventory_health: dict[str, Any],
    *,
    entry_rows: list[dict[str, Any]],
    program_rows: list[dict[str, Any]],
    function_rows: list[dict[str, Any]],
    unresolved_source_mappings: list[dict[str, Any]],
) -> dict[str, Any]:
    program_kind_counts = summarize_program_kinds(program_rows, key="program_path")
    function_kind_counts = summarize_program_kinds(function_rows, key="program_path")
    match_stats = build_match_stats(function_rows)
    lifted_c_functions = sum(
        1 for row in function_rows if row.get("has_source_mapping")
    )
    attempted_functions = sum(
        1 for row in function_rows if int(row.get("attempt_count") or 0) > 0
    )
    stalled_functions = sum(1 for row in function_rows if bool(row.get("stalled")))
    group_sizes: dict[str, int] = {}
    for row in entry_rows:
        group_key = row.get("duplicate_group_key")
        if not group_key:
            continue
        group_sizes[str(group_key)] = max(
            int(group_sizes.get(str(group_key), 0)),
            int(row.get("duplicate_group_size") or 0),
        )
    multi_group_sizes = [size for size in group_sizes.values() if size > 1]
    summary = {
        "inventory": inventory_health,
        "code_candidate_entries": len(entry_rows),
        "code_entries_missing_programs": sum(
            1
            for row in entry_rows
            if row.get("entry_state") == "candidate_missing_program"
        ),
        "code_entries_missing_functions": sum(
            1
            for row in entry_rows
            if row.get("entry_state") == "candidate_missing_functions"
        ),
        "reviewed_non_code_entries": sum(
            1 for row in entry_rows if row.get("entry_state") == "reviewed_non_code"
        ),
        "programs": len(program_rows),
        "imported_overlay_programs": sum(
            1 for row in program_rows if row.get("archive_id") not in (None, "")
        ),
        "bin_programs": int(program_kind_counts["bin"]),
        "boot_programs": int(program_kind_counts["boot"]),
        "logo_programs": int(program_kind_counts["logo"]),
        "other_programs": int(program_kind_counts["other"]),
        "inventory_functions": len(function_rows),
        "bin_functions": int(function_kind_counts["bin"]),
        "boot_functions": int(function_kind_counts["boot"]),
        "logo_functions": int(function_kind_counts["logo"]),
        "other_functions": int(function_kind_counts["other"]),
        "lifted_c_functions": lifted_c_functions,
        "functions_without_source": max(len(function_rows) - lifted_c_functions, 0),
        "source_coverage_percent": 0.0
        if not function_rows
        else round((float(lifted_c_functions) / float(len(function_rows))) * 100.0, 3),
        "attempted_functions": attempted_functions,
        "stalled_functions": stalled_functions,
        "build_ok_functions": sum(
            1 for row in function_rows if row.get("build_succeeded")
        ),
        "build_failed_functions": sum(
            1
            for row in function_rows
            if row.get("build_status_present") and not row.get("build_succeeded")
        ),
        "diffed_functions": sum(
            1 for row in function_rows if row.get("diff_report_present")
        ),
        "exact_match_functions": sum(
            1
            for row in function_rows
            if float(row.get("objdiff_match_percent") or 0.0) >= 100.0
        ),
        "asm_exact_functions": count_asm_exact(function_rows),
        "unresolved_source_mappings": len(unresolved_source_mappings),
        "duplicate_groups": len(group_sizes),
        "multi_entry_duplicate_groups": len(multi_group_sizes),
        "entries_in_multi_groups": sum(multi_group_sizes),
        "largest_duplicate_group": max(group_sizes.values(), default=0),
        **match_stats,
    }
    blocking_issues: list[str] = []
    if int(inventory_health.get("slot_map") or 0) == 0:
        blocking_issues.append(
            "slot_map has no rows; slot-based validation is not ready"
        )
    if int(inventory_health.get("disc_lba_entries") or 0) == 0:
        blocking_issues.append(
            "disc_lba_entries has no rows; LBA-based validation is not ready"
        )
    if int(summary["code_entries_missing_programs"]) > 0:
        blocking_issues.append(
            f"{summary['code_entries_missing_programs']} code-candidate EMI entries are missing program rows"
        )
    if int(summary["unresolved_source_mappings"]) > 0:
        blocking_issues.append(
            f"{summary['unresolved_source_mappings']} source mappings could not be tied back to inventory functions"
        )
    summary["blocking_issues"] = blocking_issues
    summary["campaign_ready"] = not blocking_issues
    return summary


def build_scoreboard_payload(
    *,
    inventory_db: Path,
    match_root: Path,
    source_root: Path,
    artifact_root: Path,
) -> dict[str, Any]:
    inventory_health = load_inventory_health(inventory_db)
    code_entry_rows = load_code_entry_rows(inventory_db)
    function_rows = dedupe_function_inventory_rows(
        workspace_lib.load_function_rows(inventory_db)
    )
    program_rows = dedupe_program_rows(workspace_lib.load_program_rows(inventory_db))
    program_meta_rows = dedupe_program_rows(load_program_overlay_rows(inventory_db))
    program_meta_by_path = {
        canonical_program_path(str(row.get("program_path") or "")): row
        for row in program_meta_rows
    }
    source_mappings, unresolved_source_mappings = collect_resolved_source_mappings(
        function_rows,
        program_rows=program_rows,
        artifact_root=artifact_root,
        source_root=source_root,
    )
    build_statuses = collect_latest_workspace_payloads(match_root, "build.json")
    diff_payloads = collect_latest_workspace_payloads(match_root, "diff.json")
    history_summaries = collect_workspace_history_summaries(match_root)
    function_rows = dedupe_function_inventory_rows(
        function_rows
        + infer_missing_function_rows(
            function_rows,
            program_rows=program_rows,
            source_mappings=source_mappings,
            build_statuses=build_statuses,
            diff_payloads=diff_payloads,
            history_summaries=history_summaries,
            artifact_root=artifact_root,
        )
    )
    function_summaries = build_function_rows(
        function_rows,
        program_meta_by_path=program_meta_by_path,
        source_mappings=source_mappings,
        build_statuses=build_statuses,
        diff_payloads=diff_payloads,
        history_summaries=history_summaries,
    )
    program_summaries = build_program_rows(
        program_meta_rows,
        function_summaries=function_summaries,
    )
    entry_summaries = build_entry_rows(
        code_entry_rows,
        program_rows=program_summaries,
    )
    family_summaries = build_family_rows(
        entry_summaries,
        program_rows=program_summaries,
        function_rows=function_summaries,
    )
    summary = build_summary(
        inventory_health,
        entry_rows=entry_summaries,
        program_rows=program_summaries,
        function_rows=function_summaries,
        unresolved_source_mappings=unresolved_source_mappings,
    )
    return {
        "generated_at": utc_now(),
        "inventory_db": str(inventory_db),
        "match_root": str(match_root),
        "source_root": str(source_root),
        "artifact_root": str(artifact_root),
        "summary": summary,
        "families": family_summaries,
        "entries": entry_summaries,
        "programs": program_summaries,
        "functions": function_summaries,
        "unresolved_source_mappings": unresolved_source_mappings,
    }


def render_summary_json(payload: dict[str, Any]) -> str:
    return json.dumps(payload, indent=2, sort_keys=True) + "\n"


def _render_tsv_rows(header: list[str], rows: list[dict[str, Any]]) -> str:
    lines = ["\t".join(header)]
    for row in rows:
        values: list[str] = []
        for column in header:
            value = row.get(column)
            values.append("" if value is None else str(value))
        lines.append("\t".join(values))
    return "\n".join(lines) + "\n"


def render_tsv(function_rows: list[dict[str, Any]]) -> str:
    header = [
        "family",
        "program_path",
        "entry_hex",
        "name",
        "archive_id",
        "entry_index",
        "duplicate_group_key",
        "duplicate_group_size",
        "function_state",
        "source_file",
        "source_function",
        "build_succeeded",
        "diff_status",
        "match_bucket",
        "objdiff_match_percent",
        "asm_score",
        "asm_exact",
        "semantic_status",
        "attempt_count",
        "diff_attempt_count",
        "permuter_attempt_count",
        "non_improving_scored_attempts",
        "best_objdiff_match_percent",
        "best_asm_score",
        "stalled",
    ]
    lines = ["\t".join(header)]
    for row in function_rows:
        lines.append(
            "\t".join(
                [
                    str(row.get("family") or ""),
                    str(row.get("program_path") or ""),
                    str(row.get("entry_hex") or ""),
                    str(row.get("name") or ""),
                    str(row.get("archive_id") or ""),
                    str(row.get("entry_index") or ""),
                    str(row.get("duplicate_group_key") or ""),
                    str(row.get("duplicate_group_size") or ""),
                    str(row.get("function_state") or ""),
                    str(row.get("source_file") or ""),
                    str(row.get("source_function") or ""),
                    str(row.get("build_succeeded") or ""),
                    str(row.get("diff_status") or ""),
                    str(row.get("match_bucket") or ""),
                    str(row.get("objdiff_match_percent") or ""),
                    str(row.get("asm_score") or ""),
                    str(row.get("asm_exact") or ""),
                    str(row.get("semantic_status") or ""),
                    str(row.get("attempt_count") or ""),
                    str(row.get("diff_attempt_count") or ""),
                    str(row.get("permuter_attempt_count") or ""),
                    str(row.get("non_improving_scored_attempts") or ""),
                    str(row.get("best_objdiff_match_percent") or ""),
                    str(row.get("best_asm_score") or ""),
                    str(row.get("stalled") or ""),
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_programs_tsv(program_rows: list[dict[str, Any]]) -> str:
    return _render_tsv_rows(
        [
            "family",
            "program_path",
            "program_name",
            "archive_id",
            "entry_index",
            "duplicate_group_key",
            "duplicate_group_size",
            "function_count",
            "lifted_c_functions",
            "functions_without_source",
            "source_coverage_percent",
            "attempted_functions",
            "stalled_functions",
            "build_ok_functions",
            "build_failed_functions",
            "diffed_functions",
            "exact_match_functions",
            "asm_exact_functions",
            "matched_function_count",
            "highest_objdiff_match_percent",
            "lowest_objdiff_match_percent",
            "average_objdiff_match_percent",
            "median_objdiff_match_percent",
            "program_state",
        ],
        program_rows,
    )


def render_families_tsv(family_rows: list[dict[str, Any]]) -> str:
    return _render_tsv_rows(
        [
            "family",
            "code_candidate_entries",
            "entries_missing_programs",
            "entries_missing_functions",
            "reviewed_non_code_entries",
            "imported_programs",
            "programs",
            "inventory_functions",
            "lifted_c_functions",
            "attempted_functions",
            "stalled_functions",
            "build_ok_functions",
            "build_failed_functions",
            "diffed_functions",
            "exact_match_functions",
            "asm_exact_functions",
            "duplicate_groups",
            "multi_entry_duplicate_groups",
            "matched_function_count",
            "highest_objdiff_match_percent",
            "lowest_objdiff_match_percent",
            "average_objdiff_match_percent",
            "median_objdiff_match_percent",
        ],
        family_rows,
    )


def render_entries_tsv(entry_rows: list[dict[str, Any]]) -> str:
    lines = [
        "\t".join(
            [
                "family",
                "archive_id",
                "entry_index",
                "payload_path",
                "duplicate_group_key",
                "duplicate_group_size",
                "imported_program_count",
                "function_count",
                "entry_state",
                "entry_table_confidence",
                "review_reason",
                "program_paths",
            ]
        )
    ]
    for row in entry_rows:
        lines.append(
            "\t".join(
                [
                    str(row.get("family") or ""),
                    str(row.get("archive_id") or ""),
                    str(row.get("entry_index") or ""),
                    str(row.get("payload_path") or ""),
                    str(row.get("duplicate_group_key") or ""),
                    str(row.get("duplicate_group_size") or ""),
                    str(row.get("imported_program_count") or ""),
                    str(row.get("function_count") or ""),
                    str(row.get("entry_state") or ""),
                    str(row.get("entry_table_confidence") or ""),
                    str(row.get("review_reason") or ""),
                    ",".join(str(path) for path in row.get("program_paths") or []),
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_brief_rows(
    function_rows: list[dict[str, Any]], *, limit: int = 5
) -> list[str]:
    lines: list[str] = []
    for row in function_rows[: max(limit, 0)]:
        name = (
            row.get("source_function")
            or row.get("name")
            or row.get("entry_hex")
            or "<unknown>"
        )
        lines.append(
            f"{name}: {row.get('function_state') or 'unknown'}, "
            f"bucket {row.get('match_bucket') or 'unknown'}, "
            f"match {row.get('objdiff_match_percent') if row.get('objdiff_match_percent') not in (None, '') else 'n/a'}, "
            f"asm {'exact' if bool(row.get('asm_exact')) else row.get('asm_score') if row.get('asm_score') not in (None, '') else 'n/a'}, "
            f"attempts {int(row.get('attempt_count') or 0)}, "
            f"stalled {'yes' if bool(row.get('stalled')) else 'no'}, "
            f"{row.get('program_path') or ''}"
        )
    return lines


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "scoreboard"),
        description=(
            "Audit inventory coverage and emit a duplicate-aware decomp scoreboard."
        ),
    )
    add_logging_args(parser)
    parser.add_argument("--inventory-db", type=Path, default=DEFAULT_INVENTORY_DB)
    parser.add_argument("--match-root", type=Path, default=DEFAULT_MATCH_ROOT)
    parser.add_argument("--source-root", type=Path, default=DEFAULT_SOURCE_ROOT)
    parser.add_argument(
        "--artifact-root",
        type=Path,
        default=workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
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
    logger = logger_from_args(args, "match_scoreboard")
    if not args.inventory_db.exists():
        logger.error(f"inventory db not found: {args.inventory_db}")
        return 1
    payload = build_scoreboard_payload(
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
    write_text_output(output_tsv, render_tsv(payload["functions"]))
    logger.summary(
        " ".join(
            [
                f"functions={len(payload['functions'])}",
                f"entries={len(payload['entries'])}",
                f"campaign_ready={payload['summary']['campaign_ready']}",
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
