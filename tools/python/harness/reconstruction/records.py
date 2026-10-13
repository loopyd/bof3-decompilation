"""Closed bundle, reference, producer and Catalog records with shared scalars."""

from __future__ import annotations

import re
from datetime import datetime

from harness.domain.ids import normalize_target_id, parse_function_id
from harness.reconstruction import contract

ROLE_DOCUMENT_SCHEMA = {
    "problem": contract.PROBLEM_SCHEMA,
    "answer": contract.ANSWER_SCHEMA,
    "preparation": contract.PREPARATION_SCHEMA,
}

TIMESTAMP = re.compile(r"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$")

U8_MAX = 2**8 - 1
U16_MAX = 2**16 - 1
U32_MAX = 2**32 - 1
U64_MAX = 2**64 - 1
PACKAGE_MODULE = re.compile(r"^[a-z][a-z0-9_]*(\.[a-z][a-z0-9_]*)+$")

IMAGE_FORMAT = ("psx-exe", "raw-payload")
TARGET_KIND = ("executable", "emi")
TOOL_ROLE = ("compiler", "assembler", "linker", "runtime", "library")
DEPENDENCY_KIND = ("executable", "library", "data")
OPTION_PHASE = ("cpp", "cc1", "as", "ld", "linker-script")
INVENTORY = ("complete", "unresolved")
OBLIGATION_CATEGORY = (
    "function",
    "initialized_data",
    "zero_initialized",
    "relocations",
    "external_bindings",
)
EXCLUSION_REASON_CODE = (
    "extent-mismatch",
    "native-byte-check",
    "boundary-unverifiable",
    "other",
)
MODE = ("autonomous", "signature_only", "metadata_assisted")


def _fields(value: object, allowed: set[str], subject: str) -> dict:
    contract.require(isinstance(value, dict), f"{subject} must be an object")
    extra = sorted(set(value) - allowed)
    missing = sorted(allowed - set(value))
    contract.require(not extra, f"{subject} has unknown fields: {extra}")
    contract.require(not missing, f"{subject} is missing fields: {missing}")
    return value


def _text(value: object, subject: str, maximum: int) -> str:

    contract.require(isinstance(value, str), f"{subject} must be text")
    contract.require(
        len(value) <= maximum,
        f"{subject} must be text of at most {maximum} characters",
    )
    contract.require("\0" not in value, f"{subject} must not contain NUL")
    try:
        value.encode("utf-8")
    except UnicodeEncodeError as error:
        raise contract.PreparationRefused(
            f"{subject} is not encodable UTF-8 text: {error}"
        ) from error
    return value


def _identity_text(value: object, subject: str) -> str:

    return _text(value, subject, 128)


def _nullable_text(value: object, subject: str, maximum: int) -> None:
    if value is None:
        return
    _text(value, subject, maximum)


def _relative(value: object, subject: str) -> str:
    return contract.validate_rel_path(value)


def _sha(value: object, subject: str) -> str:
    return contract.validate_sha256(value, subject)


def _nullable_digest(value: object, subject: str) -> None:
    if value is None:
        return
    contract.require(
        isinstance(value, str) and contract.DOC_DIGEST.fullmatch(value) is not None,
        f"{subject} must be null or a bundle digest",
    )


def _integer(value: object, subject: str, minimum: int | None = None) -> int:
    contract.require(
        isinstance(value, int) and not isinstance(value, bool),
        f"{subject} must be an integer",
    )
    if minimum is not None:
        contract.require(value >= minimum, f"{subject} must be >= {minimum}")
    return value


def _bounded(value: object, subject: str, maximum: int, minimum: int = 0) -> int:

    number = _integer(value, subject, minimum)
    contract.require(number <= maximum, f"{subject} exceeds {maximum}: {number}")
    return number


def _u8(value: object, subject: str, minimum: int = 0) -> int:
    return _bounded(value, subject, U8_MAX, minimum)


def _u16(value: object, subject: str, minimum: int = 0) -> int:
    return _bounded(value, subject, U16_MAX, minimum)


def _u32(value: object, subject: str, minimum: int = 0) -> int:
    return _bounded(value, subject, U32_MAX, minimum)


def _u64(value: object, subject: str, minimum: int = 0) -> int:
    return _bounded(value, subject, U64_MAX, minimum)


def _timestamp(value: object, subject: str) -> str:

    contract.require(
        isinstance(value, str) and TIMESTAMP.fullmatch(value) is not None,
        f"{subject} is not an RFC3339 UTC instant: {value!r}",
    )
    try:
        datetime.strptime(str(value), "%Y-%m-%dT%H:%M:%SZ")
    except ValueError as error:
        raise contract.PreparationRefused(
            f"{subject} is not a real calendar instant: {value!r}"
        ) from error
    return str(value)


def _enum(value: object, allowed: tuple, subject: str) -> str:
    contract.require(
        isinstance(value, str),
        f"{subject} must be text, observed {type(value).__name__}",
    )
    contract.require(value in allowed, f"{subject} must be one of {allowed}: {value!r}")
    return value


def _list(value: object, subject: str, length: int | None = None) -> list:
    contract.require(isinstance(value, list), f"{subject} must be a list")
    if length is not None:
        contract.require(len(value) == length, f"{subject} must hold {length} entries")
    return value


def _member_key(value: object, subject: str) -> str:
    _identity_text(value, subject)
    try:
        function = parse_function_id(str(value))
    except (ValueError, TypeError) as error:
        raise contract.PreparationRefused(
            f"{subject} is not a member key: {error}"
        ) from error
    contract.require(function.value == value, f"{subject} is not canonical: {value!r}")
    return str(value)


def _target(value: object, subject: str) -> str:
    _identity_text(value, subject)
    try:
        normalized = normalize_target_id(str(value))
    except (ValueError, TypeError) as error:
        raise contract.PreparationRefused(
            f"{subject} is not a target id: {error}"
        ) from error
    contract.require(str(normalized) == value, f"{subject} is not canonical: {value!r}")
    return str(value)


def _image_id(value: object, subject: str) -> str:
    contract.require(
        isinstance(value, str) and contract.IMAGE_ID.fullmatch(value) is not None,
        f"{subject} is not an image id: {value!r}",
    )
    return value


def validate_doc_ref(ref: object, subject: str) -> None:
    _fields(ref, {"bundle", "digest", "expected_bundle"}, subject)
    _relative(ref["bundle"], f"{subject}.bundle")
    _nullable_digest(ref["digest"], f"{subject}.digest")
    contract.require(
        isinstance(ref["digest"], str), f"{subject}.digest must be a bundle digest"
    )
    _nullable_digest(ref["expected_bundle"], f"{subject}.expected_bundle")


def validate_file_ref(ref: object, subject: str) -> None:
    _fields(ref, {"bundle", "path", "sha256", "bytes", "expected_bundle"}, subject)
    _relative(ref["bundle"], f"{subject}.bundle")
    _relative(ref["path"], f"{subject}.path")
    _sha(ref["sha256"], f"{subject}.sha256")
    _u64(ref["bytes"], f"{subject}.bytes")
    _nullable_digest(ref["expected_bundle"], f"{subject}.expected_bundle")


def validate_producer(producer: object, subject: str) -> None:
    _fields(producer, {"module", "revision", "code_sha256"}, subject)
    module = producer["module"]
    contract.require(
        isinstance(module, str) and PACKAGE_MODULE.fullmatch(module) is not None,
        f"{subject}.module is not a dotted module: {module!r}",
    )
    revision = producer["revision"]
    contract.require(
        isinstance(revision, str) and contract.REVISION.fullmatch(revision) is not None,
        f"{subject}.revision is invalid: {revision!r}",
    )
    _sha(producer["code_sha256"], f"{subject}.code_sha256")


def _validate_document_entry(entry: object, role: str, subject: str) -> None:
    _fields(entry, {"digest", "path", "schema", "sha256"}, subject)
    digest = entry["digest"]
    contract.require(
        isinstance(digest, str) and contract.DOC_DIGEST.fullmatch(digest) is not None,
        f"{subject}.digest must be a document digest",
    )
    _relative(entry["path"], f"{subject}.path")
    contract.require(
        entry["schema"] == ROLE_DOCUMENT_SCHEMA[role],
        f"{subject}.schema does not match role {role}: {entry['schema']!r}",
    )
    _sha(entry["sha256"], f"{subject}.sha256")


def validate_index(index: object, subject: str, *, coordinate: str) -> None:
    """Closed ``BundleIndex`` validation, including the role/origin/schema triple."""

    contract.validate_rel_path(coordinate)
    _fields(
        index,
        {
            "schema",
            "role",
            "run",
            "origin",
            "producer",
            "documents",
            "files",
            "upstream",
            "sealed_at",
            "bundle_digest",
        },
        subject,
    )
    contract.require(
        index["schema"] == contract.BUNDLE_SCHEMA, f"{subject}.schema is wrong"
    )
    role = index["role"]
    contract.require(
        isinstance(role, str),
        f"{subject}.role must be text, observed {type(role).__name__}",
    )
    contract.require(
        role in ROLE_DOCUMENT_SCHEMA,
        f"{subject}.role is not implemented here: {role!r}",
    )
    contract.require(
        index["origin"] == contract.ROLE_ORIGIN[str(role)],
        f"{subject}.origin does not match its role",
    )
    contract.validate_run_id(index["run"])
    validate_producer(index["producer"], f"{subject}.producer")
    _timestamp(index["sealed_at"], f"{subject}.sealed_at")
    bundle_digest = index["bundle_digest"]
    contract.require(
        isinstance(bundle_digest, str)
        and contract.DOC_DIGEST.fullmatch(bundle_digest) is not None,
        f"{subject}.bundle_digest is invalid",
    )
    documents = _list(index["documents"], f"{subject}.documents")
    contract.require(bool(documents), f"{subject} must index at least one document")
    for position, entry in enumerate(documents):
        _validate_document_entry(entry, str(role), f"{subject}.documents[{position}]")
    contract.require(
        len({entry["path"] for entry in documents}) == len(documents),
        f"{subject} repeats a document path",
    )
    contract.require(
        len({entry["digest"] for entry in documents}) == len(documents),
        f"{subject} repeats a document digest",
    )
    for position, entry in enumerate(_list(index["files"], f"{subject}.files")):
        validate_file_ref(entry, f"{subject}.files[{position}]")
        contract.require(
            entry["bundle"] == coordinate,
            f"{subject}.files[{position}] must belong to its own bundle",
        )
        contract.require(
            entry["expected_bundle"] is None,
            f"{subject}.files[{position}] must not pin its own bundle digest",
        )
    contract.require(
        len({entry["path"] for entry in index["files"]}) == len(index["files"]),
        f"{subject} repeats a file path",
    )
    for position, ref in enumerate(_list(index["upstream"], f"{subject}.upstream")):
        validate_doc_ref(ref, f"{subject}.upstream[{position}]")


def _validate_dependency(dependency: object, subject: str) -> None:
    _fields(dependency, {"kind", "identity", "sha256"}, subject)
    _enum(dependency["kind"], DEPENDENCY_KIND, f"{subject}.kind")
    identity = dependency["identity"]
    contract.require(
        isinstance(identity, str) and contract.REVISION.fullmatch(identity) is not None,
        f"{subject}.identity must match {contract.REVISION.pattern}: {identity!r}",
    )
    _sha(dependency["sha256"], f"{subject}.sha256")


def validate_catalog(document: object, subject: str) -> None:
    """Closed ``Catalog`` validation for the reviewed toolchain export."""

    _fields(document, {"catalog_id", "host", "tools"}, subject)
    contract.validate_tool_id(document["catalog_id"], f"{subject}.catalog_id")
    contract.validate_tool_id(document["host"], f"{subject}.host")
    tools = _list(document["tools"], f"{subject}.tools")
    contract.require(bool(tools), f"{subject}.tools must admit at least one tool")
    for position, tool in enumerate(tools):
        path = f"{subject}.tools[{position}]"
        _fields(
            tool,
            {
                "tool_id",
                "role",
                "identity",
                "archive_sha256",
                "resolved",
                "option_domains",
            },
            path,
        )
        contract.validate_tool_id(tool["tool_id"], f"{path}.tool_id")
        _enum(tool["role"], TOOL_ROLE, f"{path}.role")
        identity = tool["identity"]
        contract.require(
            isinstance(identity, str)
            and contract.REVISION.fullmatch(identity) is not None,
            f"{path}.identity must match {contract.REVISION.pattern}: {identity!r}",
        )
        if tool["archive_sha256"] is not None:
            _sha(tool["archive_sha256"], f"{path}.archive_sha256")
        resolved = _list(tool["resolved"], f"{path}.resolved", None)
        contract.require(
            bool(resolved), f"{path}.resolved must bind at least one dependency"
        )
        for index, dependency in enumerate(resolved):
            _validate_dependency(dependency, f"{path}.resolved[{index}]")
        domains = tool["option_domains"]
        contract.require(
            isinstance(domains, dict),
            f"{path}.option_domains must be an object",
        )
        contract.require(
            set(domains) == set(OPTION_PHASE),
            f"{path}.option_domains must declare every phase: {sorted(domains)}",
        )
        for phase in OPTION_PHASE:
            for index, value in enumerate(
                _list(domains[phase], f"{path}.option_domains.{phase}")
            ):
                _text(value, f"{path}.option_domains.{phase}[{index}]", 128)
