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
from .match.asm_diff import AsmDiffRequest, run_asm_diff_one
from .reverse_index import SCHEMA_VERSION, index_path
from .snapshot import read_snapshot, snapshot_path, validate_snapshot_identity


_SOURCE = re.compile(r"@source 0x[0-9A-F]{8}\b")
_BEHAVIOR = re.compile(r"@behavior (?:UNKNOWN: .+|[^\n]+)")
_UNDEFINED = re.compile(r"undefined reference to `([^']+)'")
_FUNCTION = re.compile(r"func_[0-9A-F]{8}\Z")

DiffRunner = Callable[[AsmDiffRequest], dict[str, Any]]


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


def collect_lifts(
    root: Path,
    manifests: Iterable[tuple[str, TargetManifest]],
    *,
    diff_runner: DiffRunner = run_asm_diff_one,
) -> list[dict[str, Any]]:
    """Compile and compare every lift in the supplied target manifests."""

    records: list[dict[str, Any]] = []
    for target, manifest in manifests:
        source_dir = root / manifest.source_dir
        for source in sorted(source_dir.glob("func_*.c")):
            if _FUNCTION.fullmatch(source.stem) is None:
                records.append(_invalid_record(root, target, source, "invalid lifted filename"))
                continue
            try:
                address = int(source.stem.removeprefix("func_"), 16)
            except ValueError:
                records.append(_invalid_record(root, target, source, "invalid lifted filename"))
                continue
            try:
                text = source.read_text(encoding="utf-8")
            except (OSError, UnicodeError) as exc:
                records.append(_invalid_record(root, target, source, str(exc), address))
                continue
            if _SOURCE.search(text) is None or _BEHAVIOR.search(text) is None:
                records.append(
                    _invalid_record(root, target, source, "missing required metadata", address)
                )
                continue
            try:
                result = diff_runner(
                    AsmDiffRequest(
                        source_path=source,
                        address=address,
                        binary_path=root / manifest.binary,
                        load_address=manifest.load_address,
                        output_root=root / "out" / "matching",
                    )
                )
            except (FileNotFoundError, RuntimeError, ValueError) as exc:
                records.append(
                    _invalid_record(root, target, source, _failure_detail(exc), address)
                )
                continue
            instructions = result["instruction_count"]
            records.append(
                {
                    "target": target,
                    "function": source.stem,
                    "address": f"0x{address:08X}",
                    "source": source.relative_to(root).as_posix(),
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
            )
    return records


def _binary_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def index_coverage(
    root: Path, manifests: Iterable[tuple[str, TargetManifest]]
) -> dict[str, int]:
    """Return index counts only when the index and source snapshots are fresh."""

    path = index_path(root)
    if not path.is_file():
        raise FileNotFoundError(f"reverse index not found: {path.relative_to(root)}; run just index")
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
                    raise ValueError("reverse index has invalid Rizin snapshot; run just index")
                if snapshot.inputs.get("binary_sha256") != _binary_hash(binary):
                    raise ValueError("reverse index has stale Rizin snapshot; run just index")
                row = connection.execute(
                    "SELECT binary_sha256 FROM targets WHERE id = ?", (target,)
                ).fetchone()
                if row is None or row[0] != _binary_hash(binary):
                    raise ValueError("reverse index is incomplete or stale; run just index")
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
) -> dict[str, Any]:
    """Build a complete live status report; index coverage is supplementary."""

    manifests = select_manifests(root, target_ids)
    records = collect_lifts(root, manifests, diff_runner=diff_runner)
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
        counts = {status: sum(row["status"] == status for row in functions) for status in totals}
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


def render_text(report: dict[str, Any]) -> str:
    """Render a complete deterministic human report without generated paths."""

    lifts = report["lifts"]
    coverage = (
        f"{lifts['total']}/{report['indexed_functions']}"
        if report["indexed_functions"] is not None
        else f"unavailable ({report['coverage_error']})"
    )
    lines = [
        "decompilation status",
        f"lifts: exact={lifts['exact']} partial={lifts['partial']} invalid={lifts['invalid']} total={lifts['total']}",
        f"index coverage: {coverage}",
    ]
    for target in report["targets"]:
        counts = target["lifts"]
        indexed = target["indexed_functions"]
        target_coverage = (
            f"{counts['total']}/{indexed}"
            if indexed is not None
            else "unavailable"
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
            instructions = function["instruction_count"]
            lines.append(
                f"  {function['status'].upper()} {function['function']}@{function['address']} "
                f"insn={instructions['matching']}/{max(instructions['original'], instructions['current'], 1)}"
                f"({function['match_percent']:.2f}%) "
                f"bytes={function['original_size']}->{function['current_size']}({function['size_delta']:+d})"
            )
    return "\n".join(lines)


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
