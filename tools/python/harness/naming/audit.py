"""``bin/harness naming``: one grouped naming-audit command.

``prepare`` is the readiness preflight (with optional safe metadata repair
behind live exact proof; a prepared-row selector restricts the repair to the
requested target rows, which must be sorted, unique, and already classified
``safe_metadata_repair`` by the unscoped preflight); ``validate`` runs the full-report or isolated
pre-apply transaction check and captures the immutable pre-apply digest;
``verify`` proves that captured transaction was applied exactly.  Each
subcommand is one explicit surface: no ``--row`` selector alias and no
boolean ``--post-apply`` mode combinations.  Row construction and report-set
publication live in ``initialize``; scoped repair lives in ``prepare``.
"""

from __future__ import annotations

import hashlib
import json
import subprocess
from collections.abc import Sequence
from pathlib import Path
from typing import Any, Mapping

from harness.common.commands import command_at
from harness.common.deadlines import check_deadline
from harness.naming.capabilities import PRODUCTION_EXACT_CAPABILITIES
from harness.naming.context import SCHEMA_V2, SCHEMA_V3, TargetContext
from harness.naming.editing import report_mutation
from harness.naming.inventory import (
    _initialize_all,
    initialize_with_context,
)
from harness.naming.metadata import read_source_metadata
from harness.naming.preparation import (
    merge_findings_notes,
    repair_selected_rows,
    select_rows_findings,
    source_addresses,
    validate_row_selectors,
)
from harness.naming.readiness import progress_metadata_findings, required_work_snapshot
from harness.naming.reports import validate as validate_v3

from ..analysis.index import connect as connect_index
from ..analysis.project import status as project_status
from ..domain.claims import manifest_source_paths
from ..domain.ids import normalize_target_id
from ..domain.manifests import load_target_manifests
from harness.naming.debt import address_of
from ..io import unique_object
from .proposal import (
    DATA_KIND as DATA_PROPOSAL_KIND,
    KIND as PROPOSAL_KIND,
)
from .proposal import (
    PROVENANCE_FIELD,
    atomic_write_if_unchanged,
    build_provenance,
    canonical_report_path,
    replace_initializer,
    require_provenance,
    validate_authored_digests,
    validate_proposal,
)


def _live_exact(root: Path, target: str, address: int) -> tuple[bool, list[str]]:
    selector = f"{target}@0x{address:08X}"
    commands = [
        command_at(root, "asm-diff", selector, "--detail", "normal"),
        command_at(root, "byte-match", selector),
    ]
    results = []
    exact = True
    for command in commands:
        result = subprocess.run(command, cwd=root, text=True, capture_output=True)
        results.append(f"{' '.join(command)}: exit {result.returncode}")
        exact = exact and result.returncode == 0
    return exact, results


def prepare(
    root: Path,
    target: str,
    *,
    repair: bool = False,
    rows: Sequence[str] | None = None,
) -> dict[str, object]:
    """Read-only readiness preflight; ``repair`` rewrites only proven-exact metadata.

    ``rows=None`` (the flag omitted) keeps the legacy repair of every safe
    row.  A supplied list repairs only those prepared rows, in order, and
    rolls those files' edits back if a row fails; an explicitly empty list
    is safe and repairs nothing.  Every other finding is reported unchanged.
    """

    target = normalize_target_id(target).value
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    manifest = manifests[target]
    findings = progress_metadata_findings(root, target, manifest)
    repaired: list[dict[str, Any]] = []
    if rows is not None and rows and not repair:
        raise ValueError("--rows requires --repair")
    if repair:
        if rows is not None:
            validate_row_selectors(list(rows))
            select_rows_findings(list(rows), findings)
        selected_rows = (
            {str(row) for row in rows}
            if rows is not None
            else {
                str(finding["row"])
                for finding in findings
                if finding.get("class") == "safe_metadata_repair"
            }
        )
        selected_addresses = {
            address_of(row) for row in selected_rows if row.startswith("function:")
        }
        by_address = source_addresses(root, manifest) if selected_addresses else {}
        addresses = selected_addresses & by_address.keys()
        proofs = {address: _live_exact(root, target, address) for address in addresses}
        repaired, notes = repair_selected_rows(
            root,
            findings,
            by_address,
            rows=list(rows) if rows is not None else None,
            live_exact=lambda address: proofs.get(
                address, (False, [f"live exact proof missing 0x{address:08X}"])
            ),
        )
        findings = merge_findings_notes(
            progress_metadata_findings(root, target, manifest), notes
        )
    blocked = [
        finding for finding in findings if finding.get("class") == "review_required"
    ]
    repairable = [
        finding
        for finding in findings
        if finding.get("class") == "safe_metadata_repair"
    ]
    return {
        "schema": "bof3.naming-preflight/v1",
        "target": target,
        "ready": not findings,
        "findings": findings,
        "repaired": repaired,
        "counts": {"blocked": len(blocked), "repairable": len(repairable)},
    }


def _context(
    root: Path,
    target: str,
    *,
    bulk_work: bool = False,
    manifests: Mapping[str, Any] | None = None,
    connection: Any = None,
) -> TargetContext:
    target = normalize_target_id(target).value
    loaded = load_target_manifests(root) if manifests is None else manifests
    if target not in loaded:
        raise ValueError(f"unknown target: {target}")
    manifest = loaded[target]
    if not bulk_work:
        return TargetContext(root, target, manifest)
    owned_connection = connection is None
    if owned_connection:
        connection = connect_index(root, manifests=loaded)
    try:
        work = required_work_snapshot(root, target, manifest, connection)
    finally:
        if owned_connection:
            connection.close()
    return TargetContext(
        root,
        target,
        manifest,
        work_snapshot=work,
        source_metadata=read_source_metadata(
            manifest_source_paths(root, manifest)
            if manifest.has_explicit_sources
            else ()
        ),
        payload_end=work.payload_end,
    )


def initialize(root: Path, target: str) -> dict[str, Any]:
    """Create a complete v3 inventory whose unreviewed rows are explicit gaps."""

    target = normalize_target_id(target).value
    snapshot = project_status(root, target)
    if not snapshot.get("fresh"):
        raise ValueError(
            f"analysis snapshot is stale for {target}; run bin/harness analysis index --recover"
        )
    manifests = load_target_manifests(root)
    return initialize_with_context(
        root, target, _context(root, target, bulk_work=True), manifests
    )


def initialize_all(root: Path, output: Path) -> dict[str, Any]:
    """Build one validated, repository-contained report set."""

    output = canonical_report_path(root, output)
    return _initialize_all(
        root,
        output,
        load_manifests=load_target_manifests,
        connect=connect_index,
        build_context=_context,
        validate_report=validate_v3,
    )


def validate(
    root: Path,
    target: str,
    report: dict[str, Any],
    *,
    transaction: str | None = None,
    report_path: Path | None = None,
    registry=PRODUCTION_EXACT_CAPABILITIES,
) -> dict[str, Any]:
    """Pre-apply check: full report, or one isolated transaction selection.

    Full-report validation builds one bulk `TargetContext`: a single
    ``required_work_snapshot`` and a single index connection.  An isolated
    ``transaction`` selection keeps the one-row live lookup.
    """

    if report_path is not None:
        report_path = canonical_report_path(root, report_path)
    if not isinstance(report, dict):
        raise ValueError("report must be an object")
    if report.get("schema") == SCHEMA_V2:
        raise ValueError(
            "bof3.naming-audit/v2 is retired; regenerate as bof3.naming-audit/v3"
        )
    if report.get("schema") != SCHEMA_V3:
        raise ValueError(f"report schema must be {SCHEMA_V3}")
    normalized_target = normalize_target_id(target).value
    from harness.naming.provenance import require_canonical_provenance
    from harness.naming.equivalence import validate_terminal_capability

    rows = [row for row in report.get("rows", []) if isinstance(row, dict)]
    for row in rows:
        selector = f"{row.get('kind')}:{row.get('name')}"
        state = row.get("rung_status")
        if state == "exhausted":
            require_canonical_provenance(row, selector)
        elif state == "proposed":
            require_provenance(row, selector)
        elif state == "blocked" and PROVENANCE_FIELD in row:
            raise ValueError(f"{selector} blocked row carries conclusion provenance")
    ctx = _context(root, target, bulk_work=transaction is None)
    result = validate_v3(root, normalized_target, report, ctx, transaction=transaction)
    terminal = [
        row
        for row in rows
        if row.get("rung_status") in {"exhausted", "proposed"}
        and (
            transaction is None or f"{row.get('kind')}:{row.get('name')}" == transaction
        )
    ]
    if terminal:
        if report_path is None:
            raise ValueError("terminal report validation requires report path")
        for row in terminal:
            provenance = row[PROVENANCE_FIELD]
            if provenance.get("kind") in {PROPOSAL_KIND, DATA_PROPOSAL_KIND}:
                validate_proposal(root, report_path, report, row, ctx)
            else:
                validate_terminal_capability(
                    root,
                    normalized_target,
                    report_path,
                    row,
                    registry=registry,
                )
    return result


def prepare_transaction(
    root: Path,
    target: str,
    report_path: Path,
    transaction: str,
    *,
    candidate: dict[str, Any] | None = None,
    expected_sha256: str | None = None,
    check_only: bool = False,
) -> dict[str, Any]:
    """Validate one ready proposal, optionally publishing its bound report."""
    if (candidate is None) != (expected_sha256 is None):
        raise ValueError("candidate and expected SHA-256 must be supplied together")
    path = canonical_report_path(root, report_path)
    with report_mutation(path, report=True):
        original = path.read_bytes()
        report = json.loads(original, object_pairs_hook=unique_object)
        if not isinstance(report, dict):
            raise ValueError("report must be a JSON object")
        if candidate is not None:
            assert expected_sha256 is not None
            replace_initializer(
                report,
                original,
                normalize_target_id(target).value,
                transaction,
                candidate,
                expected_sha256,
            )
        result = validate_v3(
            root,
            normalize_target_id(target).value,
            report,
            _context(root, target),
            transaction=transaction,
        )
        if result.get("ready") is not True or not transaction.startswith(
            ("function:", "data:")
        ):
            raise ValueError(
                "prepare-transaction requires one ready FUNCTION or DATA proposal"
            )
        kind, name = transaction.split(":", 1)
        row = next(
            item
            for item in report["rows"]
            if item.get("kind") == kind and item.get("name") == name
        )
        if kind == "data":
            from harness.naming.context import naming_manifest, pre_apply

            context = _context(root, target)
            result["pre_apply"] = pre_apply(
                context, kind, name, row, data_proposal=True
            )
            result["manifest"] = naming_manifest(
                context, kind, name, row, result["pre_apply"]
            )
        row["pre_apply"] = result["pre_apply"]
        row["manifest"] = result["manifest"]
        row[PROVENANCE_FIELD] = build_provenance(
            root, path, report, row, result["pre_apply"], result["manifest"]
        )
        validate(root, target, report, report_path=path)
        check_deadline()
        if check_only:
            if path.read_bytes() != original:
                raise ValueError("proposal report changed concurrently")
            result = {
                "schema": SCHEMA_V3,
                "target": target,
                "transaction": transaction,
                "checked": True,
                "prepared": False,
                "report_sha256": hashlib.sha256(original).hexdigest(),
            }
            check_deadline()
            return result
        atomic_write_if_unchanged(path, original, report)
    return {
        "schema": SCHEMA_V3,
        "target": target,
        "transaction": transaction,
        "prepared": True,
    }


def verify(
    root: Path,
    target: str,
    report: dict[str, Any],
    transaction: str,
    *,
    report_path: Path | None = None,
    post_apply_receipts: Path | None = None,
) -> dict[str, Any]:
    """Post-apply proof of the captured transaction against current truth."""
    if report_path is not None:
        report_path = canonical_report_path(root, report_path)
    if not isinstance(report, dict):
        raise ValueError("report must be an object")
    if transaction is None:
        raise ValueError("verify requires --transaction KIND:NAME")
    if report.get("schema") == SCHEMA_V2:
        raise ValueError(
            "bof3.naming-audit/v2 is retired; regenerate as bof3.naming-audit/v3"
        )
    if report.get("schema") != SCHEMA_V3:
        raise ValueError(f"report schema must be {SCHEMA_V3}")
    rows = [row for row in report.get("rows", []) if isinstance(row, dict)]
    selected = [
        row for row in rows if f"{row.get('kind')}:{row.get('name')}" == transaction
    ]
    if len(selected) != 1 or report_path is None:
        raise ValueError("verify requires exactly one transaction and report path")
    require_provenance(selected[0], transaction)
    provenance = selected[0][PROVENANCE_FIELD]
    validate_authored_digests(report, selected[0], provenance)
    if selected[0].get("pre_apply") != provenance.get("pre_apply") or selected[0].get(
        "manifest"
    ) != provenance.get("manifest"):
        raise ValueError("proposal transaction records differ from provenance")
    # Original authored provenance precedes any copied receipt overlay.
    if provenance.get("report") != report_path.relative_to(root.resolve()).as_posix():
        raise ValueError("proposal provenance report path mismatch")
    if post_apply_receipts is not None:
        if any("post_apply_receipts" in row for row in rows):
            raise ValueError("in-row and external receipts are ambiguous")
        from harness.naming.review import verify_external

        return verify_external(
            root,
            target,
            report_path,
            transaction,
            post_apply_receipts,
            lambda copied: validate_v3(
                root,
                normalize_target_id(target).value,
                copied,
                _context(root, target),
                transaction=transaction,
                post_apply=True,
            ),
        )
    return validate_v3(
        root,
        normalize_target_id(target).value,
        report,
        _context(root, target),
        transaction=transaction,
        post_apply=True,
    )


__all__ = ["initialize", "initialize_all", "prepare", "validate", "verify"]
