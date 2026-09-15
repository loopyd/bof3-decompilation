"""Reconcile canonical inventory and accounting without replacing evidence."""

from __future__ import annotations

import hashlib
import json
from contextlib import nullcontext
from pathlib import Path

from harness.common.deadlines import (
    DeadlineExpired,
    check_deadline,
    suspend_work_deadline,
)
from harness.common.lease import acquire_writer
from harness.domain.ids import normalize_target_id
from harness.io import unique_object
from harness.naming.audit import initialize, validate
from harness.naming.campaign import resolve_campaign_report
from harness.naming.editing import report_mutation
from harness.naming.history import (
    encode,
    read_state,
    select_target_entry,
    sync_directory,
)
from harness.naming.proposal import atomic_write_if_unchanged


def _reconcile_summary(
    summary: dict, target: str, report: dict, validation: dict
) -> dict:
    updated = json.loads(json.dumps(summary))
    entry = select_target_entry(updated, target)
    entries = updated["targets"]
    seen = set()
    total = 0
    for item in entries:
        if not isinstance(item, dict):
            raise ValueError("campaign entries must be objects")
        name = item.get("target")
        if (
            not isinstance(name, str)
            or normalize_target_id(name).value != name
            or name in seen
        ):
            raise ValueError("campaign targets must be unique and canonical")
        seen.add(name)
        count = item.get("rows")
        if (
            type(count) is not int
            or count < 0
            or type(item.get("complete")) is not bool
        ):
            raise ValueError(
                "campaign entries require nonnegative counts and boolean completeness"
            )
        total += count
    if (
        type(updated.get("target_count")) is not int
        or updated["target_count"] != len(entries)
        or type(updated.get("row_count")) is not int
        or updated["row_count"] != total
    ):
        raise ValueError("campaign totals differ from cached target accounting")
    if "history" in entry:
        if entry["rows"] != validation["rows"]:
            raise ValueError(
                "retained generation inventory requires a reviewed successor"
            )
        return updated
    if entry["rows"] > len(report["rows"]):
        raise ValueError("reconciliation cannot remove accounted inventory rows")
    if entry["complete"] != report["complete"]:
        raise ValueError("initial report completeness differs from campaign accounting")
    updated["row_count"] += validation["rows"] - entry["rows"]
    entry["rows"] = validation["rows"]
    entry["complete"] = validation["complete"]
    return updated


def _publish(
    path: Path,
    original: bytes,
    candidate: dict,
    summary_path: Path,
    summary_bytes: bytes,
    updated: dict,
    report_state: dict,
    summary_state: dict,
    *,
    missing: bool,
    summary_changed: bool,
) -> tuple[dict, dict]:
    expected_report = dict(report_state)
    expected_summary = dict(summary_state)
    try:
        check_deadline()
        if missing:
            atomic_write_if_unchanged(path, original, candidate)
            expected_report["sha256"] = hashlib.sha256(encode(candidate)).hexdigest()
        sync_directory(path.parent)
        if (
            read_state(path) != expected_report
            or read_state(summary_path) != summary_state
        ):
            raise ValueError("inputs changed before accounting publication")
        if summary_changed:
            check_deadline()
            atomic_write_if_unchanged(summary_path, summary_bytes, updated)
            sync_directory(path.parent)
            expected_summary["sha256"] = hashlib.sha256(encode(updated)).hexdigest()
        if (
            read_state(path) != expected_report
            or read_state(summary_path) != expected_summary
        ):
            raise ValueError("publication changed before acceptance")
    except Exception as error:
        states = {}
        with suspend_work_deadline():
            for selected_path, before in (
                (path, report_state),
                (summary_path, summary_state),
            ):
                try:
                    current = read_state(selected_path)
                except (OSError, ValueError) as inspection_error:
                    current = {"inspection_error": str(inspection_error)}
                states[str(selected_path)] = {"before": before, "current": current}
        message = (
            "reconciliation publication failed; retain state and inspect before "
            f"authorized retry; states={json.dumps(states, sort_keys=True)}; {error}"
        )
        if isinstance(error, DeadlineExpired):
            raise DeadlineExpired(message) from error
        raise ValueError(message) from error
    return expected_report, expected_summary


def _reconcile(root: Path, target: str, path: Path, *, apply: bool) -> dict:
    if resolve_campaign_report(root, target) != path:
        raise ValueError("report identity changed")
    report_state = read_state(path)
    original = path.read_bytes()
    report = json.loads(original, object_pairs_hook=unique_object)
    if not isinstance(report, dict) or not isinstance(report.get("rows"), list):
        raise ValueError("report must contain a rows array")
    summary_path = path.parent / "summary.json"
    summary_state = read_state(summary_path)
    summary_bytes = summary_path.read_bytes()
    summary = json.loads(summary_bytes, object_pairs_hook=unique_object)
    fresh = initialize(root, target)
    expected = {(row["kind"], row["name"]): row for row in fresh["rows"]}
    seen = set()
    for row in report["rows"]:
        if not isinstance(row, dict):
            raise ValueError("rows must be objects")
        key = (str(row.get("kind")), str(row.get("name")))
        if key in seen or key not in expected:
            raise ValueError(f"duplicate or extra inventory row: {key}")
        seen.add(key)
    missing = sorted(expected.keys() - seen)
    if missing and "history" in select_target_entry(summary, target):
        raise ValueError("retained generation inventory requires a reviewed successor")
    candidate = dict(report)
    candidate["rows"] = report["rows"] + [expected[key] for key in missing]
    if missing:
        candidate["complete"] = False
    validation = validate(root, target, candidate, report_path=path)
    updated = _reconcile_summary(summary, target, report, validation)
    summary_changed = updated != summary
    for selected_path, state, content in (
        (path, report_state, original),
        (summary_path, summary_state, summary_bytes),
    ):
        if (
            read_state(selected_path) != state
            or hashlib.sha256(content).hexdigest() != state["sha256"]
        ):
            raise ValueError("reconciliation inputs changed during validation")
    expected_report = dict(report_state)
    expected_summary = dict(summary_state)
    if apply:
        expected_report, expected_summary = _publish(
            path,
            original,
            candidate,
            summary_path,
            summary_bytes,
            updated,
            report_state,
            summary_state,
            missing=bool(missing),
            summary_changed=summary_changed,
        )
    if (
        read_state(path) != expected_report
        or read_state(summary_path) != expected_summary
    ):
        raise ValueError(
            "reconciliation publication changed; retain state for inspection"
        )
    return {
        "target": target,
        "report": str(path.relative_to(root)),
        "added": [f"{kind}:{name}" for kind, name in missing],
        "applied": bool(apply and (missing or summary_changed)),
        "before_sha256": report_state["sha256"],
        "after_sha256": expected_report["sha256"],
        "summary": {
            "changed": summary_changed,
            "before_sha256": summary_state["sha256"],
            "after_sha256": expected_summary["sha256"],
            "candidate_sha256": hashlib.sha256(encode(updated)).hexdigest(),
            "rows_before": select_target_entry(summary, target)["rows"],
            "rows_after": select_target_entry(updated, target)["rows"],
            "row_count_before": summary["row_count"],
            "row_count_after": updated["row_count"],
        },
        "validation": validation,
    }


def reconcile(root: Path, target: str, *, apply: bool = False) -> dict:
    """Preview or publish additive inventory and recover its cached accounting."""
    root = root.resolve()
    target = normalize_target_id(target).value
    path = resolve_campaign_report(root, target)
    with acquire_writer(root) if apply else nullcontext():
        with report_mutation(path, report=True):
            return _reconcile(root, target, path, apply=apply)
