"""Reject macro transactions that omit known consumers of writable owners."""

from __future__ import annotations

from pathlib import Path
from typing import Any

from harness.domain.claims import manifest_source_paths
from harness.macros.impact import definition_impact_payload
from harness.macros.review import _wrapper_dependencies


def validate_consumer_coverage(
    connection: Any,
    root: Path,
    owners: list[str],
    functions: list[dict[str, str]],
    manifests: dict[str, Any],
) -> None:
    """Require gates for known lexical consumers, not proof of complete coverage."""
    owner_paths = set(owners) | {row["source"] for row in functions}
    definitions = {
        identity
        for identity, source in connection.execute(
            "SELECT id, source_path FROM macro_definitions"
        )
        if source in owner_paths
    }
    if not definitions:
        return
    impact = definition_impact_payload(connection, definitions)
    if impact["unresolved_definition_uses"]:
        raise ValueError("macro consumer definition-body ownership is unresolved")
    required: set[tuple[str, str]] = set()
    dependencies: dict[tuple[str, str], set[str]] = {}
    claims: dict[str, set[str]] = {}
    for use in impact["uses"]:
        if use["use_context"] == "definition_body":
            continue
        target = use["target_id"]
        if target not in manifests:
            raise ValueError(f"macro consumer target is unknown: {target}")
        if target not in claims:
            claims[target] = {
                path.relative_to(root).as_posix()
                for path in manifest_source_paths(root, manifests[target])
            }
        source = use["source_path"]
        if source in claims[target]:
            required.add((target, source))
            continue
        for claimed in sorted(claims[target]):
            key = (target, claimed)
            if key not in dependencies:
                dependencies[key] = _wrapper_dependencies(root, claimed)
            if source in dependencies[key]:
                required.add(key)
    covered = {(row["target"], row["source"]) for row in functions}
    missing = required - covered
    if missing:
        names = ", ".join(f"{target}:{source}" for target, source in sorted(missing))
        raise ValueError(
            "macro consumer coverage omits claimed sources from affected_functions: "
            + names
        )
