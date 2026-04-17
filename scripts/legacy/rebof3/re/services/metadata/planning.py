from __future__ import annotations

import json
import re
from pathlib import Path

from ....inventory.db.connection import inventory_db
from ....inventory.layout import INVENTORY_SQLITE
from ....inventory.repositories.programs import ProgramRepository
from ....models.metadata import (
    MetadataSyncBatch,
    MetadataSyncPlan,
    MetadataSyncRowPlan,
    MetadataTypeNormalization,
)
from .. import resolver


KIND_CHOICES = (
    "function",
    "data",
    "label",
    "symbol",
    "structure",
    "enum",
    "typedef",
    "all",
)
SYNC_PHASES = {
    "structure": "types",
    "enum": "types",
    "typedef": "types",
    "data": "data",
    "function": "functions",
    "label": "labels",
    "symbol": "labels",
}
SYNC_ORDER = (
    "structure",
    "enum",
    "typedef",
    "data",
    "function",
    "label",
    "symbol",
)
ANALYSIS_LABELISH_DATA_RE = re.compile(
    r"^(switchD(?:_[0-9a-fA-F]+)?|caseD_[0-9a-fA-F]+|default)$"
)


def selected_program_paths(
    *,
    db_path: Path,
    owner: str | None,
    selectors: tuple[str, ...],
) -> tuple[str, ...]:
    if selectors:
        return tuple(dict.fromkeys(selectors))
    if not owner:
        return ()
    owner_upper = owner.upper()
    with inventory_db(db_path) as connection:
        rows = connection.execute(
            "SELECT program_path, folder FROM programs ORDER BY program_path"
        ).fetchall()
    resolved: list[str] = []
    for row in rows:
        program_path = str(row["program_path"] or "")
        if not program_path:
            continue
        if program_path == "/boot/SLUS_004.22":
            candidate_owner = "SLUS"
        else:
            candidate_owner = Path(str(row["folder"] or "")).name.upper()
        if candidate_owner == owner_upper:
            resolved.append(program_path)
    return tuple(resolved)


def selected_program_selectors(
    *,
    db_path: Path,
    owner: str | None,
    selectors: tuple[str, ...],
) -> tuple[str, ...]:
    resolved: list[str] = []
    with inventory_db(db_path) as connection:
        programs = ProgramRepository(connection)
        if selectors:
            selected_programs = selectors
        else:
            selected_programs = selected_program_paths(
                db_path=db_path,
                owner=owner,
                selectors=(),
            )
        for program_path in selected_programs:
            selector = programs.resolve_program_selector(program_path=program_path)
            if selector not in resolved:
                resolved.append(selector)
    return tuple(resolved)


def _load_rows(
    *,
    db_path: Path,
    requested_kind: str,
    selectors: tuple[str, ...],
) -> list[dict[str, object]]:
    clauses: list[str] = []
    params: list[object] = []
    if requested_kind != "all":
        clauses.append("kind = ?")
        params.append(requested_kind)
    if selectors:
        clauses.append("(" + " OR ".join(["program_path = ?"] * len(selectors)) + ")")
        params.extend(selectors)
    where_sql = f"WHERE {' AND '.join(clauses)}" if clauses else ""
    query = (
        "SELECT row_key, program_path, kind, address_key, address, path, name, comment, "
        "repeatable_comment, type_spec, source, confidence, extra_json "
        f"FROM metadata_rows {where_sql} ORDER BY program_path, kind, COALESCE(address_key, path, row_key), row_key"
    )
    with inventory_db(db_path) as connection:
        rows = connection.execute(query, params).fetchall()
    payloads: list[dict[str, object]] = []
    for row in rows:
        extra_payload: dict[str, object] = {}
        raw_extra = row["extra_json"]
        if raw_extra:
            try:
                decoded = json.loads(str(raw_extra))
                if isinstance(decoded, dict):
                    extra_payload = decoded
            except json.JSONDecodeError:
                extra_payload = {}
        payload = {key: row[key] for key in row.keys() if key != "extra_json"}
        payload.update(extra_payload)
        payloads.append(payload)
    return payloads


def _defined_type_names(rows: list[dict[str, object]]) -> set[str]:
    names: set[str] = set()
    for row in rows:
        if str(row.get("kind") or "") not in {"structure", "enum", "typedef"}:
            continue
        name = str(row.get("name") or "").strip()
        if name:
            names.add(name)
        path_text = str(row.get("path") or "").strip("/")
        if path_text:
            names.add(path_text.split("/")[-1])
    return names


def _normalized_row_payload(
    row: dict[str, object], normalization: MetadataTypeNormalization
) -> dict[str, object]:
    payload = dict(row)
    if normalization.normalized:
        payload["type_spec"] = normalization.normalized
    if (
        normalization.original
        and normalization.normalized
        and normalization.original != normalization.normalized
    ):
        payload["original_type_spec"] = normalization.original
        payload["normalized_type_spec"] = normalization.normalized
    return payload


def _resolved_program_selectors_by_path(
    *,
    db_path: Path,
    rows: list[dict[str, object]],
) -> dict[str, str]:
    program_paths = sorted(
        {
            str(row.get("program_path") or "").strip()
            for row in rows
            if str(row.get("program_path") or "").strip()
        }
    )
    selectors_by_path: dict[str, str] = {}
    if not program_paths:
        return selectors_by_path
    with inventory_db(db_path) as connection:
        programs = ProgramRepository(connection)
        for program_path in program_paths:
            selectors_by_path[program_path] = programs.resolve_program_selector(
                program_path=program_path
            )
    return selectors_by_path


def _synthesized_typedef_row(
    *,
    function_row: dict[str, object],
    typedef_name: str,
    target_type: str,
) -> dict[str, object]:
    program_path = function_row.get("program_path")
    function_name = str(function_row.get("name") or "function")
    row_key = f"{program_path}|typedef|{typedef_name}"
    return {
        "row_key": row_key,
        "program_path": program_path,
        "kind": "typedef",
        "address_key": None,
        "address": None,
        "path": f"/generated/typedefs/{typedef_name}",
        "name": typedef_name,
        "comment": f"Synthesized typedef for function-pointer parameter in {function_name}.",
        "repeatable_comment": None,
        "type_spec": f"typedef {typedef_name}",
        "target_type": target_type,
        "source": "metadata_sync",
        "confidence": "medium",
        "synthetic": True,
    }


def _should_skip_missing_type_row(row: dict[str, object], row_kind: str) -> bool:
    if row_kind != "data":
        return False
    name_source = str(row.get("name_source") or "").strip().upper()
    if name_source == "DEFAULT":
        return True
    if name_source != "ANALYSIS":
        return False
    name = str(row.get("name") or "").strip()
    return bool(ANALYSIS_LABELISH_DATA_RE.fullmatch(name))


def build_plan(
    *,
    db_path: Path = INVENTORY_SQLITE,
    owner: str | None = None,
    selectors: tuple[str, ...] = (),
    kind: str = "all",
    mode: str = "preflight",
    known_type_names: tuple[str, ...] = (),
) -> MetadataSyncPlan:
    selected_programs = selected_program_paths(
        db_path=db_path,
        owner=owner,
        selectors=selectors,
    )
    rows = _load_rows(
        db_path=db_path,
        requested_kind=kind,
        selectors=selected_programs,
    )
    program_selectors = _resolved_program_selectors_by_path(
        db_path=db_path,
        rows=rows,
    )
    defined_types = _defined_type_names(rows)
    satisfied_type_names = set(defined_types)
    satisfied_type_names.update(
        str(name).strip() for name in known_type_names if str(name).strip()
    )
    synthetic_rows: list[dict[str, object]] = []
    seen_synthetic_typedefs: set[str] = set()
    planned_rows: list[MetadataSyncRowPlan] = []
    for row in rows:
        row_kind = str(row.get("kind") or "")
        normalization = resolver.normalize_type_spec(
            row.get("type_spec"),
            kind=row_kind,
        )
        classification = "apply_direct"
        blocked_reason = None
        if row_kind == "function" and normalization.normalized:
            rewritten_signature, typedef_specs = (
                resolver.rewrite_function_pointer_signature(normalization.normalized)
            )
            if typedef_specs:
                row = dict(row)
                row["type_spec"] = rewritten_signature
                normalization = resolver.normalize_type_spec(
                    rewritten_signature,
                    kind=row_kind,
                )
                if normalization.original != str(row.get("type_spec") or "").strip():
                    row["original_type_spec"] = str(row.get("type_spec") or "").strip()
                    row["normalized_type_spec"] = normalization.normalized
                for typedef_spec in typedef_specs:
                    typedef_name = typedef_spec["name"]
                    if typedef_name in seen_synthetic_typedefs:
                        continue
                    synthetic_row = _synthesized_typedef_row(
                        function_row=row,
                        typedef_name=typedef_name,
                        target_type=typedef_spec["target_type"],
                    )
                    synthetic_rows.append(synthetic_row)
                    seen_synthetic_typedefs.add(typedef_name)
                    satisfied_type_names.add(typedef_name)
        if normalization.is_pseudo_type:
            classification = "skip_pseudo_type"
        elif normalization.status == "normalized":
            classification = "apply_normalized"
        elif normalization.status == "missing" and row_kind in {"label", "symbol"}:
            classification = "apply_direct"
        elif normalization.status == "missing" and row_kind in {"data", "function"}:
            if row_kind == "data" and str(row.get("name_source") or "").strip():
                classification = "apply_direct"
            elif _should_skip_missing_type_row(row, row_kind):
                classification = "skip_missing_type_spec"
            else:
                classification = "blocked_missing_type_spec"
                blocked_reason = normalization.reason
        dependency_names: tuple[str, ...] = ()
        if row_kind == "typedef":
            dependency_names = resolver.referenced_type_names(row.get("target_type"))
        elif row_kind == "structure":
            dependency_buffer: list[str] = []
            for component in row.get("components") or []:
                if not isinstance(component, dict):
                    continue
                for name in resolver.referenced_type_names(component.get("type_spec")):
                    if name not in dependency_buffer:
                        dependency_buffer.append(name)
            dependency_names = tuple(dependency_buffer)
        elif row_kind in {"data", "function"} and normalization.normalized:
            dependency_names = resolver.referenced_type_names(
                normalization.normalized,
                kind=row_kind,
            )
        if row_kind == "function" and normalization.normalized:
            if resolver.contains_unsupported_signature_shape(normalization.normalized):
                classification = "blocked_unsupported_signature"
                blocked_reason = "function signature needs typedef-first normalization"
        if (
            classification
            not in {
                "skip_pseudo_type",
                "skip_missing_type_spec",
                "blocked_missing_type_spec",
                "blocked_unsupported_signature",
            }
            and dependency_names
        ):
            missing = [
                name for name in dependency_names if name not in satisfied_type_names
            ]
            if missing:
                classification = "blocked_missing_dependency"
                blocked_reason = f"missing type dependencies: {', '.join(missing)}"
        planned_rows.append(
            MetadataSyncRowPlan(
                row_key=str(row.get("row_key") or ""),
                kind=row_kind,
                program_path=(
                    None
                    if row.get("program_path") is None
                    else str(row.get("program_path"))
                ),
                phase=SYNC_PHASES.get(row_kind, "other"),
                classification=classification,
                normalization=normalization,
                row=_normalized_row_payload(row, normalization),
                dependency_names=dependency_names,
                blocked_reason=blocked_reason,
            )
        )
    for row in synthetic_rows:
        normalization = resolver.normalize_type_spec(
            row.get("type_spec"), kind="typedef"
        )
        dependency_names = resolver.referenced_type_names(row.get("target_type"))
        classification = "apply_direct"
        blocked_reason = None
        if dependency_names:
            missing = [
                name for name in dependency_names if name not in satisfied_type_names
            ]
            if missing:
                classification = "blocked_missing_dependency"
                blocked_reason = f"missing type dependencies: {', '.join(missing)}"
        planned_rows.append(
            MetadataSyncRowPlan(
                row_key=str(row.get("row_key") or ""),
                kind="typedef",
                program_path=(
                    None
                    if row.get("program_path") is None
                    else str(row.get("program_path"))
                ),
                phase=SYNC_PHASES["typedef"],
                classification=classification,
                normalization=normalization,
                row=_normalized_row_payload(row, normalization),
                dependency_names=dependency_names,
                blocked_reason=blocked_reason,
            )
        )
    batches: list[MetadataSyncBatch] = []
    for ordered_kind in SYNC_ORDER:
        batch_rows = tuple(
            row
            for row in planned_rows
            if row.kind == ordered_kind
            and row.classification
            in {
                "apply_direct",
                "apply_normalized",
                "skip_pseudo_type",
                "skip_missing_type_spec",
            }
        )
        if batch_rows:
            batches.append(MetadataSyncBatch(phase=ordered_kind, rows=batch_rows))
    return MetadataSyncPlan(
        mode=mode,
        db_path=str(db_path),
        selector_scope=selected_programs,
        program_selectors=program_selectors,
        requested_kind=kind,
        total_rows=len(planned_rows),
        row_plans=tuple(planned_rows),
        batches=tuple(batches),
    )
