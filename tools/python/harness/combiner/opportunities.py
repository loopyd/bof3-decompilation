"""Discover complete finite pools of target-local C pairs without editing."""

from __future__ import annotations

import re
from contextlib import closing
from hashlib import sha256
from itertools import combinations
from pathlib import Path

from harness.analysis.index import connect, validate_index
from harness.common.deadlines import check_deadline
from harness.common.digests import digest
from harness.common.directory import validate_repo_path
from harness.common.inputs import InputBatch
from harness.domain import includes as include_facts
from harness.domain.functions import parse_function_records
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.domain.layout import parse_splat_text
from harness.domain.mips import static_jals
from harness.domain.psx import (
    is_psx_exe,
    payload_for,
    reviewed_range_digest,
    validate_psx_header,
)
from harness.domain.registry import resolve_function, resolve_target
from harness.macros import (
    facts as macro_facts,
    index as macro_index,
    queries as macro_queries,
)

_POOL_LIMIT = 32
_EDGE_LIMIT = 4096
_FILE_LIMIT = 16 * 1024 * 1024
_IMAGE_LIMIT = 64 * 1024 * 1024


def _require_integer(value: int, name: str, maximum: int) -> None:
    if type(value) is not int or not 1 <= value <= maximum:
        raise ValueError(f"{name} must be an integer between one and {maximum}")


def _describe_function(root, resolved, layout, payload, binary, read, *, member):
    selector = resolved.id.value
    address = resolved.id.address
    boundary = layout.find_boundary_at(address)
    if (
        boundary is None
        or not boundary.is_function
        or boundary.virtual_end is None
        or address % 4
        or boundary.virtual_end % 4
        or not 0 <= address < boundary.virtual_end <= 2**32
        or not resolved.compiled_symbol
    ):
        raise ValueError(
            f"function requires an aligned reviewed map/layout range: {selector}"
        )
    identity = reviewed_range_digest(
        payload, address, boundary.virtual_end, binary=binary
    )
    if identity is None:
        raise ValueError(f"function range escapes original payload: {selector}")
    source = None
    if resolved.source is not None:
        source = resolved.source.relative_to(root).as_posix()
        records = parse_function_records(read(resolved.source).decode("utf-8"))
        selected = [record for record in records if record.address == address]
        if len(selected) != 1 or selected[0].spelling != resolved.compiled_symbol:
            raise ValueError(
                f"source metadata/spelling differs from ownership: {selector}"
            )
    if member and (
        source is None
        or boundary.kind != "c"
        or source not in resolved.manifest.sources
        or not resolved.manifest.has_explicit_sources
    ):
        raise ValueError(
            f"pool member requires an explicit reviewed C lift: {selector}"
        )
    return {
        "id": resolved.id.value,
        "source": source,
        "compiled_symbol": resolved.compiled_symbol,
        "address": address,
        "end": boundary.virtual_end,
        "original_sha256": identity[0],
        "original_bytes": identity[1],
    }


def _rank_pairs(members, relations, minimum_shared, top_n, *, feature="shared_callees"):
    pairs = []
    for left, right in combinations(members, 2):
        check_deadline()
        shared = sorted(relations[left] & relations[right])
        pairs.append(
            {
                "id": digest([left, right]),
                "members": [left, right],
                feature: shared,
                "eligible": len(shared) >= minimum_shared,
                "exclusion_reasons": (
                    []
                    if len(shared) >= minimum_shared
                    else [
                        "below minimum shared direct callees"
                        if feature == "shared_callees"
                        else "below minimum shared target-private macros"
                    ]
                ),
                "rank": None,
            }
        )
    ranking = sorted(
        (pair for pair in pairs if pair["eligible"]),
        key=lambda pair: (-len(pair[feature]), pair["members"]),
    )
    for rank, pair in enumerate(ranking, 1):
        pair["rank"] = rank
    for pair in pairs:
        check_deadline()
        pair["overlaps"] = [
            other["id"]
            for other in pairs
            if other["id"] != pair["id"]
            and set(other["members"]) & set(pair["members"])
        ]
    return (
        pairs,
        [pair["id"] for pair in ranking],
        [pair["id"] for pair in ranking[:top_n]],
    )


def _macro_relations(root, resolved, connection, members, functions, read):
    uses = macro_queries.macro_uses_payload(
        connection,
        target=resolved.id.value,
        pattern=None,
        limit=_EDGE_LIMIT + 1,
        function_ids=members,
    )
    if len(uses) > _EDGE_LIMIT:
        raise ValueError("discovery pool exceeds 4096 indexed macro uses")
    ids = sorted({use["definition_id"] for use in uses})
    definitions = macro_queries.macros_payload(
        connection,
        target=None,
        pattern=None,
        classification=None,
        limit=_EDGE_LIMIT + 1,
        definition_ids=ids,
    )
    if {item["id"] for item in definitions} != set(ids):
        raise ValueError("indexed macro use lacks its complete definition")
    by_id = {item["id"]: item for item in definitions}
    parsed_definitions, parsed_uses, closures, selected, line_offsets = (
        {},
        {},
        {},
        {},
        {},
    )
    for member in members:
        check_deadline()
        source = functions[member]["source"]
        path = root / source
        text = read(path).decode("utf-8")
        records = parse_function_records(text)
        selected[member] = next(
            record
            for record in records
            if record.address == functions[member]["address"]
        )
        if source not in closures:
            line_offsets[source] = [0]
            for line in text.splitlines(keepends=True):
                line_offsets[source].append(line_offsets[source][-1] + len(line))
            closures[source] = {
                source,
                *(
                    p.relative_to(root).as_posix()
                    for p in include_facts.local_include_files(
                        root,
                        [path],
                        strict=True,
                        require_literal=True,
                        include_angle=True,
                        read_source=lambda p: read(p).decode("utf-8"),
                    )
                ),
            }
            names = {use["name"] for use in uses if use["source_path"] == source}
            parsed_uses[source] = {
                (use.name, use.source_line, use.source_column): use
                for use in macro_facts.parse_macro_uses(text, names)
            }
    for definition in definitions:
        check_deadline()
        source = validate_repo_path(definition["source_path"])
        raw = read(root / source)
        if sha256(raw).hexdigest() != definition["source_sha256"]:
            raise ValueError(
                "indexed macro definition source differs from captured bytes"
            )
        if source not in parsed_definitions:
            parsed_definitions[source] = {
                (item.name, item.source_line): item
                for item in macro_facts.parse_macro_definitions(
                    raw.decode("utf-8"), source=source
                )
            }
        actual = parsed_definitions[source].get(
            (definition["name"], definition["source_line"])
        )
        if (
            actual is None
            or definition["id"]
            != macro_index._definition_id(
                definition["owner_target"],
                source,
                actual.source_line,
                actual.name,
            )
            or any(
                definition[key] != value
                for key, value in {
                    "parameters": None
                    if actual.parameters is None
                    else list(actual.parameters),
                    "body": actual.body,
                    "conditional_context": list(actual.conditions),
                    "classification": actual.classification,
                    "diagnostic": actual.diagnostic,
                }.items()
            )
        ):
            raise ValueError("indexed macro definition differs from its lexical source")
    alternatives = {}
    for use in uses:
        key = (
            use["function_id"],
            use["source_path"],
            use["source_line"],
            use["source_column"],
            use["name"],
        )
        alternatives.setdefault(key, set()).add(use["definition_id"])
    relations = {member: set() for member in members}
    owned_paths = {p.relative_to(root).as_posix() for p in resolved.header_paths} | {
        functions[member]["source"] for member in members
    }
    for use in uses:
        check_deadline()
        member, source = use["function_id"], use["source_path"]
        if source != functions[member]["source"]:
            raise ValueError("indexed macro use differs from its function source")
        actual = parsed_uses[source].get(
            (use["name"], use["source_line"], use["source_column"])
        )
        if actual is None or any(
            use[key] != value
            for key, value in {
                "arguments": actual.arguments,
                "conditional_context": list(actual.conditions),
                "use_context": actual.context,
            }.items()
        ):
            raise ValueError("indexed macro use differs from its lexical source")
        offset = line_offsets[source][actual.source_line - 1] + actual.source_column - 1
        record = selected[member]
        definition = by_id[use["definition_id"]]
        if use["name"] != definition["name"]:
            raise ValueError("indexed macro use names a different definition")
        key = (member, source, actual.source_line, actual.source_column, actual.name)
        reason = None
        if not record.implementation_start <= offset < record.implementation_end:
            reason = "macro use is outside the selected implementation"
        elif len(alternatives[key]) != 1:
            reason = "macro use has multiple indexed definitions"
        elif (
            definition["owner_target"] != resolved.id.value
            or definition["source_path"] not in owned_paths
        ):
            reason = "macro definition is not private to the selected pool target"
        elif definition["source_path"] not in closures[source]:
            reason = "macro definition is outside the source include closure"
        elif (
            use["generated"]
            or definition["generated"]
            or actual.generated
            or parsed_definitions[definition["source_path"]][
                (definition["name"], definition["source_line"])
            ].generated
            or definition["diagnostic"] is not None
        ):
            reason = "macro use or definition is generated or unsupported"
        elif definition["parameters"] is None or actual.arguments is None:
            reason = "macro use is not a function-like invocation"
        elif (
            definition["classification"] != "function_like"
            or definition["restrictions"]
            or use["restrictions"]
        ):
            reason = "macro definition is not an ordinary unrestricted function-like macro"
        use["qualified"] = reason is None
        use["exclusion_reason"] = reason
        if reason is None:
            relations[member].add(definition["id"])
    return relations, definitions, uses


def discover_pairs(
    root: Path,
    target: str,
    selectors: list[str],
    *,
    method: str = "helper-call-cluster",
    minimum_members: int = 2,
    minimum_shared_callees: int = 1,
    minimum_shared_macros: int = 1,
    top_n: int = 1,
    overlap_policy: str = "retain-all",
    expected_fingerprint: str | None = None,
) -> dict:
    """Rank every unordered pair in an explicit target-local reviewed C pool."""
    check_deadline()
    if (
        method not in {"helper-call-cluster", "macro-family"}
        or overlap_policy != "retain-all"
    ):
        raise ValueError(
            "only helper-call-cluster or macro-family with retain-all overlap is supported"
        )
    if type(minimum_members) is not int or minimum_members != 2:
        raise ValueError(
            "combiner discovery enumerates pairs; minimum members must be two"
        )
    _require_integer(minimum_shared_callees, "minimum shared callees", _EDGE_LIMIT)
    _require_integer(minimum_shared_macros, "minimum shared macros", _EDGE_LIMIT)
    if (method == "helper-call-cluster" and minimum_shared_macros != 1) or (
        method == "macro-family" and minimum_shared_callees != 1
    ):
        raise ValueError("discovery rejects a threshold for another method")
    _require_integer(top_n, "top N", _POOL_LIMIT * (_POOL_LIMIT - 1) // 2)
    target = normalize_target_id(target).value
    parsed = [parse_function_id(selector) for selector in selectors]
    members = sorted(function.value for function in parsed)
    if (
        not 2 <= len(members) <= _POOL_LIMIT
        or len(set(members)) != len(members)
        or any(function.target.value != target for function in parsed)
    ):
        raise ValueError(
            "pool requires two to 32 distinct functions of the selected target"
        )
    if expected_fingerprint is not None and not re.fullmatch(
        r"v1:[0-9a-f]{64}", expected_fingerprint
    ):
        raise ValueError("expected discovery fingerprint must be a v1 SHA-256 digest")
    root = root.resolve()
    resolved = resolve_target(root, target)
    pins, content = {}, {}
    with InputBatch(root) as inputs, closing(connect(root)) as connection:
        connection.execute("PRAGMA query_only = ON")
        connection.execute("BEGIN")

        def read(path, limit=_FILE_LIMIT):
            name = path.relative_to(root).as_posix()
            if name not in content:
                state, data = inputs.read(path, max_bytes=limit, single_link=True)
                if data is None:
                    raise ValueError(f"missing discovery input: {name}")
                pins[name], content[name] = state, data
            return content[name]

        for path in (
            resolved.manifest_path,
            resolved.splat_path,
            root / "config" / "targets" / target / "symbols.txt",
            Path(__file__),
            Path(__file__).with_name("cli.py"),
        ):
            read(path)
        if method == "macro-family":
            for module in (macro_facts, macro_index, macro_queries, include_facts):
                read(Path(module.__file__))
        binary = read(resolved.binary_path, _IMAGE_LIMIT)
        if is_psx_exe(binary):
            validate_psx_header(
                binary, resolved.load_address, binary_name=str(resolved.binary_path)
            )
        payload = payload_for(
            binary, resolved.load_address, binary_name=str(resolved.binary_path)
        )
        layout = parse_splat_text(
            read(resolved.splat_path).decode("utf-8"),
            resolved.load_address,
            origin=resolved.splat_path,
        )
        functions = {
            member: _describe_function(
                root,
                resolve_function(root, member),
                layout,
                payload,
                binary,
                read,
                member=True,
            )
            for member in members
        }
        original_calls = {}
        for member in members:
            function = functions[member]
            start = payload.binary_offset + function["address"] - payload.load_address
            end = start + function["original_bytes"]
            original_calls[member] = dict(
                static_jals(binary[start:end], function["address"])
            )
        placeholders = ",".join("?" for _ in members)
        calls = [
            dict(row)
            for row in connection.execute(
                f"SELECT caller, callee, callsite FROM calls WHERE caller IN ({placeholders}) "
                "ORDER BY caller, callee, callsite LIMIT ?",
                (*members, _EDGE_LIMIT + 1),
            )
        ]
        unresolved = [
            dict(row)
            for row in connection.execute(
                f"SELECT caller, target_address, callsite, kind FROM unresolved_calls "
                f"WHERE caller IN ({placeholders}) ORDER BY caller, callsite, target_address, kind LIMIT ?",
                (*members, _EDGE_LIMIT + 1),
            )
        ]
        if len(calls) + len(unresolved) > _EDGE_LIMIT:
            raise ValueError("discovery pool exceeds 4096 indexed call edges")
        direct_callees = {member: set() for member in members}
        for call in calls:
            check_deadline()
            caller = functions[call["caller"]]
            callee = parse_function_id(call["callee"])
            pc = call["callsite"]
            reason = None
            if callee.target.value != target:
                reason = "callee belongs to another target"
            elif pc % 4 or not caller["address"] <= pc < caller["end"]:
                raise ValueError("indexed callsite escapes its reviewed caller range")
            else:
                offset = payload.binary_offset + pc - payload.load_address
                call["original_bytes"] = binary[offset : offset + 4].hex()
                if pc not in original_calls[call["caller"]]:
                    reason = "original instruction is not a direct JAL"
                elif original_calls[call["caller"]][pc] != callee.address:
                    raise ValueError(
                        "indexed callee differs from original JAL destination"
                    )
                else:
                    boundary = layout.find_boundary_at(callee.address)
                    owned = resolve_function(root, callee)
                    if boundary is None or not boundary.is_function:
                        reason = "callee lacks a reviewed function boundary"
                    elif (
                        boundary.virtual_end is None
                        or callee.address % 4
                        or boundary.virtual_end % 4
                        or not 0 <= callee.address < boundary.virtual_end <= 2**32
                        or reviewed_range_digest(
                            payload, callee.address, boundary.virtual_end, binary=binary
                        )
                        is None
                    ):
                        reason = "callee lacks a finite aligned payload-contained range"
                    elif not owned.compiled_symbol:
                        reason = "callee lacks map/layout-agreed compiled spelling"
                    else:
                        if callee.value not in functions:
                            functions[callee.value] = _describe_function(
                                root, owned, layout, payload, binary, read, member=False
                            )
                        direct_callees[call["caller"]].add(callee.value)
            call["qualified"] = reason is None
            call["exclusion_reason"] = reason
        index_target = dict(
            connection.execute(
                "SELECT * FROM targets WHERE id = ?", (target,)
            ).fetchone()
        )
        macro_definitions, macro_uses = [], []
        if method == "macro-family":
            direct_callees, macro_definitions, macro_uses = _macro_relations(
                root,
                resolved,
                connection,
                members,
                functions,
                read,
            )
        validate_index(connection, root)
        for name in sorted(pins):
            state, _ = inputs.read(
                root / name, max_bytes=_IMAGE_LIMIT, single_link=True
            )
            if state != pins[name]:
                raise ValueError(f"discovery input changed during enumeration: {name}")
    pairs, ranking, selection = _rank_pairs(
        members,
        direct_callees,
        minimum_shared_callees
        if method == "helper-call-cluster"
        else minimum_shared_macros,
        top_n,
        feature="shared_callees"
        if method == "helper-call-cluster"
        else "shared_macros",
    )
    result = {
        "schema": "bof3.combiner-discovery/v1",
        "method": method,
        "method_version": 1,
        "target": target,
        "minimum_members": minimum_members,
        "minimum_shared_callees": minimum_shared_callees,
        "overlap_policy": overlap_policy,
        "top_n": top_n,
        "pool_members": members,
        "pool_fingerprint": digest(members),
        "inputs": pins,
        "index_target": index_target,
        "index_fresh": True,
        "index_snapshot_fingerprint": digest(
            {
                "target": index_target,
                "calls": calls,
                "unresolved_calls": unresolved,
                "functions": [functions[key] for key in sorted(functions)],
            }
        ),
        "functions": [functions[key] for key in sorted(functions)],
        "calls": calls,
        "original_direct_calls": [
            {"caller": member, "callsite": pc, "target_address": destination}
            for member in members
            for pc, destination in sorted(original_calls[member].items())
        ],
        "unresolved_calls": unresolved,
        "pairs": pairs,
        "ranking": ranking,
        "ranking_fingerprint": digest(pairs),
        "top_candidates": selection,
        "complete": True,
        "write_authorized": False,
        "human_value_reviewed": False,
        "native_verified": False,
        "coverage_verified": False,
        "accepted": False,
    }
    result["fingerprint"] = digest(result)
    if method == "macro-family":
        result.pop("minimum_shared_callees")
        result.update(
            minimum_shared_macros=minimum_shared_macros,
            macro_definitions=macro_definitions,
            macro_uses=macro_uses,
        )
        result["index_snapshot_fingerprint"] = digest(
            {
                "target": index_target,
                "functions": result["functions"],
                "macro_definitions": macro_definitions,
                "macro_uses": macro_uses,
                "calls": calls,
                "unresolved_calls": unresolved,
            }
        )
        result["fingerprint"] = digest(
            {key: value for key, value in result.items() if key != "fingerprint"}
        )
    if (
        expected_fingerprint is not None
        and result["fingerprint"] != expected_fingerprint
    ):
        raise ValueError("discovery fingerprint differs from the external pin")
    check_deadline()
    return result
