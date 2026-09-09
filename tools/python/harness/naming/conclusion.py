"""Import one expert-authored naming conclusion bound to collected evidence."""

from __future__ import annotations

import hashlib
import json
import os
import stat
import tempfile
from pathlib import Path
from typing import Any

from harness.naming.capabilities import PRODUCTION_EXACT_CAPABILITIES
from harness.naming.collection import evidence_namespace_for_digest, namespace_identity
from harness.naming.editing import report_mutation
from harness.naming.equivalence import _validated_capability
from harness.naming.validation import (
    _reject_unsupported_semantic_facts,
    _validate_evidence,
    _validate_next_commands,
)

SCHEMA = "bof3.naming-conclusion/v1"
INITIALIZER_STATE = "bof3.naming-audit-initializer/v1"
PROVENANCE_FIELD = "conclusion_provenance"

FACT_SCHEMA = "bof3.naming-evidence-facts/v1"


def _digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _validate_conclusion(
    row: object, capability: dict[str, Any] | None = None
) -> dict[str, Any]:
    if not isinstance(row, dict) or row.get("rung_status") not in {
        "exhausted",
        "proposed",
    }:
        raise ValueError("conclusion must be one explicit exhausted or proposed row")
    if row["rung_status"] == "exhausted":
        if capability is None:
            invalid_rungs = any(
                rung.get("status") != "negative" or not rung.get("negative_result")
                for rung in row.get("rungs", {}).values()
            )
        else:
            expected = capability.get("rungs")
            invalid_rungs = (
                set(row.get("rungs", {})) != set(expected or {})
                or any(
                    rung.get("status") != "passed" or not rung.get("observations")
                    for rung in row.get("rungs", {}).values()
                )
                or row.get("missing_fact") != capability.get("missing_fact")
                or [item.get("id") for item in row.get("required_work", [])]
                != capability.get("required_work")
            )
        if invalid_rungs:
            raise ValueError("exhausted conclusion rungs do not match trusted facts")
        if capability is not None:
            corroborators = row.get("corroborators")
            source_ids = (
                [item.get("source_id") for item in corroborators.values()]
                if isinstance(corroborators, dict)
                and len(corroborators) == 2
                and all(isinstance(item, dict) for item in corroborators.values())
                else []
            )
            expected_sources = {
                capability["rungs"]["storage_class"][0],
                capability["rungs"]["one_level_beyond"][0],
            }
            if set(source_ids) != expected_sources or len(source_ids) != 2:
                raise ValueError(
                    "exhausted conclusion requires the two trusted original-byte corroborators"
                )
            if row.get("new_name") is not None or row.get("identity") is not None:
                raise ValueError(
                    "exhausted capability cannot carry a semantic name term"
                )
        if not row.get("missing_fact") or not row.get("ceiling_next_command"):
            raise ValueError(
                "exhausted conclusion requires reviewed evidence gap and bounded next command/owner"
            )
        if any(
            not isinstance(work, dict) or work.get("status") != "completed"
            for work in row.get("required_work", [])
        ):
            raise ValueError(
                "exhaustion requires every generated work item to have a passed exact receipt"
            )
    else:
        if not isinstance(row.get("identity"), dict) or not row.get("new_name"):
            raise ValueError(
                "proposed conclusion requires an explicit semantic identity"
            )
        if (
            not isinstance(row.get("corroborators"), dict)
            or len(row["corroborators"]) < 2
        ):
            raise ValueError(
                "proposed conclusion requires two independent corroborators"
            )
        if row.get("kind") == "data" and not isinstance(row.get("storage"), dict):
            raise ValueError(
                "proposed data conclusion requires storage/type/width facts"
            )
    return row


def _atomic_write(path: Path, data: bytes, mode: int) -> None:
    descriptor, temporary = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        os.fchmod(descriptor, mode)
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        Path(temporary).replace(path)
    except BaseException:
        Path(temporary).unlink(missing_ok=True)
        raise


def import_conclusion(
    root: Path,
    target: str,
    report_path: Path,
    input_path: Path,
    *,
    evidence_root: Path | None = None,
    registry=PRODUCTION_EXACT_CAPABILITIES,
) -> dict[str, Any]:
    """Validate and atomically import exactly one explicit row conclusion."""
    from harness.naming.namespace import reset_evidence_root
    from harness.naming.namespace import set_evidence_root

    from ..domain.receipts import reset_receipt_root, set_receipt_root
    from .proposal import canonical_report_path

    report_path = canonical_report_path(root, report_path)
    if evidence_root is not None and not evidence_root.is_absolute():
        raise ValueError("evidence_root must be absolute")
    token = set_evidence_root(evidence_root)
    receipt_token = set_receipt_root(evidence_root)
    try:
        return _import_conclusion(
            root,
            target,
            report_path,
            input_path,
            evidence_root=evidence_root,
            registry=registry,
        )
    finally:
        reset_receipt_root(receipt_token)
        reset_evidence_root(token)


def _import_conclusion(
    root: Path,
    target: str,
    report_path: Path,
    input_path: Path,
    *,
    evidence_root: Path | None,
    registry,
) -> dict[str, Any]:
    authored = json.loads(input_path.read_text(encoding="utf-8"))
    if (
        not isinstance(authored, dict)
        or authored.get("schema") != SCHEMA
        or authored.get("target") != target
    ):
        raise ValueError(f"input schema/target must be {SCHEMA} and {target}")
    authored_row = authored.get("conclusion")
    if not isinstance(authored_row, dict):
        raise ValueError("conclusion must be one explicit exhausted or proposed row")
    key = (authored_row.get("kind"), authored_row.get("name"))
    selector = f"{key[0]}:{key[1]}"
    source = authored.get("source")
    if not isinstance(source, dict):
        raise ValueError("source must bind collected evidence")
    report_digest = source.get("report_digest")
    if not isinstance(report_digest, str):
        raise ValueError("source requires report_digest")
    expected_namespace = evidence_namespace_for_digest(
        root, target, report_digest, evidence_root
    )
    namespace_provenance = namespace_identity(expected_namespace, evidence_root)
    capability = _validated_capability(
        root,
        target,
        report_path,
        authored_row,
        source,
        expected_namespace,
        registry=registry,
    )
    row = _validate_conclusion(authored_row, capability)
    provenance = _validate_evidence(
        root,
        target,
        selector,
        row,
        source,
        expected_namespace,
        namespace_provenance,
        capability,
    )
    with report_mutation(report_path, report=True):
        original = report_path.read_bytes()
        report = json.loads(original)
        if (
            report.get("schema") != "bof3.naming-audit/v3"
            or report.get("target") != target
        ):
            raise ValueError("report schema/target mismatch")
        matches = [
            index
            for index, current in enumerate(report.get("rows", []))
            if isinstance(current, dict)
            and (current.get("kind"), current.get("name")) == key
        ]
        if len(matches) != 1:
            raise ValueError("conclusion row is not in the report exactly once")
        current = report["rows"][matches[0]]
        _validate_next_commands(row, current)
        _reject_unsupported_semantic_facts(row, capability)
        replay_row = {
            key: value for key, value in current.items() if key != PROVENANCE_FIELD
        }
        from .audit import validate

        if replay_row == row:
            if (
                authored.get("report_sha256")
                != current.get(PROVENANCE_FIELD, {}).get("initializer_report_sha256")
                or current.get(PROVENANCE_FIELD) != provenance
            ):
                raise ValueError(
                    "identical conclusion has different provenance binding"
                )
            validate(root, target, report, report_path=report_path, registry=registry)
            if row["rung_status"] == "proposed":
                transaction = validate(
                    root,
                    target,
                    report,
                    transaction=selector,
                    report_path=report_path,
                    registry=registry,
                )
                if (
                    not isinstance(transaction, dict)
                    or transaction.get("ready") is not True
                    or transaction.get("errors")
                ):
                    raise ValueError("proposed transaction is not ready")
            return {
                "schema": SCHEMA,
                "target": target,
                "row": selector,
                "changed": False,
            }
        if authored.get("report_sha256") != _digest(original):
            raise ValueError("report is stale or report_sha256 mismatches")
        if (
            current.get("initializer_state") != INITIALIZER_STATE
            or current.get("rung_status") != "blocked"
        ):
            raise ValueError(
                "current row is not validator-owned initializer state; conflicting import rejected"
            )
        replacement = dict(row)
        replacement.pop("initializer_state", None)
        replacement[PROVENANCE_FIELD] = provenance
        report["rows"][matches[0]] = replacement
        report["complete"] = not any(
            item.get("rung_status") == "blocked"
            for item in report["rows"]
            if isinstance(item, dict)
        )
        validate(root, target, report, report_path=report_path, registry=registry)
        if row["rung_status"] == "proposed":
            transaction = validate(
                root,
                target,
                report,
                transaction=selector,
                report_path=report_path,
                registry=registry,
            )
            if (
                not isinstance(transaction, dict)
                or transaction.get("ready") is not True
                or transaction.get("errors")
            ):
                raise ValueError("proposed transaction is not ready")
        if report_path.read_bytes() != original:
            raise ValueError("report changed concurrently; stale import rejected")
        rendered = (json.dumps(report, indent=2, sort_keys=True) + "\n").encode()
        mode = stat.S_IMODE(report_path.stat().st_mode)
        _atomic_write(report_path, rendered, mode)
    return {
        "schema": SCHEMA,
        "target": target,
        "row": selector,
        "changed": True,
        "sha256": _digest(rendered),
    }
