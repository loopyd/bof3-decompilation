"""Validate current typed naming conclusion evidence."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from harness.domain.receipts import command_records
from harness.naming.collection import EvidenceNamespace
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

    if operation not in {
        "calls",
        "access",
        "owner",
        "semantic",
        "describe",
        "xrefs",
        "data_original_range",
        "data_dispatch_consumer",
    } or not isinstance(selector, str):
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
        "bin/harness analysis query --json owners exe/test@0x80100010"
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
    trusted_facts: list[dict[str, Any]] | None = None,
) -> None:
    used: set[str] = set()
    sources: set[str] = set()
    observation_sources: dict[str, str] = {}
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
            if trusted_facts is not None and _canonical_json_digest(
                record
            ) != _canonical_json_digest(payload_record):
                raise ValueError("typed command differs from its exact runner record")
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
            if trusted_facts is not None:
                receipt = json.loads((root / record["receipt"]).read_bytes())
                observations = rung.get("observations", [])
                declared_observations = [
                    observation
                    for observation in observations
                    if observation.get("evidence_receipt") == record["receipt"]
                ]
                if not declared_observations:
                    raise ValueError(f"{rung_name} lacks exact receipt observations")
                for observation in declared_observations:
                    projected = {
                        "class": observation.get("fact_class"),
                        "polarity": observation.get("polarity"),
                        "value": observation.get("value"),
                    }
                    if (
                        observation.get("producer") != "analyzer"
                        or projected["class"] != rung_name
                        or projected["polarity"] != "positive"
                        or _canonical_json_digest(projected)
                        not in {
                            _canonical_json_digest(fact)
                            for fact in receipt.get("observations", [])
                        }
                        or _canonical_json_digest(
                            {**projected, "source_id": observation.get("source_id")}
                        )
                        not in {_canonical_json_digest(fact) for fact in trusted_facts}
                        or not isinstance(observation.get("id"), str)
                        or observation["id"] in observation_sources
                    ):
                        raise ValueError(
                            f"{rung_name} observation differs from replayed facts"
                        )
                    observation_sources[observation["id"]] = source_id
        if trusted_facts is not None and any(
            observation.get("id") not in observation_sources
            for observation in rung.get("observations", [])
        ):
            raise ValueError(f"{rung_name} contains observations outside its receipts")
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
        or (
            cited_sources != sources
            if trusted_facts is None
            else len(cited_sources) < 2 or not cited_sources <= sources
        )
    ):
        raise ValueError(
            "corroborators must cite two distinct validator-derived source IDs"
        )
    if trusted_facts is not None:
        from harness.naming.facts import corroborators, row_observation_ids

        corroborators(row, str(row["name"]), row_observation_ids(row, str(row["name"])))
        for spec in row["corroborators"].values():
            if any(
                observation_sources.get(identity) != spec["source_id"]
                for identity in spec["observation_ids"]
            ):
                raise ValueError(
                    "corroborator source differs from its exact observations"
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
    if capability is None or capability.get("proposal_allowed") is not True:
        raise ValueError("semantic fact capability is unsupported")
    selected_root = expected_namespace.path
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
    allowed_root = expected_namespace.path
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
            if not allowed_record:
                allowed_record = any(
                    capture.get("selector") == record.get("selector")
                    and capture.get("instruction_id") is not None
                    and record.get("target")
                    == _selector_target("semantic", capture.get("selector"))
                    for capture in payload.get("semantic", [])
                )
        else:
            allowed_record = item is not None and _record_allowed(record, item, target)
        if (
            record.get("target") != target
            and not (record.get("operation") == "semantic" and allowed_record)
        ) or not allowed_record:
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
    journal_root = expected_namespace.path
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
    derived = json.loads((path.parent / "derived.json").read_bytes())
    trusted_facts = derived.get("facts")
    if (
        not isinstance(trusted_facts, list)
        or _canonical_json_digest(trusted_facts) != capability["fact_digest"]
        or set(row.get("rungs", {})) != set(capability["rungs"])
    ):
        raise ValueError("typed capability facts or required rungs differ")
    _validate_typed_rungs(
        root,
        row,
        payload,
        report_digest=report_digest,
        target=target,
        row_selector=selector,
        journal_entry=entry,
        trusted_facts=trusted_facts,
    )
    provenance = {
        "kind": "typed-capability",
        "evidence": evidence,
        "evidence_sha256": digest,
        "initializer_report_sha256": report_digest,
        "evidence_namespace": namespace_provenance,
    }
    provenance["conclusion_sha256"] = _canonical_json_digest(row)
    return provenance
