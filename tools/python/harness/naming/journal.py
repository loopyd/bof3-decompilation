"""Digest-bound, stale-aware, torn-tail-safe evidence journal.

One file holds the committed row operations for a (report, target) run.
Each committed row operation is a canonical JSON line whose ``entry_sha256``
field hashes the canonical prefix of that same line, so a kill mid-append
never corrupts a committed line: a truncated (or tampered) tail fails the
digest check, is kept for forensics under ``checkpoint.jsonl.tail`` and
discarded, and its rows are reselected.  The manifest additionally binds
the whole journal to the repository, index, and operation inputs, so a
journal from a stale generation is never silently reused.
"""

from __future__ import annotations

import hashlib
import json
import os
from collections.abc import Mapping
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any
from harness.common.deadlines import check_deadline

from harness.domain.receipts import command_records
from harness.naming.capabilities import PRODUCTION_EXACT_CAPABILITIES
from harness.naming.evidence import _RUNNER_TOKEN, _analyze_validated_records
from harness.naming.evidence import SCHEMA as ANALYZER_SCHEMA
from harness.naming.namespace import (
    _artifact_is_fresh,
    _artifact_path,
    journal_dir,
    journal_path,
    manifest_path,
)
from harness.naming.plan import collection_row, row_operation_sequence

MANIFEST_SCHEMA = "bof3.naming-evidence-manifest/v2"


@dataclass(frozen=True)
class JournalInputs:
    """Repository/index/operation inputs a journal must match to reuse."""

    target: str
    report_digest: str
    index_generation: str
    operation_digest: str
    repository_inputs: Mapping[str, str] = field(default_factory=dict)


def operation_digest(target: str, report: dict[str, Any]) -> str:
    """Digest of the exact report-derived operation sequence per row."""

    operations = [
        (f"{row.get('kind')}:{row.get('name')}", row_operation_sequence(row, target))
        for row in report.get("rows", [])
        if isinstance(row, dict)
    ]
    payload = json.dumps({"target": target, "operations": operations}, sort_keys=True)
    return hashlib.sha256(payload.encode()).hexdigest()


def _canonical_prefix(entry: dict[str, Any]) -> bytes:
    payload = dict(entry)
    payload.pop("entry_sha256", None)
    return (json.dumps(payload, sort_keys=True) + ",").encode()


def entry_sha256(entry: dict[str, Any]) -> str:
    return hashlib.sha256(_canonical_prefix(entry)).hexdigest()


def commit_entry(
    namespace: Any, report: Path, target: str, entry: dict[str, Any]
) -> dict[str, Any]:
    """Append one self-hashing committed row operation, durably.

    The line is written, fsync'd, and the file fsynced again while the
    handle is open: a kill at any point leaves either the old complete
    journal or the complete new line, never a torn tail.  A torn tail is
    discarded on the next load, so the committed set never depends on an
    un-durable line.
    """

    path = (
        namespace.path / "checkpoint.jsonl"
        if hasattr(namespace, "path")
        else journal_path(namespace, report, target)
    )
    check_deadline()
    path.parent.mkdir(parents=True, exist_ok=True)
    committed = dict(entry)
    committed["entry_sha256"] = entry_sha256(committed)
    check_deadline()
    with path.open("a", encoding="utf-8", newline="") as handle:
        handle.write(json.dumps(committed, sort_keys=True) + "\n")
        handle.flush()
        os.fsync(handle.fileno())
    return committed


def load_journal(
    root: Path, report: Path, target: str
) -> tuple[dict[str, dict[str, Any]], bool]:
    """Valid committed operations keyed by ``KIND:NAME`` plus a torn flag.

    Every line must parse, self-hash, and reference no duplicate row.
    The first invalid line marks the journal torn: from that line to the
    end is preserved under ``checkpoint.jsonl.tail`` for forensics and
    discarded, and those rows are simply collected again.
    """

    path = journal_path(root, report, target)
    if not path.is_file():
        return {}, False
    entries: dict[str, dict[str, Any]] = {}
    lines = path.read_text(encoding="utf-8").splitlines()
    valid: list[str] = []
    for index, line in enumerate(lines):
        if not line.strip():
            continue
        try:
            entry = json.loads(line)
            if not isinstance(entry, dict):
                raise ValueError("not an object")
            if (
                entry.get("entry_sha256")
                != hashlib.sha256(_canonical_prefix(entry)).hexdigest()
            ):
                raise ValueError("entry digest mismatch")
            row = str(entry.get("row"))
            if row in entries:
                raise ValueError(f"duplicate row: {row}")
            entries[row] = entry
            valid.append(line)
        except (json.JSONDecodeError, ValueError) as error:
            tail = "\n".join(lines[index:])
            forensic = path.with_name("checkpoint.jsonl.tail")
            check_deadline()
            forensic.write_text(tail + "\n", encoding="utf-8")
            torn_final = isinstance(error, json.JSONDecodeError) and not any(
                item.strip() for item in lines[index + 1 :]
            )
            if not torn_final:
                check_deadline()
                path.with_name("checkpoint.jsonl.corrupt").write_bytes(
                    path.read_bytes()
                )
                return {}, True
            check_deadline()
            path.write_text(
                "\n".join(valid) + ("\n" if valid else ""), encoding="utf-8"
            )
            with path.open("a", encoding="utf-8") as handle:
                handle.flush()
                os.fsync(handle.fileno())
            return entries, True
    return entries, False


def rotate_journal(root: Path, report: Path, target: str, reason: str) -> None:
    """Preserve stale/corrupt active state and start a fresh journal."""

    for path in (
        journal_path(root, report, target),
        manifest_path(root, report, target),
    ):
        if path.exists():
            forensic = path.with_name(f"{path.name}.{reason}")
            suffix = 1
            while forensic.exists():
                forensic = path.with_name(f"{path.name}.{reason}.{suffix}")
                suffix += 1
            check_deadline()
            path.replace(forensic)


def write_manifest(
    root: Path,
    report: Path,
    target: str,
    inputs: JournalInputs,
    entries: Mapping[str, dict[str, Any]],
) -> dict[str, Any]:
    """Digest-keyed manifest binding the journal to its input generation."""

    manifest = {
        "schema": MANIFEST_SCHEMA,
        "target": target,
        "report": report.as_posix(),
        "report_digest": inputs.report_digest,
        "index_generation": inputs.index_generation,
        "operation_digest": inputs.operation_digest,
        "repository_inputs": dict(sorted(inputs.repository_inputs.items())),
        "rows": [
            {
                "row": row,
                "status": str(entry.get("status")),
                "evidence_sha256": entry.get("evidence_sha256"),
                "operations": entry.get("operations", []),
                "derived": entry.get("derived"),
                "derived_sha256": entry.get("derived_sha256"),
                "derived_size": entry.get("derived_size"),
                "analyzer_version": entry.get("analyzer_version"),
            }
            for row, entry in sorted(entries.items())
        ],
    }
    path = manifest_path(root, report, target)
    check_deadline()
    path.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps(manifest, indent=2, sort_keys=True) + "\n"
    temporary = path.with_name(f".{path.name}.tmp")
    check_deadline()
    with temporary.open("w", encoding="utf-8") as handle:
        handle.write(text)
        handle.flush()
        os.fsync(handle.fileno())
    check_deadline()
    os.replace(temporary, path)
    directory = os.open(path.parent, os.O_RDONLY)
    try:
        os.fsync(directory)
    finally:
        os.close(directory)
    manifest["sha256"] = hashlib.sha256(text.encode()).hexdigest()
    return manifest


def _payload_is_fresh(
    root: Path,
    evidence: Path,
    *,
    namespace: Path,
    target: str,
    row: str,
    operations: object,
) -> bool:
    try:
        payload = json.loads(evidence.read_text(encoding="utf-8"))
        items = payload.get("items")
        semantic = payload.get("semantic")
        if not isinstance(items, list) or not all(
            isinstance(item, dict) for item in items
        ):
            return False
        if not isinstance(semantic, list) or not all(
            isinstance(item, dict) for item in semantic
        ):
            return False
        payload_operations = [
            f"indexed:{item.get('id')}:{item.get('operation')}:{item.get('selector')}:{int(bool(item.get('supplemental')))}"
            for item in items
        ]
        from harness.naming.instructions import replay_instructions, semantic_identities

        payload_operations.extend(semantic_identities(semantic))
        command_records(
            payload.get("commands"), "commands", root, evidence_root=namespace
        )
    except (AttributeError, json.JSONDecodeError, UnicodeDecodeError, ValueError):
        return False
    if (
        payload.get("target") != target
        or payload.get("row") != row
        or not isinstance(operations, list)
        or payload_operations != operations
    ):
        return False
    semantic = payload.get("semantic")
    if not isinstance(semantic, list):
        return False
    artifacts_fresh = all(
        isinstance(item, dict)
        and _artifact_is_fresh(
            root,
            namespace,
            item.get("raw_file"),
            item.get("raw_sha256"),
            item.get("raw_size"),
        )
        and _artifact_is_fresh(
            root,
            namespace,
            item.get("stderr_file"),
            item.get("stderr_sha256"),
            item.get("stderr_size"),
        )
        for item in semantic
    )
    try:
        return artifacts_fresh and replay_instructions(root, payload, semantic)
    except (OSError, KeyError, TypeError, ValueError):
        return False


def journal_is_fresh(
    root: Path,
    report: Path,
    target: str,
    inputs: JournalInputs,
    *,
    registry=PRODUCTION_EXACT_CAPABILITIES,
) -> bool:
    """Reuse only when inputs, receipts, and raw evidence remain digest-bound."""

    path = manifest_path(root, report, target)
    if not path.is_file():
        return False
    try:
        recorded = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return False
    for key, expected in (
        ("target", inputs.target),
        ("report_digest", inputs.report_digest),
        ("index_generation", inputs.index_generation),
        ("operation_digest", inputs.operation_digest),
    ):
        if recorded.get(key) != expected:
            return False
    if recorded.get("repository_inputs") != dict(
        sorted(inputs.repository_inputs.items())
    ):
        return False
    entries, _ = load_journal(root, report, target)
    records = recorded.get("rows")
    if not isinstance(records, list) or len(records) > len(entries):
        return False
    manifest_rows: dict[str, dict[str, Any]] = {}
    for record in records:
        if not isinstance(record, dict):
            return False
        row = str(record.get("row"))
        if row in manifest_rows:
            return False
        manifest_rows[row] = record
    report_row_values = {
        f"{row.get('kind')}:{row.get('name')}": row
        for row in json.loads(report.read_text(encoding="utf-8")).get("rows", [])
        if isinstance(row, dict)
    }
    report_rows = {
        key: row_operation_sequence(
            collection_row(value, target, registry=registry),
            target,
            registry=registry,
        )
        for key, value in report_row_values.items()
    }
    for row, entry in entries.items():
        expected_operations = report_rows.get(row)
        if (
            expected_operations is None
            or entry.get("status") != "committed"
            or entry.get("operations") != expected_operations
        ):
            return False
        record = manifest_rows.get(row)
        if record is not None and (
            record.get("evidence_sha256") != entry.get("evidence_sha256")
            or record.get("status") != entry.get("status")
            or record.get("operations") != entry.get("operations", [])
            or record.get("derived") != entry.get("derived")
            or record.get("derived_sha256") != entry.get("derived_sha256")
            or record.get("derived_size") != entry.get("derived_size")
            or record.get("analyzer_version") != entry.get("analyzer_version")
        ):
            return False
        namespace = journal_dir(root, report, target)
        try:
            evidence = _artifact_path(root, namespace, entry.get("evidence"))
            expected_evidence = namespace / row.split(":", 1)[1] / "payload.json"
            if evidence != expected_evidence:
                return False
            evidence_payload = json.loads(evidence.read_text(encoding="utf-8"))
            operations = tuple(
                {
                    "item": {"id": item["id"]},
                    "operation": item["operation"],
                    "selector": item["selector"],
                    "payload": item["payload"],
                    "supplemental": item["supplemental"],
                    "role": item.get("role"),
                }
                for item in evidence_payload["items"]
            )
            recomputed = _analyze_validated_records(
                _RUNNER_TOKEN,
                target=target,
                report_digest=inputs.report_digest,
                row=collection_row(report_row_values[row], target, registry=registry),
                operations=operations,
                registry=registry,
            )
            derived = _artifact_path(root, namespace, entry.get("derived"))
            if derived != namespace / row.split(":", 1)[1] / "derived.json":
                return False
            recorded_derived = json.loads(derived.read_text(encoding="utf-8"))
        except (KeyError, OSError, TypeError, ValueError, json.JSONDecodeError):
            return False
        if (
            recomputed != recorded_derived
            or namespace not in evidence.parents
            or not evidence.is_file()
            or hashlib.sha256(evidence.read_bytes()).hexdigest()
            != entry.get("evidence_sha256")
            or not _payload_is_fresh(
                root,
                evidence,
                namespace=namespace,
                target=target,
                row=row,
                operations=expected_operations,
            )
            or entry.get("analyzer_version") != ANALYZER_SCHEMA
            or not _artifact_is_fresh(
                root,
                namespace,
                entry.get("derived"),
                entry.get("derived_sha256"),
                entry.get("derived_size"),
            )
        ):
            return False
    if set(manifest_rows) - set(entries):
        return False
    if len(records) < len(entries):
        write_manifest(root, report, target, inputs, entries)
    return True


__all__ = [
    "JournalInputs",
    "commit_entry",
    "entry_sha256",
    "journal_is_fresh",
    "load_journal",
    "operation_digest",
    "rotate_journal",
    "write_manifest",
]
