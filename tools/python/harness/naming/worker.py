"""Single-connection JSON-lines worker for naming-evidence indexed queries."""

from __future__ import annotations

import hashlib
import importlib
import json
import os
import struct
import sys
from pathlib import Path
from typing import Any

from harness.analysis.index import connect
from harness.analysis.index import rows as index_rows
from harness.analysis.project import prepare_target
from harness.domain.ids import parse_function_id
from harness.domain.manifests import load_target_manifests
from harness.naming.debt import address_of


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
                            "bin/naming-audit init-all out/reviews/plan-audit-naming"
                        ) from error
                    external_owner = (
                        operation == "owner"
                        and item_id.startswith("owner:")
                        and item_id.split(":", 1)[1] == selector
                        and request["row"].get("outside_payload") is True
                    )
                    if operation_target != init["target"] and not external_owner:
                        raise ValueError(
                            "indexed operation target does not match report"
                        )
                    if external_owner and operation_target not in manifests:
                        raise ValueError(
                            "external owner target is absent from manifests; "
                            "run bin/index --recover"
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
            if row.get("kind") == "data":
                selected = parse_function_id(
                    f"{init['target']}@{address_of(str(row['name'])):08X}"
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
                    selected_description = queries.describe_payload(
                        connection, selected, root=root, manifests=manifests, limit=0
                    )
                    function_description = queries.describe_payload(
                        connection, function, root=root, manifests=manifests, limit=0
                    )
                    if len(selected_description) == len(function_description) == 1:
                        selected_storage = selected_description[0].get("storage") or {}
                        consumer_splat = function_description[0].get("splat") or {}
                        start = int(str(selected_storage.get("start")), 16)
                        end = int(str(selected_storage.get("end")), 16)
                        consumer_start = int(str(consumer_splat.get("start")), 16)
                        consumer_end = int(str(consumer_splat.get("end")), 16)
                        spec = prepare_target(
                            root, init["target"], manifest=manifests[init["target"]]
                        )
                        image = spec.binary.read_bytes()

                        def original_bytes(address: int, size: int) -> bytes:
                            offset = spec.binary_offset + address - spec.load_address
                            if offset < spec.binary_offset or offset + size > len(
                                image
                            ):
                                raise ValueError(
                                    "typed analyzer range is outside original payload"
                                )
                            return image[offset : offset + size]

                        table = original_bytes(start, end - start)
                        # ponytail: A non-word extent proves no table here; add a
                        # target-qualified TableConsumerSpec if later evidence does.
                        pointer_descriptions = []
                        if not len(table) % 4:
                            pointers = struct.unpack(f"<{len(table) // 4}I", table)
                            pointer_descriptions = [
                                queries.describe_payload(
                                    connection,
                                    parse_function_id(
                                        f"{init['target']}@{pointer:08X}"
                                    ),
                                    root=root,
                                    manifests=manifests,
                                    limit=0,
                                )
                                for pointer in pointers
                            ]
                        item_id = f"data_dispatch_consumer:{selected}:{function}"
                        operations.append(
                            {
                                "item": {"id": item_id},
                                "operation": "data_dispatch_consumer",
                                "selector": str(function),
                                "supplemental": True,
                                "role": "data_dispatch_consumer",
                                "payload": {
                                    "selected": str(selected),
                                    "consumer": str(function),
                                    "selected_range": [start, end],
                                    "consumer_range": [consumer_start, consumer_end],
                                    "selected_bytes": table.hex(),
                                    "consumer_bytes": original_bytes(
                                        consumer_start, consumer_end - consumer_start
                                    ).hex(),
                                    "image_sha256": hashlib.sha256(image).hexdigest(),
                                    "pointer_descriptions": pointer_descriptions,
                                },
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
