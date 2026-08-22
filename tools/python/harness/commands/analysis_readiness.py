"""Read-only aggregate reverse-analysis readiness."""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path

from ..analysis.index import connect
from ..analysis.macro_accounting import candidate_account as macro_account
from ..analysis.naming import inventory_expected
from ..analysis.naming_readiness import required_work_snapshot
from ..analysis.project import status as project_status
from ..analysis.type_transactions import candidate_account as type_account
from ..domain import load_target_manifests, normalize_target_id
from ..domain.manifests import TargetManifest
from ..domain.naming_debt import address_of, collect_naming_debt
from ._common import add_root_argument, resolved_root, run_main


def _count(
    connection, table: str, target: str | None = None, target_column: str = "target_id"
) -> int:
    where = "" if target is None else f" WHERE {target_column} = ?"
    parameters = () if target is None else (target,)
    return int(
        connection.execute(
            f"SELECT COUNT(*) FROM {table}{where}", parameters
        ).fetchone()[0]
    )


def _macro_count(connection, table: str, target: str | None) -> int:
    if target is None:
        return _count(connection, table)
    return int(
        connection.execute(
            f"SELECT COUNT(*) FROM {table} WHERE owner_target IN (?, '__shared__')",
            (target,),
        ).fetchone()[0]
    )


def _work_summary(rows: list[dict[str, object]]) -> list[dict[str, object]]:
    counts = Counter(str(row["status"]) for row in rows)
    return [
        {"status": status, "count": count} for status, count in sorted(counts.items())
    ]


def _target_macro_rows(
    rows: list[dict[str, object]], target: str | None
) -> list[dict[str, object]]:
    if target is None:
        return rows
    result = []
    for row in rows:
        targets = row.get("targets")
        if row.get("shared") is True or (
            isinstance(targets, list) and target in targets
        ):
            result.append(row)
    return result


def _summaries(
    root: Path,
    manifests: dict[str, TargetManifest],
    targets: list[str],
    detail: bool = False,
) -> dict[str, object]:
    connection = connect(root)
    try:
        target = targets[0] if len(targets) == 1 else None
        expected = {item: inventory_expected(root, item, manifests) for item in targets}
        work = {
            item: required_work_snapshot(root, item, manifests[item], connection)
            for item in targets
        }
        debt_rows = collect_naming_debt(root, manifests).to_rows()
        if target is not None:
            claimed_sources = set(manifests[target].sources)
            debt_rows = {
                key: [
                    row
                    for row in rows
                    if row.startswith(f"{target}:")
                    or (key.endswith("files") and row in claimed_sources)
                ]
                for key, rows in debt_rows.items()
            }
        type_report = type_account(root)
        type_rows = [
            row
            for row in type_report["rows"]
            if target is None or row["target"] == target
        ]
        macro_report = macro_account(root)
        macro_rows = _target_macro_rows(macro_report["rows"], target)
        naming_rows = {item: len(expected[item]) for item in targets}
        naming_detail = {
            item: [
                {
                    "kind": kind,
                    "name": name,
                    "status": "blocked",
                    "required_work": work[item].items(address_of(name), kind),
                }
                for kind, name in sorted(expected[item])
            ]
            for item in targets
        }
        naming_work = {
            item: sum(len(row["required_work"]) for row in naming_detail[item])
            for item in targets
        }
        naming_summary = [
            {
                "target": item,
                "inventory_count": naming_rows[item],
                "required_work_count": naming_work[item],
            }
            for item in targets
        ]
        type_counts = Counter(row["status"] for row in type_rows)
        macro_counts = Counter(row["status"] for row in macro_rows)
        return {
            "naming": {
                "inventory_count": sum(naming_rows.values()),
                "target_rows": naming_rows,
                "required_work_count": sum(naming_work.values()),
                "target_required_work": naming_work,
                "proposed_transactions": 0,
                "unresolved_evidence_ceiling_count": sum(naming_rows.values()),
                "debt": {key: len(value) for key, value in debt_rows.items()},
                "work_graph": naming_detail if detail else naming_summary,
            },
            "types": {
                "inventory_count": _count(connection, "type_declarations", target),
                "field_count": _count(connection, "type_fields", target),
                "usage_count": _count(connection, "type_usages", target),
                "conflict_count": _count(connection, "type_conflicts", target),
                "diagnostic_count": int(
                    connection.execute(
                        "SELECT COUNT(*) FROM type_declarations"
                        " WHERE diagnostic IS NOT NULL"
                        + (" AND target_id = ?" if target else ""),
                        (target,) if target else (),
                    ).fetchone()[0]
                ),
                "candidate_count": len(type_rows),
                "proposed_transactions": type_counts.get("proposed", 0),
                "unresolved_evidence_ceiling_count": type_counts.get("blocked", 0),
                "work_graph": type_rows if detail else _work_summary(type_rows),
            },
            "macros": {
                "inventory_count": _macro_count(
                    connection, "macro_definitions", target
                ),
                "use_count": _count(connection, "macro_uses", target),
                "template_count": _macro_count(connection, "macro_templates", target),
                "candidate_count": len(macro_rows),
                "proposed_transactions": macro_counts.get("accepted", 0),
                "unresolved_evidence_ceiling_count": macro_counts.get("blocked", 0),
                "work_graph": macro_rows if detail else _work_summary(macro_rows),
            },
        }
    finally:
        connection.close()


def _unavailable(error: str) -> dict[str, object]:
    return {
        "naming": {"available": False, "error": error},
        "types": {"available": False, "error": error},
        "macros": {"available": False, "error": error},
    }


def _stale_facts(
    snapshots: list[dict[str, object]], index_ready: bool
) -> dict[str, object]:
    stale_targets = sorted(
        str(item["target"]) for item in snapshots if not item.get("fresh")
    )
    return {
        "count": len(stale_targets) + int(not index_ready),
        "targets": stale_targets,
        "index": not index_ready,
    }


def readiness(
    root: Path, target: str | None = None, detail: bool = False
) -> dict[str, object]:
    manifests = load_target_manifests(root)
    targets = [normalize_target_id(target).value] if target else sorted(manifests)
    unknown = [item for item in targets if item not in manifests]
    if unknown:
        raise ValueError(f"unknown target: {unknown[0]}")
    snapshots = [project_status(root, item) for item in targets]
    try:
        summaries = _summaries(root, manifests, targets, detail)
        index_ready = True
        index_error = None
    except (FileNotFoundError, ValueError) as error:
        index_ready = False
        index_error = str(error)
        summaries = _unavailable(index_error)
    ready = index_ready and all(item.get("fresh") for item in snapshots)
    return {
        "schema": "bof3.analysis-readiness/v2",
        "ready": ready,
        "index_ready": index_ready,
        "index_error": index_error,
        "snapshots": snapshots,
        "stale_facts": _stale_facts(snapshots, index_ready),
        "summaries": summaries,
        "recovery": None if ready else "bin/index --recover",
    }


def run(args: argparse.Namespace) -> int:
    payload = readiness(resolved_root(args), args.target, args.detail == "full")
    print(json.dumps(payload, indent=2, sort_keys=True))
    return 0 if payload["ready"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="analysis-readiness")
    add_root_argument(parser)
    parser.add_argument("target", nargs="?")
    parser.add_argument("--detail", choices=("summary", "full"), default="summary")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
