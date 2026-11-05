"""Current macro facts, repetition leads and mechanical consumer checks."""

from __future__ import annotations

from contextlib import closing
from pathlib import Path
import re
import sqlite3

from harness.analysis.index import connect
from harness.common.lexicon import mask_c_text
from harness.decomp import metadata
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.domain.manifests import load_target_manifests
from .blocks import block_opportunities_payload
from .impact import definition_impact_payload
from .opportunities import repetition_rows
from .queries import macros_payload, macro_uses_payload
from .similarity import near_duplicates_payload

KINDS = ("definitions", "uses", "repetitions", "byte-repeats", "all")


def _target(root: Path, target: str) -> str:
    target = normalize_target_id(target).value
    if target not in load_target_manifests(root):
        raise ValueError(f"unknown target: {target}")
    return target


def _definition(row: dict) -> dict:
    return {
        "record_kind": "definition",
        **{key: value for key, value in row.items() if key != "candidate_status"},
        "parameter_mentions": {
            name: len(
                re.findall(r"\b" + re.escape(name) + r"\b", mask_c_text(row["body"]))
            )
            for name in row["parameters"] or ()
        },
    }


def list_report(
    root: Path, target: str, *, kind: str = "definitions", limit: int | None = None
) -> dict:
    root = root.resolve()
    target = _target(root, target)
    if kind not in KINDS or limit is not None and limit < 1:
        raise ValueError("listing requires a supported kind and a positive limit")
    rows, findings = [], []
    try:
        with closing(connect(root)) as connection:
            connection.execute("PRAGMA query_only = ON")
            if kind in {"definitions", "all"}:
                rows.extend(
                    _definition(row)
                    for row in macros_payload(
                        connection,
                        target=target,
                        pattern=None,
                        classification=None,
                        limit=0,
                    )
                )
            if kind in {"uses", "all"}:
                rows.extend(
                    {
                        "record_kind": "use",
                        **{
                            key: value
                            for key, value in row.items()
                            if key != "candidate_status"
                        },
                    }
                    for row in macro_uses_payload(
                        connection, target=target, pattern=None, limit=0
                    )
                )
            if kind in {"repetitions", "all"}:
                rows.extend(
                    {"record_kind": "repetition", **row}
                    for row in repetition_rows(connection, root, target)
                )
            if kind == "byte-repeats":
                rows.extend(
                    {"record_kind": "function_similarity", **row}
                    for row in near_duplicates_payload(
                        connection, root, target=None, limit=0
                    )
                    if any(member["target"] == target for member in row["members"])
                )
                rows.extend(
                    {"record_kind": "byte_repetition", **row}
                    for row in block_opportunities_payload(
                        connection, root, min_instructions=4
                    )
                    if any(member["target"] == target for member in row["members"])
                )
    except (OSError, RuntimeError, ValueError, sqlite3.Error) as error:
        findings.append({"status": "unavailable", "message": str(error)})
    rows.sort(
        key=lambda row: (
            row["record_kind"],
            row.get("source_path", ""),
            row.get("source_line", 0),
            row.get("name", row.get("spelling", row.get("id", ""))),
        )
    )
    return {
        "operation": "macros",
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
    findings, definitions = [], []
    manifest = load_target_manifests(root)[target]
    support_sources = {str(path) for path in manifest.support_sources}
    # Index ids spell addresses without the 0x prefix, so every identity here is
    # normalised through parse_function_id before it is compared or reported.
    consumers = {str(parse_function_id(str(value))) for value in selected}
    try:
        with closing(connect(root, targets=(target,))) as connection:
            connection.execute("PRAGMA query_only = ON")
            # A target-scoped check covers the macros this target defines and
            # the macros its functions consume. Shared macros owned elsewhere are
            # checked where they are owned; counting their corpus-wide uses here
            # made every target look like it needed all 395 sibling targets.
            definitions = [
                row
                for row in macros_payload(
                    connection, target=target, pattern=None, classification=None, limit=0
                )
                if row["owner_target"] == target
            ]
            owned_ids = {row["id"] for row in definitions}
            uses = macro_uses_payload(
                connection, target=target, pattern=None, limit=0, generated=False
            )
            if selected:
                ids = {
                    row["definition_id"]
                    for row in uses
                    if row["function_id"]
                    and parse_function_id(row["function_id"]) in selected
                }
            else:
                ids = owned_ids | {
                    row["definition_id"] for row in uses if row["function_id"]
                }
            impact = definition_impact_payload(connection, ids)
            definitions = impact["definitions"]
            for row in definitions:
                if row["diagnostic"]:
                    findings.append(
                        {
                            "status": "unavailable",
                            "code": "definition",
                            "source": row["source_path"],
                            "message": row["diagnostic"],
                        }
                    )
                if re.search(r"##|(?<!#)#\s*[A-Za-z_]", mask_c_text(row["body"])):
                    findings.append(
                        {
                            "status": "unavailable",
                            "code": "computed_expansion",
                            "source": row["source_path"],
                            "message": "stringification or token pasting needs explicit expansion and consumer review",
                        }
                    )
                parameters = row["parameters"]
                if parameters is not None and len(parameters) != len(set(parameters)):
                    findings.append(
                        {
                            "status": "failed",
                            "code": "parameters",
                            "source": row["source_path"],
                            "message": f"duplicate macro parameters: {row['name']}",
                        }
                    )
            foreign = set()
            for use in impact["uses"]:
                if use["generated"] or use["use_context"] == "definition_body":
                    continue
                if use["target_id"] != target:
                    # Only a definition this target owns creates an obligation to
                    # check someone else's consumers.
                    if use["definition_id"] in owned_ids:
                        foreign.add(use["target_id"])
                elif use["function_id"]:
                    consumers.add(str(parse_function_id(use["function_id"])))
                elif (
                    use["source_path"] in support_sources
                    or use["source_path"].endswith((".h", ".inc"))
                ):
                    # A declaration or include-guard use names no function by
                    # construction; it is a coverage note, not a gap.
                    findings.append(
                        {
                            "status": "coverage",
                            "code": "header_use",
                            "source": use["source_path"],
                            "message": "header-scope macro use; consumers are the functions including it",
                        }
                    )
                else:
                    findings.append(
                        {
                            "status": "unavailable",
                            "code": "consumer",
                            "source": use["source_path"],
                            "message": "macro use has no native function identity",
                        }
                    )
            authored = {
                str(parse_function_id(row[0]))
                for row in connection.execute(
                    "SELECT id FROM functions WHERE target_id = ? AND lifted = 1 "
                    "AND source IS NOT NULL",
                    (target,),
                )
            }
            consumers &= authored
            if foreign:
                findings.append(
                    {
                        "status": "coverage",
                        "code": "shared_consumers",
                        "targets": sorted(foreign),
                        "message": f"macro consumers require explicit checks of {len(foreign)} other targets",
                    }
                )
            if impact["unresolved_definition_uses"]:
                findings.append(
                    {
                        "status": "unavailable",
                        "code": "definition_binding",
                        "message": "unresolved containing macro definitions",
                    }
                )
    except (OSError, RuntimeError, ValueError, sqlite3.Error) as error:
        findings.append(
            {"status": "unavailable", "code": "macro_inputs", "message": str(error)}
        )
    # Mechanical consistency only. The per-consumer native comparison that used
    # to run here compiled every consumer separately, which is one build per
    # function - the same evidence `lift gate` produces from a single target
    # build for exactly these selectors. Duplicating it made one macro question
    # cost minutes, so byte evidence stays with the gate and this check stays
    # cheap enough to run on every edit.
    result = {
        "operation": "macros",
        "action": "check",
        "scope": {"target": target, "selectors": [], "whole_target": not bool(selected)},
        "consumers": sorted(consumers),
        "total": 0,
        "truncated": False,
        "records": [],
        "findings": [],
        "incomplete": False,
        "passed": True,
        "live": "not measured here; run bin/harness lift gate for the same selectors",
    }
    result["operation"] = "macros"
    result["scope"]["selectors"] = sorted(str(value) for value in selected)
    result["scope"]["whole_target"] = not bool(selected)
    result["definitions"] = [_definition(row) for row in definitions]
    result["findings"].extend(findings)
    blocking = [row for row in result["findings"] if row["status"] != "coverage"]
    result["incomplete"] = any(row["status"] == "unavailable" for row in blocking)
    result["passed"] = not blocking
    return result
