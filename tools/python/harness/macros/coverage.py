"""Reject macro checks that omit known consumers of scoped definitions."""

from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Any

from harness.common.files import read_file
from harness.domain.claims import manifest_source_paths
from harness.domain.functions import (
    FunctionRecord,
    collect_lift_metadata,
    parse_function_records,
)
from harness.domain.ids import parse_function_id
from harness.domain.tags import count_function_metadata
from harness.macros.impact import definition_impact_payload
from harness.macros.review import _wrapper_dependencies


def _collect_members(
    connection: Any, root: Path, target: str, source: str
) -> tuple[dict[int, str], tuple[FunctionRecord, ...], list[int]]:
    content = read_file(root, source)
    fingerprints = connection.execute(
        "SELECT sha256 FROM macro_input_fingerprints WHERE target_id = ? "
        "AND source_path = ? AND input_kind = 'source_claim' AND owner_target = ?",
        (target, source, target),
    ).fetchall()
    if (
        len(fingerprints) != 1
        or fingerprints[0][0] != hashlib.sha256(content).hexdigest()
    ):
        raise ValueError(f"macro consumer source differs from indexed input: {source}")
    text = content.decode("utf-8").replace("\r\n", "\n").replace("\r", "\n")
    records = (
        parse_function_records(text, validate_progress=False)
        if count_function_metadata(text) > 1
        else ()
    )
    addresses = (
        {record.address for record in records}
        if records
        else set(collect_lift_metadata(text))
    )
    identities = {}
    for identity, address in connection.execute(
        "SELECT id, address FROM functions WHERE target_id = ? AND source IN (?, ?)",
        (target, source, (root / source).as_posix()),
    ):
        function = parse_function_id(identity)
        if (
            function.target.value != target
            or function.address != address
            or address in identities
        ):
            raise ValueError(f"macro consumer indexed identity is ambiguous: {source}")
        identities[address] = function.value
    if not addresses or addresses != identities.keys():
        raise ValueError(f"macro consumer source membership is unresolved: {source}")
    offsets = [0]
    for line in text.splitlines(keepends=True):
        offsets.append(offsets[-1] + len(line))
    return identities, records, offsets


def _resolve_use(
    use: dict[str, Any],
    members: tuple[dict[int, str], tuple[FunctionRecord, ...], list[int]],
) -> str | None:
    identities, records, offsets = members
    line, column = use["source_line"], use["source_column"]
    if (
        type(line) is not int
        or type(column) is not int
        or not 0 < line < len(offsets)
        or not 0 < column <= offsets[line] - offsets[line - 1]
    ):
        raise ValueError("macro consumer location is outside its indexed source")
    offset = offsets[line - 1] + column - 1
    expected = (
        next(
            (
                identities[record.address]
                for record in records
                if record.implementation_start <= offset < record.implementation_end
            ),
            None,
        )
        if records
        else next(iter(identities.values()))
    )
    identity = use["function_id"]
    actual = parse_function_id(identity).value if isinstance(identity, str) else None
    if (identity is not None and not isinstance(identity, str)) or actual != expected:
        raise ValueError("macro consumer function identity differs from authored range")
    return expected


def validate_consumer_coverage(
    connection: Any,
    root: Path,
    owners: list[str],
    functions: list[dict[str, str]],
    manifests: dict[str, Any],
    *,
    definition_ids: set[str] | None = None,
    require_function_identity: bool = False,
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
    if definition_ids is not None:
        if not definition_ids <= definitions:
            raise ValueError(
                "macro coverage definitions are outside the reviewed owners"
            )
        definitions = definition_ids
    if not definitions:
        return
    impact = definition_impact_payload(connection, definitions)
    if impact["unresolved_definition_uses"]:
        raise ValueError("macro consumer definition-body ownership is unresolved")
    required: set[tuple[str, str, str]] = set()
    covered = set()
    for row in functions:
        identity = parse_function_id(row["selector"])
        if identity.target.value != row["target"]:
            raise ValueError("macro covered function identity has the wrong target")
        covered.add((row["target"], row["source"], identity.value))
    covered_identities = {identity for _target, _source, identity in covered}
    dependencies: dict[tuple[str, str], set[str]] = {}
    claims: dict[str, set[str]] = {}
    members = {}

    def collect_members(target: str, source: str):
        key = (target, source)
        if key not in members:
            members[key] = _collect_members(connection, root, target, source)
        return members[key]

    for use in impact["uses"]:
        if use["use_context"] == "definition_body":
            continue
        target = use["target_id"]
        if require_function_identity:
            identity = use["function_id"]
            if (
                not isinstance(identity, str)
                or identity.rsplit("@", 1)[0] != target
                or parse_function_id(identity).value not in covered_identities
            ):
                raise ValueError(
                    "macro consumer function identity is unresolved or uncovered"
                )
        if target not in manifests:
            raise ValueError(f"macro consumer target is unknown: {target}")
        if target not in claims:
            claims[target] = {
                path.relative_to(root).as_posix()
                for path in manifest_source_paths(root, manifests[target])
            }
        source = use["source_path"]
        if source in claims[target]:
            sampled = collect_members(target, source)
            identity = _resolve_use(use, sampled)
            required.update(
                (target, source, member)
                for member in ({identity} if identity else sampled[0].values())
            )
            continue
        if use["function_id"] is not None:
            raise ValueError("macro include consumer cannot claim a function identity")
        for claimed in sorted(claims[target]):
            key = (target, claimed)
            if key not in dependencies:
                dependencies[key] = _wrapper_dependencies(root, claimed)
            if source in dependencies[key]:
                required.update(
                    (target, claimed, identity)
                    for identity in collect_members(target, claimed)[0].values()
                )
    missing = required - covered
    if missing:
        names = ", ".join(
            f"{identity}:{source}" for _target, source, identity in sorted(missing)
        )
        raise ValueError(
            "macro consumer coverage omits claimed functions from affected_functions: "
            + names
        )
