"""Live, target-qualified status for every tracked C lift."""

from __future__ import annotations

import json
from pathlib import Path
import sqlite3
import tempfile
from typing import Any, Iterable

from ..analysis.index import SCHEMA_VERSION, index_path
from ..analysis.snapshot import read_snapshot, snapshot_path, validate_snapshot_identity
from ..discovery import file_sha256
from ..domain import TargetManifest, load_target_manifests, normalize_target_id
from ..match.asm_diff import (
    run_asm_diff_one,
)
from ..match.status_cache import MatchStatusCache

from .preflight import DiffRunner, _build_preflight, _run_batch_misses

"""Worklist item: (target, source_path, address, source_name, fingerprint_key, manifest)."""


def select_manifests(
    root: Path, target_ids: Iterable[str] = ()
) -> list[tuple[str, TargetManifest]]:
    """Resolve optional target operands and retain canonical target ordering."""

    manifests = load_target_manifests(root)
    requested = list(target_ids)
    if not requested:
        return sorted(manifests.items())
    selected: dict[str, TargetManifest] = {}
    for target in requested:
        normalized = normalize_target_id(target).value
        if normalized not in manifests:
            raise ValueError(f"unknown target: {target}")
        selected[normalized] = manifests[normalized]
    return sorted(selected.items())


def collect_lifts(
    root: Path,
    manifests: Iterable[tuple[str, TargetManifest]],
    *,
    diff_runner: DiffRunner = run_asm_diff_one,
    cache: MatchStatusCache | None = None,
) -> list[dict[str, Any]]:
    """Compile and compare every lift in the supplied target manifests.

    Uses a preflight + batch build strategy (Phases 2.3.1 and 2.3.2):
    1. Preflight separates invalid/cached from valid cache-miss sources.
    2. Valid misses are batch-built per owning target, then compared
       individually without issuing another build per source.
    3. On batch failure, falls back to per-source build-and-compare.
    """

    ready, worklist = _build_preflight(root, manifests, cache)
    batch_records = _run_batch_misses(root, worklist, diff_runner, cache)
    records = ready + batch_records
    records.sort(key=lambda r: (r["target"], r["source"]))
    return records


def index_coverage(
    root: Path, manifests: Iterable[tuple[str, TargetManifest]]
) -> tuple[dict[str, int], dict[str, list[dict[str, str]]]]:
    """Return index counts and contains-data functions; only when fresh."""

    path = index_path(root)
    if not path.is_file():
        raise FileNotFoundError(
            f"reverse index not found: {path.relative_to(root)}; run just index"
        )
    index_mtime = path.stat().st_mtime_ns
    try:
        connection = sqlite3.connect(path)
        try:
            schema = connection.execute(
                "SELECT value FROM metadata WHERE key = 'schema'"
            ).fetchone()
            if schema is None or schema[0] != SCHEMA_VERSION:
                raise ValueError("reverse index schema is stale; run just index")
            counts: dict[str, int] = {}
            contains_data: dict[str, list[dict[str, str]]] = {}
            for target, manifest in manifests:
                binary = root / manifest.binary
                snapshot_file = snapshot_path(root, target)
                if not snapshot_file.is_file():
                    raise ValueError(
                        f"missing Rizin snapshot: {snapshot_file.relative_to(root)}"
                    )
                if snapshot_file.stat().st_mtime_ns > index_mtime:
                    raise ValueError("reverse index is stale; run just index")
                snapshot = read_snapshot(snapshot_file)
                errors = validate_snapshot_identity(snapshot)
                if errors or snapshot.target != target:
                    raise ValueError(
                        "reverse index has invalid Rizin snapshot; run just index"
                    )
                if snapshot.inputs.get("binary_sha256") != file_sha256(binary):
                    raise ValueError(
                        "reverse index has stale Rizin snapshot; run just index"
                    )
                row = connection.execute(
                    "SELECT binary_sha256 FROM targets WHERE id = ?", (target,)
                ).fetchone()
                if row is None or row[0] != file_sha256(binary):
                    raise ValueError(
                        "reverse index is incomplete or stale; run just index"
                    )
                counts[target] = connection.execute(
                    "SELECT COUNT(*) FROM functions WHERE target_id = ?", (target,)
                ).fetchone()[0]
                contains_data[target] = [
                    {"address": f"0x{row[0]:08X}", "name": row[1]}
                    for row in connection.execute(
                        "SELECT address, name FROM functions "
                        "WHERE target_id = ? AND contains_data",
                        (target,),
                    )
                ]
            return counts, contains_data
        finally:
            connection.close()
    except sqlite3.DatabaseError as exc:
        raise ValueError(f"invalid reverse index: {exc}") from exc


def build_report(
    root: Path,
    target_ids: Iterable[str] = (),
    *,
    diff_runner: DiffRunner = run_asm_diff_one,
    use_cache: bool = True,
) -> dict[str, Any]:
    """Build a complete live status report; index coverage is supplementary."""

    manifests = select_manifests(root, target_ids)
    try:
        cache = MatchStatusCache(root) if use_cache else None
    except sqlite3.DatabaseError:
        cache = None
    try:
        records = collect_lifts(root, manifests, diff_runner=diff_runner, cache=cache)
    finally:
        if cache is not None:
            cache.close()
    try:
        coverage, contains_data = index_coverage(root, manifests)
        coverage_error: str | None = None
    except (FileNotFoundError, ValueError) as exc:
        coverage = {}
        contains_data = {}
        coverage_error = str(exc)

    targets: list[dict[str, Any]] = []
    totals = {"exact": 0, "partial": 0, "invalid": 0}
    for target, _ in manifests:
        functions = [record for record in records if record["target"] == target]
        counts = {
            status: sum(row["status"] == status for row in functions)
            for status in totals
        }
        for status, count in counts.items():
            totals[status] += count
        targets.append(
            {
                "target": target,
                "lifts": {**counts, "total": len(functions)},
                "indexed_functions": coverage.get(target),
                "contains_data": contains_data.get(target, []),
                "coverage_error": coverage_error,
                "functions": functions,
            }
        )
    return {
        "schema": "bof3.decomp-status/v1",
        "targets": targets,
        "lifts": {**totals, "total": len(records)},
        "indexed_functions": sum(coverage.values()) if coverage_error is None else None,
        "contains_data": [row for rows in contains_data.values() for row in rows],
        "coverage_error": coverage_error,
    }


def render_text(report: dict[str, Any], detail: str = "full") -> str:
    """Render deterministic status at the requested context budget."""

    lifts = report["lifts"]
    coverage = (
        f"{lifts['total']}/{report['indexed_functions']}"
        if report["indexed_functions"] is not None
        else f"unavailable ({report['coverage_error']})"
    )
    lines = [
        f"lifts: exact={lifts['exact']} partial={lifts['partial']} invalid={lifts['invalid']} total={lifts['total']}",
    ]
    if detail == "minimal":
        return lines[0]
    lines.insert(0, "decompilation status")
    lines.append(f"index coverage: {coverage}")
    for target in report["targets"]:
        counts = target["lifts"]
        indexed = target["indexed_functions"]
        target_coverage = (
            f"{counts['total']}/{indexed}" if indexed is not None else "unavailable"
        )
        lines.append(
            f"{target['target']}: exact={counts['exact']} partial={counts['partial']} invalid={counts['invalid']} lifts={counts['total']} indexed={target_coverage}"
        )
        for entry in target.get("contains_data", []):
            lines.append(
                f"  CONTAINS-DATA {entry['name']}@{entry['address']} not liftable until the Splat segment is split"
            )
        for function in target["functions"]:
            if function["status"] == "invalid":
                lines.append(
                    f"  INVALID {function['function']}@{function['address'] or '?'} {function['reason']}"
                )
                continue
            if detail != "full":
                continue
            instructions = function["instruction_count"]
            lines.append(
                f"  {function['status'].upper()} {function['function']}@{function['address']} "
                f"insn={instructions['matching']}/{max(instructions['original'], instructions['current'], 1)}"
                f"({function['match_percent']:.2f}%) "
                f"bytes={function['original_size']}->{function['current_size']}({function['size_delta'] if function['size_delta'] is not None else 0:+d})"
            )
    return "\n".join(lines)


def project_report(report: dict[str, Any], detail: str) -> dict[str, Any]:
    """Project display JSON without changing the complete persisted report."""

    if detail == "full":
        return report
    projected: dict[str, Any] = {
        "schema": report["schema"],
        "lifts": report["lifts"],
        "indexed_functions": report["indexed_functions"],
        "contains_data": report.get("contains_data", []),
        "coverage_error": report["coverage_error"],
    }
    if detail == "minimal":
        return projected
    projected["targets"] = [
        {
            "target": target["target"],
            "lifts": target["lifts"],
            "indexed_functions": target["indexed_functions"],
            "invalid": [
                function
                for function in target["functions"]
                if function["status"] == "invalid"
            ],
        }
        for target in report["targets"]
    ]
    return projected


def write_report(path: Path, report: dict[str, Any]) -> None:
    """Atomically persist the JSON representation requested by the caller."""

    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", dir=path.parent, delete=False
    ) as stream:
        temporary = Path(stream.name)
        json.dump(report, stream, indent=2, sort_keys=True)
        stream.write("\n")
    temporary.replace(path)
