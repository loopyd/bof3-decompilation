from __future__ import annotations

import json
import sqlite3
from pathlib import Path
from typing import Any

from ..common import format_hex, parse_hexish
from ..config import DEFAULT_GHIDRA_DECOMP_ROOT
from ..program_identity import normalize_program_selector, selector_matches, slugify
from . import source_map


DEFAULT_GHIDRA_ARTIFACT_ROOT = DEFAULT_GHIDRA_DECOMP_ROOT


def load_function_rows(inventory_db: Path) -> list[dict[str, Any]]:
    connection = sqlite3.connect(inventory_db)
    connection.row_factory = sqlite3.Row
    try:
        rows = connection.execute(
            """
            SELECT
                programs.program_name AS program_name,
                programs.program_path AS program_path,
                programs.program_slug AS program_slug,
                programs.folder AS folder,
                functions.entry_address AS entry,
                functions.entry_hex AS entry_hex,
                functions.name AS name,
                functions.signature AS signature,
                functions.namespace AS namespace,
                functions.name_source AS name_source,
                functions.source_hint AS source_hint,
                functions.comment AS comment,
                functions.repeatable_comment AS repeatable_comment
            FROM functions
            JOIN programs ON programs.id = functions.program_id
            ORDER BY programs.program_path, functions.entry_address, functions.name
            """
        ).fetchall()
    finally:
        connection.close()
    normalized: list[dict[str, Any]] = []
    for row in rows:
        normalized.append(
            {
                "program_name": row["program_name"],
                "program_path": row["program_path"],
                "program_slug": row["program_slug"],
                "folder": row["folder"],
                "entry": f"{int(row['entry']):08x}",
                "entry_hex": row["entry_hex"],
                "name": row["name"],
                "signature": row["signature"],
                "namespace": row["namespace"],
                "name_source": row["name_source"],
                "source_hint": row["source_hint"],
                "comment": row["comment"],
                "repeatable_comment": row["repeatable_comment"],
            }
        )
    return normalized


def load_program_rows(inventory_db: Path) -> list[dict[str, Any]]:
    connection = sqlite3.connect(inventory_db)
    connection.row_factory = sqlite3.Row
    try:
        try:
            rows = connection.execute(
                """
                SELECT
                    program_name,
                    program_path,
                    program_slug,
                    folder,
                    source_hint
                FROM programs
                ORDER BY program_path
                """
            ).fetchall()
        except sqlite3.OperationalError:
            rows = []
    finally:
        connection.close()
    return [
        {
            "program_name": row["program_name"],
            "program_path": row["program_path"],
            "program_slug": row["program_slug"],
            "folder": row["folder"],
            "source_hint": row["source_hint"],
        }
        for row in rows
    ]


def row_matches_program(row: dict[str, Any], selector: str) -> bool:
    for key in ("program_path", "program_name", "program_slug"):
        if selector_matches(row.get(key), selector):
            return True
    return False


def find_program_row(
    program_rows: list[dict[str, Any]],
    *,
    program: str,
) -> dict[str, Any] | None:
    matches = [row for row in program_rows if row_matches_program(row, program)]
    if not matches:
        return None
    if len(matches) > 1:
        raise LookupError(
            f"multiple programs matched selector={program}: "
            + ", ".join(sorted(str(row.get("program_path") or "?") for row in matches))
        )
    return matches[0]


def build_synthetic_function_row(
    program_row: dict[str, Any],
    *,
    entry: str,
    source_function: str | None = None,
    source_signature: str | None = None,
) -> dict[str, Any]:
    entry_value = parse_hexish(entry)
    return {
        "program_name": program_row.get("program_name"),
        "program_path": program_row.get("program_path"),
        "program_slug": program_row.get("program_slug"),
        "folder": program_row.get("folder"),
        "entry": f"{entry_value:08x}",
        "entry_hex": format_hex(entry_value),
        "name": source_function or f"func_{entry_value:08x}",
        "signature": source_signature,
        "namespace": None,
        "name_source": "SOURCE_FALLBACK",
        "source_hint": program_row.get("source_hint"),
        "comment": None,
        "repeatable_comment": None,
    }


def bundle_supports_entry(bundle_json: Path, *, entry_value: int) -> bool:
    try:
        payload = json.loads(bundle_json.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError, UnicodeDecodeError):
        return False

    requested = (
        payload.get("requested_address")
        or (payload.get("function") or {}).get("requested_address")
        or (payload.get("function") or {}).get("entry")
    )
    if requested is None:
        return False
    try:
        requested_value = parse_hexish(str(requested))
    except ValueError:
        return False
    if requested_value != entry_value:
        return False

    load_address = payload.get("load_address")
    if load_address not in (None, ""):
        try:
            load_value = parse_hexish(str(load_address))
        except ValueError:
            load_value = None
        if load_value is not None and entry_value < load_value:
            return False
    return True


def infer_function_row_for_program(
    *,
    program_row: dict[str, Any],
    entry: str,
    artifact_root: Path,
    source_root: Path = source_map.DEFAULT_SOURCE_ROOT,
    artifacts_dir_func: Any = None,
) -> dict[str, Any] | None:
    source_hint = str(program_row.get("source_hint") or "").strip()
    if not source_hint:
        return None
    mapping = source_map.find_source_mapping(
        entry,
        source_root=source_root,
        program_path=str(program_row.get("program_path") or ""),
        program_name=str(program_row.get("program_name") or ""),
        source_hint=source_hint,
    )
    if mapping is None:
        return None

    row = build_synthetic_function_row(
        program_row,
        entry=entry,
        source_function=str(mapping.get("source_function") or "") or None,
        source_signature=str(mapping.get("source_signature") or "") or None,
    )
    if artifacts_dir_func is None:
        from .workspace_store import suggested_artifacts_dir as artifacts_dir_func

    artifacts_dir = artifacts_dir_func(row, artifact_root, source_override=None)
    if artifacts_dir is None:
        return None
    bundle_json = artifacts_dir / "func.json"
    if not bundle_json.exists():
        return None
    if not bundle_supports_entry(bundle_json, entry_value=parse_hexish(entry)):
        return None
    return row


def infer_function_row_from_program_selector(
    program_rows: list[dict[str, Any]],
    *,
    program: str,
    entry: str,
    artifact_root: Path,
    source_root: Path = source_map.DEFAULT_SOURCE_ROOT,
) -> dict[str, Any] | None:
    program_row = find_program_row(program_rows, program=program)
    if program_row is None:
        return None
    return infer_function_row_for_program(
        program_row=program_row,
        entry=entry,
        artifact_root=artifact_root,
        source_root=source_root,
    )


def infer_function_row_for_mapping(
    mapping: dict[str, Any],
    *,
    program_rows: list[dict[str, Any]],
    artifact_root: Path,
) -> dict[str, Any] | None:
    entry = str(mapping.get("entry_hex") or "")
    if not entry:
        return None
    candidates: list[tuple[tuple[int, int, str], dict[str, Any]]] = []
    for program_row in program_rows:
        row = infer_function_row_for_program(
            program_row=program_row,
            entry=entry,
            artifact_root=artifact_root,
        )
        if row is None:
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


def find_function_row(
    rows: list[dict[str, Any]],
    *,
    program: str,
    entry: str,
    program_rows: list[dict[str, Any]] | None = None,
    artifact_root: Path = DEFAULT_GHIDRA_ARTIFACT_ROOT,
    source_root: Path = source_map.DEFAULT_SOURCE_ROOT,
) -> dict[str, Any]:
    target_entry = parse_hexish(entry)
    matches = [
        row
        for row in rows
        if row_matches_program(row, program)
        and parse_hexish(str(row.get("entry") or "0")) == target_entry
    ]
    if not matches:
        if program_rows is not None:
            inferred = infer_function_row_from_program_selector(
                program_rows,
                program=program,
                entry=entry,
                artifact_root=artifact_root,
                source_root=source_root,
            )
            if inferred is not None:
                return inferred
        raise LookupError(f"no function found for program={program} entry={entry}")
    if len(matches) > 1:
        candidates = ", ".join(
            sorted(str(row.get("program_path") or "?") for row in matches)
        )
        raise LookupError(
            f"multiple functions matched program={program} entry={entry}: {candidates}"
        )
    return matches[0]


def program_slug_for_row(row: dict[str, Any]) -> str:
    return str(
        row.get("program_slug")
        or slugify(str(row.get("program_path") or row.get("program_name") or "program"))
    )
