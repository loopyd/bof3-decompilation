"""Pinned, reviewed pointer-cell and pointee contracts for the linked type route."""

from __future__ import annotations

import json
import re
from pathlib import Path

from harness.common.digests import digest
from harness.common.directory import validate_repo_path
from harness.common.inputs import InputBatch
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.domain.includes import local_include_files
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.registry import resolve_function
from harness.domain.receipts import (
    _nonempty_strings,
    _observations as candidate_observations,
)
from harness.io import unique_object
from harness.types.review import SCHEMA as CANDIDATE_SCHEMA

REQUEST_SCHEMA = "bof3.type-transaction-request/v3"
MANIFEST_SCHEMA = "bof3.type-transaction/v3"
SCHEMA = "bof3.type-linked-candidate/v1"
CONTRACT = "bof3.type-linked-storage-layout/v1"
TARGET = "emi/battle/battle/15"
HEADER = "include/bof3/battle/battle15_internal.h"
STORAGE_ID = f"{TARGET}@1F800044:storage"
AGGREGATE_ID = f"{TARGET}@1F800044:aggregate_region"
RELATION = "pointer-cell-pointee"
_LIMIT = 4 * 1024 * 1024
_SCALARS = frozenset(
    {
        "char",
        "signed char",
        "unsigned char",
        "short",
        "short int",
        "signed short",
        "signed short int",
        "unsigned short",
        "unsigned short int",
        "int",
        "signed",
        "signed int",
        "unsigned",
        "unsigned int",
        "long",
        "long int",
        "signed long",
        "signed long int",
        "unsigned long",
        "unsigned long int",
        "float",
        "double",
        "long double",
    }
)


def _closed(value, names: str, label: str) -> dict:
    if not isinstance(value, dict) or set(value) != set(names.split()):
        raise ValueError(f"invalid linked type {label} keys")
    return value


def _text(value, label: str) -> str:
    if (
        not isinstance(value, str)
        or not value.strip()
        or value != value.strip()
        or value.casefold() in {"unknown", "unresolved", "todo", "?"}
    ):
        raise ValueError(f"linked type {label} must be a resolved fact")
    return value


def _strings(value, label: str, *, empty: bool = False) -> list[str]:
    if not isinstance(value, list) or (not value and not empty):
        raise ValueError(f"linked type {label} must be a string array")
    for item in value:
        _text(item, label)
    if len({item.casefold() for item in value}) != len(value):
        raise ValueError(f"linked type {label} must be unique")
    return value


def _integer(value, label: str, *, minimum: int = 1, maximum: int = 0xFFFFFFFF) -> int:
    if type(value) is not int or not minimum <= value <= maximum:
        raise ValueError(f"invalid linked type {label}")
    return value


def _observations(value, candidate: dict, label: str) -> None:
    known = {item["id"] for item in candidate["observations"]}
    if not set(_strings(value, label)) <= known:
        raise ValueError(f"linked type {label} contains an unreviewed observation")


def _json(content: bytes) -> dict:
    try:
        value = json.loads(content.decode("utf-8"), object_pairs_hook=unique_object)
    except (ValueError, UnicodeDecodeError, RecursionError) as error:
        raise ValueError("linked type artifact is not bounded JSON") from error
    if not isinstance(value, dict):
        raise ValueError("linked type artifact must be an object")
    return value


def _read(batch: InputBatch, root: Path, name: object) -> tuple[dict, bytes]:
    path = validate_repo_path(name)
    state, content = batch.read(root / path, max_bytes=_LIMIT)
    if state is None or not content.strip():
        raise ValueError(f"linked type artifact missing or empty: {path}")
    return {"path": path, "sha256": state["sha256"]}, content


def _candidate_projection(value: object) -> dict:
    """Bind the existing candidate normalization without rereading PRE owners."""
    if not isinstance(value, dict):
        raise ValueError("linked type role requires a complete candidate")
    fingerprints = value.get("fingerprints")
    missing = value.get("missing_facts")
    receipts = value.get("receipts")
    if (
        not isinstance(fingerprints, dict)
        or any(
            not isinstance(name, str) or not isinstance(sha256, str)
            for name, sha256 in fingerprints.items()
        )
        or not isinstance(missing, list)
        or any(not isinstance(item, str) or not item.strip() for item in missing)
        or not isinstance(receipts, list)
    ):
        raise ValueError("linked type role candidate evidence is malformed")
    result = {
        key: value.get(key)
        for key in (
            "schema",
            "id",
            "kind",
            "status",
            "target",
            "address",
            "end",
            "authority",
        )
    }
    result.update(
        owners=_nonempty_strings(value.get("owners"), "owners"),
        locations=_nonempty_strings(value.get("locations"), "locations"),
        fingerprints={name: sha256.lower() for name, sha256 in fingerprints.items()},
        provenance=_nonempty_strings(value.get("provenance"), "provenance"),
        observations=candidate_observations(value.get("observations")),
        missing_facts=sorted(set(missing)),
        receipts=receipts,
    )
    return result


def validate_request(request: dict) -> None:
    _closed(
        {k: v for k, v in request.items() if k != "adopted_baseline"},
        "schema target concern header index_candidate_ids candidate_artifacts affected_functions linked_pair",
        "request",
    )
    if (
        request["schema"] != REQUEST_SCHEMA
        or request["concern"] != "linked"
        or request["target"] != TARGET
        or request["header"] != HEADER
    ):
        raise ValueError(
            "linked type route requires the frozen target and private header"
        )
    ids = _strings(request["index_candidate_ids"], "index identities")
    if len(ids) != 2 or set(ids) != {STORAGE_ID, AGGREGATE_ID}:
        raise ValueError("linked type route requires exactly both frozen identities")
    paths = _strings(request["candidate_artifacts"], "candidate artifacts")
    if len(paths) != 2:
        raise ValueError("linked type route requires one artifact per role")
    pair = _closed(
        request["linked_pair"],
        "storage_id aggregate_id relation artifact artifact_sha256 artifact_digest",
        "request pair",
    )
    if (pair["storage_id"], pair["aggregate_id"], pair["relation"]) != (
        STORAGE_ID,
        AGGREGATE_ID,
        RELATION,
    ):
        raise ValueError("linked type request relation or identities differ")
    validate_repo_path(pair["artifact"])
    if (
        not isinstance(pair["artifact_sha256"], str)
        or not re.fullmatch(r"[0-9a-f]{64}", pair["artifact_sha256"])
        or not isinstance(pair["artifact_digest"], str)
        or not re.fullmatch(r"v1:[0-9a-f]{64}", pair["artifact_digest"])
    ):
        raise ValueError("linked type request requires external artifact pins")


def _owner(root: Path, value, manifests: dict) -> str:
    owner = _closed(value, "target paths", "owner")
    target = _text(owner["target"], "owner target")
    if normalize_target_id(target).value != target or target not in manifests:
        raise ValueError("linked type owner target is not canonical")
    manifest = manifests[target]
    claimed = {
        *manifest.sources,
        *manifest.support_sources,
        *manifest.headers,
        manifest.binary,
        manifest.splat,
        f"config/targets/{target}/target.toml",
        f"config/targets/{target}/symbols.txt",
        f"config/targets/{target}/reviewed.rz",
    }
    with InputBatch(root) as batch:
        for name in _strings(owner["paths"], "owner paths"):
            name = validate_repo_path(name)
            if (
                name not in claimed
                or batch.read(root / name, max_bytes=64 * 1024 * 1024)[0] is None
            ):
                raise ValueError("linked type owner path is not manifest-backed")
    return target


def _cell(value, candidate: dict) -> None:
    cell = _closed(
        value,
        "owner address end width alignment pointer_depth cell_qualifiers pointee_qualifiers initialization lifetime observations",
        "cell",
    )
    for key in ("address", "end", "width", "pointer_depth", "alignment"):
        _integer(cell[key], f"cell {key}")
    if (cell["address"], cell["end"], cell["width"], cell["pointer_depth"]) != (
        0x1F800044,
        0x1F800048,
        4,
        1,
    ) or cell["address"] % cell["alignment"]:
        raise ValueError(
            "linked type cell geometry differs from the frozen pointer cell"
        )
    for key in ("cell_qualifiers", "pointee_qualifiers"):
        if not set(_strings(cell[key], key, empty=True)) <= {"const", "volatile"}:
            raise ValueError("linked type cell qualifiers are unsupported")
    _text(cell["initialization"], "cell initialization")
    _text(cell["lifetime"], "cell lifetime")
    _observations(cell["observations"], candidate, "cell observations")


def _object(
    root: Path, value, candidate: dict, manifests: dict
) -> tuple[set[str], bool]:
    obj = _closed(
        value, "owner base extent alignment fields padding observations", "object"
    )
    extent = _integer(obj["extent"], "object extent")
    alignment = _integer(obj["alignment"], "object alignment")
    _observations(obj["observations"], candidate, "object observations")
    base = obj["base"]
    targets = {_owner(root, obj["owner"], manifests)}
    unconditional = False
    if isinstance(base, dict) and base.get("kind") == "fixed":
        _closed(base, "kind address", "fixed base")
        address = _integer(base["address"], "fixed base address", minimum=0)
        if address + extent > 0x100000000 or address % alignment:
            raise ValueError(
                "linked type object does not fit its address space or alignment"
            )
        manifest = manifests[obj["owner"]["target"]]
        layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
        boundary = layout.find_containing_boundary(address)
        if (
            boundary is None
            or boundary.virtual_end is None
            or address + extent > boundary.virtual_end
        ):
            raise ValueError(
                "linked type fixed object does not fit a reviewed owner range"
            )
    elif isinstance(base, dict) and base.get("kind") == "runtime":
        _closed(
            base,
            "kind storage_id indirection producer entry_conditions observations",
            "runtime base",
        )
        if (
            base["storage_id"] != STORAGE_ID
            or type(base["indirection"]) is not int
            or base["indirection"] != 1
        ):
            raise ValueError(
                "linked type runtime base must dereference the frozen storage cell once"
            )
        producer = _text(base["producer"], "runtime producer")
        identity = parse_function_id(producer)
        if producer != f"{identity.target.value}@0x{identity.address:08X}":
            raise ValueError("linked type runtime producer must be canonical")
        resolve_function(root, producer)
        targets.add(identity.target.value)
        unconditional = not _strings(
            base["entry_conditions"], "runtime entries", empty=True
        )
        _observations(base["observations"], candidate, "runtime observations")
    else:
        raise ValueError("linked type object requires a reviewed fixed or runtime base")
    spans = []
    for key in ("fields", "padding"):
        if not isinstance(obj[key], list):
            raise ValueError(f"linked type object {key} must be an array")
        for span in obj[key]:
            _closed(
                span,
                "offset width type signedness role observations"
                if key == "fields"
                else "offset width observations",
                key,
            )
            offset = _integer(span["offset"], "span offset", minimum=0)
            width = _integer(span["width"], "span width")
            _observations(span["observations"], candidate, "span observations")
            if key == "fields":
                _text(span["type"], "field type")
                _text(span["role"], "field role")
                if span["signedness"] not in ("signed", "unsigned", "not-applicable"):
                    raise ValueError("linked type field signedness is unresolved")
            spans.append((offset, offset + width))
    position = 0
    for start, end in sorted(spans):
        if start != position or end > extent:
            raise ValueError(
                "linked type fields and padding must cover the complete extent without overlap"
            )
        position = end
    if not obj["fields"] or position != extent or extent % alignment:
        raise ValueError("linked type object layout is incomplete or misaligned")
    return targets, unconditional


def capture_pair(root: Path, request: dict, reviewed: list[dict]) -> dict:
    """Check the externally pinned joint record against both complete role proofs."""
    validate_request(request)
    proofs = {item["index_id"]: item for item in reviewed}
    if len(reviewed) != 2 or set(proofs) != {STORAGE_ID, AGGREGATE_ID}:
        raise ValueError("linked type manifest requires exactly both reviewed roles")
    pin = request["linked_pair"]
    with InputBatch(root) as batch:
        artifact, content = _read(batch, root, pin["artifact"])
        record = _json(content)
        _closed(
            record, "schema target storage aggregate link review digest", "joint record"
        )
        if (
            artifact["sha256"] != pin["artifact_sha256"]
            or record["schema"] != SCHEMA
            or record["target"] != TARGET
            or record["digest"] != pin["artifact_digest"]
            or digest({k: v for k, v in record.items() if k != "digest"})
            != record["digest"]
        ):
            raise ValueError(
                "linked type joint artifact differs from its external pins"
            )
        inputs = [artifact]
        candidates = {}
        for role, index_id, representation in (
            ("storage", STORAGE_ID, "cell"),
            ("aggregate", AGGREGATE_ID, "object"),
        ):
            part = _closed(record[role], f"binding {representation} semantics", role)
            binding = _closed(
                part["binding"],
                "index_id index_row_digest artifact artifact_sha256 artifact_digest",
                "role binding",
            )
            proof = proofs[index_id]
            ref, raw = _read(batch, root, proof["artifact"])
            value = _json(raw)
            expected = {
                "index_id": index_id,
                "index_row_digest": value.get("index_row_digest"),
                "artifact": ref["path"],
                "artifact_sha256": ref["sha256"],
                "artifact_digest": value.get("digest"),
            }
            candidate = proof["candidate"]
            if (
                binding != expected
                or ref["sha256"] != proof["artifact_sha256"]
                or value.get("schema") != CANDIDATE_SCHEMA
                or value.get("index_id") != index_id
                or value.get("digest")
                != digest({k: v for k, v in value.items() if k != "digest"})
                or candidate != _candidate_projection(value.get("candidate"))
                or candidate["target"] != TARGET
                or candidate["address"] != 0x1F800044
                or candidate["end"] != 0x1F800048
                or candidate["status"] != "accepted"
                or candidate["missing_facts"]
                or len(candidate["observations"]) < 2
                or "original_bytes" not in candidate["provenance"]
                or part[representation] != proof["representation"]
                or part["semantics"] != proof["semantics"]
                or value["representation"].get("contract") != proof["representation"]
                or value["semantics"].get("contract") != proof["semantics"]
                or value["review"] != proof["review"]
            ):
                raise ValueError(
                    "linked type role binding or physical contract differs"
                )
            semantics = _closed(
                part["semantics"], "role observations", "role semantics"
            )
            _text(semantics["role"], "role semantics")
            _observations(semantics["observations"], candidate, "semantic observations")
            candidates[role] = candidate
            inputs.append(ref)
        review = _closed(
            record["review"],
            "verdict reviewer artifact artifact_sha256",
            "joint review",
        )
        _text(review["reviewer"], "joint reviewer")
        ref, review_bytes = _read(batch, root, review["artifact"])
        if (
            review["verdict"] != "accepted"
            or ref["sha256"] != review["artifact_sha256"]
        ):
            raise ValueError("linked type joint review is not accepted or drifted")
        inputs.append(ref)
    _cell(record["storage"]["cell"], candidates["storage"])
    manifests = load_target_manifests(root)
    targets = {_owner(root, record["storage"]["cell"]["owner"], manifests)}
    other_targets, unconditional = _object(
        root, record["aggregate"]["object"], candidates["aggregate"], manifests
    )
    targets.update(other_targets)
    link = _closed(
        record["link"],
        "relation storage_id aggregate_id storage_artifact_digest aggregate_artifact_digest entry_conditions observations",
        "relation",
    )
    if (
        link["relation"] != RELATION
        or link["storage_id"] != STORAGE_ID
        or link["aggregate_id"] != AGGREGATE_ID
        or link["storage_artifact_digest"]
        != record["storage"]["binding"]["artifact_digest"]
        or link["aggregate_artifact_digest"]
        != record["aggregate"]["binding"]["artifact_digest"]
    ):
        raise ValueError("linked type relation does not bind both accepted role proofs")
    unconditional |= not _strings(link["entry_conditions"], "link entries", empty=True)
    observations = _closed(
        link["observations"], "storage aggregate", "link observations"
    )
    for role in candidates:
        _observations(observations[role], candidates[role], f"link {role} observations")
    if unconditional and b"unconditional entry coverage" not in review_bytes.lower():
        raise ValueError(
            "linked type unconditional entry coverage requires explicit joint review"
        )
    if len({ref["path"] for ref in inputs}) != 4:
        raise ValueError(
            "linked type role, joint and review artifacts must be distinct"
        )
    return {
        "contract_revision": CONTRACT,
        "record": record,
        "inputs": sorted(inputs, key=lambda ref: ref["path"]),
        "proof_targets": sorted(targets),
    }


def validate_consumers(
    root: Path, connection, rows: list[dict], functions: list[dict], pair: dict
) -> None:
    """Require original indexed uses and all live transitive header consumers."""
    required = set()
    for row in rows:
        for item in row["evidence"]:
            if not isinstance(item, dict):
                raise ValueError("linked type indexed consumer evidence is unsupported")
            values = ([item["function"]] if "function" in item else []) + item.get(
                "independent_consumers", []
            )
            for value in values:
                identity = parse_function_id(value)
                required.add(f"{identity.target.value}@0x{identity.address:08X}")
    with InputBatch(root) as batch:

        def read_source(path):
            state, raw = batch.read(path, max_bytes=_LIMIT)
            if state is None:
                raise ValueError("linked type declaration consumer source is missing")
            return raw.decode("utf-8")

        for row in connection.execute(
            "SELECT target_id,address,source FROM functions WHERE source IS NOT NULL"
        ):
            target, address, source = row
            path = Path(source)
            if not path.is_absolute():
                path = root / validate_repo_path(source)
            if path == root / HEADER or root / HEADER in local_include_files(
                root,
                [path],
                strict=True,
                require_literal=True,
                include_angle=True,
                read_source=read_source,
            ):
                required.add(f"{target}@0x{address:08X}")
    if not required <= {item["selector"] for item in functions}:
        raise ValueError(
            "linked type affected_functions omit current original or declaration consumers"
        )
    for field in pair["record"]["aggregate"]["object"]["fields"]:
        name = field["type"]
        if (
            name not in _SCALARS
            and connection.execute(
                "SELECT 1 FROM type_declarations WHERE target_id IN (?, '__shared__') "
                "AND (name=? OR kind || ' ' || name=?) AND diagnostic IS NULL AND complete=1",
                (TARGET, name, name),
            ).fetchone()
            is None
        ):
            raise ValueError(
                "linked type field requires an existing resolved declaration"
            )


def validate_manifest(root: Path, manifest: dict) -> None:
    if (
        manifest.get("target") != TARGET
        or manifest.get("targets") != [TARGET]
        or manifest.get("concern") != "linked"
        or manifest.get("header") != HEADER
        or manifest.get("private_transaction_proofs") != []
        or manifest.get("linked_pair")
        != capture_pair(root, manifest["request"], manifest["reviewed_candidates"])
    ):
        raise ValueError("linked type manifest pair or scope differs")


def summaries(manifest: dict) -> dict:
    try:
        record = manifest["linked_pair"]["record"]
        return {
            "representation": {
                "storage": record["storage"]["cell"],
                "aggregate": record["aggregate"]["object"],
                "link": record["link"],
            },
            "semantics": {
                "storage": record["storage"]["semantics"],
                "aggregate": record["aggregate"]["semantics"],
                "link": record["link"],
            },
        }
    except (KeyError, TypeError) as error:
        raise ValueError("linked type manifest requires both complete roles") from error


def validate_application(value: object) -> None:
    if not isinstance(value, dict):
        return
    manifest = value.get("manifest")
    if not isinstance(manifest, dict) or manifest.get("schema") != MANIFEST_SCHEMA:
        return
    if (
        value.get("target") != TARGET
        or value.get("concern") != "linked"
        or any(
            value.get(key) != expected for key, expected in summaries(manifest).items()
        )
    ):
        raise ValueError(
            "linked type application must retain both complete role summaries and relation"
        )


def validate_participation(
    manifest: dict, implementation_run_id, participating_targets
) -> None:
    if manifest["schema"] != MANIFEST_SCHEMA:
        return
    if not implementation_run_id:
        raise ValueError(
            "linked type application requires captured implementation context"
        )
    record = manifest["linked_pair"]["record"]
    reviewers = {
        record["review"]["reviewer"],
        *(item["review"]["reviewer"] for item in manifest["reviewed_candidates"]),
    }
    if implementation_run_id in reviewers:
        raise ValueError("linked type implementation cannot review its own candidates")
    if not set(manifest["linked_pair"]["proof_targets"]) <= set(
        participating_targets or manifest["targets"]
    ):
        raise ValueError(
            "linked type additional proof owners require explicit participation"
        )


def validate_parent(root: Path, application: object, parent: object) -> None:
    if not isinstance(application, dict):
        return
    manifest = application.get("manifest")
    if not isinstance(manifest, dict) or manifest.get("schema") != MANIFEST_SCHEMA:
        return
    validate_application(application)
    if not isinstance(parent, dict) or not isinstance(
        parent.get("review_artifact"), dict
    ):
        raise ValueError("linked type parent requires a review artifact")
    ref = parent["review_artifact"]
    name = ref.get("path")
    if (
        not isinstance(name, str)
        or not Path(name).is_absolute()
        or str(Path(name).resolve()) != name
    ):
        raise ValueError("linked type parent artifact must be canonical absolute")
    path = Path(name)
    with InputBatch(path.parent) as batch:
        state, content = batch.read(path, max_bytes=_LIMIT)
    if (
        state is None
        or state["sha256"] != ref.get("sha256")
        or any(
            value.encode() not in content
            for value in (
                STORAGE_ID,
                AGGREGATE_ID,
                application["manifest"]["linked_pair"]["record"]["digest"],
            )
        )
    ):
        raise ValueError(
            "linked type parent artifact must explicitly cover both frozen IDs and the joint digest"
        )


def validate_envelopes(root: Path, value: object) -> None:
    """Retain paired summaries and parent scope through private history adapters."""
    if not isinstance(value, dict):
        return
    if "application" in value and "parent_review" in value:
        validate_parent(root, value["application"], value["parent_review"])
    if "prerequisite" in value:
        validate_envelopes(root, value["prerequisite"])
        pins = value.get("intervening")
        if isinstance(pins, list):
            for pin in pins:
                if isinstance(pin, dict):
                    validate_envelopes(root, pin.get("envelope"))
    if "revalidation" in value:
        validate_envelopes(root, value["revalidation"])
        record = value["revalidation"]
        if isinstance(record, dict) and isinstance(record.get("prerequisite"), dict):
            validate_parent(
                root,
                record["prerequisite"].get("application"),
                value.get("parent_review"),
            )
