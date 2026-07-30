"""Live, target-qualified status for every tracked C lift."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import sqlite3
import tempfile
from typing import Any, Callable, Iterable

from .domain import TargetManifest, load_target_manifests, normalize_target_id
from .build import batch_build, cmake_target_for_source, configure
from .io import repo_layout
from .match.asm_diff import (
    AsmDiffRequest,
    run_asm_diff_one,
    _asm_diff_resolve,
    _asm_diff_compare,
)
from .match.status_cache import StatusCache, source_fingerprint, target_fingerprint
from .reverse_index import SCHEMA_VERSION, index_path
from .snapshot import read_snapshot, snapshot_path, validate_snapshot_identity


_SOURCE = re.compile(r"@source 0x[0-9A-F]{8}\b")
_BEHAVIOR = re.compile(r"@behavior (?:UNKNOWN: .+|[^\n]+)")
_UNDEFINED = re.compile(r"undefined reference to `([^']+)'")
_FUNCTION = re.compile(r"func_[0-9A-F]{8}\Z")

DiffRunner = Callable[[AsmDiffRequest], dict[str, Any]]

_Record = dict[str, Any]
_WorkItem = tuple[str, Path, int, str, str, TargetManifest]
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


def _failure_detail(error: Exception) -> str:
    symbols = sorted(set(_UNDEFINED.findall(str(error))))
    if symbols:
        return f"unbound symbols: {', '.join(symbols)}"
    return str(error).splitlines()[0]


def _invalid_record(
    root: Path, target: str, source: Path, reason: str, address: int | None = None
) -> dict[str, Any]:
    return {
        "target": target,
        "function": source.stem,
        "address": None if address is None else f"0x{address:08X}",
        "source": source.relative_to(root).as_posix(),
        "status": "invalid",
        "reason": reason,
        "instruction_count": None,
        "match_percent": None,
        "original_size": None,
        "current_size": None,
        "size_delta": None,
    }


def _batch_result(
    root: Path,
    target: str,
    source: Path,
    address: int,
    source_name: str,
    result: dict[str, Any],
) -> _Record:
    """Build a status record from a live comparison result."""
    instructions = result["instruction_count"]
    return {
        "target": target,
        "function": source.stem,
        "address": f"0x{address:08X}",
        "source": source_name,
        "status": "exact" if result["byte_match"] else "partial",
        "reason": "",
        "instruction_count": {
            "original": instructions["original"],
            "current": instructions["current"],
            "matching": instructions["matching"],
        },
        "match_percent": instructions["match_percent"],
        "original_size": result["original_size"],
        "current_size": result["current_size"],
        "size_delta": result["size_delta"],
    }


def _build_preflight(
    root: Path,
    manifests: Iterable[tuple[str, TargetManifest]],
    cache: StatusCache | None,
) -> tuple[list[_Record], dict[str, list[_WorkItem]]]:
    """
    Phase 2.3.1 — separate audit discovery from comparison.

    Returns (ready_records, worklist_by_target) where:
    - ready_records contains invalid and cache-hit records (report-ready).
    - worklist_by_target groups valid cache misses by owning manifest target
      so the caller can batch-build per target.

    Ordering, cache semantics, and label rules are unchanged.
    """
    ready: list[_Record] = []
    worklist: dict[str, list[_WorkItem]] = {}

    for target, manifest in manifests:
        source_dir = root / manifest.source_dir
        target_key = target_fingerprint(root, manifest) if cache is not None else ""
        for source in sorted(source_dir.glob("func_*.c")):
            if _FUNCTION.fullmatch(source.stem) is None:
                ready.append(
                    _invalid_record(root, target, source, "invalid lifted filename")
                )
                continue
            try:
                address = int(source.stem.removeprefix("func_"), 16)
            except ValueError:
                ready.append(
                    _invalid_record(root, target, source, "invalid lifted filename")
                )
                continue
            try:
                text = source.read_text(encoding="utf-8")
            except (OSError, UnicodeError) as exc:
                ready.append(_invalid_record(root, target, source, str(exc), address))
                continue
            if _SOURCE.search(text) is None or _BEHAVIOR.search(text) is None:
                ready.append(
                    _invalid_record(
                        root, target, source, "missing required metadata", address
                    )
                )
                continue
            source_name = source.relative_to(root).as_posix()
            key = source_fingerprint(source, target_key) if cache is not None else ""
            record = cache.get(target, source_name, key) if cache is not None else None
            if record is not None:
                ready.append(record)
                continue
            worklist.setdefault(target, []).append(
                (target, source, address, source_name, key, manifest)
            )

    return ready, worklist


def _request_for_source(
    root: Path, source: Path, address: int, manifest: TargetManifest
) -> AsmDiffRequest:
    return AsmDiffRequest(
        source_path=source,
        address=address,
        binary_path=root / manifest.binary,
        load_address=manifest.load_address,
        output_root=root / "out" / "matching",
        section_placements=manifest.section_placements.get(address, ()),
    )


def _run_batch_misses(
    root: Path,
    worklist: dict[str, list[_WorkItem]],
    diff_runner: DiffRunner,
    cache: StatusCache | None,
) -> list[_Record]:
    """
    Phase 2.3.2 — build selected source objects once per target.

    For each owning target, batch-build all valid cache-miss lifts, then
    compare individually without rebuilding.  Falls back to per-source builds
    when a batch fails, preserving individual error attribution.
    """
    records: list[_Record] = []
    repo = repo_layout(root)

    for target, items in worklist.items():
        sources = [item[1] for item in items]
        cmake_targets = [cmake_target_for_source(root, s) for s in sources]
        batch_ok = True

        try:
            configure(root)
            result = batch_build(root, cmake_targets)
            if result.returncode != 0:
                batch_ok = False
        except (RuntimeError, ValueError, FileNotFoundError):
            batch_ok = False

        if batch_ok:
            # Check freshness before comparing so every work item produces one
            # record and stale objects cannot enter the matcher.
            resolved_items: list[tuple[_WorkItem, AsmDiffRequest, dict[str, Any]]] = []
            stale_items: list[_WorkItem] = []
            for item in items:
                _, source, address, _, _, manifest = item
                try:
                    request = _request_for_source(root, source, address, manifest)
                    resolved = _asm_diff_resolve(repo, request)
                    obj = resolved["object_path"]
                    if (
                        not obj.is_file()
                        or obj.stat().st_mtime < source.stat().st_mtime
                    ):
                        stale_items.append(item)
                        continue
                    resolved_items.append((item, request, resolved))
                except (FileNotFoundError, RuntimeError, ValueError):
                    stale_items.append(item)

            # A successful CMake request that did not refresh an object needs
            # the existing per-source path for a trustworthy diagnosis.
            for item in stale_items:
                _, source, address, source_name, key, manifest = item
                request = _request_for_source(root, source, address, manifest)
                try:
                    result = diff_runner(request)
                except (FileNotFoundError, RuntimeError, ValueError) as exc:
                    records.append(
                        _invalid_record(
                            root, target, source, _failure_detail(exc), address
                        )
                    )
                    continue
                record = _batch_result(
                    root, target, source, address, source_name, result
                )
                if cache is not None:
                    cache.put(target, source_name, key, record)
                records.append(record)

            for item, request, resolved in resolved_items:
                _, source, address, source_name, key, _manifest = item
                try:
                    result = _asm_diff_compare(repo, request, resolved)
                except (FileNotFoundError, RuntimeError, ValueError) as exc:
                    records.append(
                        _invalid_record(
                            root, target, source, _failure_detail(exc), address
                        )
                    )
                    continue
                record = _batch_result(
                    root, target, source, address, source_name, result
                )
                if cache is not None:
                    cache.put(target, source_name, key, record)
                records.append(record)
        else:
            # Batch failed — fall back to per-source build + compare
            for item in items:
                _, source, address, source_name, key, _manifest = item
                request = _request_for_source(root, source, address, _manifest)
                try:
                    result = diff_runner(request)
                except (FileNotFoundError, RuntimeError, ValueError) as exc:
                    records.append(
                        _invalid_record(
                            root, target, source, _failure_detail(exc), address
                        )
                    )
                    continue
                record = _batch_result(
                    root, target, source, address, source_name, result
                )
                if cache is not None:
                    cache.put(target, source_name, key, record)
                records.append(record)

    return records


def collect_lifts(
    root: Path,
    manifests: Iterable[tuple[str, TargetManifest]],
    *,
    diff_runner: DiffRunner = run_asm_diff_one,
    cache: StatusCache | None = None,
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


def _binary_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def index_coverage(
    root: Path, manifests: Iterable[tuple[str, TargetManifest]]
) -> dict[str, int]:
    """Return index counts only when the index and source snapshots are fresh."""

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
                if snapshot.inputs.get("binary_sha256") != _binary_hash(binary):
                    raise ValueError(
                        "reverse index has stale Rizin snapshot; run just index"
                    )
                row = connection.execute(
                    "SELECT binary_sha256 FROM targets WHERE id = ?", (target,)
                ).fetchone()
                if row is None or row[0] != _binary_hash(binary):
                    raise ValueError(
                        "reverse index is incomplete or stale; run just index"
                    )
                counts[target] = connection.execute(
                    "SELECT COUNT(*) FROM functions WHERE target_id = ?", (target,)
                ).fetchone()[0]
            return counts
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
        cache = StatusCache(root) if use_cache else None
    except sqlite3.DatabaseError:
        cache = None
    try:
        records = collect_lifts(root, manifests, diff_runner=diff_runner, cache=cache)
    finally:
        if cache is not None:
            cache.close()
    try:
        coverage = index_coverage(root, manifests)
        coverage_error: str | None = None
    except (FileNotFoundError, ValueError) as exc:
        coverage = {}
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
                "coverage_error": coverage_error,
                "functions": functions,
            }
        )
    return {
        "schema": "bof3.decomp-status/v1",
        "targets": targets,
        "lifts": {**totals, "total": len(records)},
        "indexed_functions": sum(coverage.values()) if coverage_error is None else None,
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
                f"bytes={function['original_size']}->{function['current_size']}({function['size_delta']:+d})"
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
