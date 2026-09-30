"""Derive typed naming facts only from runner-validated indexed records."""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path
from typing import Any

from harness.domain.ids import parse_function_id
from harness.naming.debt import address_of
from harness.naming.capabilities import (
    PRODUCTION_EXACT_CAPABILITIES,
    ExactCapabilityRegistry,
)
from harness.naming.consumers import decode_table_consumer
from harness.naming.description import _describe, _validated_row

SCHEMA = "bof3.naming-analyzer-facts/v3"
CONCLUSION_CAPABILITY_SCHEMA = "bof3.naming-conclusion-capability/v1"
_CAPABILITY_MISSING_FACT = (
    "a second independent target-local initializer or consumer that independently "
    "proves the same dispatch role and any additional proposed name term"
)
_RUNNER_TOKEN = object()
_REPOSITORY_ROOT = Path(__file__).resolve().parents[4]
_WIDTHS = {"lb": 1, "lbu": 1, "sb": 1, "lh": 2, "lhu": 2, "sh": 2, "lw": 4, "sw": 4}
_LOAD_OPS = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr"}
_STORE_OPS = {"sb", "sh", "sw", "swl", "swr"}
_ADDRESS_OPS = {"addiu", "addi", "addu", "add", "lui", "ori"}
_ACCESS_OPS = {"address": _ADDRESS_OPS, "load": _LOAD_OPS, "store": _STORE_OPS}
_MNEMONIC = re.compile(r"[a-z][a-z0-9]*")


def _opcode(value: object, access_kind: object) -> str | None:
    """Return one exact indexed MIPS mnemonic, never a spelling lookalike."""

    if (
        not isinstance(value, str)
        or _MNEMONIC.fullmatch(value) is None
        or not isinstance(access_kind, str)
        or access_kind not in _ACCESS_OPS
        or value not in _ACCESS_OPS[access_kind]
    ):
        return None
    return value


def _digest(value: object) -> str:
    encoded = json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(encoded).hexdigest()


def _source(family: str, target: str, value: object, input_digest: str) -> str:
    return f"{family}:{_digest([target, value, input_digest])}"


def _same_function(value: object, expected: object) -> bool:
    if not isinstance(value, str):
        return False
    try:
        return parse_function_id(value) == expected
    except ValueError:
        return False


def _analyze_validated_records(
    token: object,
    *,
    target: str,
    report_digest: str,
    row: dict[str, Any],
    operations: tuple[dict[str, Any], ...],
    registry: ExactCapabilityRegistry = PRODUCTION_EXACT_CAPABILITIES,
    instructions: tuple[dict[str, Any], ...] = (),
) -> dict[str, Any]:
    """Analyze records from the canonical runner boundary."""
    if (
        token is not _RUNNER_TOKEN
        or len(report_digest) != 64
        or not _REPOSITORY_ROOT.is_dir()
    ):
        raise ValueError("analyzer input is not bound to the repository/report digest")
    selector = f"{row.get('kind')}:{row.get('name')}"
    address = address_of(str(row["name"]))
    external_owners: list[dict[str, Any]] = []
    for item in operations:
        try:
            operation_selector = parse_function_id(str(item.get("selector", "")))
        except ValueError as error:
            raise ValueError("analyzer operation target mismatch") from error
        item_id = str(item.get("item", {}).get("id", ""))
        try:
            owner_selector = (
                parse_function_id(item_id.split(":", 1)[1])
                if item_id.startswith("owner:")
                else None
            )
        except ValueError:
            owner_selector = None
        external_owner = (
            item.get("operation") == "owner"
            and item.get("supplemental") is False
            and owner_selector == operation_selector
            and row.get("outside_payload") is True
        )
        if operation_selector.target.value != target and not external_owner:
            raise ValueError("analyzer operation target mismatch")
        if external_owner:
            payload = item.get("payload")
            if not isinstance(payload, list) or any(
                not isinstance(owner, dict)
                or owner.get("target_id") != operation_selector.target.value
                for owner in payload
            ):
                raise ValueError("external owner payload target mismatch")
            external_owners.append(item)
        if not isinstance(item.get("supplemental"), bool):
            raise ValueError("analyzer operation provenance is invalid")
    normalized_operations = [
        {
            "id": str(item.get("item", {}).get("id")),
            "operation": item.get("operation"),
            "selector": item.get("selector"),
            "payload": item.get("payload"),
            "supplemental": item.get("supplemental"),
            "role": item.get("role"),
        }
        for item in operations
    ]
    input_digest = _digest(
        {
            "target": target,
            "report_digest": report_digest,
            "row": selector,
            "operations": normalized_operations,
            "analyzer": SCHEMA,
            **({"instructions": instructions} if instructions else {}),
        }
    )
    facts: list[dict[str, Any]] = []
    opened: list[dict[str, str]] = [
        {
            "class": "owner_resolution",
            "reason": "resolution remains owned by the external target report",
        }
        for _item in external_owners
    ]
    unavailable: list[dict[str, str]] = [
        {
            "operation": "owner",
            "reason": "target-qualified external-owner evidence cannot establish ownership in the report target",
        }
        for _item in external_owners
    ]
    bounds: dict[str, tuple[int, int]] = {}
    storage = None
    for item in operations:
        if item.get("operation") != "describe":
            continue
        try:
            role = item.get("role")
            if role not in {"selected_data_describe", "access_function_describe"}:
                raise ValueError("describe supplemental role is invalid")
            selected_role = role == "selected_data_describe"
            start, end, described_storage = _describe(
                item,
                target,
                expected_name=str(row["name"]) if selected_role else None,
                expected_kind=str(row["kind"]) if selected_role else "function",
            )
            if not selected_role:
                bounds[str(item["selector"])] = (start, end)
            if selected_role:
                storage = described_storage
                value = {
                    "address": f"0x{address:08X}",
                    "boundary": {"start": f"0x{start:08X}", "end": f"0x{end:08X}"},
                }
                facts.append(
                    {
                        "class": "selected_range",
                        "polarity": "positive",
                        "value": value,
                        "source_id": _source(
                            "reviewed-layout", target, value, input_digest
                        ),
                    }
                )
        except ValueError as error:
            unavailable.append({"operation": "describe", "reason": str(error)})
    if not any(fact["class"] == "selected_range" for fact in facts):
        opened.append(
            {"class": "selected_range", "reason": "validated describe unavailable"}
        )
    if row.get("kind") == "data" and storage is not None:
        facts.append(
            {
                "class": "storage_class",
                "polarity": "positive",
                "value": storage,
                "source_id": _source("original-image", target, storage, input_digest),
            }
        )
    else:
        opened.append(
            {"class": "storage_class", "reason": "canonical storage unavailable"}
        )
    access_facts: list[dict[str, Any]] = []
    access_seen = False
    for item in operations:
        if item.get("operation") != "access":
            continue
        if item.get("role") is not None:
            raise ValueError("required access operation has supplemental role")
        access_seen = True
        function = parse_function_id(str(item["selector"]))
        function_id = str(function)
        function_bounds = next(
            (
                value
                for selector, value in bounds.items()
                if parse_function_id(selector) == function
            ),
            None,
        )
        for access in _validated_row(
            item, {"source", "address", "function_id", "access_kind", "opcode"}
        ):
            source, referenced = access["source"], access["address"]
            if (
                not _same_function(access["function_id"], function)
                or function_bounds is None
                or not isinstance(source, int)
                or isinstance(source, bool)
                or source < 0
                or not function_bounds[0] <= source < function_bounds[1]
                or not isinstance(referenced, int)
                or isinstance(referenced, bool)
                or _opcode(access["opcode"], access["access_kind"]) is None
            ):
                unavailable.append(
                    {
                        "operation": "access",
                        "reason": "access row failed domain validation",
                    }
                )
                continue
            # Full-function queries include other objects; valid context is not
            # selected-object evidence and must not prevent its conclusion.
            if referenced != address:
                continue
            opcode = _opcode(access["opcode"], access["access_kind"])
            assert opcode is not None
            value = {
                "site": f"0x{source:08X}",
                "function": function_id,
                "address": f"0x{address:08X}",
                "kind": access["access_kind"],
                "opcode": opcode,
            }
            if opcode in _WIDTHS:
                value["width"] = _WIDTHS[opcode]
            access_facts.append(value)
    for value in sorted(access_facts, key=lambda item: item["site"]):
        facts.append(
            {
                "class": "selected_access",
                "polarity": "positive",
                "value": value,
                "source_id": _source(
                    "reverse-index:data_references", target, value, input_digest
                ),
            }
        )
    if not access_facts:
        opened.append(
            {
                "class": "selected_access",
                "reason": "complete negative coverage not proven"
                if access_seen
                else "access query unavailable",
            }
        )
    registry_entries = [
        entry
        for (spec_row, _consumer), entry in registry.items()
        if spec_row == selector and entry.consumer.target == target
    ]
    registry_entry = registry_entries[0] if len(registry_entries) == 1 else None
    consumer_facts: list[dict[str, Any]] = []
    for item in operations:
        if item.get("operation") != "data_dispatch_consumer":
            continue
        if (
            item.get("role") != "data_dispatch_consumer"
            or item.get("supplemental") is not True
        ):
            raise ValueError("dispatch consumer operation provenance is invalid")
        try:
            key = (selector, str(item["selector"]).lower())
            entry = registry.get(key)
            if entry is None or entry is not registry_entry:
                raise ValueError("table consumer is not an independently reviewed spec")
            consumer_facts.extend(
                decode_table_consumer(item, entry.consumer, input_digest)
            )
        except ValueError as error:
            unavailable.append(
                {"operation": "data_dispatch_consumer", "reason": str(error)}
            )
    facts.extend(consumer_facts)
    if not any(fact["class"] == "one_level_beyond" for fact in consumer_facts):
        opened.append(
            {
                "class": "one_level_beyond",
                "reason": "consumer semantics require reviewed original-byte decoding",
            }
        )
    if row.get("kind") == "function":
        seen = set()
        for capture in instructions:
            observations = instruction_observations(_RUNNER_TOKEN, target, row, capture)
            for observation in observations["observations"]:
                seen.add(observation["class"])
                facts.append(
                    {
                        **observation,
                        "source_id": _source(
                            "original-instructions",
                            target,
                            observation["value"],
                            input_digest,
                        ),
                    }
                )
            opened.extend(observations["open"])
        for fact_class in ("selected_call", "owner_body"):
            if fact_class not in seen:
                opened.append(
                    {
                        "class": fact_class,
                        "reason": f"capability gap: {fact_class}: no supported original body captured; use --instructions; only canonical straight-line call wrappers are supported",
                    }
                )
    result = {
        "schema": SCHEMA,
        "target": target,
        "row": selector,
        "report_digest": report_digest,
        "input_digest": input_digest,
        "facts": facts,
        "open": opened,
        "unavailable": unavailable,
        "conclusion_enabled": False,
    }
    if registry_entry is not None and not opened and not unavailable:
        by_class = {
            name: sorted({fact["source_id"] for fact in facts if fact["class"] == name})
            for name in (
                "selected_range",
                "selected_access",
                "storage_class",
                "one_level_beyond",
            )
        }
        if all(by_class.values()):
            result["conclusion_enabled"] = True
            result["conclusion_capability"] = {
                "schema": CONCLUSION_CAPABILITY_SCHEMA,
                "target": target,
                "row": selector,
                "allowed_conclusion": "exhausted",
                "analyzer_schema": SCHEMA,
                "input_digest": input_digest,
                "fact_digest": _digest(facts),
                "rungs": by_class,
                "required_work": [registry_entry.required_work],
                "proposal_allowed": False,
                "missing_fact": _CAPABILITY_MISSING_FACT,
            }
    return result


def selected_call_observations(
    token: object, target: str, row: dict, capture: dict
) -> tuple[list[dict], list[str]]:
    """Produce observations only at the runner's original-instruction boundary."""
    from harness.naming.calls import decode_selected_call

    if token is not _RUNNER_TOKEN:
        raise ValueError("selected_call requires the native runner")
    function = parse_function_id(capture["selector"])
    if function.target.value != target:
        raise ValueError("selected_call target mismatch")
    selected = address_of(str(row["name"]))
    try:
        value = decode_selected_call(
            bytes.fromhex(capture["bytes"]), capture["start"], selected
        )
    except ValueError as error:
        return [], [f"{function}: {error}"]
    return [{"caller": str(function), "binding": capture["binding"], **value}], []


def instruction_observations(
    token: object, target: str, row: dict, capture: dict
) -> dict:
    """Share receipt production and replay at the native original-byte boundary."""
    from harness.naming.calls import decode_owner_body

    if token is not _RUNNER_TOKEN:
        raise ValueError("instruction observations require the native runner")
    function = parse_function_id(capture["selector"])
    from harness.naming.plan import owner_instruction_selectors

    owners = owner_instruction_selectors(row)
    if function.target.value != target and str(function) not in owners:
        raise ValueError("instruction capture is not a planned owner selector")
    observations, opened = [], []
    if function.target.value == target:
        values, reasons = selected_call_observations(token, target, row, capture)
        observations.extend(
            {"class": "selected_call", "polarity": "positive", "value": value}
            for value in values
        )
        opened.extend(
            {"class": "selected_call", "reason": reason} for reason in reasons
        )
    if str(function) in owners:
        try:
            value = decode_owner_body(bytes.fromhex(capture["bytes"]), capture["start"])
        except ValueError as error:
            opened.append(
                {
                    "class": "owner_body",
                    "reason": f"capability gap: owner_body: {function}: {error}",
                }
            )
        else:
            observations.append(
                {
                    "class": "owner_body",
                    "polarity": "positive",
                    "value": {
                        "owner": str(function),
                        "binding": capture["binding"],
                        **value,
                    },
                }
            )
    return {
        "schema": "bof3.naming-evidence-facts/v1",
        "facts": sorted({observation["class"] for observation in observations}),
        "observations": observations,
        "open": opened,
    }
