"""Problem and Answer document payloads for the preparation seam.

The problem payload is built field by field from revalidated facts only; the
field inventory below is a structural allowlist checked before sealing, so no
authored name, signature, source locator, map, profile or result can reach the
generator input.  Authored material is written into the evaluator-only answer.
"""

from __future__ import annotations

from harness.reconstruction import contract
from harness.reconstruction.authored import (
    UNRESOLVED_CATEGORIES,
    AuthoredMember,
    ExclusionRecord,
)
from harness.reconstruction.baseline import MemberFacts

ALLOWED_PROBLEM_KEYS = frozenset(
    {
        "admitted",
        "address",
        "baseline",
        "bundle",
        "bytes",
        "case_id",
        "count",
        "end",
        "entries",
        "exclusions",
        "expected_bundle",
        "file",
        "format",
        "id",
        "image_id",
        "images",
        "key",
        "kind",
        "label",
        "load_address",
        "membership",
        "members",
        "milestone",
        "path",
        "payload_offset",
        "payload_size",
        "range",
        "range_sha256",
        "rules",
        "rules_id",
        "schema",
        "sha256",
        "shipped",
        "source",
        "start",
        "target",
        "targets",
    }
)


def check_problem_inventory(document: object, path: str = "problem") -> None:
    """Refuse a problem field the reviewed record does not allow at any depth."""

    if isinstance(document, dict):
        for key, value in document.items():
            contract.require(
                key in ALLOWED_PROBLEM_KEYS,
                f"problem field is not allowed at {path}: {key!r}",
            )
            check_problem_inventory(value, f"{path}.{key}")
    elif isinstance(document, list):
        for index, value in enumerate(document):
            check_problem_inventory(value, f"{path}[{index}]")


def problem_payload(
    baseline: dict, members: list[MemberFacts], rules_path: str, rules_bytes: bytes
) -> tuple[dict, dict]:
    """Build the sanitized generator input and the bundle files it binds."""

    files: dict[str, bytes] = {rules_path: rules_bytes}
    images: dict[str, dict] = {}
    image_digests: dict[str, str] = {}
    targets: dict[str, dict] = {}
    for fact in members:
        image = fact.image
        if image.path not in images:
            image_id = f"img/{image.sha256[:12]}"
            prior = image_digests.get(image_id)
            contract.require(
                prior is None or prior == image.sha256,
                f"image identity collides for {image.path}",
            )
            image_digests[image_id] = image.sha256
            relative = f"images/{image.sha256[:12]}.bin"
            files[relative] = image.content
            images[image.path] = {
                "image_id": image_id,
                "label": image.label,
                "sha256": image.sha256,
                "bytes": len(image.content),
                "format": image.format,
                "load_address": image.load_address,
                "payload_offset": image.payload_offset,
                "payload_size": image.payload_size,
                "file": contract.file_ref("problem", relative, image.content),
            }
        image_document = images[image.path]
        target_document = targets.get(fact.target)
        if target_document is None:
            targets[fact.target] = {
                "target": fact.target,
                "kind": "executable" if fact.target.startswith("exe/") else "emi",
                "shipped": None,
                "image_id": image_document["image_id"],
            }
        else:
            contract.require(
                target_document["image_id"] == image_document["image_id"],
                f"target maps to more than one image: {fact.target}",
            )
    document = {
        "schema": contract.PROBLEM_SCHEMA,
        "milestone": {
            "id": contract.MILESTONE_ID,
            "admitted": contract.ADMITTED,
            "exclusions": contract.EXCLUSIONS,
        },
        "baseline": {
            "schema": contract.BASELINE_SCHEMA,
            "sha256": contract.PINNED_BASELINE_SHA256,
            "admitted": contract.ADMITTED,
            "exclusions": contract.EXCLUSIONS,
        },
        "membership": {"count": contract.ADMITTED, "exclusions": contract.EXCLUSIONS},
        "images": sorted(images.values(), key=lambda entry: entry["image_id"]),
        "targets": sorted(targets.values(), key=lambda entry: entry["target"]),
        "members": [
            {
                "key": fact.key,
                "case_id": fact.case_id,
                "target": fact.target,
                "address": fact.address,
                "range": {"start": fact.address, "end": fact.end},
                "range_sha256": fact.range_sha256,
                "image_id": images[fact.image.path]["image_id"],
            }
            for fact in members
        ],
        "rules": {
            "rules_id": contract.validate_tool_id(contract.RULES_ID, "rules id"),
            "source": contract.file_ref("problem", rules_path, rules_bytes),
            "entries": [],
        },
    }
    _check_membership(baseline, document)
    check_problem_inventory(document)
    return document, files


def _check_membership(baseline: dict, document: dict) -> None:
    contract.require(
        len(document["members"]) == len(baseline["members"]),
        "problem membership does not match the corpus",
    )
    contract.require("exclusions" not in document, "problem must not carry exclusions")
    keys = [member["key"] for member in document["members"]]
    contract.require(len(keys) == contract.ADMITTED, "problem membership count differs")
    contract.require(keys == sorted(keys), "problem membership is not sorted")
    contract.require(len(set(keys)) == len(keys), "problem membership repeats a key")
    images = {image["image_id"] for image in document["images"]}
    for member in document["members"]:
        contract.require(
            member["image_id"] in images, f"member image is unresolved: {member['key']}"
        )
        contract.require(
            member["address"] == member["range"]["start"]
            and member["range"]["end"] > member["range"]["start"],
            f"member range is invalid: {member['key']}",
        )


def _obligations(fact: MemberFacts) -> dict:
    unresolved = [
        {
            "category": category,
            "address": None,
            "reason": (
                f"{category} obligations are not independently derived during preparation; "
                "independent evaluator verification owns this category"
            ),
        }
        for category in UNRESOLVED_CATEGORIES
    ]
    return {
        "inventory": "unresolved",
        "function": {
            "id": contract.validate_ident(
                f"fn-{fact.case_id}", "function obligation id"
            ),
            "extent": {"start": fact.address, "end": fact.end},
        },
        "initialized_data": [],
        "zero_initialized": [],
        "relocations": [],
        "external_bindings": [],
        "unresolved": unresolved,
    }


def answer_payload(
    *,
    baseline_bytes: bytes,
    closure: list[dict],
    authored: list[AuthoredMember],
    exclusions: list[ExclusionRecord],
    problem_digest: str,
    problem_bundle_digest: str,
) -> tuple[dict, dict]:
    """Build the evaluator-only answer with corpus, source and map snapshots."""

    files: dict[str, bytes] = {"baseline-audit.json": baseline_bytes}
    members: dict[str, dict] = {}
    for member in authored:
        fact = member.facts
        source_path = f"sources/{member.source}"
        files[source_path] = member.source_bytes
        map_refs = []
        for path, data in member.maps:
            bundle_path = f"maps/{path}"
            files[bundle_path] = data
            map_refs.append(contract.file_ref("answer", bundle_path, data))
        members[fact.key] = {
            "reference": {
                "authored_symbol": member.symbol,
                "authored_signature": member.signature,
                "source_locator": member.source,
                "profile": None,
                "historical": None,
            },
            "retained": {
                "sources": [
                    contract.file_ref("answer", source_path, member.source_bytes)
                ],
                "declarations": [],
                "maps": map_refs,
            },
            "obligations": _obligations(fact),
        }
    contract.require(
        len(members) == contract.ADMITTED, f"answer member count is {len(members)}"
    )
    _check_exclusions(members, exclusions)
    document = {
        "schema": contract.ANSWER_SCHEMA,
        "problem": contract.doc_ref("problem", problem_digest, problem_bundle_digest),
        "members": members,
        "exclusions": [
            {
                "case_id": record.case_id,
                "key": record.key,
                "target": record.target,
                "address": record.address,
                "size": record.size,
                "reason": record.reason,
                "reason_code": record.reason_code,
                "evidence": None,
            }
            for record in exclusions
        ],
        "baseline_audit": contract.file_ref(
            "answer", "baseline-audit.json", baseline_bytes
        ),
        "source_closure": closure,
    }
    return document, files


def _check_exclusions(members: dict, exclusions: list[ExclusionRecord]) -> None:
    contract.require(
        len(exclusions) == contract.EXCLUSIONS,
        f"answer carries {len(exclusions)} exclusions, expected {contract.EXCLUSIONS}",
    )
    admitted = set(members)
    for record in exclusions:
        contract.require(
            record.key not in admitted, f"exclusion is admitted: {record.key}"
        )
    reason_codes = {record.reason_code for record in exclusions}
    contract.require(reason_codes, "exclusions carry no reason code")
