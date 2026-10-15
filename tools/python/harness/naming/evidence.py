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
from harness.naming.facts import RUNG_PROFILES

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
    registry: ExactCapabilityRegistry = PRODUCTION_EXACT_CAPABILITIES,
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

    planned_row = collection_row(row, target, root=root, registry=registry)
    derived = _analyze_validated_records(
        token,
        target=target,
        report_digest=report_digest,
        row=planned_row,
        operations=operations,
        registry=registry,
        instructions=analyzer_instructions(root, payload["semantic"]),
    )
    if row.get("kind") == "data":
        _validate_indexed_receipts(
            root, planned_row, payload, operations, derived, registry=registry
        )
    return derived


def indexed_observations(
    token: object,
    row: dict,
    operation: dict,
    derived: dict,
    *,
    registry: ExactCapabilityRegistry = PRODUCTION_EXACT_CAPABILITIES,
) -> dict:
    """Project facts onto the indexed operation that actually produced them."""
    if token is not _RUNNER_TOKEN:
        raise ValueError("indexed observations require the native runner")
    target, input_digest = derived["target"], derived["input_digest"]
    candidates = []
    kind, role = operation["operation"], operation.get("role")
    if kind == "describe" and role == "selected_data_describe":
        start, end, storage = _describe(
            operation, target, expected_name=row["name"], expected_kind="data"
        )
        value = {
            "address": f"0x{address_of(row['name']):08X}",
            "boundary": {"start": f"0x{start:08X}", "end": f"0x{end:08X}"},
        }
        for fact_class, family, fact_value in (
            ("selected_range", "reviewed-layout", value),
            ("storage_class", "original-image", storage),
        ):
            if fact_value is not None:
                candidates.append(
                    {
                        "class": fact_class,
                        "polarity": "positive",
                        "value": fact_value,
                        "source_id": _source(family, target, fact_value, input_digest),
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
        entry = registry.get((f"data:{row['name']}", operation["selector"].lower()))
        if entry is not None:
            try:
                candidates = decode_table_consumer(
                    operation, entry.consumer, input_digest
                )
            except ValueError:
                pass
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
    }


def _validate_indexed_receipts(root, row, payload, operations, derived, *, registry):
    """Require the complete current receipt set and its recomputed projections."""
    from harness.naming.plan import indexed_operation_plan

    planned = indexed_operation_plan(row, derived["target"], registry=registry)
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
            or command.get("target") != derived["target"]
        ):
            raise ValueError(
                "typed indexed command differs from its executed operation"
            )
        if operation["operation"] == "data_dispatch_consumer":
            _validate_consumer_bytes(root, derived["target"], operation["payload"])
        receipt = json.loads((root / command["receipt"]).read_bytes())
        expected = indexed_observations(
            _RUNNER_TOKEN, row, operation, derived, registry=registry
        )
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


def _validate_consumer_bytes(root: Path, target: str, payload: dict) -> None:
    """Re-read the actual original image, not its recorded hash or copied bytes."""
    from harness.analysis.project import prepare_target

    spec = prepare_target(root, target)
    image = spec.binary.read_bytes()
    if hashlib.sha256(image).hexdigest() != payload["image_sha256"]:
        raise ValueError("data consumer original image is stale")
    for kind in ("selected", "consumer"):
        start, end = payload[f"{kind}_range"]
        offset = spec.binary_offset + start - spec.load_address
        if (
            type(start) is not int
            or type(end) is not int
            or start >= end
            or offset < spec.binary_offset
            or offset + end - start > len(image)
            or image[offset : offset + end - start].hex() != payload[f"{kind}_bytes"]
        ):
            raise ValueError(
                "data consumer capture differs from current original bytes"
            )


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
        opened = [
            {
                **gap,
                "reason": gap["reason"]
                if gap["reason"].startswith("capability gap:")
                else f"capability gap: {gap['class']}: {gap['reason']}",
            }
            for gap in opened
        ]
        for fact_class in sorted(required - seen):
            if not any(gap["class"] == fact_class for gap in opened):
                opened.append(
                    {
                        "class": fact_class,
                        "reason": f"capability gap: {fact_class}: no supported original observation captured; use --instructions; selected calls support canonical wrappers and bounded acyclic callers; owner bodies support canonical wrappers and global-pointer zero-byte stores",
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
                "required_work": (
                    [registry_entry.required_work]
                    if registry_entry is not None
                    else sorted(
                        item["id"]
                        for item in normalized_operations
                        if not item["supplemental"]
                    )
                ),
                "proposal_allowed": registry_entry is None,
                **(
                    {"missing_fact": _CAPABILITY_MISSING_FACT}
                    if registry_entry is not None
                    else {}
                ),
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
    if function.target.value != target and str(function) not in owners:
        raise ValueError("instruction capture is not a planned owner selector")
    observations, opened = [], []
    selected = function.target.value == target and function.address == address_of(
        str(row["name"])
    )
    callers = {
        str(parse_function_id(work["id"].split(":", 1)[1]))
        for work in row.get("required_work", [])
        if work.get("status") == "open"
        and str(work.get("id", "")).startswith("caller:")
    }
    if selected:
        observations.append(_selected_range(capture, str(function)))
    if function.target.value == target and str(function) in callers:
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
