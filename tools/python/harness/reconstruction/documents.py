"""Closed reconstruction document validation and schema dispatch."""

from __future__ import annotations

from harness.reconstruction import contract, obligations, relations
from harness.reconstruction.records import (
    IMAGE_FORMAT,
    TARGET_KIND,
    _enum,
    _fields,
    _image_id,
    _list,
    _member_key,
    _sha,
    _target,
    _text,
    _u8,
    _u16,
    _u32,
    _u64,
    _timestamp,
    validate_doc_ref,
    validate_file_ref,
)


def _validate_image(image: object, subject: str) -> None:
    _fields(
        image,
        {
            "image_id",
            "label",
            "sha256",
            "bytes",
            "format",
            "load_address",
            "payload_offset",
            "payload_size",
            "file",
        },
        subject,
    )
    _image_id(image["image_id"], f"{subject}.image_id")
    _text(image["label"], f"{subject}.label", 64)
    _sha(image["sha256"], f"{subject}.sha256")
    _u64(image["bytes"], f"{subject}.bytes", minimum=1)
    _enum(image["format"], IMAGE_FORMAT, f"{subject}.format")
    _u32(image["load_address"], f"{subject}.load_address")
    _u32(image["payload_offset"], f"{subject}.payload_offset")
    _u64(image["payload_size"], f"{subject}.payload_size", minimum=1)
    validate_file_ref(image["file"], f"{subject}.file")


def _validate_target(target: object, subject: str) -> None:
    _fields(target, {"target", "kind", "shipped", "image_id"}, subject)
    _target(target["target"], f"{subject}.target")
    _enum(target["kind"], TARGET_KIND, f"{subject}.kind")
    if target["shipped"] is not None:
        _text(target["shipped"], f"{subject}.shipped", 256)
    _image_id(target["image_id"], f"{subject}.image_id")


def _validate_member(member: object, subject: str) -> None:
    _fields(
        member,
        {"key", "case_id", "target", "address", "range", "range_sha256", "image_id"},
        subject,
    )
    _member_key(member["key"], f"{subject}.key")
    contract.validate_ident(member["case_id"], f"{subject}.case_id")
    _target(member["target"], f"{subject}.target")
    _u32(member["address"], f"{subject}.address")
    extent = _fields(member["range"], {"start", "end"}, f"{subject}.range")
    start = _u32(extent["start"], f"{subject}.range.start")
    end = _u32(extent["end"], f"{subject}.range.end")
    contract.require(end > start, f"{subject}.range must be half-open with end > start")
    _sha(member["range_sha256"], f"{subject}.range_sha256")
    _image_id(member["image_id"], f"{subject}.image_id")


def _validate_milestone(milestone: object, subject: str) -> None:
    _fields(milestone, {"id", "admitted", "exclusions"}, subject)
    contract.require(
        milestone["id"] == contract.MILESTONE_ID,
        f"{subject}.id must be {contract.MILESTONE_ID!r}: {milestone['id']!r}",
    )
    admitted = _u16(milestone["admitted"], f"{subject}.admitted")
    contract.require(
        admitted == contract.ADMITTED, f"{subject}.admitted must be {contract.ADMITTED}"
    )
    exclusions = _u8(milestone["exclusions"], f"{subject}.exclusions")
    contract.require(
        exclusions == contract.EXCLUSIONS,
        f"{subject}.exclusions must be {contract.EXCLUSIONS}",
    )


def _validate_baseline(baseline: object, subject: str) -> None:
    _fields(
        baseline,
        {"schema", "sha256", "admitted", "exclusions"},
        subject,
    )
    contract.require(
        baseline["schema"] == contract.BASELINE_SCHEMA,
        f"{subject}.schema must be {contract.BASELINE_SCHEMA!r}",
    )
    _sha(baseline["sha256"], f"{subject}.sha256")
    admitted = _u16(baseline["admitted"], f"{subject}.admitted")
    contract.require(
        admitted == contract.ADMITTED, f"{subject}.admitted must be {contract.ADMITTED}"
    )
    exclusions = _u8(baseline["exclusions"], f"{subject}.exclusions")
    contract.require(
        exclusions == contract.EXCLUSIONS,
        f"{subject}.exclusions must be {contract.EXCLUSIONS}",
    )


def _validate_membership(membership: object, subject: str) -> None:
    _fields(membership, {"count", "exclusions"}, subject)
    count = _u16(membership["count"], f"{subject}.count")
    contract.require(
        count == contract.ADMITTED, f"{subject}.count must be {contract.ADMITTED}"
    )
    exclusions = _u8(membership["exclusions"], f"{subject}.exclusions")
    contract.require(
        exclusions == contract.EXCLUSIONS,
        f"{subject}.exclusions must be {contract.EXCLUSIONS}",
    )


def _validate_rules(rules: object, subject: str) -> None:
    _fields(rules, {"rules_id", "source", "entries"}, subject)
    contract.validate_tool_id(rules["rules_id"], f"{subject}.rules_id")
    validate_file_ref(rules["source"], f"{subject}.source")
    for position, rule in enumerate(_list(rules["entries"], f"{subject}.entries")):
        obligations.validate_rule(rule, f"{subject}.entries[{position}]")


def validate_problem(document: object, subject: str) -> None:
    _fields(
        document,
        {
            "schema",
            "milestone",
            "baseline",
            "membership",
            "images",
            "targets",
            "members",
            "rules",
        },
        subject,
    )
    _validate_milestone(document["milestone"], f"{subject}.milestone")
    _validate_baseline(document["baseline"], f"{subject}.baseline")
    _validate_membership(document["membership"], f"{subject}.membership")
    for position, image in enumerate(_list(document["images"], f"{subject}.images")):
        _validate_image(image, f"{subject}.images[{position}]")
    for position, target in enumerate(_list(document["targets"], f"{subject}.targets")):
        _validate_target(target, f"{subject}.targets[{position}]")
    members = _list(document["members"], f"{subject}.members", contract.ADMITTED)
    for position, member in enumerate(members):
        _validate_member(member, f"{subject}.members[{position}]")
    _validate_rules(document["rules"], f"{subject}.rules")
    relations.check_problem_internal(document, subject)


def validate_answer(document: object, subject: str) -> None:
    _fields(
        document,
        {
            "schema",
            "problem",
            "members",
            "exclusions",
            "baseline_audit",
            "source_closure",
        },
        subject,
    )
    validate_doc_ref(document["problem"], f"{subject}.problem")
    members = document["members"]
    contract.require(isinstance(members, dict), f"{subject}.members must be an object")
    contract.require(
        len(members) == contract.ADMITTED,
        f"{subject}.members must hold {contract.ADMITTED} entries",
    )
    for key, member in members.items():
        _member_key(key, f"{subject}.members key")
        obligations.validate_answer_member(member, f"{subject}.members[{key}]")
    exclusions = _list(
        document["exclusions"], f"{subject}.exclusions", contract.EXCLUSIONS
    )
    for position, exclusion in enumerate(exclusions):
        obligations.validate_exclusion(exclusion, f"{subject}.exclusions[{position}]")
    validate_file_ref(document["baseline_audit"], f"{subject}.baseline_audit")
    for position, entry in enumerate(
        _list(document["source_closure"], f"{subject}.source_closure")
    ):
        _fields(entry, {"path", "sha256"}, f"{subject}.source_closure[{position}]")
        _text(entry["path"], f"{subject}.source_closure[{position}].path", 256)
        _sha(entry["sha256"], f"{subject}.source_closure[{position}].sha256")
    relations.check_answer_internal(document, subject)


def _validate_publication_entry(entry: object, role: str, subject: str) -> None:
    _fields(entry, {"role", "bundle", "bundle_digest"}, subject)
    contract.require(entry["role"] == role, f"{subject}.role must be {role!r}")
    contract.require(entry["bundle"] == role, f"{subject}.bundle must be {role!r}")
    digest = entry["bundle_digest"]
    contract.require(
        isinstance(digest, str) and contract.DOC_DIGEST.fullmatch(digest) is not None,
        f"{subject}.bundle_digest is invalid: {digest!r}",
    )


def validate_preparation(document: object, subject: str) -> None:
    _fields(document, {"schema", "problem", "answer", "complete", "sealed_at"}, subject)
    _validate_publication_entry(document["problem"], "problem", f"{subject}.problem")
    _validate_publication_entry(document["answer"], "answer", f"{subject}.answer")
    contract.require(
        isinstance(document["complete"], bool) and document["complete"] is True,
        f"{subject}.complete must be the boolean true",
    )
    _timestamp(document["sealed_at"], f"{subject}.sealed_at")


def _validate_tool_ref(ref: object, subject: str) -> None:
    _fields(ref, {"tool_id", "sha256"}, subject)
    contract.validate_tool_id(ref["tool_id"], f"{subject}.tool_id")
    _sha(ref["sha256"], f"{subject}.sha256")


def validate_build_config(document: object, subject: str) -> None:
    """Closed ``BuildConfig`` validation, including the empty environment rule."""

    _fields(
        document,
        {
            "compiler",
            "assembler",
            "linker",
            "options",
            "include_roots",
            "libraries",
            "entry_symbol",
            "environment",
        },
        subject,
    )
    for role in ("compiler", "assembler", "linker"):
        _validate_tool_ref(document[role], f"{subject}.{role}")
    options = _list(document["options"], f"{subject}.options", None)
    contract.require(len(options) <= 64, f"{subject}.options exceeds its bound")
    for index, value in enumerate(options):
        _text(value, f"{subject}.options[{index}]", 128)
    roots = _list(document["include_roots"], f"{subject}.include_roots", None)
    contract.require(len(roots) <= 32, f"{subject}.include_roots exceeds its bound")
    for index, value in enumerate(roots):
        contract.validate_rel_path(value)
    libraries = _list(document["libraries"], f"{subject}.libraries", None)
    contract.require(len(libraries) <= 16, f"{subject}.libraries exceeds its bound")
    for index, ref in enumerate(libraries):
        _validate_tool_ref(ref, f"{subject}.libraries[{index}]")
    symbol = document["entry_symbol"]
    contract.require(
        isinstance(symbol, str) and contract.TOKEN.fullmatch(symbol) is not None,
        f"{subject}.entry_symbol must match {contract.TOKEN.pattern}: {symbol!r}",
    )
    contract.require(
        document["environment"] == {},
        f"{subject}.environment must be the empty object",
    )


DOCUMENT_VALIDATORS = {
    contract.PROBLEM_SCHEMA: validate_problem,
    contract.ANSWER_SCHEMA: validate_answer,
    contract.PREPARATION_SCHEMA: validate_preparation,
}


def validate_document(document: object, subject: str) -> None:
    """Dispatch one document to its closed validator by declared schema."""

    contract.require(isinstance(document, dict), f"{subject} must be an object")
    schema = document.get("schema")
    contract.require(
        isinstance(schema, str),
        f"{subject}.schema must be text, observed {type(schema).__name__}",
    )
    validator = DOCUMENT_VALIDATORS.get(schema)
    contract.require(
        validator is not None, f"{subject} has an unsupported schema: {schema!r}"
    )
    validator(document, subject)
