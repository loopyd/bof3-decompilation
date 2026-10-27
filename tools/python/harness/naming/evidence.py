"""Derive typed naming facts only from runner-validated indexed records."""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path
from typing import Any

from harness.domain.ids import parse_function_id
from harness.naming.debt import address_of
from harness.naming.bytelookup import (
    byte_lookup_gaps,
    capture_byte_lookup_access,
    capture_byte_lookup_context,
    decode_byte_lookup_access,
    decode_byte_lookup_context,
)
from harness.naming.consumers import capture_dispatch_consumer, decode_dispatch_consumer
from harness.naming.description import (
    _describe,
    _validated_row,
    capture_data_range,
    data_range_value,
)
from harness.naming.facts import RUNG_PROFILES

SCHEMA = "bof3.naming-analyzer-facts/v3"
CONCLUSION_CAPABILITY_SCHEMA = "bof3.naming-conclusion-capability/v1"
_RUNNER_TOKEN = object()
_REPOSITORY_ROOT = Path(__file__).resolve().parents[4]
_WIDTHS = {"lb": 1, "lbu": 1, "sb": 1, "lh": 2, "lhu": 2, "sh": 2, "lw": 4, "sw": 4}
_LOAD_OPS = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr"}
_STORE_OPS = {"sb", "sh", "sw", "swl", "swr"}
_ADDRESS_OPS = {"addiu", "addi", "addu", "add", "lui", "ori"}
_ACCESS_OPS = {"address": _ADDRESS_OPS, "load": _LOAD_OPS, "store": _STORE_OPS}
_MNEMONIC = re.compile(r"[a-z][a-z0-9]*")

_BYTE_LOOKUP_CAPTURES = {
    "data_byte_lookup_access": capture_byte_lookup_access,
    "data_byte_lookup_context": capture_byte_lookup_context,
}
_BYTE_LOOKUP_DECODERS = {
    "data_byte_lookup_access": decode_byte_lookup_access,
    "data_byte_lookup_context": decode_byte_lookup_context,
}


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


def positive_rung_sources(row: dict, facts: list[dict]) -> dict[str, list[str]]:
    """Inventory positive profile facts, without claiming receipt independence."""
    required = RUNG_PROFILES[row["kind"], row.get("outside_payload") is True]
    if row.get("partial_used") is True:
        required = required | {"partial_baseline"}
    return {
        name: sorted(
            {
                fact["source_id"]
                for fact in facts
                if fact["class"] == name and fact["polarity"] == "positive"
            }
        )
        for name in sorted(required)
    }


def recompute_collected_records(
    token: object,
    *,
    root: Path,
    target: str,
    report_digest: str,
    row: dict,
    payload: dict,
    native_ops=None,
) -> dict:
    """Replay indexed records and live instruction bytes behind caller-owned guards."""
    from harness.naming.instructions import analyzer_instructions

    if (
        not isinstance(payload, dict)
        or payload.get("target") != target
        or payload.get("row") != f"{row.get('kind')}:{row.get('name')}"
    ):
        raise ValueError("collected evidence target/row mismatch")
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
    from harness.naming.plan import collection_row

    planned_row = collection_row(row, target, root=root)
    from harness.naming.partial import replay_partial

    derived = _analyze_validated_records(
        token,
        target=target,
        report_digest=report_digest,
        row=planned_row,
        operations=operations,
        instructions=analyzer_instructions(root, payload["semantic"]),
        partial=replay_partial(
            root, planned_row, target, payload, native_ops=native_ops
        ),
        partial_gap=next(
            (
                item["partial_gap"]
                for item in payload["semantic"]
                if "partial_gap" in item
            ),
            None,
        ),
    )
    if row.get("kind") == "data" or any(
        item["operation"] == "function_predicate_context" for item in operations
    ):
        _validate_indexed_receipts(root, planned_row, payload, operations, derived)
    return derived


def indexed_observations(
    token: object,
    row: dict,
    operation: dict,
    derived: dict,
) -> dict:
    """Project facts onto the indexed operation that actually produced them."""
    if token is not _RUNNER_TOKEN:
        raise ValueError("indexed observations require the native runner")
    target, input_digest = derived["target"], derived["input_digest"]
    candidates = []
    opened = []
    kind, role = operation["operation"], operation.get("role")
    if kind == "data_original_range" and role == kind:
        try:
            value = data_range_value(operation, target, address_of(row["name"]))
            candidates.append(
                {
                    "class": "selected_range",
                    "polarity": "positive",
                    "value": value,
                    "source_id": _source(
                        "original-image:data-range", target, value, input_digest
                    ),
                }
            )
        except ValueError as error:
            opened.append({"class": "selected_range", "reason": str(error)})
    elif kind == "describe" and role == "selected_data_describe":
        _start, _end, storage = _describe(
            operation, target, expected_name=row["name"], expected_kind="data"
        )
        if storage is not None:
            candidates.append(
                {
                    "class": "storage_class",
                    "polarity": "positive",
                    "value": storage,
                    "source_id": _source(
                        "original-image", target, storage, input_digest
                    ),
                }
            )
    elif kind == "access" and role is None:
        sites = {access["source"] for access in operation["payload"]}
        candidates = [
            fact
            for fact in derived["facts"]
            if fact["class"] == "selected_access"
            and fact["value"].get("function") == operation["selector"]
            and int(fact["value"]["site"], 16) in sites
        ]
    elif kind == "data_dispatch_consumer" and role == kind:
        selected_range = next(
            (
                fact["value"]
                for fact in derived["facts"]
                if fact["class"] == "selected_range"
            ),
            {},
        )
        try:
            candidates = decode_dispatch_consumer(
                operation, target, selected_range, input_digest
            )
        except ValueError as error:
            opened.append(
                {"class": "one_level_beyond", "reason": f"capability gap: {error}"}
            )
    elif (
        kind in {"data_byte_lookup_access", "data_byte_lookup_context"} and role == kind
    ):
        fact_class = (
            "selected_access"
            if kind == "data_byte_lookup_access"
            else "one_level_beyond"
        )
        selected_range = next(
            (
                fact["value"]
                for fact in derived["facts"]
                if fact["class"] == "selected_range"
            ),
            {},
        )
        try:
            candidates = _BYTE_LOOKUP_DECODERS[kind](
                operation, target, selected_range, input_digest
            )
        except ValueError as error:
            opened.append({"class": fact_class, "reason": f"capability gap: {error}"})
        else:
            opened.extend(byte_lookup_gaps(operation["payload"], fact_class))
    elif kind == "function_predicate_context" and role == kind:
        from harness.naming.predicates import predicate_context

        try:
            value = predicate_context(operation, target, str(row["name"]))
            candidates = [
                {
                    "class": "one_level_beyond",
                    "polarity": "positive",
                    "value": value,
                    "source_id": _source(
                        "original-image:predicate-context", target, value, input_digest
                    ),
                }
            ]
        except ValueError as error:
            opened.append(
                {"class": "one_level_beyond", "reason": f"capability gap: {error}"}
            )
    trusted = {_digest(fact) for fact in derived["facts"]}
    facts = [
        {key: fact[key] for key in ("class", "polarity", "value")}
        for fact in candidates
        if _digest(fact) in trusted
    ]
    return {
        "schema": "bof3.naming-evidence-facts/v1",
        "facts": sorted({fact["class"] for fact in facts}),
        "observations": facts,
        "open": opened,
    }


def _validate_indexed_receipts(root, row, payload, operations, derived):
    """Require the complete current receipt set and its recomputed projections."""
    from harness.naming.plan import indexed_operation_plan

    planned = indexed_operation_plan(row, derived["target"], root=root)
    if len(planned) != len(operations) or any(
        expected["id"] != actual["item"]["id"]
        or any(
            expected.get(key) != actual.get(key)
            for key in ("operation", "selector", "role", "supplemental")
        )
        for expected, actual in zip(planned, operations, strict=True)
    ):
        raise ValueError("indexed data operations differ from the current plan")
    commands = [
        record
        for record in payload["commands"]
        if record.get("operation") != "semantic"
    ]
    if len(commands) != len(operations):
        raise ValueError(
            "data receipts do not cover every executed operation exactly once"
        )
    for operation, expected_operation in zip(operations, planned, strict=True):
        matches = [
            record
            for record in commands
            if record.get("item") == operation["item"]["id"]
        ]
        if len(matches) != 1:
            raise ValueError("data receipt operation is missing or duplicated")
        command = matches[0]
        if any(
            command.get(key) != operation.get(key)
            for key in ("operation", "selector", "role", "supplemental")
        ):
            raise ValueError("typed indexed receipt operation differs from its capture")
        if (
            command.get("command") != expected_operation["command"]
            or command.get("status") != "passed"
            or command.get("target")
            != parse_function_id(operation["selector"]).target.value
        ):
            raise ValueError(
                "typed indexed command differs from its executed operation"
            )
        if operation["operation"] == "data_dispatch_consumer":
            _validate_consumer_bytes(root, derived["target"], operation["payload"])
        if operation["operation"] in _BYTE_LOOKUP_DECODERS:
            _validate_byte_lookup_bytes(
                root, operation["operation"], operation["payload"]
            )
        if operation["operation"] == "function_predicate_context":
            from harness.naming.consumers import capture_reviewed_consumer

            current = capture_reviewed_consumer(
                root, operation["selector"], operation["selector"]
            )
            if _digest(current) != _digest(operation["payload"]):
                raise ValueError(
                    "predicate context differs from current original range"
                )
        if operation["operation"] == "data_original_range":
            current = capture_data_range(root, operation["selector"])
            if _digest(operation["payload"]) != _digest(current):
                raise ValueError(
                    "data range capture differs from current original range"
                )
        receipt = json.loads((root / command["receipt"]).read_bytes())
        if row.get("kind") == "function" and operation["operation"] == "calls":
            if any(
                receipt.get(key) != command.get(key)
                for key in ("command", "status", "target", "selector", "output")
            ):
                raise ValueError(
                    "caller query receipt differs from its captured operation"
                )
            continue
        expected = indexed_observations(_RUNNER_TOKEN, row, operation, derived)
        if any(
            _digest(receipt.get(key)) != _digest(value)
            for key, value in expected.items()
        ) or any(
            receipt.get(key) != command.get(key)
            for key in ("command", "status", "target", "selector", "output")
        ):
            raise ValueError(
                "typed indexed receipt observations differ from replayed facts"
            )


def _validate_byte_lookup_bytes(root: Path, operation: str, payload: dict) -> None:
    """Re-read the original image for this receipt's own captured source."""

    current = _BYTE_LOOKUP_CAPTURES[operation](root, payload["selected"])
    if _digest(current) != _digest(payload):
        raise ValueError(
            "byte lookup receipt differs from current original bytes or range"
        )


def _validate_consumer_bytes(root: Path, target: str, payload: dict) -> None:
    """Re-read the actual original image, not its recorded hash or copied bytes."""
    current = capture_dispatch_consumer(root, payload["selected"], payload["consumer"])
    if _digest(current) != _digest(payload):
        raise ValueError(
            "data consumer differs from current original bytes or reviewed range"
        )


def _analyze_validated_records(
    token: object,
    *,
    target: str,
    report_digest: str,
    row: dict[str, Any],
    operations: tuple[dict[str, Any], ...],
    instructions: tuple[dict[str, Any], ...] = (),
    partial: tuple[dict[str, Any], ...] = (),
    partial_gap: str | None = None,
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
        from harness.naming.crosscalls import generated_callers

        external_caller = (
            item.get("operation") == "calls"
            and item.get("supplemental") is False
            and item_id.startswith("caller:")
            and str(operation_selector) in generated_callers(row, target)
            and item_id.split(":", 1)[1] == str(operation_selector)
        )
        if operation_selector.target.value != target and not (
            external_owner or external_caller
        ):
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
            **({"partial": partial} if partial else {}),
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
        except ValueError as error:
            unavailable.append({"operation": "describe", "reason": str(error)})
    for item in operations:
        if item.get("operation") != "data_original_range":
            continue
        if (
            item.get("role") != "data_original_range"
            or item.get("supplemental") is not True
            or row.get("kind") != "data"
        ):
            raise ValueError("data range operation provenance is invalid")
        try:
            value = data_range_value(item, target, address)
            facts.append(
                {
                    "class": "selected_range",
                    "polarity": "positive",
                    "value": value,
                    "source_id": _source(
                        "original-image:data-range", target, value, input_digest
                    ),
                }
            )
        except ValueError as error:
            opened.append({"class": "selected_range", "reason": str(error)})
            unavailable.append(
                {"operation": "data_original_range", "reason": str(error)}
            )
    if not any(fact["class"] == "selected_range" for fact in facts):
        opened.append(
            {
                "class": "selected_range",
                "reason": "validated original range unavailable",
            }
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
    for item in operations:
        operation = item.get("operation")
        if operation not in _BYTE_LOOKUP_CAPTURES:
            continue
        if item.get("role") != operation or item.get("supplemental") is not True:
            raise ValueError("byte lookup operation provenance is invalid")
        fact_class = (
            "selected_access"
            if operation == "data_byte_lookup_access"
            else "one_level_beyond"
        )
        try:
            selected_range = next(
                (fact["value"] for fact in facts if fact["class"] == "selected_range"),
                {},
            )
            facts.extend(
                _BYTE_LOOKUP_DECODERS[operation](
                    item, target, selected_range, input_digest
                )
            )
        except ValueError as error:
            opened.append({"class": fact_class, "reason": f"capability gap: {error}"})
        else:
            opened.extend(byte_lookup_gaps(item["payload"], fact_class))
    if not any(fact["class"] == "selected_access" for fact in facts):
        opened.append(
            {
                "class": "selected_access",
                "reason": "complete negative coverage not proven"
                if access_seen
                else "access query unavailable",
            }
        )
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
            selected_range = next(
                (fact["value"] for fact in facts if fact["class"] == "selected_range"),
                {},
            )
            consumer_facts.extend(
                decode_dispatch_consumer(item, target, selected_range, input_digest)
            )
        except ValueError as error:
            unavailable.append(
                {
                    "operation": "data_dispatch_consumer",
                    "reason": f"capability gap: {error}",
                }
            )
    facts.extend(consumer_facts)
    for item in operations:
        if item.get("operation") != "function_predicate_context":
            continue
        from harness.naming.predicates import predicate_context

        try:
            value = predicate_context(item, target, str(row["name"]))
        except ValueError as error:
            opened.append(
                {"class": "one_level_beyond", "reason": f"capability gap: {error}"}
            )
        else:
            facts.append(
                {
                    "class": "one_level_beyond",
                    "polarity": "positive",
                    "value": value,
                    "source_id": _source(
                        "original-image:predicate-context", target, value, input_digest
                    ),
                }
            )
    if not any(fact["class"] == "one_level_beyond" for fact in facts):
        opened.append(
            {
                "class": "one_level_beyond",
                "reason": "consumer semantics require reviewed original-byte decoding",
            }
        )
    if row.get("kind") == "function":
        seen = set()
        capture_gaps = []
        for capture in instructions:
            observations = instruction_observations(_RUNNER_TOKEN, target, row, capture)
            for observation in observations["observations"]:
                seen.add(observation["class"])
                facts.append(
                    {
                        **observation,
                        "source_id": _source(
                            {
                                "selected_range": "original-instructions:selected-range",
                                "one_level_beyond": "original-instructions:caller-state-dispatch",
                            }.get(observation["class"], "original-instructions"),
                            target,
                            observation["value"],
                            input_digest,
                        ),
                    }
                )
            capture_gaps.extend(observations["open"])
        required = RUNG_PROFILES["function", row.get("outside_payload") is True]
        opened = [
            gap
            for gap in opened
            if gap["class"] in required and gap["class"] not in seen
        ]
        opened.extend(gap for gap in capture_gaps if gap["class"] in required)
    for observation in partial:
        facts.append(
            {
                **observation,
                "source_id": _source(
                    "native-partial-baseline",
                    target,
                    observation["value"],
                    input_digest,
                ),
            }
        )
    if partial_gap is not None and not partial:
        opened.append({"class": "partial_baseline", "reason": partial_gap})
    required = set(RUNG_PROFILES[row["kind"], row.get("outside_payload") is True])
    if row.get("partial_used") is True:
        required.add("partial_baseline")
    positive = {fact["class"] for fact in facts if fact["polarity"] == "positive"}
    for fact_class in sorted(required - positive):
        if any(gap["class"] == fact_class for gap in opened):
            continue
        reason = (
            "positive producer unavailable"
            if fact_class in {"owner_resolution", "owner_data", "partial_baseline"}
            else "no supported runner observation captured"
        )
        opened.append({"class": fact_class, "reason": reason})
    opened = [
        {
            **gap,
            "reason": gap["reason"]
            if gap["reason"].startswith("capability gap:")
            else f"capability gap: {gap['class']}: {gap['reason']}",
        }
        for gap in opened
    ]
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
    if not opened and not unavailable:
        by_class = positive_rung_sources(row, facts)
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
                "required_work": sorted(
                    item["id"]
                    for item in normalized_operations
                    if not item["supplemental"]
                ),
                "proposal_allowed": True,
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
    from harness.naming.crosscalls import generated_callers

    if str(function) not in generated_callers(row, target):
        raise ValueError("selected_call capture is not generated caller work")
    selected = address_of(str(row["name"]))
    try:
        if function.target.value != target:
            from harness.naming.callereffects import decode_caller_body

            value = decode_caller_body(
                bytes.fromhex(capture["bytes"]), capture["start"], selected
            )
        else:
            value = decode_selected_call(
                bytes.fromhex(capture["bytes"]), capture["start"], selected
            )
    except ValueError as error:
        return [], [f"capability gap: selected_call: {function}: {error}"]
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
    from harness.naming.crosscalls import generated_callers

    callers = generated_callers(row, target)
    if function.target.value != target and str(function) not in owners | callers:
        raise ValueError(
            "instruction capture is not a planned owner or caller selector"
        )
    observations, opened = [], []
    selected = function.target.value == target and function.address == address_of(
        str(row["name"])
    )
    if selected:
        observations.append(_selected_range(capture, str(function)))
    if str(function) in callers:
        values, reasons = selected_call_observations(token, target, row, capture)
        observations.extend(
            {"class": "selected_call", "polarity": "positive", "value": value}
            for value in values
        )
        opened.extend(
            {"class": "selected_call", "reason": reason} for reason in reasons
        )
        for value in values:
            context = _caller_state_dispatch(value)
            if context is not None:
                observations.append(
                    {
                        "class": "one_level_beyond",
                        "polarity": "positive",
                        "value": context,
                    }
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


def _selected_range(capture: dict, selector: str) -> dict:
    """Describe the live range and direct effects only when the full body decodes."""
    from harness.naming.effects import decode_leaf_effects

    binding = capture["binding"]
    original = bytes.fromhex(capture["bytes"])
    start, end = binding.get("start"), binding.get("end")
    if (
        binding.get("selector") != selector
        or type(start) is not int
        or type(end) is not int
        or start != parse_function_id(selector).address
        or capture["start"] != start
        or start % 4
        or not 0 < len(original) <= 65536
        or len(original) % 4
        or end - start != len(original)
    ):
        raise ValueError(
            "selected_range capture differs from its complete original binding"
        )
    value = {
        "selector": selector,
        "boundary": {"start": f"0x{start:08X}", "end": f"0x{end:08X}"},
        "byte_count": len(original),
        "original_sha256": hashlib.sha256(original).hexdigest(),
        "binding": binding,
        "scope": "complete reviewed original range; no C return contract",
    }
    try:
        value["body_effects"] = decode_leaf_effects(original, start)
    except ValueError as error:
        value["body_effects_gap"] = str(error)
    return {
        "class": "selected_range",
        "polarity": "positive",
        "value": value,
    }


def _caller_state_dispatch(value: dict) -> dict | None:
    """Project a decoded two-way state dispatch with an identical later call suffix."""
    if value.get("kind") == "complete_static_caller_body" or "transition" not in value:
        return None
    transition = value["transition"]
    paths = transition.get("paths", [])
    if len(paths) != 2 or any(
        len(path["guards"]) != 1 or len(path["calls"]) < 2 for path in paths
    ):
        return None
    first, second = (path["guards"][0] for path in paths)
    state = first["left"]
    if (
        {first["relation"], second["relation"]} != {"equal", "not_equal"}
        or {key: item for key, item in first.items() if key != "relation"}
        != {key: item for key, item in second.items() if key != "relation"}
        or state.get("kind") != "unsigned_halfword_read"
        or first["right"] != {"kind": "constant", "value": 0}
        or paths[0]["calls"][1:] != paths[1]["calls"][1:]
        or paths[0]["calls"][0]["callee"] == paths[1]["calls"][0]["callee"]
        or {path["calls"][0]["callee"] for path in paths}.isdisjoint({value["callee"]})
    ):
        return None
    return {
        "kind": "caller_state_dispatch",
        "caller": value["caller"],
        "selected": value["callee"],
        "binding": value["binding"],
        "state_read": {
            **state,
            "instruction": next(
                instruction
                for instruction in transition["coverage"]["instructions"]
                if instruction["address"] == state["instruction"]
            ),
        },
        "paths": paths,
        "common_later_calls": paths[0]["calls"][1:],
        "transition": transition,
        "scope": "conditional invocation and common continuation only; path feasibility, state meaning and transitive callee effects are not established",
    }
