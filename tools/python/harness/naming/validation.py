"""Validate generic and exact naming conclusion evidence."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from harness.domain.receipts import command_records
from harness.naming.collection import EvidenceNamespace
from harness.naming.equivalence import _exact_capability_row
from harness.naming.journal import entry_sha256

FACT_SCHEMA = "bof3.naming-evidence-facts/v1"


def _digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _records(row: dict[str, Any]) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for rung in row.get("rungs", {}).values():
        if isinstance(rung, dict):
            records.extend(rung.get("commands", []))
    for item in row.get("required_work", []):
        if isinstance(item, dict) and item.get("status") == "completed":
            records.extend(item.get("commands", []))
    return records


def _selector_target(operation: object, selector: object) -> str | None:
    """Derive the only authorized target relation from a typed selector."""

    if operation not in {"calls", "access", "owner", "semantic"} or not isinstance(
        selector, str
    ):
        return None
    selected_target, separator, address = selector.partition("@")
    if not separator or not selected_target or not address:
        return None
    return selected_target


def _record_allowed(
    record: dict[str, Any], item: dict[str, Any], import_target: str
) -> bool:
    operation = item.get("operation")
    selector = item.get("selector")
    if record.get("operation") != operation or record.get("selector") != selector:
        return False
    selected_target = _selector_target(operation, selector)
    if selected_target is None:
        return False
    if operation == "owner":
        return (
            selected_target
            == str(item.get("id", "")).removeprefix("owner:").partition("@")[0]
        )
    return selected_target == import_target


def _allowed_next_commands(initializer: dict[str, Any]) -> frozenset[str]:
    """Return the finite command set generated for this exact initializer row."""

    commands = {
        initializer.get("ceiling_next_command"),
        initializer.get("smallest_repair"),
    }
    commands.update(
        rung.get("next_command")
        for rung in initializer.get("rungs", {}).values()
        if isinstance(rung, dict)
    )
    return frozenset(command for command in commands if isinstance(command, str))


def _validate_next_command(command: object) -> None:
    """Reject malformed commands in shape-only fixture validation."""

    if not isinstance(command, str) or command not in {
        "bin/rev-query --json owners exe/test@0x80100010"
    }:
        raise ValueError("next command requires canonical eight-digit uppercase form")


def _validate_next_commands(row: dict[str, Any], initializer: dict[str, Any]) -> None:
    allowed = _allowed_next_commands(initializer)
    commands = [row.get("ceiling_next_command"), row.get("smallest_repair")]
    commands.extend(
        rung.get("next_command")
        for rung in row.get("rungs", {}).values()
        if isinstance(rung, dict) and rung.get("next_command") is not None
    )
    commands.extend(
        work.get("next_command")
        for work in (*row.get("required_work", []), *row.get("optional_work", []))
        if isinstance(work, dict) and work.get("next_command") is not None
    )
    if any(
        not isinstance(command, str) or command not in allowed
        for command in commands
        if command is not None
    ):
        raise ValueError(
            "authored command was not generated for this exact initializer row"
        )


def _reject_unsupported_semantic_facts(
    row: dict[str, Any], capability: dict[str, Any] | None
) -> None:
    """Fail closed except for the exact runner-recomputed one-row capability."""

    if row.get("rung_status") in {"exhausted", "proposed"} and capability is None:
        raise ValueError(
            "semantic fact capability is unsupported: receipt, checkpoint, and "
            "manifest bytes are recovery evidence, not an authenticity boundary; "
            "a trusted native-output typed analyzer is required"
        )


def _canonical_json_digest(value: object) -> str:
    return _digest(json.dumps(value, sort_keys=True, separators=(",", ":")).encode())


def _receipt_facts(
    root: Path,
    record: dict[str, Any],
    *,
    report_digest: str,
    target: str,
    row_selector: str,
    journal_entry: dict[str, Any],
    payload_record: dict[str, Any],
) -> tuple[str, frozenset[str]]:
    """Validate a typed receipt against its native runner journal registration."""

    path = root / str(record["receipt"])
    receipt = json.loads(path.read_text(encoding="utf-8"))
    if receipt.get("schema") != FACT_SCHEMA:
        return "", frozenset()
    facts = receipt.get("facts")
    if not isinstance(facts, list) or any(not isinstance(fact, str) for fact in facts):
        raise ValueError("runner fact receipt has invalid facts")
    immutable = ("command", "status", "target", "selector", "output")
    if any(receipt.get(key) != payload_record.get(key) for key in immutable):
        raise ValueError("typed receipt differs from its runner command record")
    if record.get("sha256") != _digest(path.read_bytes()):
        raise ValueError("typed receipt digest is stale")
    semantic = {
        "report_digest": report_digest,
        "target": target,
        "row": row_selector,
        "operation": record.get("operation"),
        "selector": record.get("selector"),
        "command": " ".join(str(record.get("command", "")).split()),
        "payload_digest": journal_entry.get("evidence_sha256"),
        "facts": sorted(facts),
    }
    return _canonical_json_digest(semantic), frozenset(facts)


def _validate_typed_rungs(
    root: Path,
    row: dict[str, Any],
    payload: dict[str, Any],
    *,
    report_digest: str,
    target: str,
    row_selector: str,
    journal_entry: dict[str, Any],
) -> None:
    used: set[str] = set()
    sources: set[str] = set()
    payload_records = {
        (record["receipt"], record["sha256"]): record
        for record in command_records(payload.get("commands"), "source.commands", root)
    }
    for rung_name, rung in row.get("rungs", {}).items():
        if not isinstance(rung, dict):
            continue
        rung_records = command_records(
            rung.get("commands"), f"{rung_name}.commands", root
        )
        if not rung_records:
            raise ValueError(f"{rung_name} requires its own typed evidence receipt")
        declared = {
            (item.get("fact_class"), item.get("evidence_receipt"))
            for item in rung.get("observations", [])
            if isinstance(item, dict) and item.get("producer") == "analyzer"
        }
        for record in rung_records:
            payload_record = payload_records.get((record["receipt"], record["sha256"]))
            if payload_record is None:
                raise ValueError("typed receipt is absent from runner payload")
            source_id, capabilities = _receipt_facts(
                root,
                record,
                report_digest=report_digest,
                target=target,
                row_selector=row_selector,
                journal_entry=journal_entry,
                payload_record=payload_record,
            )
            if source_id in used:
                raise ValueError(
                    "one evidence source cannot close more than one semantic rung"
                )
            used.add(source_id)
            if (
                rung_name not in capabilities
                or (
                    rung_name,
                    record.get("receipt"),
                )
                not in declared
            ):
                raise ValueError(f"receipt lacks runner-produced {rung_name} facts")
            sources.add(source_id)
    corroborators = row.get("corroborators")
    declarations = (
        [value.get("source_id") for value in corroborators.values()]
        if isinstance(corroborators, dict)
        and all(isinstance(value, dict) for value in corroborators.values())
        else []
    )
    cited_sources = set(declarations)
    if (
        len(sources) < 2
        or len(declarations) != len(cited_sources)
        or cited_sources != sources
    ):
        raise ValueError(
            "corroborators must cite two distinct validator-derived source IDs"
        )


def _validate_evidence(
    root: Path,
    target: str,
    selector: str,
    row: dict[str, Any],
    source: object,
    expected_namespace: EvidenceNamespace,
    namespace_provenance: dict[str, str],
    capability: dict[str, Any] | None = None,
) -> dict[str, Any]:
    if not isinstance(source, dict):
        raise ValueError("source must bind collected evidence")
    selected_root = expected_namespace.path if capability is not None else None
    records = command_records(
        _records(row),
        f"{selector}.evidence",
        root,
        evidence_root=selected_root,
    )
    if any(record["status"] != "passed" for record in records):
        raise ValueError("conclusion evidence must contain only passed commands")
    evidence = source.get("evidence")
    digest = source.get("evidence_sha256")
    if not isinstance(evidence, str) or not evidence or not isinstance(digest, str):
        raise ValueError("source requires evidence and evidence_sha256")
    path = (
        (root / evidence).resolve()
        if not Path(evidence).is_absolute()
        else Path(evidence).resolve()
    )
    allowed_root = (
        expected_namespace.path
        if capability is not None
        else (root / "out/reviews/evidence").resolve()
    )
    if (
        allowed_root not in path.parents
        or not path.is_file()
        or _digest(path.read_bytes()) != digest
    ):
        raise ValueError("source evidence is missing or stale")
    payload = json.loads(path.read_text(encoding="utf-8"))
    if payload.get("target") != target or payload.get("row") != selector:
        raise ValueError("source evidence target/row mismatch")
    payload_records = command_records(
        payload.get("commands"),
        "source.commands",
        root,
        evidence_root=selected_root,
    )
    allowed = {(record["receipt"], record["sha256"]) for record in payload_records}
    cited = {(record["receipt"], record["sha256"]) for record in records}
    if not cited or not cited <= allowed:
        raise ValueError("conclusion cites commands outside the bound payload")
    items = {
        item["id"]: item
        for item in payload.get("items", [])
        if isinstance(item, dict) and isinstance(item.get("id"), str)
    }
    for record in records:
        item = items.get(str(record.get("item")))
        if record.get("operation") == "semantic":
            allowed_record = (
                _selector_target("semantic", record.get("selector")) == target
            )
        else:
            allowed_record = item is not None and _record_allowed(record, item, target)
        if record.get("target") != target or not allowed_record:
            raise ValueError(
                "receipt target/selector is not authorized by its exact work item"
            )
    for work in row.get("required_work", []):
        if not isinstance(work, dict) or work.get("status") != "completed":
            continue
        item = items.get(work.get("id"))
        if item is None:
            raise ValueError(f"completed work lacks payload item: {work.get('id')}")
        for record in work.get("commands", []):
            if record.get("item") != work.get("id") or not _record_allowed(
                record, item, target
            ):
                raise ValueError(
                    f"work receipt selector/operation mismatch: {work.get('id')}"
                )
    if any(
        record.get("command") == "harness:naming-evidence-run bounded collection"
        for record in records
    ):
        raise ValueError("no-op collection receipt is not semantic evidence")
    report_digest = str(source.get("report_digest", ""))
    if not report_digest:
        raise ValueError("source requires report_digest")
    journal_root = (
        expected_namespace.path
        if capability is not None
        else root / "out/reviews/evidence" / f"{target}__{report_digest[:12]}"
    )
    journal = journal_root / "checkpoint.jsonl"
    manifest = journal_root / "manifest.json"
    try:
        entries = [
            json.loads(line) for line in journal.read_text().splitlines() if line
        ]
    except (OSError, json.JSONDecodeError):
        entries = []
    matching = [
        entry
        for entry in entries
        if entry.get("row") == selector
        and entry.get("entry_sha256") == entry_sha256(entry)
    ]
    if len(matching) != 1 or not manifest.is_file():
        raise ValueError("source evidence is absent from the native runner journal")
    entry = matching[0]
    manifest_payload = json.loads(manifest.read_text(encoding="utf-8"))
    manifest_rows = {
        item.get("row"): item
        for item in manifest_payload.get("rows", [])
        if isinstance(item, dict)
    }
    if (
        manifest_payload.get("report_digest") != report_digest
        or manifest_payload.get("target") != target
        or (root / str(entry.get("evidence"))).resolve() != path
        or entry.get("evidence_sha256") != digest
        or manifest_rows.get(selector, {}).get("evidence_sha256") != digest
    ):
        raise ValueError("source evidence is not bound to the current runner manifest")
    if capability is None:
        _validate_typed_rungs(
            root,
            row,
            payload,
            report_digest=report_digest,
            target=target,
            row_selector=selector,
            journal_entry=entry,
        )
    else:
        cited_records = {
            (record["receipt"], record["sha256"]): record for record in records
        }
        if len(cited_records) != 1:
            raise ValueError("reviewed capability requires one exact runner command")
        cited_command = next(iter(cited_records.values()))
        trusted_commands = [
            record
            for record in payload_records
            if (record["receipt"], record["sha256"])
            == (cited_command["receipt"], cited_command["sha256"])
        ]
        if len(trusted_commands) != 1 or cited_command != trusted_commands[0]:
            raise ValueError("conclusion command differs from trusted runner payload")
        trusted_command = trusted_commands[0]
        expected_row = _exact_capability_row(capability, trusted_command)
        if row != expected_row:
            raise ValueError("conclusion does not match the closed reviewed schema")
    return {
        "kind": "exact-capability" if capability is not None else "generic-evidence",
        "evidence": evidence,
        "evidence_sha256": digest,
        "initializer_report_sha256": report_digest,
        "evidence_namespace": namespace_provenance,
    }
