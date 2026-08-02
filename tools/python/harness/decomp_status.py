from .build import batch_build, cmake_target_for_source, configure
from .io import repo_layout
from .match.asm_diff import (
    AsmDiffRequest,
    run_asm_diff_one,
    _asm_diff_resolve,
    _asm_diff_compare,
)
from .match.status_cache import StatusCache, source_fingerprint, target_fingerprint
_Record = dict[str, Any]
_WorkItem = tuple[str, Path, int, str, str, TargetManifest]
"""Worklist item: (target, source_path, address, source_name, fingerprint_key, manifest)."""

def _batch_result(
    root: Path,
    target: str,
    source: Path,
    address: int,
    source_name: str,
    result: dict[str, Any],
) -> _Record:
    """Build a status record from a live comparison result."""
    return {
        "source": source_name,


def _build_preflight(
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
        target_key = target_fingerprint(root, manifest) if cache is not None else ""
                ready.append(
                ready.append(
                ready.append(_invalid_record(root, target, source, str(exc), address))
                ready.append(
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
        section_placements=manifest.section_placements.get(address, ()),


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
                        _invalid_record(
                            root, target, source, _failure_detail(exc), address
                        )
                    )
                    continue
                record = _batch_result(
                    root, target, source, address, source_name, result
                if cache is not None:
                    cache.put(target, source_name, key, record)
                records.append(record)

            for item, request, resolved in resolved_items:
                _, source, address, source_name, key, _manifest = item
                try:
                    result = _asm_diff_compare(repo, request, resolved)
                        _invalid_record(
                            root, target, source, _failure_detail(exc), address
                        )
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
    cache: StatusCache | None = None,
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
    use_cache: bool = True,
    try:
        cache = StatusCache(root) if use_cache else None
    except sqlite3.DatabaseError:
        cache = None
    try:
        records = collect_lifts(root, manifests, diff_runner=diff_runner, cache=cache)
    finally:
        if cache is not None:
            cache.close()
