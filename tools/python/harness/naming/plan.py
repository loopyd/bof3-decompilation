"""Plan closed, code-owned naming evidence operations."""

from __future__ import annotations

import shlex
from typing import Any

from harness.common.commands import command_text, tool_id, trailing_arguments
from harness.domain.ids import parse_function_id
from harness.naming.debt import address_of
from harness.naming.capabilities import (
    PRODUCTION_EXACT_CAPABILITIES,
    ExactCapabilityRegistry,
)

_NATIVE_ARGS = {
    "asm-diff": ["--json", "--detail", "full"],
    "byte-match": ["--json"],
}


def _selected(row: dict[str, Any], target: str) -> str:
    return str(parse_function_id(f"{target}@{address_of(str(row['name'])):08X}"))


def _native_selector(row: dict[str, Any], target: str) -> str:
    return f"{target}@0x{address_of(str(row['name'])):08X}"


def owner_instruction_selectors(row: dict[str, Any]) -> set[str]:
    """Only open, generated owner work authorizes cross-target instructions."""
    if row.get("kind") != "function" or row.get("outside_payload") is not True:
        return set()
    return {
        str(parse_function_id(work["id"].split(":", 1)[1]))
        for work in row.get("required_work", [])
        if work.get("status") == "open" and str(work.get("id", "")).startswith("owner:")
    }


def semantic_operation_plan(row: dict[str, Any], target: str) -> list[dict[str, Any]]:
    """Return the only analyzer/native operations authorized for one row."""

    selected = _native_selector(row, target)
    result: list[dict[str, Any]] = []
    from harness.naming.instructions import INSTRUCTION_CAPTURE

    if row.get("kind") == "function" and INSTRUCTION_CAPTURE.get():
        selectors = (
            set() if row.get("outside_payload") is True else {_selected(row, target)}
        )
        for work in row.get("required_work", []):
            if not isinstance(work, dict) or work.get("status") != "open":
                continue
            key = str(work.get("id", ""))
            if key.startswith(("caller:", "callee:")):
                function = parse_function_id(key.split(":", 1)[1])
                if function.target.value != target:
                    raise ValueError("instruction work must be target-local")
                selectors.add(str(function))
        selectors.update(owner_instruction_selectors(row))
        result.extend(
            {
                "kind": "instructions-v1",
                "target": selector,
                "command": f"semantic:instructions-v1:{selector}",
            }
            for selector in sorted(selectors)
        )
    if row.get("kind") == "function" and row.get("partial_used") is True:
        for kind, args in _NATIVE_ARGS.items():
            result.append(
                {
                    "kind": kind,
                    "target": selected,
                    "args": list(args),
                    "command": command_text(kind, selected, *args),
                }
            )
    return result


def parse_semantic_command(command: str) -> dict[str, Any] | None:
    """Parse command prose for validation only; execution never uses its values."""

    if any(character in command for character in "\r\n\0"):
        raise ValueError("semantic command contains a control character")
    tokens = shlex.split(command.strip())
    if not tokens:
        return None
    tool = tool_id(tokens)
    if tool == "rz-project":
        arguments = trailing_arguments(tokens)
        if len(arguments) != 4 or arguments[0] != "query" or arguments[2] != "-c":
            raise ValueError(f"unsupported rz-project command: {command}")
        return {"kind": tool, "target": arguments[1], "rizin": arguments[3]}
    if tool in _NATIVE_ARGS:
        arguments = trailing_arguments(tokens)
        if not arguments:
            raise ValueError(f"missing selector: {command}")
        return {"kind": tool, "target": arguments[0], "args": arguments[1:]}
    if tool == "rev-query":
        return None
    raise ValueError(f"unsupported semantic command: {command}")


def _report_command_prose(row: dict[str, Any]) -> list[str]:
    commands = [str(row.get("ceiling_next_command") or "")]
    rungs = row.get("rungs")
    if isinstance(rungs, dict):
        commands.extend(
            str(rung.get("next_command") or "")
            for rung in rungs.values()
            if isinstance(rung, dict) and rung.get("status") == "open"
        )
    return commands


def validate_report_operation_prose(row: dict[str, Any], target: str) -> None:
    """Reject report analyzer/native prose outside the exact code-owned plan."""

    allowed = {
        (item["kind"], item["target"], item.get("rizin"), tuple(item.get("args", [])))
        for item in semantic_operation_plan(row, target)
        if item["kind"] != "instructions-v1"
    }
    allowed.add(
        (
            "rz-project",
            target,
            f"axt @ 0x{address_of(str(row['name'])):08X}",
            (),
        )
    )
    for value in _report_command_prose(row):
        for part in value.split(";"):
            command = part.strip()
            if not command:
                continue
            parsed = parse_semantic_command(command)
            if parsed is None:
                continue
            key = (
                parsed["kind"],
                parsed["target"],
                parsed.get("rizin"),
                tuple(parsed.get("args", [])),
            )
            if key not in allowed:
                raise ValueError("report semantic command is not code-owned")


def validate_report_operations(rows: list[dict[str, Any]], target: str) -> None:
    for row in rows:
        validate_report_operation_prose(row, target)


def collection_row(
    row: dict[str, Any],
    target: str,
    *,
    registry: ExactCapabilityRegistry = PRODUCTION_EXACT_CAPABILITIES,
) -> dict[str, Any]:
    """Restore code-owned work only for an explicitly selected exact terminal row."""

    if row.get("rung_status") != "exhausted" or row.get("kind") != "data":
        return row
    key = f"data:{row.get('name')}"
    entries = [
        entry
        for (spec_row, _selector), entry in registry.items()
        if spec_row == key and entry.consumer.target == target
    ]
    if len(entries) != 1:
        return row
    restored = {
        key: value for key, value in row.items() if key != "conclusion_provenance"
    }
    restored["required_work"] = [
        {
            "id": entries[0].required_work,
            "status": "open",
            "profile": "data_access",
            "description": "refresh exact reviewed consumer evidence",
        }
    ]
    return restored


def indexed_operation_plan(
    row: dict[str, Any],
    target: str,
    *,
    registry: ExactCapabilityRegistry = PRODUCTION_EXACT_CAPABILITIES,
) -> list[dict[str, object]]:
    """Exact ordered required and supplemental indexed work for one row."""

    plan: list[dict[str, object]] = []
    for item in collection_row(row, target, registry=registry).get("required_work", []):
        if not (isinstance(item, dict) and item.get("status") == "open"):
            continue
        item_id = str(item.get("id"))
        if item_id.startswith(("callee:", "caller:")):
            operation = "calls"
        elif item_id.startswith("access:"):
            operation = "access"
        elif item_id.startswith("owner:"):
            operation = "owner"
        else:
            raise ValueError(f"unsupported generated work item: {item_id}")
        selector = item_id.split(":", 1)[1]
        plan.append(
            {
                "id": item_id,
                "operation": operation,
                "selector": selector,
                "command": f"harness:rev-query {operation} {selector} (work item {item_id})",
                "supplemental": False,
            }
        )
    if row.get("kind") == "data":
        selected = _selected(row, target)
        item_id = f"describe:{selected}"
        plan.append(
            {
                "id": item_id,
                "operation": "describe",
                "selector": selected,
                "command": f"harness:rev-query describe {selected} (work item {item_id})",
                "supplemental": True,
                "role": "selected_data_describe",
            }
        )
    else:
        selected = ""
    access_selectors = [
        str(item["selector"]) for item in plan if item["operation"] == "access"
    ]
    for selector in access_selectors:
        function = str(parse_function_id(selector))
        for operation in ("describe", "xrefs"):
            item_id = f"{operation}:{function}"
            plan.append(
                {
                    "id": item_id,
                    "operation": operation,
                    "selector": function,
                    "command": f"harness:rev-query {operation} {function} (work item {item_id})",
                    "supplemental": True,
                    "role": f"access_function_{operation}",
                }
            )
        item_id = f"data_dispatch_consumer:{selected}:{function}"
        plan.append(
            {
                "id": item_id,
                "operation": "data_dispatch_consumer",
                "selector": function,
                "command": f"harness:original-bytes data_dispatch_consumer {selected} {function} (work item {item_id})",
                "supplemental": True,
                "role": "data_dispatch_consumer",
            }
        )
    return plan


def row_operation_sequence(
    row: dict[str, Any],
    target: str = "",
    *,
    registry: ExactCapabilityRegistry = PRODUCTION_EXACT_CAPABILITIES,
) -> list[str]:
    """Complete code-owned indexed and semantic operation sequence."""

    operations = [
        f"indexed:{item['id']}:{item['operation']}:{item['selector']}:{int(bool(item['supplemental']))}"
        for item in indexed_operation_plan(row, target, registry=registry)
    ]
    operations.extend(
        (
            operation["command"]
            if operation["kind"] == "instructions-v1"
            else f"semantic:{operation['kind']}:{operation['command']}"
        )
        for operation in semantic_operation_plan(row, target)
    )
    return operations
