"""Validate closed exact-row Battle 15 conclusion capabilities."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from harness.naming.capabilities import (
    PRODUCTION_EXACT_CAPABILITIES,
    ExactCapabilityRegistry,
)
from harness.naming.collection import EvidenceNamespace
from harness.naming.evidence import (
    _RUNNER_TOKEN,
    CONCLUSION_CAPABILITY_SCHEMA,
    _analyze_validated_records,
)
from harness.naming.evidence import SCHEMA as ANALYZER_SCHEMA
from harness.naming.journal import JournalInputs, entry_sha256, journal_is_fresh
from harness.naming.provenance import (
    PROVENANCE_FIELD,
    capability_eligible_rows,
    require_canonical_provenance,
)

_EXACT_AUTHORITY = (
    "target manifest, reviewed Splat, original image, and fresh reverse index"
)
_EXACT_REVIEW_AUTHORITY = (
    "reviewed Splat, original image, fresh reverse index, and trusted typed analyzer"
)
_EXACT_INTERPRETATION = "positive evidence exhaustion; no semantic rename is supported"
_EXACT_REQUIRED_OBSERVATION = "native runner completed the exact generated work item"


def _exact_capability_row(
    capability: dict[str, Any], command: dict[str, Any]
) -> dict[str, Any]:
    """Generate one allowlisted reviewed terminal row; authored structure is closed."""

    row_name = capability["row"].removeprefix("data:")
    address = row_name.removeprefix("D_")
    target = capability["target"]
    next_command = (
        f"bin/harness analysis query --json xrefs {target}@0x{address}; "
        f"bin/harness analysis rz-project query {target} -c 'axt @ 0x{address}'"
    )
    rungs = {
        name: {
            "status": "passed",
            "observations": [
                {
                    "id": f"{row_name}.{name}.{index}",
                    "text": "trusted analyzer positive fact",
                    "producer": "analyzer",
                    "source_id": source_id,
                }
                for index, source_id in enumerate(source_ids, 1)
            ],
            "authority": _EXACT_AUTHORITY,
            "commands": [command],
        }
        for name, source_ids in capability["rungs"].items()
    }
    return {
        "authority": _EXACT_REVIEW_AUTHORITY,
        "ceiling_next_command": next_command,
        "interpretation": _EXACT_INTERPRETATION,
        "kind": "data",
        "missing_fact": capability["missing_fact"],
        "name": row_name,
        "optional_work": [],
        "outside_payload": False,
        "partial_used": False,
        "required_work": [
            {
                "id": capability["required_work"][0],
                "status": "completed",
                "commands": [command],
                "observations": [
                    {
                        "id": f"{row_name}.required.access",
                        "text": _EXACT_REQUIRED_OBSERVATION,
                    }
                ],
            }
        ],
        "rung_status": "exhausted",
        "rungs": rungs,
        "smallest_repair": next_command,
        "corroborators": {
            "original-layout": {
                "observation_ids": [
                    observation["id"]
                    for name in ("selected_range", "storage_class")
                    for observation in rungs[name]["observations"]
                ],
                "source_id": capability["rungs"]["storage_class"][0],
            },
            "original-consumer": {
                "observation_ids": [
                    observation["id"]
                    for name in ("selected_access", "one_level_beyond")
                    for observation in rungs[name]["observations"]
                ],
                "source_id": capability["rungs"]["one_level_beyond"][0],
            },
        },
    }


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
    return {
        key: value for key, value in matches[0].items() if key != PROVENANCE_FIELD
    } == {key: value for key, value in row.items() if key != PROVENANCE_FIELD}


def _validate_exact_source_shape(
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
    if (
        source["report"] != report_path.as_posix()
        or source["conclusion_capability"] != capability
    ):
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
    *,
    registry: ExactCapabilityRegistry = PRODUCTION_EXACT_CAPABILITIES,
) -> dict[str, Any] | None:
    """Return the one runner-recomputed exhausted capability, otherwise fail closed."""

    row_name = row.get("name")
    selector = f"data:{row_name}"
    entries = [
        entry
        for (spec_row, _consumer), entry in registry.items()
        if spec_row == selector and entry.consumer.target == target
    ]
    if row.get("kind") != "data" or len(entries) != 1:
        return None
    if row.get("rung_status") != "exhausted" or not isinstance(source, dict):
        raise ValueError("reviewed capability permits only allowlisted exhausted rows")
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
        inputs = JournalInputs(
            target=target,
            report_digest=report_digest,
            index_generation=str(manifest["index_generation"]),
            operation_digest=str(manifest["operation_digest"]),
            repository_inputs=manifest.get("repository_inputs", {}),
        )
    except (KeyError, OSError, TypeError, json.JSONDecodeError) as error:
        raise ValueError("reviewed capability runner manifest is invalid") from error
    replay = _is_exact_replay(report_path, row)
    if not replay and _digest(report_path.read_bytes()) != report_digest:
        raise ValueError("reviewed capability runner journal is stale")
    if not replay and not journal_is_fresh(
        root, report_path, target, inputs, registry=registry
    ):
        raise ValueError("reviewed capability runner journal is stale")
    if replay:
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
    else:
        checkpoint = manifest_path.with_name("checkpoint.jsonl")
        try:
            candidates = [
                json.loads(line)
                for line in checkpoint.read_text(encoding="utf-8").splitlines()
                if line
            ]
        except (OSError, json.JSONDecodeError) as error:
            raise ValueError("reviewed capability runner journal is invalid") from error
        matches = [
            item
            for item in candidates
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
        operations = tuple(
            {
                "item": {"id": item["id"]},
                "operation": item["operation"],
                "selector": item["selector"],
                "payload": item["payload"],
                "supplemental": item["supplemental"],
                "role": item.get("role"),
            }
            for item in payload["items"]
        )
        recomputed = _analyze_validated_records(
            _RUNNER_TOKEN,
            target=target,
            report_digest=report_digest,
            row=row,
            operations=operations,
            registry=registry,
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
        or recomputed != derived
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
        or capability.get("proposal_allowed") is not False
        or supplied != capability
    ):
        raise ValueError("reviewed conclusion capability mismatches recomputed facts")
    _validate_exact_source_shape(source, report_path, capability)
    return capability


def validate_terminal_capability(
    root: Path,
    target: str,
    report_path: Path,
    row: dict[str, Any],
    *,
    registry: ExactCapabilityRegistry = PRODUCTION_EXACT_CAPABILITIES,
) -> None:
    """Revalidate one capability-backed terminal row against current evidence."""
    selector = f"{row.get('kind')}:{row.get('name')}"
    provenance = require_canonical_provenance(row, selector)
    if selector not in capability_eligible_rows(target, registry=registry):
        raise ValueError("terminal row is not backed by a current exact capability")
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
        "kind": "exact-capability",
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
        registry=registry,
    )
    if validated is None:
        raise ValueError("terminal row is not backed by a current exact capability")
    payload = json.loads(resolved.read_text(encoding="utf-8"))
    expected = _exact_capability_row(validated, payload["commands"][0])
    actual = {key: value for key, value in row.items() if key != PROVENANCE_FIELD}
    if actual != expected:
        raise ValueError("terminal row differs from its current exact capability")
