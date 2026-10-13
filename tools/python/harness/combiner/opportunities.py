"""Discover complete finite pools of target-local C pairs without editing."""

from __future__ import annotations

import json
import re
from contextlib import closing
from hashlib import sha256
from itertools import combinations
from pathlib import Path

from harness import io as harness_io
from harness.analysis.index import connect, validate_index
from harness.common import lexicon
from harness.common.deadlines import check_deadline
from harness.common.digests import digest
from harness.common.directory import validate_repo_path
from harness.common.inputs import InputBatch
from harness.domain import includes as include_facts
from harness.domain import c_context, declarations, mips, tags
from harness.domain import functions as function_facts
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.domain.layout import parse_splat_text
from harness.domain.mips import static_jals
from harness.domain.psx import (
    is_psx_exe,
    payload_for,
    reviewed_range_digest,
    validate_psx_header,
)
from harness.domain.registry import (
    lookup_target_manifest,
    resolve_function,
    resolve_target,
)
from harness.macros import (
    facts as macro_facts,
    index as macro_index,
    queries as macro_queries,
)
from harness.types import (
    index as type_index,
    inputs as type_inputs,
    queries as type_queries,
    representation as type_representation,
    usages as type_usages,
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
        records = function_facts.parse_function_records(
            read(resolved.source).decode("utf-8")
        )
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
                        {
                            "shared_callees": "below minimum shared direct callees",
                            "shared_macros": "below minimum shared target-private macros",
                            "shared_relations": "below minimum shared storage/type relations",
                            "shared_subcategories": "below minimum shared authored functional subcategories",
                        }[feature]
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
        records = function_facts.parse_function_records(text)
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
            reason = (
                "macro definition is not an ordinary unrestricted function-like macro"
            )
        use["qualified"] = reason is None
        use["exclusion_reason"] = reason
        if reason is None:
            relations[member].add(definition["id"])
    return relations, definitions, uses


def _private_type_relations(root, resolved, connection, members, functions, read):
    manifest = lookup_target_manifest(root, resolved.id.value)
    if manifest is None:
        raise ValueError("type discovery requires the selected target manifest")
    uses = type_queries.type_usages_payload(
        connection,
        target=resolved.id.value,
        pattern=None,
        limit=_EDGE_LIMIT + 1,
        function_ids=members,
    )
    if len(uses) > _EDGE_LIMIT:
        raise ValueError("discovery pool exceeds 4096 indexed type uses")
    names = sorted({use["type_name"] for use in uses})
    definitions = []
    entities = {}
    occurrences = {resolved.id.value: [], "__shared__": []}
    for path, provenance in type_inputs.authored_type_headers(root, manifest):
        check_deadline()
        source = path.relative_to(root).as_posix()
        scope = resolved.id.value if provenance == "header_claim" else "__shared__"
        records = c_context.declaration_records(read(path).decode("utf-8"))
        occurrences[scope].extend(
            declarations.build_declaration_occurrences(
                records, origin=source, scope=scope
            )
        )
    for scope, rows in occurrences.items():
        for entity in declarations.reconcile_declarations(rows):
            entities[
                type_index._identity(
                    scope,
                    entity.key.namespace,
                    entity.key.name,
                    entity.key.origin,
                    str(entity.key.ordinal),
                )
            ] = entity
        definitions.extend(
            type_queries.types_payload(
                connection,
                target=scope,
                pattern=None,
                untyped=False,
                limit=_EDGE_LIMIT + 1,
                names=names,
            )
        )
    if len(definitions) > _EDGE_LIMIT:
        raise ValueError("discovery pool exceeds 4096 referenced type declarations")
    for definition in definitions:
        check_deadline()
        entity = entities.get(definition["id"])
        if entity is None:
            raise ValueError(
                "indexed type declaration lacks its current owner identity"
            )
        selected = entity.selected_definition or entity.representative
        expected_occurrence = (
            type_index._identity(
                definition["target_id"],
                entity.selected_definition.origin,
                str(entity.selected_definition.ordinal),
            )
            if entity.selected_definition is not None
            else None
        )
        expected = {
            "namespace": entity.key.namespace,
            "name": entity.key.name,
            "kind": entity.kind,
            "canonical": entity.canonical,
            "source_path": selected.origin,
            "complete": int(entity.complete),
            "diagnostic": entity.diagnostic,
            "type_expression": entity.type_expression,
            "alias_kind": entity.alias_kind,
            "definition_occurrence": expected_occurrence,
        }
        if any(definition[key] != value for key, value in expected.items()):
            raise ValueError(
                "indexed type declaration differs from its current owner facts"
            )
    relations = {member: set() for member in members}
    sources, closures, actual, tag_positions, offsets = {}, {}, {}, {}, {}
    for member in members:
        source = functions[member]["source"]
        if source in sources:
            continue
        raw = read(root / source)
        text = raw.decode("utf-8").replace("\r\n", "\n").replace("\r", "\n")
        sources[source] = raw
        records = function_facts.parse_function_records(text)
        identities = {
            functions[item]["address"]: item
            for item in members
            if functions[item]["source"] == source
        }
        actual[source] = type_usages._collect_occurrences(
            text, set(names), records, identities
        )
        offsets[source] = [0] + [match.end() for match in re.finditer("\n", text)]
        tag_positions[source] = {
            match.start(2): match.group(1)
            for match in re.finditer(
                r"\b(struct|union|enum)\s+([A-Za-z_]\w*)", c_context._mask_lexemes(text)
            )
        }
        closures[source] = {
            source,
            *(
                path.relative_to(root).as_posix()
                for path in include_facts.local_include_files(
                    root,
                    [root / source],
                    strict=True,
                    require_literal=True,
                    include_angle=True,
                    read_source=lambda path: read(path).decode("utf-8"),
                )
            ),
        }
    owned_headers = {
        path.relative_to(root).as_posix() for path in resolved.header_paths
    }
    for use in uses:
        check_deadline()
        member, source = use["function_id"], use["source_path"]
        if source != functions[member]["source"]:
            raise ValueError("indexed type use differs from its function source")
        evidence = json.loads(use["evidence"])
        positions = actual[source].get((member, use["type_name"]), [])
        expected = {
            "class": "lexical",
            "binding": "unresolved",
            "scope": "function",
            "positions": positions,
            "source_sha256": sha256(sources[source]).hexdigest(),
            "metadata_diagnostic": None,
        }
        if (
            use["use_kind"] != "lexical"
            or use["provenance"] != "source_claim"
            or use["subject"] != use["type_name"]
            or not positions
            or evidence != expected
        ):
            raise ValueError(
                "indexed type use differs from its fingerprint-bound lexical owner"
            )
        use["evidence"] = evidence
        use["declaration_candidates"] = []
        grouped = {}
        for position in positions:
            offset = offsets[source][position["line"] - 1] + position["column"] - 1
            kind = tag_positions[source].get(offset)
            grouped.setdefault(("tag" if kind else "ordinary", kind), []).append(
                position
            )
        for (namespace, kind), selected_positions in sorted(grouped.items()):
            matching = [
                item
                for item in definitions
                if item["name"] == use["type_name"]
                and item["namespace"] == namespace
                and (kind is None or item["kind"] == kind)
            ]
            visible = [
                item for item in matching if item["source_path"] in closures[source]
            ]
            for definition in matching:
                reason = None
                if (
                    definition["target_id"] != resolved.id.value
                    or definition["source_path"] not in owned_headers
                ):
                    reason = "type declaration is not private to the selected target"
                elif definition["source_path"] not in closures[source]:
                    reason = "type declaration is outside the source include closure"
                elif len(visible) != 1:
                    reason = (
                        "lexical namespace has multiple current declaration candidates"
                    )
                elif (
                    definition["review_status"] != "reviewed"
                    or definition["diagnostic"] is not None
                    or definition["kind"] not in {"typedef", "struct", "union", "enum"}
                ):
                    reason = "type declaration is unsupported or unreviewed"
                candidate = {
                    "declaration_id": definition["id"],
                    "namespace": namespace,
                    "positions": selected_positions,
                    "binding": "unresolved",
                    "qualified": reason is None,
                    "exclusion_reason": reason,
                }
                use["declaration_candidates"].append(candidate)
                if reason is None:
                    relations[member].add("type:" + definition["id"])
            if not matching:
                use["declaration_candidates"].append(
                    {
                        "declaration_id": None,
                        "namespace": namespace,
                        "positions": selected_positions,
                        "binding": "unresolved",
                        "qualified": False,
                        "exclusion_reason": "lexical namespace lacks a current declaration candidate",
                    }
                )
    return relations, definitions, uses


def _storage_relations(root, resolved, connection, members, functions, payload, binary):
    placeholders = ",".join("?" for _ in members)
    references = [
        dict(row)
        for row in connection.execute(
            "SELECT target_id, function_id, source, address, access_kind, opcode "
            f"FROM data_references WHERE target_id = ? AND function_id IN ({placeholders}) "
            "ORDER BY function_id, source, address, access_kind, opcode LIMIT ?",
            (resolved.id.value, *members, _EDGE_LIMIT + 1),
        )
    ]
    original = []
    for member in members:
        check_deadline()
        function = functions[member]
        start = payload.binary_offset + function["address"] - payload.load_address
        for offset, address, access, opcode in mips.data_references(
            binary[start : start + function["original_bytes"]]
        ):
            original.append(
                {
                    "function_id": member,
                    "source": function["address"] + offset,
                    "address": address,
                    "access_kind": access,
                    "opcode": opcode,
                }
            )
    if len(references) > _EDGE_LIMIT or len(original) > _EDGE_LIMIT:
        raise ValueError("discovery pool exceeds 4096 storage-reference facts")
    keys = ("function_id", "source", "address", "access_kind", "opcode")
    if {tuple(row[key] for key in keys) for row in references} != {
        tuple(row[key] for key in keys) for row in original
    }:
        raise ValueError(
            "indexed storage references differ from the original-range decoder"
        )
    candidates = type_queries.type_candidates_payload(
        connection,
        target=resolved.id.value,
        status=None,
        kind="storage",
        limit=_EDGE_LIMIT + 1,
        addresses=sorted({row["address"] for row in references}),
    )
    if (
        len(candidates) > _EDGE_LIMIT
        or sum(len(row["evidence"]) for row in candidates) > _EDGE_LIMIT
    ):
        raise ValueError("discovery pool exceeds 4096 storage candidate facts")
    by_address = {int(row["address"], 16): row for row in candidates}
    relations = {member: set() for member in members}
    for reference in references:
        check_deadline()
        reason = None
        candidate = by_address.get(reference["address"])
        if reference["access_kind"] not in {"load", "store"}:
            reason = "address materialization is not a memory access"
        elif candidate is None:
            reason = "type owner has no supported storage candidate for this access"
        else:
            if (
                candidate["id"]
                != f"{resolved.id.value}@{reference['address']:08X}:storage"
            ):
                raise ValueError(
                    "storage candidate differs from its target/address identity"
                )
            expected = {
                "function": reference["function_id"],
                "source": f"0x{reference['source']:08X}",
                "access": reference["access_kind"],
                "opcode": reference["opcode"],
                "evidence_class": "representation",
            }
            if expected not in candidate["evidence"]:
                raise ValueError("storage candidate lacks its indexed access evidence")
            relations[reference["function_id"]].add("storage:" + candidate["id"])
        reference["qualified"] = reason is None
        reference["exclusion_reason"] = reason
    return relations, references, original, candidates


def _require_subcategory_fields(value, fields, label):
    if not isinstance(value, dict) or set(value) != fields:
        raise ValueError(f"{label} requires exactly {', '.join(sorted(fields))}")


def _require_subcategory_text(value, label, *, identifier=False):
    if (
        not isinstance(value, str)
        or not value.strip()
        or value != value.strip()
        or len(value) > (80 if identifier else 4096)
        or (identifier and not re.fullmatch(r"[a-z][a-z0-9]*(?:-[a-z0-9]+)*", value))
    ):
        raise ValueError(f"invalid functional subcategory {label}")
    return value


def _collect_functional_relations(
    root, members, functions, read, path, expected_sha256
):
    raw = read(root / validate_repo_path(path))
    if sha256(raw).hexdigest() != expected_sha256:
        raise ValueError("functional subcategory input differs from its external SHA")
    document = json.loads(
        raw.decode("utf-8"), object_pairs_hook=harness_io.unique_object
    )
    _require_subcategory_fields(
        document, {"schema", "target", "subcategories", "members"}, "subcategory input"
    )
    if (
        document["schema"] != "bof3.combiner-subcategories/v1"
        or document["target"] != parse_function_id(members[0]).target.value
    ):
        raise ValueError(
            "functional subcategory schema or target differs from the pool"
        )
    categories, assignments = document["subcategories"], document["members"]
    if (
        not isinstance(categories, list)
        or len(categories) > _EDGE_LIMIT
        or not isinstance(assignments, list)
        or len(assignments) != len(members)
    ):
        raise ValueError("functional subcategories require a bounded exact member pool")
    by_category = {}
    for category in categories:
        check_deadline()
        _require_subcategory_fields(category, {"id", "description"}, "subcategory")
        identity = _require_subcategory_text(category["id"], "ID", identifier=True)
        _require_subcategory_text(category["description"], "description")
        if identity in by_category:
            raise ValueError("duplicate functional subcategory ID")
        by_category[identity] = category
    by_member = {}
    for assignment in assignments:
        check_deadline()
        _require_subcategory_fields(
            assignment,
            {
                "id",
                "source",
                "source_sha256",
                "original_sha256",
                "metadata_sha256",
                "implementation_sha256",
                "subcategories",
            },
            "subcategory member",
        )
        identity = assignment["id"]
        if not isinstance(identity, str) or identity not in members:
            raise ValueError("subcategory member is not a canonical ID in the pool")
        if identity in by_member:
            raise ValueError("duplicate functional subcategory member")
        by_member[identity] = assignment
    if set(by_member) != set(members):
        raise ValueError(
            "functional subcategory membership differs from the exact pool"
        )
    relations, evidence, count = {}, [], 0
    for member in members:
        check_deadline()
        function = functions[member]
        source = function["source"]
        raw = read(root / source)
        text = raw.decode("utf-8")
        record = next(
            item
            for item in function_facts.parse_function_records(text)
            if item.address == function["address"]
        )
        if record.kind != "function":
            raise ValueError(
                "functional subcategories require an ordinary C function body"
            )
        binding = {
            "source": source,
            "source_sha256": sha256(raw).hexdigest(),
            "original_sha256": function["original_sha256"],
            "metadata_sha256": sha256(record.metadata.encode("utf-8")).hexdigest(),
            "implementation_sha256": sha256(
                text[record.implementation_start : record.implementation_end].encode(
                    "utf-8"
                )
            ).hexdigest(),
        }
        assignment = by_member[member]
        if any(assignment[key] != value for key, value in binding.items()):
            raise ValueError(f"functional subcategory binding is stale: {member}")
        selected = assignment["subcategories"]
        if not isinstance(selected, list):
            raise ValueError("functional member subcategories must be a list")
        count += len(selected)
        if count > _EDGE_LIMIT:
            raise ValueError("functional pool exceeds 4096 subcategory assignments")
        relations[member] = set()
        for category in selected:
            check_deadline()
            _require_subcategory_fields(
                category, {"id", "rationale"}, "subcategory assignment"
            )
            identity = _require_subcategory_text(category["id"], "ID", identifier=True)
            _require_subcategory_text(category["rationale"], "rationale")
            if identity not in by_category or identity in relations[member]:
                raise ValueError("unknown or duplicate functional member subcategory")
            relations[member].add(identity)
        evidence.append(
            {
                **assignment,
                "subcategories": sorted(selected, key=lambda item: item["id"]),
                "metadata": record.metadata,
                "metadata_range": [record.metadata_start, record.metadata_end],
                "implementation_range": [
                    record.implementation_start,
                    record.implementation_end,
                ],
                "binding_current": True,
                "semantic_verified": False,
            }
        )
    include_facts.local_include_files(
        root,
        [root / functions[member]["source"] for member in members],
        strict=True,
        require_literal=True,
        include_angle=True,
        read_source=lambda path: read(path).decode("utf-8"),
    )
    return relations, [by_category[key] for key in sorted(by_category)], evidence


def discover_pairs(
    root: Path,
    target: str,
    selectors: list[str],
    *,
    method: str = "helper-call-cluster",
    minimum_members: int = 2,
    minimum_shared_callees: int = 1,
    minimum_shared_macros: int = 1,
    minimum_shared_relations: int = 1,
    minimum_shared_subcategories: int = 1,
    subcategories: str | None = None,
    expected_subcategories_sha256: str | None = None,
    top_n: int = 1,
    overlap_policy: str = "retain-all",
    expected_fingerprint: str | None = None,
) -> dict:
    """Rank every unordered pair in an explicit target-local reviewed C pool."""
    check_deadline()
    if (
        method
        not in {
            "helper-call-cluster",
            "macro-family",
            "shared-state-type",
            "functional-subcategory",
        }
        or overlap_policy != "retain-all"
    ):
        raise ValueError("unsupported discovery method or overlap policy")
    if type(minimum_members) is not int or minimum_members != 2:
        raise ValueError(
            "combiner discovery enumerates pairs; minimum members must be two"
        )
    _require_integer(minimum_shared_callees, "minimum shared callees", _EDGE_LIMIT)
    _require_integer(minimum_shared_macros, "minimum shared macros", _EDGE_LIMIT)
    _require_integer(minimum_shared_relations, "minimum shared relations", _EDGE_LIMIT)
    _require_integer(
        minimum_shared_subcategories, "minimum shared subcategories", _EDGE_LIMIT
    )
    if any(
        value != 1 and method != owner
        for owner, value in (
            ("helper-call-cluster", minimum_shared_callees),
            ("macro-family", minimum_shared_macros),
            ("shared-state-type", minimum_shared_relations),
            ("functional-subcategory", minimum_shared_subcategories),
        )
    ):
        raise ValueError("discovery rejects a threshold for another method")
    if method == "functional-subcategory":
        validate_repo_path(subcategories)
        if not isinstance(expected_subcategories_sha256, str) or not re.fullmatch(
            r"[0-9a-f]{64}", expected_subcategories_sha256
        ):
            raise ValueError("functional subcategories require an external SHA-256 pin")
    elif subcategories is not None or expected_subcategories_sha256 is not None:
        raise ValueError("subcategory input belongs only to functional-subcategory")
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
        elif method == "functional-subcategory":
            for module in (
                function_facts,
                tags,
                lexicon,
                include_facts,
                harness_io,
            ):
                read(Path(module.__file__))
        elif method == "shared-state-type":
            for module in (
                type_index,
                type_inputs,
                type_queries,
                type_representation,
                type_usages,
                c_context,
                declarations,
                include_facts,
                mips,
                lexicon,
            ):
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
        functional_categories, functional_assignments = [], []
        (
            type_definitions,
            lexical_uses,
            references,
            original_references,
            storage_candidates,
        ) = [], [], [], [], []
        if method == "macro-family":
            direct_callees, macro_definitions, macro_uses = _macro_relations(
                root,
                resolved,
                connection,
                members,
                functions,
                read,
            )
        elif method == "functional-subcategory":
            direct_callees, functional_categories, functional_assignments = (
                _collect_functional_relations(
                    root,
                    members,
                    functions,
                    read,
                    subcategories,
                    expected_subcategories_sha256,
                )
            )
        elif method == "shared-state-type":
            direct_callees, type_definitions, lexical_uses = _private_type_relations(
                root,
                resolved,
                connection,
                members,
                functions,
                read,
            )
            storage, references, original_references, storage_candidates = (
                _storage_relations(
                    root,
                    resolved,
                    connection,
                    members,
                    functions,
                    payload,
                    binary,
                )
            )
            direct_callees = {
                member: direct_callees[member] | storage[member] for member in members
            }
        validate_index(connection, root)
        for name in sorted(pins):
            state, _ = inputs.read(
                root / name, max_bytes=_IMAGE_LIMIT, single_link=True
            )
            if state != pins[name]:
                raise ValueError(f"discovery input changed during enumeration: {name}")
    minimum_shared, feature = {
        "helper-call-cluster": (minimum_shared_callees, "shared_callees"),
        "macro-family": (minimum_shared_macros, "shared_macros"),
        "shared-state-type": (minimum_shared_relations, "shared_relations"),
        "functional-subcategory": (
            minimum_shared_subcategories,
            "shared_subcategories",
        ),
    }[method]
    pairs, ranking, selection = _rank_pairs(
        members,
        direct_callees,
        minimum_shared,
        top_n,
        feature=feature,
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
    elif method == "functional-subcategory":
        result.pop("minimum_shared_callees")
        result.update(
            minimum_shared_subcategories=minimum_shared_subcategories,
            subcategories=functional_categories,
            subcategory_assignments=functional_assignments,
            classification_policy={
                "authority": "authored functional classification leads requiring independent review",
                "binding": "current source, attached metadata, ordinary implementation and complete original-range hashes",
                "coverage": "complete fixed classification pool, not complete semantic or native coverage",
            },
        )
        result["fingerprint"] = digest(
            {key: value for key, value in result.items() if key != "fingerprint"}
        )
    elif method == "shared-state-type":
        result.pop("minimum_shared_callees")
        for pair in pairs:
            pair["shared_type_declarations"] = [
                item.removeprefix("type:")
                for item in pair["shared_relations"]
                if item.startswith("type:")
            ]
            pair["shared_storage_candidates"] = [
                item.removeprefix("storage:")
                for item in pair["shared_relations"]
                if item.startswith("storage:")
            ]
        result.update(
            minimum_shared_relations=minimum_shared_relations,
            type_declarations=type_definitions,
            type_usages=lexical_uses,
            data_references=references,
            original_data_references=original_references,
            storage_candidates=storage_candidates,
            relation_policy={
                "types": "target-private declaration spellings; lexical binding remains unresolved",
                "storage": "exact decoded load/store addresses; object identity, extent and semantics remain unproven",
                "coverage": "complete fixed pool and decoder relation, not complete memory-access or semantic coverage",
            },
        )
        result["ranking_fingerprint"] = digest(pairs)
        result["index_snapshot_fingerprint"] = digest(
            {
                "target": index_target,
                "functions": result["functions"],
                "calls": calls,
                "unresolved_calls": unresolved,
                "type_declarations": type_definitions,
                "type_usages": lexical_uses,
                "data_references": references,
                "storage_candidates": storage_candidates,
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
