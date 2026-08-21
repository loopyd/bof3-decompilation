"""Read-only aggregate reverse-analysis readiness."""

from __future__ import annotations

import argparse
import json
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


def _count(connection, table: str) -> int:
    return int(connection.execute(f"SELECT COUNT(*) FROM {table}").fetchone()[0])


def _summaries(
    root: Path, manifests: dict[str, TargetManifest], targets: list[str]
) -> dict[str, object]:
    connection = connect(root)
    try:
        expected = {
            target: inventory_expected(root, target, manifests) for target in targets
        }
        work = {
            target: required_work_snapshot(root, target, manifests[target], connection)
            for target in targets
        }
        debt = collect_naming_debt(root, manifests)
        type_report = type_account(root)
        macro_report = macro_account(root)
        naming_rows = {target: len(expected[target]) for target in targets}
        naming_work_graph = {
            target: [
                {
                    "kind": kind,
                    "name": name,
                    "status": "blocked",
                    "required_work": work[target].items(address_of(name), kind),
                }
                for kind, name in sorted(expected[target])
            ]
            for target in targets
        }
        naming_work = {
            target: sum(len(row["required_work"]) for row in naming_work_graph[target])
            for target in targets
        }
        return {
            "naming": {
                "inventory_count": sum(naming_rows.values()),
                "target_rows": naming_rows,
                "required_work_count": sum(naming_work.values()),
                "target_required_work": naming_work,
                "proposed_transactions": 0,
                "unresolved_evidence_ceiling_count": sum(naming_rows.values()),
                "debt": {key: len(value) for key, value in debt.to_rows().items()},
                "work_graph": naming_work_graph,
            },
            "types": {
                "inventory_count": _count(connection, "type_declarations"),
                "field_count": _count(connection, "type_fields"),
                "usage_count": _count(connection, "type_usages"),
                "conflict_count": _count(connection, "type_conflicts"),
                "diagnostic_count": int(
                    connection.execute(
                        "SELECT COUNT(*) FROM type_declarations WHERE diagnostic IS NOT NULL"
                    ).fetchone()[0]
                ),
                "candidate_count": type_report["candidate_count"],
                "proposed_transactions": type_report["counts"].get("proposed", 0),
                "unresolved_evidence_ceiling_count": type_report["counts"].get(
                    "blocked", 0
                ),
                "work_graph": type_report["rows"],
            },
            "macros": {
                "inventory_count": _count(connection, "macro_definitions"),
                "use_count": _count(connection, "macro_uses"),
                "template_count": _count(connection, "macro_templates"),
                "candidate_count": macro_report["candidate_count"],
                "proposed_transactions": macro_report["safe_application_count"],
                "unresolved_evidence_ceiling_count": macro_report["counts"].get(
                    "blocked", 0
                ),
                "work_graph": macro_report["rows"],
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


def readiness(root: Path, target: str | None = None) -> dict[str, object]:
    manifests = load_target_manifests(root)
    targets = [normalize_target_id(target).value] if target else sorted(manifests)
    unknown = [item for item in targets if item not in manifests]
    if unknown:
        raise ValueError(f"unknown target: {unknown[0]}")
    snapshots = [project_status(root, item) for item in targets]
    try:
        summaries = _summaries(root, manifests, targets)
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
    payload = readiness(resolved_root(args), args.target)
    print(json.dumps(payload, indent=2, sort_keys=True))
    return 0 if payload["ready"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="analysis-readiness")
    add_root_argument(parser)
    parser.add_argument("target", nargs="?")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
