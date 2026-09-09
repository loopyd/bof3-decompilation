"""Read-only, parent-pinned semantic acceptance of one capability-backed naming row."""

from __future__ import annotations

from pathlib import Path
from typing import Any

from harness.analysis.index import connect, index_path
from harness.common.inputs import file_state
from harness.domain.ids import normalize_target_id
from harness.domain.includes import local_include_files
from harness.naming.debt import address_of
from harness.domain.repository_layout import load_repository_layout
from harness.naming.annotations import reviewed_annotations
from harness.naming.audit import _context, validate
from harness.naming.capabilities import PRODUCTION_EXACT_CAPABILITIES
from harness.naming.context import SCHEMA_V3, inventory_expected, validate_row_v3
from harness.naming.equivalence import validate_terminal_capability
from harness.naming.inputs import digest, keys, load
from harness.naming.proposal import canonical_report_path
from harness.naming.review import _artifact

PARENT_SCHEMA = "bof3.naming-terminal-parent-review/v1"
REVIEW_SCHEMA = "bof3.naming-terminal-semantic-review/v1"
PREPARATION_SCHEMA = "bof3.naming-terminal-preparation/v1"


def terminal_binding(
    root: Path,
    target: str,
    report_path: Path,
    transaction: str,
    *,
    registry=PRODUCTION_EXACT_CAPABILITIES,
) -> dict[str, Any]:
    """Inspect current integrity/scope for review; this does not accept exhaustion."""
    root = root.resolve()
    path = canonical_report_path(root, report_path)
    before = file_state(path)
    report = load(path)
    keys(report, "schema target complete rows")
    if (
        normalize_target_id(target).value != target
        or report["target"] != target
        or report["schema"] != SCHEMA_V3
        or not isinstance(report["rows"], list)
    ):
        raise ValueError("terminal review requires canonical target and v3 report")
    selected = [
        row
        for row in report["rows"]
        if isinstance(row, dict)
        and f"{row.get('kind')}:{row.get('name')}" == transaction
    ]
    if len(selected) != 1 or selected[0].get("rung_status") != "exhausted":
        raise ValueError("terminal review requires exactly one exhausted row")
    row = selected[0]
    repository = load_repository_layout(root)
    if (row["kind"], row["name"]) not in inventory_expected(
        root, target, dict(repository.manifests)
    ):
        raise ValueError("terminal row is absent from current inventory")
    connection = connect(root)
    connection.close()
    ctx = _context(root, target)
    validate_row_v3(row, row["kind"], row["name"], ctx)
    validate_terminal_capability(root, target, path, row, registry=registry)
    evidence = root / row["conclusion_provenance"]["evidence"]
    namespace = evidence.parent.parent
    derived = load(evidence.with_name("derived.json"))
    if derived.get("open") != [] or derived.get("unavailable") != []:
        raise ValueError("open or unavailable current evidence prevents exhaustion")
    manifest = load(namespace / "manifest.json")
    index = file_state(index_path(root))
    inputs = {}
    for p in sorted(root.glob("config/targets/**/target.toml")):
        state = file_state(p)
        if state is None:
            raise ValueError("target manifest disappeared during terminal inspection")
        inputs[p.relative_to(root).as_posix()] = state["sha256"]
    if (
        index is None
        or manifest.get("index_generation") != index["sha256"]
        or manifest.get("repository_inputs") != inputs
    ):
        raise ValueError("retained terminal evidence is stale against current inputs")
    # ponytail: conservative read-only repository closure, not native build gates;
    # extend via the input owner if another terminal capability needs more scope.
    paths = set(repository.files.paths)
    paths.update(root / p for p in reviewed_annotations(root, target))
    paths.update(local_include_files(root, list(paths)))
    paths.update((root / "tools/python/harness").rglob("*.py"))
    paths.update(p for p in (root / "bin").glob("*") if p.is_file())
    paths.update({index_path(root), root / "out/catalog/emi.json"})
    current = {p.relative_to(root).as_posix(): file_state(p) for p in sorted(paths)}
    retained = {
        str(p): file_state(p) for p in sorted(namespace.rglob("*")) if not p.is_dir()
    }
    if file_state(path) != before:
        raise ValueError("terminal report changed during inspection")
    return {
        "target": target,
        "transaction": transaction,
        "selector": f"{target}@0x{address_of(row['name']):08X}",
        "report": str(path),
        "report_state": before,
        "row_digest": digest(row),
        "capability_digest": digest(derived["conclusion_capability"]),
        "evidence_digest": digest(retained),
        "current_scope_digest": digest(
            {"inputs": current, "scope": ctx.scope(row["name"], kind=row["kind"])}
        ),
        "missing_fact": row["missing_fact"],
    }


def _review(ref: dict[str, Any]) -> dict[str, Any]:
    path = _artifact(ref)
    if not path.read_bytes().strip():
        raise ValueError("terminal review artifact is empty")
    return load(path)


def verify_terminal(
    root: Path,
    target: str,
    report_path: Path,
    transaction: str,
    parent: Path,
    expected_parent_digest: str,
    *,
    registry=PRODUCTION_EXACT_CAPABILITIES,
) -> dict[str, Any]:
    """Accept only a distinct semantic review explicitly pinned by its parent."""
    root = root.resolve()
    report_path = canonical_report_path(root, report_path)
    value = load(parent)
    keys(
        value,
        "schema accepted parent_run_id evidence_preparation_run_id reviewer_run_id "
        "binding preparation_artifact review_artifact ladder_exhausted "
        "unresolved_leads ceiling_rationale",
    )
    if digest(value) != expected_parent_digest:
        raise ValueError("terminal parent attestation differs from external pin")
    origins = [
        value[k]
        for k in ("parent_run_id", "evidence_preparation_run_id", "reviewer_run_id")
    ]
    if (
        value["schema"] != PARENT_SCHEMA
        or value["accepted"] is not True
        or any(
            not isinstance(x, str) or not x.strip() or x != x.strip() for x in origins
        )
        or len(set(origins)) != 3
    ):
        raise ValueError("distinct parent, preparation and reviewer runs required")
    binding = terminal_binding(
        root, target, report_path, transaction, registry=registry
    )
    if value["binding"] != binding:
        raise ValueError("terminal parent binding is stale or mismatched")
    preparation = _review(value["preparation_artifact"])
    keys(preparation, "schema evidence_preparation_run_id binding")
    if preparation != {
        "schema": PREPARATION_SCHEMA,
        "evidence_preparation_run_id": origins[1],
        "binding": binding,
    }:
        raise ValueError("terminal preparation attribution or binding differs")
    review = _review(value["review_artifact"])
    keys(
        review,
        "schema reviewer_run_id binding ladder_exhausted unresolved_leads ceiling_rationale",
    )
    if value["preparation_artifact"]["path"] == value["review_artifact"]["path"]:
        raise ValueError("separate preparation and review artifacts required")
    rationale = value["ceiling_rationale"]
    keys(rationale, "missing_fact explanation evidence")
    if (
        value["ladder_exhausted"] is not True
        or value["unresolved_leads"] != []
        or rationale["missing_fact"] != binding["missing_fact"]
        or not isinstance(rationale["explanation"], str)
        or not rationale["explanation"].strip()
        or not isinstance(rationale["evidence"], list)
        or not rationale["evidence"]
    ):
        raise ValueError(
            "reviewed evidence-backed ceiling without unresolved leads required"
        )
    for ref in rationale["evidence"]:
        if not _artifact(ref).read_bytes().strip():
            raise ValueError("ceiling evidence is empty")
    expected_review = {
        "schema": REVIEW_SCHEMA,
        "reviewer_run_id": origins[2],
        "binding": binding,
        **{
            k: value[k]
            for k in ("ladder_exhausted", "unresolved_leads", "ceiling_rationale")
        },
    }
    if review != expected_review or review["ladder_exhausted"] is not True:
        raise ValueError("semantic reviewer contradicts parent terminal acceptance")
    # Full-report failure is separate from this selected-row gate, never hidden
    # behind an accepted row or used to authorize production advancement.
    try:
        full = validate(
            root, target, load(report_path), report_path=report_path, registry=registry
        )
    except ValueError as error:
        full = {"complete": False, "blocker": str(error)}
    if (
        terminal_binding(root, target, report_path, transaction, registry=registry)
        != binding
    ):
        raise ValueError("terminal state changed during verification")
    if (
        load(parent) != value
        or _review(value["review_artifact"]) != review
        or _review(value["preparation_artifact"]) != preparation
    ):
        raise ValueError("terminal review changed during verification")
    for ref in rationale["evidence"]:
        _artifact(ref)
    return {
        "schema": "bof3.naming-terminal-verification/v1",
        "target": target,
        "transaction": transaction,
        "selected_row_accepted": True,
        "parent_digest": expected_parent_digest,
        "full_report": full,
        "production_complete": False,
    }
