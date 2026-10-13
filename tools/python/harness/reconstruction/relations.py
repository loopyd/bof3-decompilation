"""Relational checks within and across the implemented documents.

Shape validation lives in :mod:`harness.reconstruction.records`,
:mod:`harness.reconstruction.obligations` and
:mod:`harness.reconstruction.documents`.  This module owns the predicates those
individually valid shapes can still violate: identity-key/target/address
agreement, unique ids, internal reference resolution, extent containment,
admitted/exclusion disjointness, and the cross-document relations enforced when a
referenced document is actually resolved (Problem key-set equality, function
obligation extents, preparation publication bindings).
"""

from __future__ import annotations

from harness.domain.ids import parse_function_id
from harness.reconstruction import contract
from harness.reconstruction.records import _fields


def check_problem_internal(document: dict, subject: str) -> None:
    """Identity, uniqueness, reference and containment relations inside ``Problem``."""

    member_keys = [member["key"] for member in document["members"]]
    contract.require(
        member_keys == sorted(member_keys),
        f"{subject}.members must be sorted by key",
    )
    images: dict[str, dict] = {}
    for position, image in enumerate(document["images"]):
        image_id = image["image_id"]
        contract.require(
            image_id not in images,
            f"{subject}.images[{position}] repeats image id {image_id}",
        )
        images[image_id] = image
    targets: dict[str, dict] = {}
    for position, target in enumerate(document["targets"]):
        name = target["target"]
        contract.require(
            name not in targets, f"{subject}.targets[{position}] repeats target {name}"
        )
        contract.require(
            target["image_id"] in images,
            f"{subject}.targets[{position}] references an unknown image: {target['image_id']}",
        )
        targets[name] = target
    keys: set[str] = set()
    cases: set[str] = set()
    for position, member in enumerate(document["members"]):
        where = f"{subject}.members[{position}]"
        key = member["key"]
        contract.require(key not in keys, f"{where} repeats member key {key}")
        keys.add(key)
        contract.require(
            member["case_id"] not in cases,
            f"{where} repeats case id {member['case_id']}",
        )
        cases.add(member["case_id"])
        try:
            function = parse_function_id(key)
        except (ValueError, TypeError) as error:
            raise contract.PreparationRefused(
                f"{where}.key is not a member key: {error}"
            ) from error
        contract.require(
            function.target.value == member["target"],
            f"{where}.key target disagrees with member.target: {function.target.value} != {member['target']}",
        )
        contract.require(
            function.address == member["address"],
            f"{where}.key address disagrees with member.address: {function.address:#x} != {member['address']:#x}",
        )
        contract.require(
            member["target"] in targets,
            f"{where} references an unknown target: {member['target']}",
        )
        contract.require(
            member["image_id"] in images,
            f"{where} references an unknown image: {member['image_id']}",
        )
        contract.require(
            member["range"]["start"] == member["address"],
            f"{where}.range.start must equal the member address",
        )
        image = images[member["image_id"]]
        payload_end = image["load_address"] + image["payload_size"]
        start, end = member["range"]["start"], member["range"]["end"]
        contract.require(
            image["load_address"] <= start < end <= payload_end,
            f"{where}.range [{start:#x},{end:#x}) escapes image payload "
            f"[{image['load_address']:#x},{payload_end:#x})",
        )


def check_answer_internal(document: dict, subject: str) -> None:
    """Admitted/exclusion identity and disjointness relations inside ``Answer``."""

    members = document["members"]
    cases: set[str] = set()
    keys: set[str] = set()
    for position, exclusion in enumerate(document["exclusions"]):
        where = f"{subject}.exclusions[{position}]"
        contract.require(
            exclusion["case_id"] not in cases,
            f"{where} repeats exclusion case id {exclusion['case_id']}",
        )
        cases.add(exclusion["case_id"])
        contract.require(
            exclusion["key"] not in keys,
            f"{where} repeats exclusion key {exclusion['key']}",
        )
        keys.add(exclusion["key"])
        contract.require(
            exclusion["key"] not in members,
            f"{where} excludes an admitted member: {exclusion['key']}",
        )


def check_answer_against_problem(answer: dict, problem: dict, subject: str) -> None:
    """Cross-document relations once the referenced ``Problem`` is resolved."""

    contract.require(
        problem.get("schema") == contract.PROBLEM_SCHEMA,
        f"{subject} must resolve to a {contract.PROBLEM_SCHEMA} document, "
        f"observed {problem.get('schema')!r}",
    )
    problem_keys = {member["key"] for member in problem["members"]}
    answer_keys = set(answer["members"])
    contract.require(
        answer_keys == problem_keys,
        f"{subject} member keys differ from the problem: "
        f"only-answer={sorted(answer_keys - problem_keys)[:3]} "
        f"only-problem={sorted(problem_keys - answer_keys)[:3]}",
    )
    extents = {member["key"]: member["range"] for member in problem["members"]}
    for key, member in answer["members"].items():
        extent = member["obligations"]["function"]["extent"]
        contract.require(
            extent == extents[key],
            f"{subject} members[{key}] function extent {extent} "
            f"differs from the problem range {extents[key]}",
        )


def check_preparation_bindings(document: dict, digests: dict, subject: str) -> None:
    """Publication identity bindings of one ``Preparation`` document."""

    for role in ("problem", "answer"):
        entry = _fields(
            document[role], {"role", "bundle", "bundle_digest"}, f"{subject}.{role}"
        )
        contract.require(
            entry["role"] == role, f"{subject}.{role}.role must be {role!r}"
        )
        contract.require(
            entry["bundle"] == role, f"{subject}.{role}.bundle must be {role!r}"
        )
        contract.require(
            role in digests, f"{subject}.{role} has no recomputed bundle digest"
        )
        contract.require(
            entry["bundle_digest"] == digests[role],
            f"{subject}.{role} digest does not match the published bundle: "
            f"{entry['bundle_digest']} != {digests[role]}",
        )
