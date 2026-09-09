"""Target-qualified lexical consumer closure for indexed macro definitions."""

from __future__ import annotations

from collections import defaultdict, deque
from pathlib import Path
from typing import Any

from harness.analysis.index import connect
from harness.macros.queries import macro_uses_payload, macros_payload

SCHEMA = "bof3.macro-impact/v1"


def definition_impact_payload(
    connection: Any, definition_ids: set[str]
) -> dict[str, Any]:
    """Follow indexed definition-body references without claiming expansion proof."""
    definitions = macros_payload(
        connection, target=None, pattern=None, classification=None, limit=0
    )
    by_id = {row["id"]: row for row in definitions}
    unknown = definition_ids - by_id.keys()
    if unknown:
        raise ValueError(f"unknown macro definitions: {', '.join(sorted(unknown))}")
    uses = macro_uses_payload(connection, target=None, pattern=None, limit=0)
    by_source: dict[tuple[str, str], list[dict[str, Any]]] = defaultdict(list)
    for definition in definitions:
        by_source[(definition["owner_target"], definition["source_path"])].append(
            definition
        )
    consumers: dict[tuple[str, str], list[dict[str, Any]]] = defaultdict(list)
    for use in uses:
        consumers[(use["definition_id"], use["target_id"])].append(use)
    pending = deque(key for key in consumers if key[0] in definition_ids)
    reached = set(pending)
    edges = []
    unresolved = []
    while pending:
        referenced_id, target = pending.popleft()
        for use in consumers.get((referenced_id, target), []):
            if use["use_context"] != "definition_body":
                continue
            containers = [
                definition
                for owner in (target, "__shared__")
                for definition in by_source.get((owner, use["source_path"]), [])
                if definition["source_line"] <= use["source_line"]
            ]
            if not containers:
                unresolved.append(use)
                continue
            start = max(row["source_line"] for row in containers)
            for container in containers:
                if container["source_line"] != start:
                    continue
                edges.append(
                    {
                        "target": target,
                        "referenced_definition": referenced_id,
                        "containing_definition": container["id"],
                        "source_path": use["source_path"],
                        "source_line": use["source_line"],
                        "source_column": use["source_column"],
                    }
                )
                key = (container["id"], target)
                if key not in reached:
                    reached.add(key)
                    pending.append(key)
    impacted_uses = [
        {
            **use,
            "dependency": "direct"
            if use["definition_id"] in definition_ids
            else "transitive",
        }
        for use in uses
        if (use["definition_id"], use["target_id"]) in reached
    ]
    reached_ids = definition_ids | {identity for identity, _target in reached}
    return {
        "schema": SCHEMA,
        "root_definitions": sorted(definition_ids),
        "definitions": [row for row in definitions if row["id"] in reached_ids],
        "definition_contexts": [
            {"definition_id": identity, "target": target}
            for identity, target in sorted(reached)
        ],
        "uses": impacted_uses,
        "dependency_edges": sorted(
            edges,
            key=lambda row: (
                row["target"],
                row["source_path"],
                row["source_line"],
                row["source_column"],
                row["referenced_definition"],
                row["containing_definition"],
            ),
        ),
        "targets": sorted({use["target_id"] for use in impacted_uses}),
        "unresolved_definition_uses": unresolved,
        "scope": "global indexed lexical consumers; target-qualified transitive definition-body references",
        "complete": False,
        "safe_application_count": 0,
        "limitations": [
            "Name matches do not prove include order, conditional selection, undefinition, or expansion binding.",
            "Unindexed inputs and computed macro names or token-pasting need separate compiler/use-site review.",
            "Consumers are review leads, not affected-function authority or permission to edit external targets.",
        ],
    }


def describe_definition_impact(root: Path, definition_id: str) -> dict[str, Any]:
    """Inspect a current definition's consumers through one validated index."""
    connection = connect(root)
    try:
        return definition_impact_payload(connection, {definition_id})
    finally:
        connection.close()
