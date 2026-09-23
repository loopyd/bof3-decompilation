"""Lift build preflight: cache-aware batch diff planning and execution."""

from __future__ import annotations

from pathlib import Path
import re
from typing import Any, Callable, Iterable

from harness.common.deadlines import DeadlineExpired
from harness.domain.functions import collect_lift_metadata
from harness.domain.tags import count_function_metadata
from harness.domain.policy import validate_matching_source

from ..build.operations import batch_build, cmake_target_for_source
from ..domain.manifests import TargetManifest, load_target_manifests
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
        "compiled_symbol": None,
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
        "function": result.get("function", source.stem),
        "compiled_symbol": result.get("function"),
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
    checked: set[Path] = set()

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
            if count_function_metadata(text) > 1:
                try:
                    records = collect_lift_metadata(text)
                except ValueError as error:
                    ready.append(_invalid_record(root, target, source, str(error)))
                    continue
                ready.extend(
                    _invalid_record(
                        root,
                        target,
                        source,
                        "multi-function native comparison requires compilation-unit migration",
                        address,
                    )
                    for address in records
                )
                continue
            address = parse_source_tag(text)
            try:
                validate_matching_source(root, source, checked=checked)
            except ValueError as error:
                ready.append(_invalid_record(root, target, source, str(error), address))
                continue
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
                        "source address disagrees with Splat boundary "
                        + ", ".join(
                            f"0x{value:08X}"
                            for value in (
                                expected_address
                                if isinstance(expected_address, tuple)
                                else (expected_address,)
                            )
                        ),
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
            record = (
                cache.get(target, source_name, address, key)
                if cache is not None
                else None
            )
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


def _failed_objects(root: Path, result: Any) -> set[str]:
    """Repository-relative object paths the build system reported as ``FAILED``."""

    text = f"{result.stdout or ''}\n{result.stderr or ''}"
    failed: set[str] = set()
    for line in text.splitlines():
        if not line.startswith("FAILED:"):
            continue
        for token in line.split()[1:]:
            token = token.strip("'\"")
            if not token.endswith(".o"):
                continue
            path = Path(token)
            if path.is_absolute():
                candidate = path
            elif token.startswith("build/"):
                candidate = root / path
            else:
                candidate = root / "build" / path
            try:
                failed.add(candidate.resolve().relative_to(root.resolve()).as_posix())
            except (OSError, ValueError):
                continue
    return failed


def _batch_failed_objects(root: Path, cmake_targets: list[str]) -> set[str] | None:
    """Batch-build every target and report the objects that failed.

    ``None`` means the outcome cannot be attributed — the batch could not run,
    or it failed without naming an object — so the caller keeps the
    conservative per-source path for every item.
    """

    try:
        result = batch_build(root, cmake_targets)
    except DeadlineExpired:
        raise
    except (RuntimeError, ValueError, FileNotFoundError):
        return None
    if result.returncode == 0:
        return set()
    return _failed_objects(root, result) or None


def _object_key(root: Path, object_path: Path) -> str:
    """Repository-relative key for one built object."""

    try:
        return object_path.resolve().relative_to(root.resolve()).as_posix()
    except (OSError, ValueError):
        return str(object_path)


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

        # Ninja's own verdict decides the path.  One failing translation unit no
        # longer hides every later target, and only the objects the build
        # reported as FAILED need the per-source build for a trustworthy
        # diagnosis; the rest were built or verified current by the build
        # system itself.  An unattributable batch keeps the original
        # conservative per-source path for every item.
        failed_objects = _batch_failed_objects(root, cmake_targets)

        try:
            manifests = load_target_manifests(root)
        except DeadlineExpired:
            raise
        except (OSError, RuntimeError, ValueError) as error:
            for _, source, address, _, _, _manifest in items:
                records.append(
                    _invalid_record(
                        root,
                        target,
                        source,
                        f"target metadata unavailable after build: {error}",
                        address,
                    )
                )
            continue
        current_manifest = manifests.get(target)
        current_items = []
        for item in items:
            _, source, address, _, _, queued_manifest = item
            if current_manifest is None or current_manifest != queued_manifest:
                records.append(
                    _invalid_record(
                        root,
                        target,
                        source,
                        "target metadata changed during build; rerun the audit",
                        address,
                    )
                )
            else:
                current_items.append(item)
        items = current_items
        if not items:
            continue

        # Freshness and the build's own failure report decide the path.  Every
        # object the build did not report as FAILED was built or verified
        # current by ninja, so comparing it directly keeps the audit
        # proportional to the number of lifts instead of rebuilding each one.
        resolved_items: list[tuple[_WorkItem, AsmDiffRequest, dict[str, Any]]] = []
        stale_items: list[_WorkItem] = []
        for item in items:
            _, source, address, _, _, _manifest = item
            try:
                request = _request_for_source(root, source, address, current_manifest)
                resolved = _asm_diff_resolve(repo, request, manifests=manifests)
                obj = resolved["object_path"]
                if (
                    failed_objects is None
                    or _object_key(root, obj) in failed_objects
                    or not obj.is_file()
                    or obj.stat().st_mtime < source.stat().st_mtime
                ):
                    stale_items.append(item)
                    continue
                resolved_items.append((item, request, resolved))
            except DeadlineExpired:
                raise
            except (FileNotFoundError, RuntimeError, ValueError):
                stale_items.append(item)

        # A batch request that did not refresh an object needs the existing
        # per-source path for a trustworthy diagnosis.
        for item in stale_items:
            _, source, address, source_name, key, _manifest = item
            request = _request_for_source(root, source, address, current_manifest)
            try:
                result = diff_runner(request)
            except DeadlineExpired:
                raise
            except (FileNotFoundError, RuntimeError, ValueError) as exc:
                records.append(
                    _invalid_record(root, target, source, _failure_detail(exc), address)
                )
                continue
            record = _batch_result(root, target, source, address, source_name, result)
            if cache is not None:
                cache.put(target, source_name, address, key, record)
            records.append(record)

        for item, request, resolved in resolved_items:
            _, source, address, source_name, key, _manifest = item
            try:
                result = _asm_diff_compare(repo, request, resolved, manifests=manifests)
            except DeadlineExpired:
                raise
            except (FileNotFoundError, RuntimeError, ValueError) as exc:
                records.append(
                    _invalid_record(root, target, source, _failure_detail(exc), address)
                )
                continue
            record = _batch_result(root, target, source, address, source_name, result)
            if cache is not None:
                cache.put(target, source_name, address, key, record)
            records.append(record)

    return records
