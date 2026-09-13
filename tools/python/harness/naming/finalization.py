"""Verified, compare-and-swap publication of one naming report successor."""

from __future__ import annotations

import json
from contextlib import nullcontext
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.inputs import load
from harness.common.lease import acquire_writer
from harness.domain.manifests import load_target_manifests
from harness.io import unique_object
from harness.naming import audit
from harness.naming.campaign import CAMPAIGN_REPORT_DIRECTORY, resolve_campaign_report
from harness.naming.context import inventory_expected
from harness.naming.editing import report_mutation
from harness.naming.history import (
    CHECKPOINT_SCHEMA,
    PLAN_SCHEMA,
    collect_evidence_states,
    resolve_contained_path,
    encode,
    read_state,
    require_keys,
    require_pin,
    compute_sha256,
    sync_directory,
    select_target_entry,
    resolve_transition_paths,
    update_summary,
    validate_record,
    write_exclusive,
)
from harness.naming.namespace import selected_evidence_root
from harness.naming.proposal import atomic_write_if_unchanged, canonical_report_path


def _read_pinned_state(path: Path, expected: str) -> dict:
    require_pin(expected)
    state = read_state(path)
    if state["sha256"] != expected:
        raise ValueError(f"stale finalization input: {path}")
    return state


def _verify(
    root: Path, target: str, path: Path, transaction: str, bundle: Path
) -> dict:
    check_deadline()
    result = audit.verify(
        root,
        target,
        load(path),
        transaction,
        report_path=path,
        post_apply_receipts=bundle,
    )
    if (
        result
        != {
            "schema": "bof3.naming-audit/v3",
            "target": target,
            "transaction": transaction,
            "applied": True,
            "rows": 1,
        }
        or result.get("applied") is not True
    ):
        raise ValueError(
            "finalization requires successful selected public verification"
        )
    return result


def _build_successor(root: Path, target: str, report: dict, transaction: str) -> dict:
    require_keys(report, "schema target complete rows")
    rows = report["rows"]
    if not isinstance(rows, list) or not all(isinstance(row, dict) for row in rows):
        raise ValueError("finalization requires an object row inventory")
    selected = [
        row for row in rows if f"{row.get('kind')}:{row.get('name')}" == transaction
    ]
    proposals = [row for row in rows if row.get("rung_status") == "proposed"]
    if len(selected) != 1 or proposals != selected:
        raise ValueError("finalization requires exactly one applied proposal")
    keys = [(row.get("kind"), row.get("name")) for row in rows]
    removed = (selected[0]["kind"], selected[0]["name"])
    if len(set(keys)) != len(keys):
        raise ValueError("duplicate predecessor inventory row")
    current = inventory_expected(root, target, load_target_manifests(root))
    if current != set(keys) - {removed}:
        raise ValueError("live inventory must equal predecessor minus selected row")
    successor = dict(report)
    successor["rows"] = [row for row in rows if row is not selected[0]]
    successor["complete"] = not any(
        row.get("rung_status") == "blocked" for row in successor["rows"]
    )
    return successor


def _build_result(record: dict, checkpoint: Path, root: Path, disposition: str) -> dict:
    return {
        "schema": PLAN_SCHEMA,
        "target": record["request"]["target"],
        "transaction": record["request"]["transaction"],
        "disposition": disposition,
        "plan_sha256": compute_sha256(encode(record)),
        "checkpoint": checkpoint.relative_to(root).as_posix(),
        "successor": record["successor"],
        "validation": record["validation"],
        "production_complete": False,
        "current_acceptance": disposition in {"preview", "published"},
    }


def _finalize(
    root: Path, request: dict, *, apply: bool, expected_plan: str | None
) -> dict:
    path = resolve_contained_path(root, request["report"])
    bundle_path = Path(request["bundle"])
    directory = root / CAMPAIGN_REPORT_DIRECTORY
    summary_path = directory / "summary.json"
    successor_path, checkpoint_path = resolve_transition_paths(root, request)
    summary_before = read_state(summary_path)
    summary_bytes = summary_path.read_bytes()
    summary = json.loads(summary_bytes, object_pairs_hook=unique_object)
    entry = select_target_entry(summary, request["target"])
    existing = None
    if checkpoint_path.exists():
        existing = validate_record(
            root, checkpoint_path, compute_sha256(checkpoint_path.read_bytes())
        )
        if existing["request"] != request:
            raise ValueError("retained checkpoint request differs")
        if (
            expected_plan is not None
            and compute_sha256(encode(existing)) != expected_plan
        ):
            raise ValueError("retained checkpoint differs from expected plan")
        reference = {
            "path": checkpoint_path.relative_to(root).as_posix(),
            "sha256": compute_sha256(checkpoint_path.read_bytes()),
        }
        if reference in entry.get("history", []):
            resolve_campaign_report(root, request["target"])
            return _build_result(existing, checkpoint_path, root, "already-published")
    _read_pinned_state(summary_path, request["summary_sha256"])
    report_state = _read_pinned_state(path, request["report_sha256"])
    bundle_state = _read_pinned_state(bundle_path, request["bundle_sha256"])
    if resolve_campaign_report(root, request["target"]) != path:
        raise ValueError("finalization requires the active canonical report")
    verification = _verify(
        root, request["target"], path, request["transaction"], bundle_path
    )
    report = load(path)
    successor = _build_successor(
        root, request["target"], report, request["transaction"]
    )
    validation = audit.validate(root, request["target"], successor, report_path=path)
    record = {
        "schema": CHECKPOINT_SCHEMA,
        "request": request,
        "predecessor": {"path": request["report"], "state": report_state},
        "bundle": {"path": request["bundle"], "state": bundle_state},
        "summary": {"state": summary_before, "text": summary_bytes.decode()},
        "successor": {
            "path": successor_path.relative_to(root).as_posix(),
            "state": {
                "sha256": compute_sha256(encode(successor)),
                "mode": report_state["mode"],
            },
        },
        "verification": verification,
        "validation": validation,
        "artifacts": collect_evidence_states(root, path, bundle_path, report),
    }
    if existing is not None and record != existing:
        raise ValueError("retained unpublished checkpoint differs from current plan")
    plan_sha256 = compute_sha256(encode(record))
    reference = {
        "path": checkpoint_path.relative_to(root).as_posix(),
        "sha256": plan_sha256,
    }
    updated = update_summary(summary, record, reference)
    for selected_path, expected in (
        (path, report_state),
        (bundle_path, bundle_state),
        (summary_path, summary_before),
    ):
        if read_state(selected_path) != expected:
            raise ValueError("finalization inputs changed during preview")
    if not apply:
        result = _build_result(record, checkpoint_path, root, "preview")
        result["summary_after_sha256"] = compute_sha256(encode(updated))
        result["retained_artifact_count"] = len(record["artifacts"])
        return result
    if expected_plan != plan_sha256:
        raise ValueError(
            "finalization plan changed or expected plan SHA-256 mismatches"
        )
    check_deadline()
    write_exclusive(successor_path, encode(successor), report_state["mode"])
    live_validation = audit.validate(
        root, request["target"], load(successor_path), report_path=successor_path
    )
    if live_validation != validation:
        raise ValueError("published successor validation differs from preview")
    if (
        _build_successor(root, request["target"], load(path), request["transaction"])
        != successor
    ):
        raise ValueError("inventory or surviving rows changed before activation")
    if (
        _verify(root, request["target"], path, request["transaction"], bundle_path)
        != verification
    ):
        raise ValueError("public verification changed before activation")
    if (
        collect_evidence_states(root, path, bundle_path, load(path))
        != record["artifacts"]
    ):
        raise ValueError("retained evidence changed before activation")
    check_deadline()
    write_exclusive(checkpoint_path, encode(record), report_state["mode"])
    validate_record(root, checkpoint_path, plan_sha256)
    if read_state(successor_path) != record["successor"]["state"]:
        raise ValueError("successor changed before activation")
    if read_state(summary_path) != summary_before:
        raise ValueError("summary state changed before activation")
    check_deadline()
    atomic_write_if_unchanged(summary_path, summary_bytes, updated)
    sync_directory(directory)
    if resolve_campaign_report(root, request["target"]) != successor_path:
        raise ValueError("checkpoint activation is uncertain; retain publication")
    return _build_result(record, checkpoint_path, root, "published")


def finalize_transaction(
    root: Path,
    target: str,
    report_path: Path,
    transaction: str,
    *,
    post_apply_receipts: Path,
    expected_report_sha256: str,
    expected_bundle_sha256: str,
    expected_summary_sha256: str,
    apply: bool = False,
    expected_plan_sha256: str | None = None,
) -> dict:
    """Preview or activate one verified successor without rebinding old proofs."""
    root = root.resolve()
    path = canonical_report_path(root, report_path)
    bundle = (
        post_apply_receipts
        if post_apply_receipts.is_absolute()
        else root / post_apply_receipts
    )
    if str(bundle.resolve()) != str(bundle):
        raise ValueError("finalization bundle path must be canonical")
    if type(apply) is not bool or (apply and expected_plan_sha256 is None):
        raise ValueError("apply requires the exact preview plan SHA-256")
    for pin in (
        expected_report_sha256,
        expected_bundle_sha256,
        expected_summary_sha256,
    ):
        require_pin(pin)
    if expected_plan_sha256 is not None:
        require_pin(expected_plan_sha256)
    request = {
        "schema": PLAN_SCHEMA,
        "root": str(root),
        "target": target,
        "transaction": transaction,
        "report": path.relative_to(root).as_posix(),
        "report_sha256": expected_report_sha256,
        "bundle": str(bundle),
        "bundle_sha256": expected_bundle_sha256,
        "summary_sha256": expected_summary_sha256,
        "evidence_root": str(selected_evidence_root())
        if selected_evidence_root()
        else None,
    }
    directory = root / CAMPAIGN_REPORT_DIRECTORY
    if path.parent != directory:
        raise ValueError("finalization requires the canonical campaign directory")
    check_deadline()
    with acquire_writer(root) if apply else nullcontext():
        with report_mutation(directory) if apply else nullcontext():
            return _finalize(
                root, request, apply=apply, expected_plan=expected_plan_sha256
            )
