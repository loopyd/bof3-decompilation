"""Single-connection JSON-lines worker for naming-evidence indexed queries."""

from __future__ import annotations

import importlib
import json
import os
import sys
from pathlib import Path
from typing import Any

from harness.analysis.index import connect
from harness.analysis.index import rows as index_rows
from harness.domain.ids import parse_function_id
from harness.domain.manifests import load_target_manifests
from harness.naming.debt import address_of
from harness.naming.plan import indexed_operation_plan
from harness.naming.description import capture_data_range
from harness.naming.consumers import capture_dispatch_consumer
from harness.naming.bytelookup import (
    capture_byte_lookup_access,
    capture_byte_lookup_context,
)


def _query(connection: Any, item_id: str) -> tuple[str, str, list[dict[str, Any]]]:
    queries = importlib.import_module("harness.analysis.rev_queries")
    if item_id.startswith(("callee:", "caller:")):
        selector = item_id.split(":", 1)[1]
        return "calls", selector, queries.calls_payload(connection, selector, limit=0)
    if item_id.startswith("access:"):
        selector = item_id.split(":", 1)[1]
        function = parse_function_id(selector)
        payload = index_rows(
            connection,
            "SELECT source, address, function_id, access_kind, opcode FROM data_references "
            "WHERE target_id = ? AND function_id = ? ORDER BY source, address",
            (function.target.value, str(function)),
        )
        return "access", selector, payload
    if item_id.startswith("owner:"):
        selector = item_id.split(":", 1)[1]
        return (
            "owner",
            selector,
            queries.owner_payload(connection, parse_function_id(selector)),
        )
    raise ValueError(f"unsupported generated work item: {item_id}")


def main() -> int:
    evidence_root = os.environ.get("BOF3_NAMING_EVIDENCE_ROOT")
    if evidence_root:
        from harness.domain.receipts import set_receipt_root

        set_receipt_root(Path(evidence_root))
    init = json.loads(sys.stdin.readline())
    root = Path(init["root"])
    manifests = load_target_manifests(root)
    connection = connect(root, manifests=manifests, shared=True)
    try:
        command = importlib.import_module("harness.naming.audit")
        context = command._context(
            root,
            init["target"],
            bulk_work=True,
            manifests=manifests,
            connection=connection,
        )
        command.validate_v3(root, init["target"], init["report"], context)
        print(json.dumps({"ready": True}), flush=True)
        for line in sys.stdin:
            request = json.loads(line)
            if request.get("op") == "close":
                return 0
            operations = []
            for item in request["row"].get("required_work", []):
                if isinstance(item, dict) and item.get("status") == "open":
                    item_id = str(item["id"])
                    operation, selector, payload = _query(connection, item_id)
                    try:
                        operation_target = parse_function_id(selector).target.value
                    except ValueError as error:
                        raise ValueError(
                            "generated work target is invalid; run "
                            "bin/harness naming init-all out/reviews/plan-audit-naming"
                        ) from error
                    external_owner = (
                        operation == "owner"
                        and item_id.startswith("owner:")
                        and item_id.split(":", 1)[1] == selector
                        and request["row"].get("outside_payload") is True
                    )
                    external_caller = (
                        operation == "calls"
                        and item_id.startswith("caller:")
                        and item_id.split(":", 1)[1] == selector
                    )
                    if operation_target != init["target"] and not (
                        external_owner or external_caller
                    ):
                        raise ValueError(
                            "indexed operation target does not match report"
                        )
                    if external_owner and operation_target not in manifests:
                        raise ValueError(
                            "external owner target is absent from manifests; "
                            "run bin/harness analysis index --recover"
                        )
                    operations.append(
                        {
                            "item": item,
                            "operation": operation,
                            "selector": selector,
                            "payload": payload,
                            "supplemental": False,
                        }
                    )
            queries = importlib.import_module("harness.analysis.rev_queries")
            row = request["row"]
            for planned in indexed_operation_plan(row, init["target"], root=root):
                if planned["operation"] != "function_predicate_context":
                    continue
                from harness.naming.consumers import capture_reviewed_consumer

                selector = str(planned["selector"])
                operations.append(
                    {
                        "item": {"id": planned["id"]},
                        "operation": planned["operation"],
                        "selector": selector,
                        "supplemental": True,
                        "role": planned["role"],
                        "payload": capture_reviewed_consumer(root, selector, selector),
                    }
                )
            if row.get("kind") == "data":
                selected = parse_function_id(
                    f"{init['target']}@{address_of(str(row['name'])):08X}"
                )
                operations.append(
                    {
                        "item": {"id": f"data_original_range:{selected}"},
                        "operation": "data_original_range",
                        "selector": str(selected),
                        "supplemental": True,
                        "role": "data_original_range",
                        "payload": capture_data_range(root, str(selected)),
                    }
                )
                operations.append(
                    {
                        "item": {"id": f"describe:{selected}"},
                        "operation": "describe",
                        "selector": str(selected),
                        "supplemental": True,
                        "role": "selected_data_describe",
                        "payload": queries.describe_payload(
                            connection,
                            selected,
                            root=root,
                            manifests=manifests,
                            limit=0,
                        ),
                    }
                )
                captures = {
                    "data_byte_lookup_access": capture_byte_lookup_access,
                    "data_byte_lookup_context": capture_byte_lookup_context,
                }
                for planned in indexed_operation_plan(row, init["target"], root=root):
                    operation = str(planned["operation"])
                    if operation not in captures:
                        continue
                    operations.append(
                        {
                            "item": {"id": planned["id"]},
                            "operation": operation,
                            "selector": planned["selector"],
                            "supplemental": planned["supplemental"],
                            "role": planned["role"],
                            "payload": captures[operation](
                                root, str(planned["selector"])
                            ),
                        }
                    )
            access_selectors = [
                item["selector"] for item in operations if item["operation"] == "access"
            ]
            for selector in access_selectors:
                function = parse_function_id(selector)
                operations.extend(
                    [
                        {
                            "item": {"id": f"describe:{function}"},
                            "operation": "describe",
                            "selector": str(function),
                            "supplemental": True,
                            "role": "access_function_describe",
                            "payload": queries.describe_payload(
                                connection,
                                function,
                                root=root,
                                manifests=manifests,
                                limit=0,
                            ),
                        },
                        {
                            "item": {"id": f"xrefs:{function}"},
                            "operation": "xrefs",
                            "selector": str(function),
                            "supplemental": True,
                            "role": "access_function_xrefs",
                            "payload": queries.xrefs_payload(
                                connection, function, limit=0
                            ),
                        },
                    ]
                )
                if row.get("kind") == "data":
                    selected = parse_function_id(
                        f"{init['target']}@{address_of(str(row['name'])):08X}"
                    )
                    item_id = f"data_dispatch_consumer:{selected}:{function}"
                    operations.append(
                        {
                            "item": {"id": item_id},
                            "operation": "data_dispatch_consumer",
                            "selector": str(function),
                            "supplemental": True,
                            "role": "data_dispatch_consumer",
                            "payload": capture_dispatch_consumer(
                                root, str(selected), str(function)
                            ),
                        }
                    )
            print(
                json.dumps({"id": request["id"], "operations": operations}), flush=True
            )
    finally:
        connection.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
