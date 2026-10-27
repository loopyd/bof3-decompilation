"""Replay runner-backed naming conclusion capabilities against current inputs."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from harness.common.digests import digest as json_digest
from harness.naming.collection import EvidenceNamespace
from harness.naming.evidence import (
    _RUNNER_TOKEN,
    CONCLUSION_CAPABILITY_SCHEMA,
    recompute_collected_records,
)
from harness.naming.evidence import SCHEMA as ANALYZER_SCHEMA
from harness.naming.journal import entry_sha256
from harness.naming.provenance import (
    PROVENANCE_FIELD,
    require_canonical_provenance,
)


def _require_exact_keys(value: dict[str, Any], expected: set[str], label: str) -> None:
    if set(value) != expected:
        raise ValueError(f"{label} does not match the closed reviewed schema")


def _is_exact_replay(report_path: Path, row: dict[str, Any]) -> bool:
    try:
        report = json.loads(report_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return False
    matches = [
        current
        for current in report.get("rows", [])
        if isinstance(current, dict)
        and (current.get("kind"), current.get("name"))
        == (row.get("kind"), row.get("name"))
    ]
    if len(matches) != 1:
        return False
    return json_digest(
        {key: value for key, value in matches[0].items() if key != PROVENANCE_FIELD}
    ) == json_digest(
        {key: value for key, value in row.items() if key != PROVENANCE_FIELD}
    )


def _validate_source_shape(
    source: dict[str, Any], report_path: Path, capability: dict[str, Any]
) -> None:
    _require_exact_keys(
        source,
        {
            "report",
            "report_digest",
            "evidence",
            "evidence_sha256",
            "conclusion_capability",
        },
        "source",
    )
    if source["report"] != report_path.as_posix() or json_digest(
        source["conclusion_capability"]
    ) != json_digest(capability):
        raise ValueError(
            "source is not exactly bound to the current report and capability"
        )


def _digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _validated_capability(
    root: Path,
    target: str,
    report_path: Path,
    row: dict[str, Any],
    source: object,
    expected_namespace: EvidenceNamespace,
) -> dict[str, Any] | None:
    """Recompute a current capability without trusting authored observations."""

    row_name = row.get("name")
    selector = f"{row.get('kind')}:{row_name}"
    if (
        row.get("kind") not in {"function", "data"}
        or not isinstance(source, dict)
        or "conclusion_capability" not in source
    ):
        return None
    if row.get("rung_status") not in {"exhausted", "proposed"} or not isinstance(
        source, dict
    ):
        raise ValueError("capability requires an exhausted or proposed conclusion")
    report_digest = str(source.get("report_digest", ""))
    evidence = source.get("evidence")
    if not isinstance(evidence, str) or not evidence:
        raise ValueError("reviewed capability source evidence is missing")
    evidence_path = (root / evidence).resolve()
    expected_row = expected_namespace.path / str(row_name)
    if evidence_path != expected_row / "payload.json":
        raise ValueError(
            "reviewed capability evidence is outside its selected namespace"
        )
    manifest_path = expected_namespace.path / "manifest.json"
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (KeyError, OSError, TypeError, json.JSONDecodeError) as error:
        raise ValueError("reviewed capability runner manifest is invalid") from error
    replay = _is_exact_replay(report_path, row)
    if not replay and _digest(report_path.read_bytes()) != report_digest:
        raise ValueError("reviewed capability runner journal is stale")
    checkpoint = manifest_path.with_name("checkpoint.jsonl")
    try:
        checkpoint_entries = [
            json.loads(line)
            for line in checkpoint.read_text(encoding="utf-8").splitlines()
            if line
        ]
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError("reviewed capability runner journal is invalid") from error
    matches = [
        item
        for item in checkpoint_entries
        if item.get("row") == selector
        and item.get("entry_sha256") == entry_sha256(item)
    ]
    entry = matches[0] if len(matches) == 1 else None
    if not isinstance(entry, dict):
        raise ValueError("reviewed capability is absent from the runner journal")
    manifest_rows = {
        item.get("row"): item
        for item in manifest.get("rows", [])
        if isinstance(item, dict)
    }
    manifest_entry = manifest_rows.get(selector)
    derived_value = Path(str(entry.get("derived")))
    derived_path = (
        derived_value.resolve()
        if derived_value.is_absolute()
        else (root / derived_value).resolve()
    )
    if derived_path != expected_row / "derived.json":
        raise ValueError(
            "reviewed capability derived artifact is outside its selected namespace"
        )
    try:
        derived_bytes = derived_path.read_bytes()
        derived = json.loads(derived_bytes)
        payload = json.loads(
            (root / str(entry["evidence"])).read_text(encoding="utf-8")
        )
        _validate_current_plan(
            root,
            target,
            report_path,
            row,
            payload,
            manifest,
            entry,
            expected_namespace,
        )
        recomputed = recompute_collected_records(
            _RUNNER_TOKEN,
            root=root,
            target=target,
            report_digest=report_digest,
            row=row,
            payload=payload,
        )
    except (KeyError, OSError, TypeError, ValueError, json.JSONDecodeError) as error:
        raise ValueError("reviewed capability derived artifact is invalid") from error
    expected_derived = {
        "derived": entry.get("derived"),
        "derived_sha256": _digest(derived_bytes),
        "derived_size": len(derived_bytes),
        "analyzer_version": ANALYZER_SCHEMA,
    }
    if (
        not isinstance(manifest_entry, dict)
        or any(entry.get(key) != value for key, value in expected_derived.items())
        or any(
            manifest_entry.get(key) != value for key, value in expected_derived.items()
        )
        or json_digest(recomputed) != json_digest(derived)
    ):
        raise ValueError(
            "reviewed capability derived artifact is stale or inconsistent"
        )
    capability = derived.get("conclusion_capability")
    supplied = source.get("conclusion_capability")
    if (
        not isinstance(capability, dict)
        or capability.get("schema") != CONCLUSION_CAPABILITY_SCHEMA
        or capability.get("allowed_conclusion") != "exhausted"
        or capability.get("proposal_allowed") is not True
        or (row["rung_status"] == "proposed" and not capability["proposal_allowed"])
        or json_digest(supplied) != json_digest(capability)
    ):
        raise ValueError("reviewed conclusion capability mismatches recomputed facts")
    _validate_source_shape(source, report_path, capability)
    return capability


def _validate_current_plan(
    root: Path,
    target: str,
    report_path: Path,
    row: dict,
    payload: dict,
    manifest: dict,
    entry: dict,
    namespace: EvidenceNamespace,
) -> None:
    """Replay one canonical row using live inputs and current generated work."""
    from harness.analysis.index import index_path
    from harness.naming.instructions import INSTRUCTION_CAPTURE
    from harness.naming.journal import _payload_is_fresh, operation_digest
    from harness.naming.plan import collection_row, row_operation_sequence

    report = json.loads(report_path.read_bytes())
    rows = [
        current
        for current in report.get("rows", [])
        if (current.get("kind"), current.get("name")) == (row["kind"], row["name"])
    ]
    if len(rows) != 1 or any(
        rows[0].get(key) is not row.get(key)
        for key in ("outside_payload", "partial_used")
    ):
        raise ValueError(
            "capability row locality or partial metadata differs from report"
        )
    token = INSTRUCTION_CAPTURE.set(
        any(item.get("instruction_id") is not None for item in payload["semantic"])
    )
    try:
        planned = collection_row(row, target, root=root)
        operations = row_operation_sequence(planned, target, root=root)
        repository_inputs = {
            path.relative_to(root).as_posix(): _digest(path.read_bytes())
            for path in sorted(root.glob("config/targets/**/target.toml"))
        }
        manifest_rows = [
            item
            for item in manifest.get("rows", [])
            if item.get("row") == payload["row"]
        ]
        if (
            manifest.get("target") != target
            or manifest.get("report") != report_path.as_posix()
            or manifest.get("index_generation")
            != _digest(index_path(root).read_bytes())
            or manifest.get("repository_inputs") != repository_inputs
            or manifest.get("operation_digest")
            != operation_digest(target, report, root=root)
            or entry.get("status") != "committed"
            or entry.get("operations") != operations
            or len(manifest_rows) != 1
            or manifest_rows[0].get("operations") != operations
            or manifest_rows[0].get("status") != "committed"
            or not _payload_is_fresh(
                root,
                namespace.path / str(row["name"]) / "payload.json",
                namespace=namespace.path,
                target=target,
                row=payload["row"],
                operations=operations,
            )
        ):
            raise ValueError(
                "capability runner inputs, operations or receipts are stale"
            )
    finally:
        INSTRUCTION_CAPTURE.reset(token)


def validate_terminal_capability(
    root: Path,
    target: str,
    report_path: Path,
    row: dict[str, Any],
) -> None:
    """Revalidate one capability-backed terminal row against current evidence."""
    selector = f"{row.get('kind')}:{row.get('name')}"
    provenance = require_canonical_provenance(row, selector)
    evidence = provenance.get("evidence")
    report_digest = provenance.get("initializer_report_sha256")
    if not isinstance(evidence, str) or not isinstance(report_digest, str):
        raise ValueError("terminal capability provenance is incomplete")
    evidence_path = Path(evidence)
    resolved = (
        evidence_path.resolve()
        if evidence_path.is_absolute()
        else (root / evidence_path).resolve()
    )
    row_dir = resolved.parent
    namespace_path = row_dir.parent
    explicit = evidence_path.is_absolute()
    from harness.naming.namespace import selected_evidence_root

    if explicit and selected_evidence_root() != namespace_path.parent:
        raise ValueError(
            "terminal capability requires its exact selected evidence root"
        )
    namespace = EvidenceNamespace(
        namespace_path.parent if explicit else root, namespace_path, explicit
    )
    expected_provenance = {
        "kind": provenance["kind"],
        "evidence": namespace.record_path(row_dir / "payload.json"),
        "evidence_sha256": _digest((row_dir / "payload.json").read_bytes()),
        "initializer_report_sha256": report_digest,
        "evidence_namespace": {
            "mode": "explicit" if explicit else "default",
            "root": (
                namespace.path.parent.as_posix()
                if explicit
                else namespace.path.parent.relative_to(root).as_posix()
            ),
            "namespace": (
                namespace.path.as_posix()
                if explicit
                else namespace.path.relative_to(root).as_posix()
            ),
        },
    }
    if provenance["kind"] == "typed-capability":
        expected_provenance["conclusion_sha256"] = _digest(
            json.dumps(
                {key: value for key, value in row.items() if key != PROVENANCE_FIELD},
                sort_keys=True,
                separators=(",", ":"),
            ).encode()
        )
    if provenance != expected_provenance:
        raise ValueError("terminal capability provenance differs from current evidence")
    try:
        derived = json.loads((row_dir / "derived.json").read_text(encoding="utf-8"))
        capability = derived["conclusion_capability"]
    except (KeyError, OSError, TypeError, json.JSONDecodeError) as error:
        raise ValueError("terminal capability is no longer enabled") from error
    source = {
        "report": report_path.as_posix(),
        "report_digest": report_digest,
        "evidence": evidence,
        "evidence_sha256": provenance.get("evidence_sha256"),
        "conclusion_capability": capability,
    }
    validated = _validated_capability(
        root,
        target,
        report_path,
        row,
        source,
        namespace,
    )
    if validated is None:
        raise ValueError("terminal row is not backed by a current exact capability")
    actual = {key: value for key, value in row.items() if key != PROVENANCE_FIELD}
    from harness.naming.conclusion import _validate_conclusion
    from harness.naming.validation import _validate_evidence

    _validate_conclusion(actual, validated)
    current_provenance = _validate_evidence(
        root,
        target,
        selector,
        actual,
        source,
        namespace,
        expected_provenance["evidence_namespace"],
        validated,
    )
    if current_provenance != provenance:
        raise ValueError("terminal row differs from its current typed capability")
