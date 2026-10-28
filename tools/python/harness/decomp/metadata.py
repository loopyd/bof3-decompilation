"""Read authored function claims and compare them with current native evidence."""

from __future__ import annotations

from collections import Counter
from contextlib import closing
from pathlib import Path
import sqlite3

from harness.common.process import ProcessCleanupError
from harness.domain.functions import collect_lift_metadata
from harness.domain.ids import parse_function_id
from harness.domain.layout import SplatBoundary, parse_splat_layout
from harness.domain.manifests import TargetManifest
from harness.domain.tags import SOURCE_TAG_RE, parse_behavior_tag, parse_progress_tags
from harness.io import repo_layout
from harness.match.asm_diff import run_asm_diff_one
from harness.match.status_cache import MatchStatusCache, target_fingerprint

from .preflight import _build_preflight, _request_for_source
from .status import select_manifests


def _finding(
    record: dict, code: str, message: str, *, unavailable: bool = False
) -> dict:
    return {
        "selector": record["selector"],
        "source": record["source"],
        "code": code,
        "status": "unavailable" if unavailable else "failed",
        "message": message,
    }


def _claims(root: Path, rows: list[dict]) -> list[dict]:
    contents: dict[str, tuple[str, dict[int, str]]] = {}
    records = []
    for row in rows:
        address = None if row["address"] is None else int(row["address"], 16)
        record = {
            "kind": "function_metadata",
            "target": row["target"],
            "address": row["address"],
            "selector": None if address is None else f"{row['target']}@0x{address:08X}",
            "source": row["source"],
            "line": None,
            "claim": None,
            "measurement": {"state": "absent"},
            "findings": [],
        }
        if row["status"] == "invalid":
            record["findings"].append(_finding(record, "identity", row["reason"]))
        try:
            if row["source"] not in contents:
                text = (root / row["source"]).read_text(encoding="utf-8")
                contents[row["source"]] = text, collect_lift_metadata(text)
            text, metadata = contents[row["source"]]
            if address is not None and address in metadata:
                attached = metadata[address]
                start = (
                    next(
                        match.start()
                        for match in SOURCE_TAG_RE.finditer(text)
                        if int(match.group(1), 16) == address
                    )
                    if attached == text
                    else text.index(attached)
                )
                record["line"] = text.count("\n", 0, start) + 1
                progress = parse_progress_tags(attached)
                record["claim"] = {
                    "behavior": parse_behavior_tag(attached),
                    "status": None if progress is None else progress[0],
                    "match_percent": None if progress is None else progress[1],
                    "residual": None if progress is None else progress[2],
                }
        except (OSError, UnicodeError, ValueError) as error:
            record["findings"].append(_finding(record, "metadata", str(error)))
        records.append(record)
    return sorted(records, key=lambda row: (row["source"], row["address"] or ""))


def _measurement(result: dict, *, state: str) -> dict:
    return {
        "state": state,
        "status": result.get("status"),
        "original_size": result.get("original_size"),
        "current_size": result.get("current_size"),
        "instruction_count": result.get("instruction_count"),
        "match_percent": result.get("match_percent"),
    }


def _cached(
    root: Path, manifest: TargetManifest, records: list[dict], limit: int | None
) -> None:
    path = root / "out/matching/status-cache.sqlite"
    if not path.exists():
        return
    members = Counter(row["source"] for row in records)
    displayed = records if limit is None else records[:limit]
    try:
        with closing(MatchStatusCache(root, read_only=True)) as cache:
            key = target_fingerprint(root, manifest)
            for record in displayed:
                if record["address"] is None:
                    continue
                # Grouped comparisons deliberately have no reusable cache evidence.
                grouped = members[record["source"]] > 1
                fingerprint = (
                    "" if grouped else cache.fingerprint(root / record["source"], key)
                )
                state, result = cache.observation(
                    record["target"],
                    record["source"],
                    int(record["address"], 16),
                    fingerprint,
                )
                record["measurement"] = (
                    {"state": state}
                    if result is None
                    else _measurement(result, state=state)
                )
            cache.validate()
            if target_fingerprint(root, manifest) != key:
                raise ValueError("target inputs changed during metadata listing")
    except ProcessCleanupError:
        raise
    except (OSError, RuntimeError, ValueError, sqlite3.DatabaseError) as error:
        for record in displayed:
            record["measurement"] = {"state": "stale", "reason": str(error)}


def _compare(
    root: Path,
    manifest: TargetManifest,
    record: dict,
    boundaries: dict[int, SplatBoundary],
) -> None:
    if record["findings"] or record["address"] is None:
        return
    address = int(record["address"], 16)
    boundary = boundaries.get(address)
    size = None if boundary is None else boundary.file_size
    if (
        boundary is None
        or boundary.kind != "c"
        or size is None
        or size <= 0
        or size % 4
    ):
        record["findings"].append(
            _finding(
                record,
                "range",
                "a complete finite reviewed C boundary is required",
                unavailable=True,
            )
        )
        return
    try:
        request = _request_for_source(root, root / record["source"], address, manifest)
        result = run_asm_diff_one(request, layout=repo_layout(root))
    except ProcessCleanupError:
        raise
    except (OSError, RuntimeError, ValueError) as error:
        record["measurement"] = {"state": "unavailable", "reason": str(error)}
        record["findings"].append(
            _finding(record, "native", str(error), unavailable=True)
        )
        return
    instructions = result["instruction_count"]
    exact = (
        result.get("byte_match") is True
        and result.get("original_size") == size
        and result.get("current_size") == size
    )
    record["measurement"] = _measurement(
        {
            **result,
            "status": "exact" if exact else "partial",
            "match_percent": instructions["match_percent"],
        },
        state="live",
    )
    if result.get("original_size") != size:
        record["findings"].append(
            _finding(
                record,
                "range",
                f"measured original range must equal reviewed size {size}",
            )
        )
    claim = record["claim"]
    if claim is None or claim["status"] is None:
        return
    status = "exact" if exact else "partial"
    if (
        claim["status"] != status
        or claim["match_percent"] is None
        or abs(claim["match_percent"] - instructions["match_percent"]) > 0.005
    ):
        record["findings"].append(
            _finding(
                record,
                "progress",
                "authored progress disagrees with the live comparison",
            )
        )


def report(
    root: Path,
    target: str,
    *,
    check: bool = False,
    selectors: tuple[str, ...] = (),
    limit: int | None = None,
) -> dict:
    """Require a target and retain full findings even when presentation is limited."""
    root = root.resolve()
    if limit is not None and (check or limit < 1):
        raise ValueError("a positive --limit is supported only by list")
    target, manifest = select_manifests(root, (target,))[0]
    selected = {parse_function_id(value) for value in selectors}
    if any(value.target.value != target for value in selected):
        raise ValueError("every selector must belong to the selected target")
    invalid, worklist = _build_preflight(root, [(target, manifest)], None)
    rows = invalid + [
        {
            "target": owner,
            "source": name,
            "address": f"0x{address:08X}",
            "status": "uncompared",
        }
        for items in worklist.values()
        for owner, _source, address, name, _key, _manifest in items
    ]
    records = _claims(root, rows)
    addresses = {
        int(row["address"], 16) for row in records if row["address"] is not None
    }
    missing = {value.address for value in selected} - addresses
    if missing:
        raise ValueError(
            "selector has no authored function record: "
            + ", ".join(f"0x{address:08X}" for address in sorted(missing))
        )
    if selected:
        wanted = {
            row["source"]
            for row in records
            if row["address"] is not None
            and int(row["address"], 16) in {value.address for value in selected}
        }
        # A selected grouped member checks every member in its translation unit.
        records = [row for row in records if row["source"] in wanted]
    operation_findings = []
    if check:
        try:
            layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
            boundaries = {row.virtual_start: row for row in layout.boundaries}
            if len(boundaries) != len(layout.boundaries):
                raise ValueError("reviewed layout has duplicate boundary addresses")
            before = target_fingerprint(root, manifest)
            for record in records:
                _compare(root, manifest, record, boundaries)
            changed = target_fingerprint(root, manifest) != before
            for record in records:
                if changed:
                    record["findings"].append(
                        _finding(
                            record,
                            "freshness",
                            "target inputs changed during metadata checking",
                            unavailable=True,
                        )
                    )
                    record["measurement"] = {"state": "stale"}
        except ProcessCleanupError:
            raise
        except (OSError, RuntimeError, ValueError) as error:
            if not records:
                operation_findings.append(
                    {
                        "status": "unavailable",
                        "code": "inputs",
                        "source": manifest.splat,
                        "selector": None,
                        "message": str(error),
                    }
                )
            for record in records:
                record["findings"].append(
                    _finding(
                        record,
                        "inputs",
                        str(error),
                        unavailable=True,
                    )
                )
                record["measurement"] = {"state": "unavailable", "reason": str(error)}
    else:
        _cached(root, manifest, records, limit)
    findings = operation_findings + [
        finding for row in records for finding in row["findings"]
    ]
    return {
        "operation": "metadata",
        "action": "check" if check else "list",
        "scope": {
            "target": target,
            "selectors": sorted(str(value) for value in selected),
            "whole_target": not bool(selected),
        },
        "consumers": sorted({row["source"] for row in records}),
        "total": len(records),
        "truncated": limit is not None and len(records) > limit,
        "records": records if limit is None else records[:limit],
        "findings": findings,
        "incomplete": any(row["status"] == "unavailable" for row in findings),
        "passed": not findings,
    }
