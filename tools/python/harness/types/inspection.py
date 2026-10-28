"""Read-only declaration, use, storage and representation checks."""

from __future__ import annotations

from contextlib import closing
from pathlib import Path
import sqlite3
import re

from harness.common.lexicon import mask_c_text

from harness.analysis.index import connect
from harness.decomp import metadata
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.domain.manifests import load_target_manifests
from .queries import types_payload, type_usages_payload, type_candidates_payload

KINDS = ("declarations", "uses", "storage", "all")


def _target(root: Path, target: str) -> str:
    target = normalize_target_id(target).value
    if target not in load_target_manifests(root):
        raise ValueError(f"unknown target: {target}")
    return target


def _facts(root: Path, target: str) -> tuple[list[dict], list[dict], list[dict]]:
    with closing(connect(root)) as connection:
        connection.execute("PRAGMA query_only = ON")
        declarations = types_payload(
            connection, target=target, pattern=None, untyped=False, limit=0
        )
        uses = type_usages_payload(connection, target=target, pattern=None, limit=0)
        storage = type_candidates_payload(
            connection, target=target, status=None, limit=0
        )
    return declarations, uses, storage


def list_report(
    root: Path, target: str, *, kind: str = "declarations", limit: int | None = None
) -> dict:
    root = root.resolve()
    target = _target(root, target)
    if kind not in KINDS or limit is not None and limit < 1:
        raise ValueError("listing requires a supported kind and a positive limit")
    rows, findings = [], []
    try:
        declarations, uses, storage = _facts(root, target)
        if kind in {"declarations", "all"}:
            rows.extend({"record_kind": "declaration", **row} for row in declarations)
        if kind in {"uses", "all"}:
            rows.extend({"record_kind": "use", **row} for row in uses)
        if kind in {"storage", "all"}:
            rows.extend(
                {
                    "record_kind": "storage_lead",
                    **{
                        key: value
                        for key, value in row.items()
                        if key
                        not in {"status", "representation_status", "semantic_status"}
                    },
                }
                for row in storage
            )
    except (OSError, RuntimeError, ValueError, sqlite3.Error) as error:
        findings.append({"status": "unavailable", "message": str(error)})
    rows.sort(
        key=lambda row: (
            row["record_kind"],
            row.get("source_path", ""),
            row.get("name", row.get("subject", row.get("id", ""))),
        )
    )
    return {
        "operation": "types",
        "action": "list",
        "scope": {"target": target, "kind": kind},
        "total": len(rows),
        "truncated": limit is not None and len(rows) > limit,
        "records": rows if limit is None else rows[:limit],
        "findings": findings,
        "incomplete": bool(findings),
        "passed": not findings,
    }


def check_report(root: Path, target: str, selectors: tuple[str, ...] = ()) -> dict:
    root = root.resolve()
    target = _target(root, target)
    selected = {parse_function_id(value) for value in selectors}
    if any(value.target.value != target for value in selected):
        raise ValueError("every selector must belong to the selected target")
    findings, declarations = [], []
    consumers = {str(value) for value in selected}
    try:
        declarations, uses, _storage = _facts(root, target)
        if selected:
            selected_uses = [
                row
                for row in uses
                if row["function_id"]
                and parse_function_id(row["function_id"]) in selected
            ]
            names = {row["type_name"] for row in selected_uses}
            relevant = {
                row["id"]
                for row in declarations
                if row["diagnostic"]
                or row["name"] in names
                or f"{row['kind']} {row['name']}" in names
            }
            while True:
                active = [row for row in declarations if row["id"] in relevant]
                representations = {row["representation_id"] for row in active}
                active_names = {row["name"] for row in active}
                dependencies = {
                    name
                    for row in active
                    for name in re.findall(
                        r"\b[A-Za-z_][A-Za-z0-9_]*\b", mask_c_text(row["canonical"])
                    )
                }
                expanded = relevant | {
                    row["id"]
                    for row in declarations
                    if row["representation_id"] in representations
                    or row["name"] in dependencies
                    or active_names
                    & set(
                        re.findall(
                            r"\b[A-Za-z_][A-Za-z0-9_]*\b", mask_c_text(row["canonical"])
                        )
                    )
                }
                if expanded == relevant:
                    break
                relevant = expanded
            declarations = [row for row in declarations if row["id"] in relevant]
            names.update(row["name"] for row in declarations)
            names.update(f"{row['kind']} {row['name']}" for row in declarations)
            consumers.update(
                row["function_id"]
                for row in uses
                if row["function_id"] and row["type_name"] in names
            )
        for row in declarations:
            if row["diagnostic"]:
                findings.append(
                    {
                        "status": "unavailable",
                        "code": "declaration",
                        "source": row["source_path"],
                        "message": row["diagnostic"],
                    }
                )
            if (
                row["complete"]
                and row["kind"] in {"struct", "union"}
                and row["byte_size"] is None
            ):
                findings.append(
                    {
                        "status": "unavailable",
                        "code": "layout",
                        "source": row["source_path"],
                        "message": f"unknown layout for {row['name']}",
                    }
                )
            findings.extend(
                {
                    "status": "failed",
                    "code": "representation_conflict",
                    "source": conflict["source_path"],
                    "message": f"{row['name']}: {conflict['conflict_kind']}: {conflict['left_value']} != {conflict['right_value']}",
                }
                for conflict in row["conflicts"]
            )
        with closing(connect(root)) as connection:
            connection.execute("PRAGMA query_only = ON")
            foreign = set()
            for row in declarations:
                foreign.update(
                    item[0]
                    for item in connection.execute(
                        "SELECT DISTINCT target_id FROM type_declarations WHERE source_path = ? AND name = ? AND namespace = ? AND target_id != ?",
                        (row["source_path"], row["name"], row["namespace"], target),
                    )
                )
            if foreign:
                findings.append(
                    {
                        "status": "unavailable",
                        "code": "shared_consumers",
                        "targets": sorted(foreign),
                        "message": f"shared declaration consumers require explicit checks of {len(foreign)} other targets",
                    }
                )
    except (OSError, RuntimeError, ValueError, sqlite3.Error) as error:
        findings.append(
            {"status": "unavailable", "code": "type_inputs", "message": str(error)}
        )
    result = metadata.report(
        root, target, check=True, selectors=tuple(sorted(consumers)) if selected else ()
    )
    try:
        with closing(connect(root)):
            pass
    except (OSError, RuntimeError, ValueError, sqlite3.Error) as error:
        findings.append(
            {"status": "unavailable", "code": "freshness", "message": str(error)}
        )
    result["operation"] = "types"
    result["scope"]["selectors"] = sorted(str(value) for value in selected)
    result["scope"]["whole_target"] = not bool(selected)
    result["declarations"] = declarations
    result["findings"].extend(findings)
    result["incomplete"] = any(
        row["status"] == "unavailable" for row in result["findings"]
    )
    result["passed"] = not result["findings"]
    return result
