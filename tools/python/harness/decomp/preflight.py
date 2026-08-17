"""Lift build preflight: cache-aware batch diff planning and execution."""

from __future__ import annotations

from pathlib import Path
import re
from typing import Any, Callable, Iterable

from ..build.operations import batch_build, cmake_target_for_source, configure
from ..domain import TargetManifest
from ..domain.claims import manifest_source_paths
from ..domain.layout import ReviewedSplatLayout, parse_splat_layout
from ..domain.sources import (
    expected_lift_sources,
    source_expected_key,
)
from ..domain.tags import parse_behavior_tag, parse_source_tag
from ..io import repo_layout
from ..match._asm_diff_payload import AsmDiffRequest
from ..match._asm_diff_run import _asm_diff_compare, _asm_diff_resolve
from ..match.status_cache import (
    MatchStatusCache,
    source_fingerprint,
    target_fingerprint,
)


_UNDEFINED = re.compile(r"undefined reference to `([^']+)'")


DiffRunner = Callable[[AsmDiffRequest], dict[str, Any]]


_Record = dict[str, Any]


_WorkItem = tuple[str, Path, int, str, str, TargetManifest]


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
    cache: MatchStatusCache | None,
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
        try:
            layout: ReviewedSplatLayout | None = parse_splat_layout(
                root / manifest.splat, manifest.load_address
            )
            expected = expected_lift_sources(layout, source_dir)
        except (OSError, ValueError):
            expected = {}
        claimed: dict[int, Path] = {}
        for source in sorted(
            path
            for path in manifest_source_paths(root, manifest)
            if path.suffix == ".c"
        ):
            try:
                text = source.read_text(encoding="utf-8")
            except (OSError, UnicodeError) as exc:
                ready.append(_invalid_record(root, target, source, str(exc)))
                continue
            address = parse_source_tag(text)
            expected_key = source_expected_key(source_dir, source)
            expected_address = (
                None if expected_key is None else expected.get(expected_key)
            )
            if (
                address is None
                and expected_address is None
                and not re.match(r"^func_[0-9A-Fa-f]{8}$", source.stem)
            ):
                continue  # support/helper translation unit, not a lift
            if address is None:
                ready.append(
                    _invalid_record(
                        root,
                        target,
                        source,
                        "missing required metadata (@source)",
                    )
                )
                continue
            if expected_address is not None and address != expected_address:
                ready.append(
                    _invalid_record(
                        root,
                        target,
                        source,
                        f"source address disagrees with Splat boundary 0x{expected_address:08X}",
                        address,
                    )
                )
                continue
            if parse_behavior_tag(text) is None:
                ready.append(
                    _invalid_record(
                        root,
                        target,
                        source,
                        "missing required metadata (@behavior)",
                        address,
                    )
                )
                continue
            previous = claimed.get(address)
            if previous is not None:
                ready.append(
                    _invalid_record(
                        root,
                        target,
                        source,
                        f"duplicate address claim 0x{address:08X} (also {previous.name})",
                        address,
                    )
                )
                continue
            claimed[address] = source
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
    cache: MatchStatusCache | None,
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
