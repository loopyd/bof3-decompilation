"""Read-only aggregate reverse-analysis readiness."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.common.cli import add_root_argument, resolved_root, run_main

from ..analysis.index import connect
from ..analysis.project import status as project_status
from ..domain.ids import normalize_target_id
from ..domain.manifests import TargetManifest, load_target_manifests
from harness.naming.debt import collect_naming_debt


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


def _summaries(
    root: Path,
    manifests: dict[str, TargetManifest],
    targets: list[str],
    detail: bool = False,
) -> dict[str, object]:
    """Expose fresh inventory counts without constructing operation accounts."""
    connection = connect(root)
    try:
        target = targets[0] if len(targets) == 1 else None
        debt_rows = collect_naming_debt(
            root, {item: manifests[item] for item in targets}
        ).to_rows()
        if target is not None:
            sources = set(manifests[target].sources)
            debt_rows = {
                key: [
                    row
                    for row in rows
                    if row.startswith(f"{target}:")
                    or (key.endswith("files") and row in sources)
                ]
                for key, rows in debt_rows.items()
            }
        naming = {
            "inventory_count": len(debt_rows.get("raw_functions", ()))
            + len(debt_rows.get("raw_data", ())),
            "debt": {key: len(value) for key, value in debt_rows.items()},
        }
        if detail:
            naming["rows"] = debt_rows
        return {
            "naming": naming,
            "types": {
                "inventory_count": _count(connection, "type_declarations", target),
                "field_count": _count(connection, "type_fields", target),
                "usage_count": _count(connection, "type_usages", target),
                "conflict_count": _count(connection, "type_conflicts", target),
                "diagnostic_count": int(
                    connection.execute(
                        "SELECT COUNT(*) FROM type_declarations WHERE diagnostic IS NOT NULL"
                        + (" AND target_id = ?" if target else ""),
                        (target,) if target else (),
                    ).fetchone()[0]
                ),
            },
            "macros": {
                "inventory_count": _macro_count(
                    connection, "macro_definitions", target
                ),
                "use_count": _count(connection, "macro_uses", target),
                "template_count": _macro_count(connection, "macro_templates", target),
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
    snapshots = [
        project_status(root, item, manifest=manifests[item]) for item in targets
    ]
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
        "schema": "bof3.analysis-readiness/v3",
        "ready": ready,
        "index_ready": index_ready,
        "index_error": index_error,
        "snapshots": snapshots,
        "stale_facts": _stale_facts(snapshots, index_ready),
        "summaries": summaries,
        "recovery": None if ready else "bin/harness analysis index --recover",
    }


def run(args: argparse.Namespace) -> int:
    payload = readiness(resolved_root(args), args.target, args.detail == "full")
    print(json.dumps(payload, indent=2, sort_keys=True))
    return 0 if payload["ready"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness analysis readiness")
    add_root_argument(parser)
    parser.add_argument("target", nargs="?")
    parser.add_argument("--detail", choices=("summary", "full"), default="summary")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
