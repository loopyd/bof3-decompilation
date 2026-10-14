"""Validate independent obligations and measure separate associated predicates."""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass, asdict
import hashlib
import re
from typing import Literal

from harness.match.extraction import LinkedImage, LinkedStorage
from harness.reconstruction.objectfile import ObjectImage, ObjectStorage
from harness.domain.ids import parse_function_id
from harness.reconstruction import contract
from harness.reconstruction.records import (
    EXCLUSION_REASON_CODE,
    MODE,
    OBLIGATION_CATEGORY,
    _enum,
    _fields,
    _list,
    _member_key,
    _nullable_text,
    _target,
    _text,
    _u32,
    _u64,
    validate_file_ref,
)

RULE_KIND = ("instruction-semantics", "abi", "psyq-signature", "exception", "platform")


def validate_rule(rule: object, subject: str) -> None:
    _fields(rule, {"rule_id", "kind", "description"}, subject)
    contract.validate_ident(rule["rule_id"], f"{subject}.rule_id")
    _enum(rule["kind"], RULE_KIND, f"{subject}.kind")
    _text(rule["description"], f"{subject}.description", 512)


def validate_profile(profile: object, subject: str) -> None:
    _fields(profile, {"compiler_id", "options"}, subject)
    contract.validate_tool_id(profile["compiler_id"], f"{subject}.compiler_id")
    for position, option in enumerate(_list(profile["options"], f"{subject}.options")):
        _text(option, f"{subject}.options[{position}]", 128)


def validate_historical(historical: object, subject: str) -> None:
    _fields(historical, {"mode", "evidence_level", "result", "receipt"}, subject)
    _enum(historical["mode"], MODE, f"{subject}.mode")
    _text(historical["evidence_level"], f"{subject}.evidence_level", 64)
    _text(historical["result"], f"{subject}.result", 64)
    if historical["receipt"] is not None:
        validate_file_ref(historical["receipt"], f"{subject}.receipt")


def validate_reference(reference: object, subject: str) -> None:
    _fields(
        reference,
        {
            "authored_symbol",
            "authored_signature",
            "source_locator",
            "profile",
            "historical",
        },
        subject,
    )
    symbol = reference["authored_symbol"]
    if symbol is not None:
        contract.require(
            isinstance(symbol, str) and contract.TOKEN.fullmatch(symbol) is not None,
            f"{subject}.authored_symbol is not a C token: {symbol!r}",
        )
    _nullable_text(
        reference["authored_signature"], f"{subject}.authored_signature", 512
    )
    _nullable_text(reference["source_locator"], f"{subject}.source_locator", 256)
    if reference["profile"] is not None:
        validate_profile(reference["profile"], f"{subject}.profile")
    if reference["historical"] is not None:
        validate_historical(reference["historical"], f"{subject}.historical")


def validate_retained(retained: object, subject: str) -> None:
    _fields(retained, {"sources", "declarations", "maps"}, subject)
    for name in ("sources", "declarations", "maps"):
        for position, ref in enumerate(_list(retained[name], f"{subject}.{name}")):
            validate_file_ref(ref, f"{subject}.{name}[{position}]")


def validate_function_obligation(value: object, subject: str) -> None:
    _fields(value, {"id", "extent"}, subject)
    contract.validate_ident(value["id"], f"{subject}.id")
    _fields(value["extent"], {"start", "end"}, f"{subject}.extent")
    start = _u32(value["extent"]["start"], f"{subject}.extent.start")
    end = _u32(value["extent"]["end"], f"{subject}.extent.end")
    contract.require(
        end > start, f"{subject}.extent must be half-open with end > start"
    )


def validate_initialized_data(value: object, subject: str) -> None:
    _fields(value, {"id", "address", "size", "contents", "placement"}, subject)
    contract.validate_ident(value["id"], f"{subject}.id")
    _u32(value["address"], f"{subject}.address")
    _u64(value["size"], f"{subject}.size", minimum=1)
    validate_file_ref(value["contents"], f"{subject}.contents")
    _text(value["placement"], f"{subject}.placement", 256)


def validate_zero_initialized(value: object, subject: str) -> None:
    _fields(value, {"id", "address", "size", "placement"}, subject)
    contract.validate_ident(value["id"], f"{subject}.id")
    _u32(value["address"], f"{subject}.address")
    _u64(value["size"], f"{subject}.size", minimum=1)
    _text(value["placement"], f"{subject}.placement", 256)


def validate_relocation(value: object, subject: str) -> None:
    _fields(value, {"id", "location", "kind", "symbol"}, subject)
    contract.validate_ident(value["id"], f"{subject}.id")
    _u32(value["location"], f"{subject}.location")
    _text(value["kind"], f"{subject}.kind", 64)
    contract.require(
        isinstance(value["symbol"], str)
        and contract.TOKEN.fullmatch(value["symbol"]) is not None,
        f"{subject}.symbol is not a C token: {value['symbol']!r}",
    )


def validate_binding(value: object, subject: str) -> None:
    _fields(value, {"id", "symbol", "address", "target"}, subject)
    contract.validate_ident(value["id"], f"{subject}.id")
    contract.require(
        isinstance(value["symbol"], str)
        and contract.TOKEN.fullmatch(value["symbol"]) is not None,
        f"{subject}.symbol is not a C token: {value['symbol']!r}",
    )
    _u32(value["address"], f"{subject}.address")
    _target(value["target"], f"{subject}.target")


def validate_unresolved(value: object, subject: str) -> None:
    _fields(value, {"category", "address", "reason"}, subject)
    _enum(value["category"], OBLIGATION_CATEGORY, f"{subject}.category")
    if value["address"] is not None:
        _u32(value["address"], f"{subject}.address")
    _text(value["reason"], f"{subject}.reason", 256)


def validate_obligations(value: object, subject: str) -> None:
    _fields(
        value,
        {
            "inventory",
            "function",
            "initialized_data",
            "zero_initialized",
            "relocations",
            "external_bindings",
            "unresolved",
        },
        subject,
    )
    inventory = value["inventory"]
    _enum(inventory, ("complete", "unresolved"), f"{subject}.inventory")
    validate_function_obligation(value["function"], f"{subject}.function")
    for name, validator in (
        ("initialized_data", validate_initialized_data),
        ("zero_initialized", validate_zero_initialized),
        ("relocations", validate_relocation),
        ("external_bindings", validate_binding),
    ):
        for position, entry in enumerate(_list(value[name], f"{subject}.{name}")):
            validator(entry, f"{subject}.{name}[{position}]")
    unresolved = _list(value["unresolved"], f"{subject}.unresolved")
    for position, entry in enumerate(unresolved):
        validate_unresolved(entry, f"{subject}.unresolved[{position}]")
    if inventory == "complete":
        contract.require(
            not unresolved,
            f"{subject}.inventory is complete with {len(unresolved)} unresolved entries",
        )
    else:
        contract.require(
            bool(unresolved),
            f"{subject}.inventory is unresolved without any unresolved entry",
        )
    ids = [value["function"]["id"]]
    for name in (
        "initialized_data",
        "zero_initialized",
        "relocations",
        "external_bindings",
    ):
        ids.extend(entry["id"] for entry in value[name])
    contract.require(
        len(set(ids)) == len(ids),
        f"{subject} repeats an obligation id: {sorted(ids)}",
    )


def validate_answer_member(value: object, subject: str) -> None:
    _fields(value, {"reference", "retained", "obligations"}, subject)
    validate_reference(value["reference"], f"{subject}.reference")
    validate_retained(value["retained"], f"{subject}.retained")
    validate_obligations(value["obligations"], f"{subject}.obligations")


def validate_exclusion(value: object, subject: str) -> None:
    _fields(
        value,
        {
            "case_id",
            "key",
            "target",
            "address",
            "size",
            "reason",
            "reason_code",
            "evidence",
        },
        subject,
    )
    contract.validate_ident(value["case_id"], f"{subject}.case_id")
    _member_key(value["key"], f"{subject}.key")
    _target(value["target"], f"{subject}.target")
    _u32(value["address"], f"{subject}.address")
    _u64(value["size"], f"{subject}.size", minimum=1)
    _text(value["reason"], f"{subject}.reason", 256)
    _enum(value["reason_code"], EXCLUSION_REASON_CODE, f"{subject}.reason_code")
    if value["evidence"] is not None:
        validate_file_ref(value["evidence"], f"{subject}.evidence")
    function = parse_function_id(value["key"])
    contract.require(
        function.target.value == value["target"]
        and function.address == value["address"],
        f"{subject}.key disagrees with its target/address",
    )


__all__ = [
    "RULE_KIND",
    "validate_answer_member",
    "validate_binding",
    "validate_exclusion",
    "validate_function_obligation",
    "validate_historical",
    "validate_initialized_data",
    "validate_obligations",
    "validate_profile",
    "validate_reference",
    "validate_relocation",
    "validate_retained",
    "validate_rule",
    "validate_unresolved",
    "validate_zero_initialized",
]


@dataclass(frozen=True)
class InitializedRequirement:
    id: str
    address: int
    size: int
    expected: bytes
    placement: str


@dataclass(frozen=True)
class ZeroRequirement:
    id: str
    address: int
    size: int
    placement: str


@dataclass(frozen=True)
class RelocationRequirement:
    id: str
    location: int
    kind: str
    symbol: str


@dataclass(frozen=True)
class BindingRequirement:
    id: str
    symbol: str
    address: int
    target: str


KnownRequirement = (
    InitializedRequirement
    | ZeroRequirement
    | RelocationRequirement
    | BindingRequirement
)


@dataclass(frozen=True)
class UnresolvedRequirement:
    category: str
    address: int | None
    reason: str


@dataclass(frozen=True)
class MemberRequirements:
    member_key: str
    known: tuple[KnownRequirement, ...]
    unresolved: tuple[UnresolvedRequirement, ...]
    inventory_claim: Literal["complete", "unresolved"]


@dataclass(frozen=True)
class OriginalImageFacts:
    target: str
    image_id: str
    load_address: int
    payload_offset: int
    content: bytes


@dataclass(frozen=True)
class OriginalAssociatedFacts:
    member_target: str
    images: tuple[OriginalImageFacts, ...]


@dataclass(frozen=True)
class SymbolProducerFacts:
    object_sha256: str
    linked_sha256: str
    assembly_labels: frozenset[str]
    restricted_link: bool


def derive_symbol_producer_facts(
    assemblies: tuple[bytes, ...],
    object_image: ObjectImage,
    linked_image: LinkedImage,
    *,
    restricted_link: bool,
) -> SymbolProducerFacts | None:
    if (
        object_image is None
        or linked_image is None
        or len(assemblies) != 2
        or any(type(content) is not bytes for content in assemblies)
        or type(restricted_link) is not bool
    ):
        return None
    definitions = []
    for content in assemblies:
        definitions.append(
            Counter(re.findall(rb"(?m)^[ \t]*([A-Za-z_.$][A-Za-z0-9_.$]*):", content))
        )
    labels = frozenset(
        name.decode("ascii")
        for name, count in definitions[0].items()
        if count == definitions[1][name] == 1
    )
    return SymbolProducerFacts(
        object_image.sha256, linked_image.sha256, labels, restricted_link
    )


def _producer_symbol_ids(
    producer,
    object_image,
    linked_image,
    function_sections,
    section_roles,
    relocations,
):
    object_names = Counter(symbol.name for symbol in object_image.symbols)
    linked_names = Counter(symbol.name for symbol in linked_image.symbols)
    linked_by_name = {symbol.name: symbol for symbol in linked_image.symbols}
    object_tables = [section for section in object_image.sections if section.kind == 2]
    if (
        len(object_tables) != 1
        or section_roles.get(("object", object_tables[0].index)) != "symbol-table"
    ):
        return set()
    qualified = set()
    object_function = function_sections["object"]
    linked_function = function_sections["linked"]
    delta = linked_function.address - object_function.address
    references = {}
    for row in relocations:
        key = (row["owner"], row.get("symbol_table", 0), row["symbol_index"])
        references.setdefault(key, []).append(row)
    for symbol in object_image.symbols:
        counterpart = linked_by_name.get(symbol.name)
        if (
            not symbol.name
            or symbol.name not in producer.assembly_labels
            or object_names[symbol.name] != 1
            or linked_names[symbol.name] != 1
            or symbol.type != 0
            or symbol.binding != 0
            or symbol.size != 0
            or symbol.visibility != 0
            or symbol.section != object_function.index
            or not 0 <= symbol.value <= object_function.size
            or counterpart is None
            or counterpart.info != 0
            or counterpart.size != 0
            or counterpart.visibility != 0
            or counterpart.section != linked_function.index
            or counterpart.value != symbol.value + delta
            or not (
                linked_function.address
                <= counterpart.value
                <= linked_function.address + linked_function.size
            )
            or section_roles.get(("linked", counterpart.table)) != "symbol-table"
        ):
            continue
        identities = (
            ("object", 0, symbol.index),
            ("linked", counterpart.table, counterpart.index),
        )
        if all(
            row["status"] == "matched"
            for identity in identities
            for row in references.get(identity, ())
        ):
            qualified.update(identities)
    for symbol in linked_image.symbols:
        identity = ("linked", symbol.table, symbol.index)
        if (
            symbol.name
            and symbol.name not in object_names
            and linked_names[symbol.name] == 1
            and symbol.info & 15 == 0
            and symbol.info >> 4 in {0, 1}
            and symbol.visibility == symbol.size == 0
            and (
                0 < symbol.section < len(linked_image.sections)
                or symbol.section == 0xFFF1
            )
            and section_roles.get(("linked", symbol.table)) == "symbol-table"
            and not references.get(identity)
        ):
            qualified.add(identity)
    return qualified


@dataclass(frozen=True)
class PredicateResult:
    name: str
    status: Literal["matched", "failed", "missing", "unknown"]
    reason: str
    expected: object = None
    actual: object = None


@dataclass(frozen=True)
class RequirementResult:
    id: str
    category: str
    predicates: tuple[PredicateResult, ...]

    @property
    def status(self) -> str:
        statuses = {predicate.status for predicate in self.predicates}
        if "failed" in statuses:
            return "failed"
        if "missing" in statuses:
            return "missing"
        return "unknown" if "unknown" in statuses else "matched"


@dataclass(frozen=True)
class ProfileRequirementsResult:
    profile_id: str
    known: tuple[RequirementResult, ...]
    independent_unknowns: tuple[UnresolvedRequirement, ...]
    allocated: tuple[dict, ...]
    relocations: tuple[dict, ...]
    prior_failures: tuple[str, ...]
    symbols: tuple[dict, ...] = ()
    sections: tuple[dict, ...] = ()
    segments: tuple[dict, ...] = ()
    linked_relocations: tuple[dict, ...] = ()

    @property
    def status(self) -> str:
        if self.prior_failures or any(
            row.status in {"failed", "missing"} for row in self.known
        ):
            return "measured-failed"
        return "measured-unresolved"

    def as_record(self) -> dict:
        record = asdict(self)
        for row, result in zip(record["known"], self.known):
            row["status"] = result.status
        record["status"] = self.status
        record["inventory_closure"] = "unknown"
        return record


@dataclass(frozen=True)
class OriginalFootprint:
    member_key: str
    instruction_pcs: tuple[int, ...]
    initialized_uses: tuple
    zero_uses: tuple
    relocation_uses: tuple
    binding_uses: tuple
    conditions: tuple[str, ...]
    predicates: tuple[PredicateResult, ...]


@dataclass(frozen=True)
class AssociatedClosureFacts:
    member_key: str
    profile_id: str
    original_footprint: OriginalFootprint
    measurements: ProfileRequirementsResult
    storage: tuple[dict, ...]
    relocations: tuple[dict, ...]
    unresolved_requirement_ids: tuple[str, ...]
    uncovered_sections: tuple[tuple[str, int], ...]
    uncovered_symbols: tuple[tuple[str, int, int], ...]
    uncovered_relocations: tuple[tuple[str, int, int], ...]
    uncovered_load_ranges: tuple[tuple[int, int], ...]
    predicates: tuple[PredicateResult, ...]


def derive_original_footprint(member, original, entry) -> OriginalFootprint:
    from harness.reconstruction.machinecode import ByteComparisonLeaf

    leaf = original.leaf
    byte = isinstance(leaf, ByteComparisonLeaf)
    return OriginalFootprint(
        member.key,
        original.covered_pcs,
        (),
        (),
        (),
        ((leaf.read_pc, leaf.address, 1),) if byte else (),
        () if leaf is None else leaf.conditions,
        (
            PredicateResult(
                "original-associated-uses",
                "matched" if leaf else "unknown",
                "complete conditional leaf observes one arbitrary current byte; backed initial contents do not establish runtime value or historical containing-object extent"
                if byte
                else "complete conditional leaf has no memory, call, relocation or binding effects"
                if leaf
                else "original instruction/effect census is unsupported",
            ),
            *entry.predicates,
        ),
    )


def derive_associated_closure(
    member,
    profile_id,
    requirements,
    original_facts,
    object_image,
    linked_image,
    measurements,
    original_leaf,
    compiled_leaf,
    entry,
    entry_symbol,
    *,
    symbol_producer: SymbolProducerFacts | None = None,
    candidate_backing: OriginalAssociatedFacts | None = None,
    source=None,
    link_inputs=None,
):
    from harness.reconstruction import calltargets, proofvalues
    from harness.reconstruction.objectfile import FunctionImage
    from harness.reconstruction.nativeoutput import ByteComparisonUnit
    from harness.reconstruction.machinecode import (
        ZERO_LEAF_CONDITIONS,
        BYTE_LEAF_CONDITIONS,
        ByteComparisonLeaf,
        EmptyReturnLeaf,
        ZeroReturnLeaf,
    )

    footprint = derive_original_footprint(member, original_leaf, entry)
    byte = isinstance(original_leaf.leaf, ByteComparisonLeaf)
    parsed = source.parsed if source is not None else None
    byte_source = (
        byte
        and isinstance(compiled_leaf.leaf, ByteComparisonLeaf)
        and isinstance(parsed, ByteComparisonUnit)
        and parsed.entry_symbol == entry_symbol
        and all(
            leaf.address == parsed.address
            and leaf.read_pc == parsed.read_pc
            and leaf.comparison == parsed.comparison
            for leaf in (original_leaf.leaf, compiled_leaf.leaf)
        )
    )
    visible = None
    expected_byte = None
    backing_byte = None
    byte_backed = False
    if (
        byte_source
        and isinstance(candidate_backing, OriginalAssociatedFacts)
        and candidate_backing.member_target == member.target
        and isinstance(original_facts, OriginalAssociatedFacts)
        and original_facts.member_target == member.target
    ):
        visible = next(
            (
                image
                for image in reversed(candidate_backing.images)
                if image.load_address
                <= parsed.address
                < image.load_address + len(image.content) - image.payload_offset
            ),
            None,
        )
        if visible is not None:
            originals = [
                image
                for image in original_facts.images
                if image.image_id == visible.image_id and image.target == visible.target
            ]
            expected_byte = _original_range(
                original_facts, visible.target, parsed.address, 1
            )
            offset = visible.payload_offset + parsed.address - visible.load_address
            backing_byte = visible.content[offset : offset + 1]
            byte_backed = (
                len(originals) == 1
                and originals[0].load_address == visible.load_address
                and expected_byte is not None
                and backing_byte == expected_byte
            )
    byte_symbols = set()
    byte_relocations = set()
    byte_binding = False
    byte_object_content = None
    binding_reason = (
        "single-byte source and complete compiled relocation flow are not established"
    )
    if byte_source and object_image is not None and linked_image is not None:
        functions = [
            symbol for symbol in object_image.symbols if symbol.name == entry_symbol
        ]
        objects = [
            symbol for symbol in object_image.symbols if symbol.name == parsed.symbol
        ]
        linked = [
            symbol for symbol in linked_image.symbols if symbol.name == parsed.symbol
        ]
        if len(functions) == len(objects) == len(linked) == 1:
            function, undefined, absolute = functions[0], objects[0], linked[0]
            section = next(
                (row for row in object_image.sections if row.index == function.section),
                None,
            )
            if (
                section is not None
                and section.content is not None
                and section.file_offset is not None
                and function.type == 2
                and function.value == 0
                and function.size == section.size == member.end - member.start
                and undefined.type
                == undefined.visibility
                == undefined.section
                == undefined.value
                == undefined.size
                == 0
                and undefined.binding == 1
                and absolute.info == 16
                and absolute.visibility == absolute.size == 0
                and absolute.section == 0xFFF1
                and absolute.value == parsed.address
            ):
                pair = [
                    (ordinal, row)
                    for ordinal, row in enumerate(object_image.relocations)
                    if row.symbol_name == parsed.symbol
                ]
                byte_object_content = b"".join(
                    (item.word & 0xFFFF0000 if index < 2 else item.word).to_bytes(
                        4, "little"
                    )
                    for index, item in enumerate(compiled_leaf.leaf.instructions)
                )
                image = FunctionImage(
                    entry_symbol,
                    object_image.sha256,
                    section.index,
                    section.name,
                    function.value,
                    section.file_offset,
                    section.content,
                    hashlib.sha256(section.content).hexdigest(),
                    object_image.relocations,
                )
                try:
                    byte_binding = (
                        section.content == byte_object_content
                        and len(pair) == 2
                        and all(row.symbol_index == undefined.index for _, row in pair)
                        and [row.offset for _, row in pair] == [0, 4]
                        and calltargets._data_view_bindings(
                            image, {parsed.symbol: (parsed.address, 1)}
                        )
                        == [[parsed.symbol, parsed.address]]
                        and not any(
                            row.get("symbol_name") == parsed.symbol
                            for row in linked_image.relocations
                        )
                    )
                except proofvalues.CallContractRefused as error:
                    binding_reason = str(error)
                if byte_binding:
                    byte_symbols = {
                        ("object", 0, undefined.index),
                        ("linked", absolute.table, absolute.index),
                    }
                    byte_relocations = {
                        ("object", row.section_index, ordinal) for ordinal, row in pair
                    }
                    binding_reason = "unique unsigned extern, actual complete compiled LUI/LBU words, one bounded HI16/LO16 pair and exact linked absolute address"
    (
        storage,
        uncovered_sections,
        uncovered_symbols,
        relocations,
        uncovered_relocations,
    ) = [], [], [], [], []
    if byte:
        storage.append(
            dict(
                owner="candidate-backing",
                image_id=visible.image_id if visible else None,
                target=visible.target if visible else None,
                address=original_leaf.leaf.address,
                memory_size=1,
                allocated=False,
                role="observed-external-byte",
                status="matched" if byte_backed else "unknown",
                initial_bytes_hex=backing_byte.hex()
                if backing_byte is not None
                else None,
                execution_value="arbitrary-current-byte-0-through-255",
            )
        )
    function_sections = {}
    section_roles = {}
    abi_sections = {}
    abi_descriptor = bytes.fromhex("000001000101000100000000000000000000000000000000")
    requirement_ranges = [
        (row.address, row.address + row.size)
        if isinstance(row, (InitializedRequirement, ZeroRequirement))
        else (row.location, row.location + 4)
        if isinstance(row, RelocationRequirement)
        else (row.address, row.address + 1)
        for row in requirements.known
    ]
    requirement_ranges.extend(
        (row.address, row.address + 1)
        for row in requirements.unresolved
        if row.address is not None
    )
    if byte:
        requirement_ranges.append(
            (original_leaf.leaf.address, original_leaf.leaf.address + 1)
        )
    supported_leaf = original_leaf.leaf is not None and compiled_leaf.leaf is not None
    for owner, image in (("object", object_image), ("linked", linked_image)):
        if image is None or image.layout is None:
            uncovered_sections.append((owner, -1))
            continue
        if image.layout.flags not in {0, 1} or image.layout.entry != (
            0 if owner == "object" else member.start
        ):
            uncovered_sections.append((f"{owner}-header-role", -1))
        for offset, size, zero_padding in image.layout.padding:
            storage.append(
                dict(
                    owner=owner,
                    section_index=-1,
                    file_offset=offset,
                    memory_size=size,
                    allocated=False,
                    role="container-padding",
                    status="matched" if zero_padding else "unknown",
                )
            )
            if not zero_padding:
                uncovered_sections.append((f"{owner}-physical-gap", offset))
        ranges = [
            (row.file_offset, row.file_offset + row.size, row.index)
            for row in image.sections
            if row.file_offset is not None and row.size
        ]
        for index, (first, end, section_index) in enumerate(ranges):
            for other_first, other_end, other_index in ranges[index + 1 :]:
                if first < other_end and other_first < end:
                    uncovered_sections.extend(
                        ((owner, section_index), (owner, other_index))
                    )
        matches = [symbol for symbol in image.symbols if symbol.name == entry_symbol]
        function = matches[0] if len(matches) == 1 else None
        if function is not None and 0 < function.section < len(image.sections):
            function_sections[owner] = image.sections[function.section]
        descriptors = [
            row
            for row in image.sections
            if row.name == ".MIPS.abiflags" or row.kind == 0x7000002A
        ]
        for section in image.sections:
            role, state = "unaccounted", "unknown"
            candidate_function = function_sections.get(owner)
            raw = image.layout.sections[section.index]
            link, info, entry_size = (
                (raw[6], raw[7], raw[9])
                if owner == "object"
                else (raw.link, raw.info, raw.entry_size)
            )
            if (
                section.index == 0
                and section.kind == 0
                and section.size == section.flags == 0
            ):
                role, state = "null", "matched"
            elif (
                section.name in {".data", ".bss"}
                and section.kind == (1 if section.name == ".data" else 8)
                and section.flags == 3
                and section.size == link == info == entry_size == 0
                and section.content == (b"" if section.kind == 1 else None)
            ):
                role, state = "empty-data", "matched"
            elif (
                len(descriptors) == 1
                and descriptors[0] == section
                and section.name == ".MIPS.abiflags"
                and section.kind == 0x7000002A
                and section.flags == 2
                and section.size == entry_size == 24
                and section.alignment == 8
                and link == info == 0
                and section.content == abi_descriptor
                and (
                    owner == "object"
                    and section.address == 0
                    or owner == "linked"
                    and section.address % 8 == 0
                    and all(
                        not (
                            section.address < row.address + row.size
                            and row.address < section.address + section.size
                        )
                        for row in image.sections
                        if row.index != section.index and row.flags & 2 and row.size
                    )
                    and all(
                        not (section.address < end and first < section.address + 24)
                        for first, end in requirement_ranges
                    )
                    and all(
                        not (
                            section.address < row.address + row.memory_size
                            and row.address < section.address + 24
                        )
                        or (
                            row.address == section.address
                            and row.offset == section.file_offset
                            and row.file_size == row.memory_size == 24
                            and row.flags == 4
                        )
                        for row in image.segments
                    )
                )
            ):
                role, state = "abi-metadata", "matched"
                abi_sections[owner] = section
            elif section.flags & 2:
                expected_value = 0 if owner == "object" else member.start
                if (
                    supported_leaf
                    and candidate_function == section
                    and function.value == expected_value
                    and function.size == member.end - member.start
                    and section.size == function.size
                    and section.kind == 1
                    and section.flags == 6
                    and section.content is not None
                    and section.content
                    == (
                        byte_object_content
                        if owner == "object" and byte_binding
                        else b"".join(
                            item.word.to_bytes(4, "little")
                            for item in original_leaf.leaf.instructions
                        )
                    )
                    and (
                        owner == "object"
                        and section.address == 0
                        or owner == "linked"
                        and section.address == member.start
                    )
                    and section.alignment > 0
                    and member.start % section.alignment == 0
                ):
                    role, state = "function", "matched"
            elif (
                section.kind in {2, 3}
                and section.flags == 0
                or section.kind in {4, 9}
                and section.flags in {0, 0x40}
            ):
                role, state = (
                    {
                        2: "symbol-table",
                        3: "string-table",
                        4: "relocation-table",
                        9: "relocation-table",
                    }[section.kind],
                    "matched",
                )
            elif (
                section.name == ".comment"
                and section.kind == 1
                and section.flags in {0, 0x30}
                and section.content is not None
                and section.size <= 65536
                and (not section.content or section.content[-1] == 0)
                and all(
                    byte == 0 or byte in {9, 10, 13} or 32 <= byte < 127
                    for byte in section.content
                )
            ):
                role, state = "compiler-comment", "matched"
            elif (
                section.content is not None
                and len(section.content) == section.size
                and section.flags == link == info == 0
                and (
                    section.name == ".reginfo"
                    and section.kind == 0x70000006
                    and section.size == 24
                    and section.alignment == 4
                    and entry_size == 1
                    or section.name == ".pdr"
                    and section.kind == 1
                    and section.size % 32 == 0
                    and section.alignment == 4
                    and entry_size == 0
                    or section.name
                    in {".debug_line", ".debug_info", ".debug_abbrev", ".debug_aranges"}
                    and section.kind == 0x7000001E
                    and entry_size == 0
                )
            ):
                role, state = "nonallocated-metadata", "matched"
            elif (
                section.name == ".debug_str"
                and section.kind == 0x7000001E
                and section.flags == 0x30
                and link == info == 0
                and entry_size == 1
                and section.content is not None
                and len(section.content) == section.size
                and (not section.content or section.content[-1] == 0)
            ):
                role, state = "nonallocated-metadata", "matched"
            elif (
                section.name == ".gnu.attributes"
                and section.kind == 0x6FFFFFF5
                and section.flags == link == info == entry_size == 0
                and section.alignment == 1
                and section.size == 16
                and section.content is not None
                and len(section.content) == 16
                and section.content[0:1] == b"A"
                and int.from_bytes(section.content[1:5], "little") == 15
                and section.content[5:10] == b"gnu\x00\x01"
                and int.from_bytes(section.content[10:14], "little") == 7
                and section.content[14:16] == b"\x04\x01"
            ):
                role, state = "nonallocated-metadata", "matched"
            record = dict(
                owner=owner,
                section_index=section.index,
                file_offset=section.file_offset,
                memory_size=section.size,
                address=section.address,
                allocated=bool(section.flags & 2),
                role=role,
                status=state,
            )
            storage.append(record)
            if state != "matched":
                uncovered_sections.append((owner, section.index))
            else:
                section_roles[owner, section.index] = role
        for symbol in image.symbols:
            typ = symbol.type if owner == "object" else symbol.info & 15
            table = 0 if owner == "object" else symbol.table
            ordinary_container = (
                symbol.index == 0
                and not symbol.value
                and not symbol.size
                and not symbol.name
                or typ == 4
                and symbol.section == 0xFFF1
                and not symbol.value
                and not symbol.size
                or typ == 3
                and 0 < symbol.section < len(image.sections)
                and not symbol.size
                and symbol.value
                == (0 if owner == "object" else image.sections[symbol.section].address)
            )
            selected_function = (
                function is not None
                and symbol == function
                and typ == 2
                and symbol.size == member.end - member.start
            )
            if not (
                ordinary_container
                or selected_function
                or (owner, table, symbol.index) in byte_symbols
            ):
                uncovered_symbols.append((owner, table, symbol.index))
        for ordinal, relocation in enumerate(image.relocations):
            row = asdict(relocation) if owner == "object" else dict(relocation)
            target = image.sections[row["target_section"]]
            relative = row["offset"] - (0 if owner == "object" else target.address)
            metadata_only = (
                row["kind"] == 2
                and section_roles.get((owner, row["section_index"]))
                == "relocation-table"
                and section_roles.get((owner, target.index)) == "nonallocated-metadata"
                and not target.flags & 6
                and target.content is not None
                and 0 <= relative <= target.size - 4
                and (owner != "object" or row["storage_bytes"] == 4)
            )
            address_pair = (owner, row["section_index"], ordinal) in byte_relocations
            row.update(
                owner=owner,
                ordinal=ordinal,
                status="matched" if metadata_only or address_pair else "unknown",
                reason="bounded R_MIPS_32 write into nonallocated measurement metadata"
                if metadata_only
                else binding_reason
                if address_pair
                else "actual relocation target storage and original accounting meaning require an independent rule",
            )
            relocations.append(row)
            if not (metadata_only or address_pair):
                uncovered_relocations.append((owner, row["section_index"], ordinal))
    load_ranges = []
    section = function_sections.get("linked")
    if (
        linked_image is None
        or linked_image.layout is None
        or section is None
        or not linked_image.segments
    ):
        load_ranges.append((member.start, member.end))
    else:
        descriptor = abi_sections.get("linked")
        function_loads = metadata_loads = abi_programs = 0
        accepted_loads = []
        for program in linked_image.layout.programs:
            (
                kind,
                offset,
                address,
                physical,
                file_size,
                memory_size,
                flags,
                alignment,
            ) = program
            valid_alignment = alignment in {0, 1} or (
                alignment > 1
                and not alignment & (alignment - 1)
                and address % alignment == offset % alignment
            )
            if (
                kind == 1
                and address == physical == member.start
                and memory_size == member.end - member.start
                and file_size == memory_size
                and offset == section.file_offset
                and flags == 5
                and valid_alignment
            ):
                function_loads += 1
                accepted_loads.append(
                    (address, address + memory_size, offset, offset + file_size)
                )
            elif (
                descriptor is not None
                and address == physical == descriptor.address
                and offset == descriptor.file_offset
                and file_size == memory_size == descriptor.size
                and flags == 4
                and valid_alignment
                and (kind == 1 or kind == 0x70000003 and alignment == 8)
            ):
                if kind == 1:
                    metadata_loads += 1
                    accepted_loads.append(
                        (address, address + memory_size, offset, offset + file_size)
                    )
                else:
                    abi_programs += 1
            else:
                load_ranges.append((address, address + memory_size))
        if function_loads != 1:
            load_ranges.append((member.start, member.end))
        if descriptor is not None and (metadata_loads != 1 or abi_programs != 1):
            load_ranges.append(
                (descriptor.address, descriptor.address + descriptor.size)
            )
        for index, first in enumerate(accepted_loads):
            for other in accepted_loads[index + 1 :]:
                if (
                    first[0] < other[1]
                    and other[0] < first[1]
                    or first[2] < other[3]
                    and other[2] < first[3]
                ):
                    load_ranges.extend((first[:2], other[:2]))
    if object_image.layout is not None and object_image.layout.program_table[1]:
        uncovered_sections.append(("object-program-table", -1))
    unresolved = tuple(
        f"{row.category}:{index}" for index, row in enumerate(requirements.unresolved)
    )
    known_states = {row.status for row in measurements.known}
    entry_closed = all(row.status == "matched" for row in entry.predicates)
    conditional_empty = (
        type(original_leaf.leaf) in (EmptyReturnLeaf, ZeroReturnLeaf)
        and supported_leaf
        and not requirements.known
        and not uncovered_relocations
    )
    unresolved_resolved = (
        conditional_empty
        and entry_closed
        and all(
            row.category
            in {
                "initialized_data",
                "zero_initialized",
                "relocations",
                "external_bindings",
            }
            and row.address is None
            for row in requirements.unresolved
        )
    )
    if unresolved_resolved:
        unresolved = ()
    byte_link = (
        byte_source
        and byte_binding
        and byte_backed
        and bool(source.predicates)
        and all(row.status == "matched" for row in source.predicates)
        and type(link_inputs) is dict
        and link_inputs.get("status") == "measured"
        and link_inputs.get("entry_symbol") == entry_symbol
        and link_inputs.get("bindings") == [[parsed.symbol, parsed.address]]
        and link_inputs.get("section_addresses") == {}
    )
    complete_leaf = (
        bool(entry.predicates)
        and entry.member_key == member.key
        and entry.conditions == (BYTE_LEAF_CONDITIONS if byte else ZERO_LEAF_CONDITIONS)
        and not entry.uncovered_original_code
        and type(original_leaf.leaf)
        in (EmptyReturnLeaf, ZeroReturnLeaf, ByteComparisonLeaf)
        and type(compiled_leaf.leaf) is type(original_leaf.leaf)
        and original_leaf.leaf == compiled_leaf.leaf
        and all(
            facts.leaf.member_key == member.key
            and facts.expected_pcs == tuple(range(member.start, member.end, 4))
            and facts.covered_pcs == facts.expected_pcs
            and not facts.unrecognized_pcs
            and tuple(item.pc for item in facts.leaf.instructions) == facts.expected_pcs
            and facts.leaf.conditions == entry.conditions
            and bool(facts.predicates)
            and all(row.status == "matched" for row in facts.predicates)
            for facts in (original_leaf, compiled_leaf)
        )
    )
    if (
        type(symbol_producer) is SymbolProducerFacts
        and object_image is not None
        and linked_image is not None
        and symbol_producer.object_sha256 == object_image.sha256
        and symbol_producer.linked_sha256 == linked_image.sha256
        and (symbol_producer.restricted_link or byte_link)
        and (unresolved_resolved or byte_link)
        and not measurements.prior_failures
        and not (uncovered_sections or uncovered_relocations or load_ranges)
        and complete_leaf
    ):
        qualified = _producer_symbol_ids(
            symbol_producer,
            object_image,
            linked_image,
            function_sections,
            section_roles,
            relocations,
        )
        uncovered_symbols = [
            identity for identity in uncovered_symbols if identity not in qualified
        ]
    physical_closed = not (
        uncovered_sections or uncovered_symbols or uncovered_relocations or load_ranges
    )
    byte_defaults_closed = (
        byte
        and complete_leaf
        and entry_closed
        and byte_link
        and physical_closed
        and not measurements.prior_failures
        and requirements.member_key == footprint.member_key == member.key
        and measurements.profile_id == profile_id
        and requirements.inventory_claim == "unresolved"
        and not requirements.known
        and not measurements.known
        and measurements.independent_unknowns == requirements.unresolved
        and Counter(row.category for row in requirements.unresolved)
        == Counter(OBLIGATION_CATEGORY[1:])
        and all(
            row.address is None
            and row.reason
            == (
                f"{row.category} obligations are not independently derived during preparation; "
                "independent evaluator verification owns this category"
            )
            for row in requirements.unresolved
        )
        and footprint.instruction_pcs == original_leaf.expected_pcs
        and footprint.conditions == BYTE_LEAF_CONDITIONS
        and not footprint.initialized_uses
        and not footprint.zero_uses
        and not footprint.relocation_uses
        and footprint.binding_uses
        == ((original_leaf.leaf.read_pc, original_leaf.leaf.address, 1),)
        and bool(footprint.predicates)
        and all(row.status == "matched" for row in footprint.predicates)
    )
    if byte_defaults_closed:
        unresolved = ()
    state = (
        "failed"
        if measurements.prior_failures or "failed" in known_states
        else "missing"
        if "missing" in known_states
        else "matched"
        if supported_leaf
        and entry_closed
        and physical_closed
        and (
            not byte
            or byte_source
            and byte_binding
            and byte_backed
            and all(row.status == "matched" for row in source.predicates)
        )
        and not unresolved
        and known_states <= {"matched"}
        else "unknown"
    )
    predicates = [
        PredicateResult(
            "inventory-closure",
            state,
            "exact preparation defaults resolved by the complete conditional one-byte relation and physical census; historical object extent, initializer, loader initialization, qualifiers, aliases, spelling and current runtime value remain unknown"
            if state == "matched" and byte_defaults_closed
            else "strict observed-byte storage, symbol, relocation and load census"
            if state == "matched" and byte
            else "strict empty-footprint storage, symbols, relocation and load census"
            if state == "matched"
            else "unresolved requirements, entry scope or actual container/loading/storage residuals remain",
        )
    ]
    if byte:
        predicates.extend(
            (
                PredicateResult(
                    "observed-byte-backing",
                    "matched" if byte_backed else "unknown",
                    "visible candidate-delivered byte matches independent original bytes and placement; current execution byte, original initializer and containing-object extent remain unknown",
                    actual=dict(
                        address=original_leaf.leaf.address,
                        image_id=visible.image_id if visible else None,
                        target=visible.target if visible else None,
                        candidate_initial_hex=backing_byte.hex()
                        if backing_byte is not None
                        else None,
                        original_initial_hex=expected_byte.hex()
                        if expected_byte is not None
                        else None,
                    ),
                ),
                PredicateResult(
                    "observed-byte-binding",
                    "matched" if byte_binding else "unknown",
                    binding_reason,
                    actual=dict(
                        symbol=parsed.symbol,
                        address=parsed.address,
                        read_pc=parsed.read_pc,
                        width=8,
                        compiled_words=[
                            item.word for item in compiled_leaf.leaf.instructions[:2]
                        ],
                    )
                    if byte_source
                    else None,
                ),
                PredicateResult(
                    "physical-census",
                    "matched" if physical_closed else "unknown",
                    "complete actual storage, symbol, relocation and load census; historical associated inventory remains separate",
                ),
            )
        )
    byte_category_reasons = {
        "associated-contents": "one observed byte has independently matched delivered initial backing; execution relates the same arbitrary stable current byte without proving a historical initializer or current runtime value",
        "placement": "the observed one-byte read has the same proven address and target-qualified visible owner in candidate backing and independent originals; historical containing-object extent remains unknown",
        "initialization": "the complete read-only leaf and extern byte view without a storage definition impose no zero-value dependence or allocated initialization; historical initializer and loader initialization behavior remain unknown",
        "relocations": "original operands prove the fixed byte address, the actual HI16/LO16 pair reproduces it, and the full census accounts for every remaining relocation; historical relocation spelling remains unknown",
        "bindings": "the sole target-qualified current-byte read matches the mutable unsigned extern, visible backing and actual linked/register-flow binding; historical object extent, qualifiers, aliases and name remain unknown",
    }
    for name in (
        "associated-contents",
        "placement",
        "initialization",
        "relocations",
        "bindings",
    ):
        predicates.append(
            PredicateResult(
                name,
                state,
                byte_category_reasons[name]
                if state == "matched" and byte_defaults_closed
                else "complete independently derived conditional observed-byte category coverage"
                if state == "matched" and byte
                else "complete independently derived empty associated closure"
                if state == "matched"
                else "associated category requires full original-qualified scope and storage/requirement coverage",
            )
        )
    return AssociatedClosureFacts(
        member.key,
        profile_id,
        footprint,
        measurements,
        tuple(storage),
        tuple(relocations),
        unresolved,
        tuple(uncovered_sections),
        tuple(uncovered_symbols),
        tuple(uncovered_relocations),
        tuple(load_ranges),
        tuple(predicates),
    )


def parse_member_requirements(
    member_key: str, value: dict, contents: dict[str, bytes]
) -> MemberRequirements:
    validate_obligations(value, f"member {member_key} obligations")
    known: list[KnownRequirement] = []
    for row in value["initialized_data"]:
        expected = contents.get(row["id"])
        contract.require(
            type(expected) is bytes and len(expected) == row["size"],
            "initialized requirement contents do not have its exact size",
        )
        known.append(
            InitializedRequirement(
                row["id"], row["address"], row["size"], expected, row["placement"]
            )
        )
    known.extend(ZeroRequirement(**row) for row in value["zero_initialized"])
    known.extend(RelocationRequirement(**row) for row in value["relocations"])
    known.extend(BindingRequirement(**row) for row in value["external_bindings"])
    for requirement in known:
        if isinstance(requirement, (InitializedRequirement, ZeroRequirement)):
            contract.require(
                requirement.address + requirement.size <= 2**32,
                "independent associated extent exceeds the address space",
            )
    return MemberRequirements(
        member_key,
        tuple(known),
        tuple(UnresolvedRequirement(**row) for row in value["unresolved"]),
        value["inventory"],
    )


def _predicate(
    name: str, condition: bool, *, expected=None, actual=None, reason=""
) -> PredicateResult:
    return PredicateResult(
        name, "matched" if condition else "failed", reason, expected, actual
    )


def _unknown(name: str, reason: str, *, expected=None, actual=None) -> PredicateResult:
    return PredicateResult(name, "unknown", reason, expected, actual)


def _original_range(
    facts: OriginalAssociatedFacts, target: str, address: int, size: int
) -> bytes | None:
    images = [
        image
        for image in facts.images
        if image.target == target
        and image.load_address <= address
        and address + size
        <= image.load_address + len(image.content) - image.payload_offset
    ]
    if len(images) != 1:
        return None
    image = images[0]
    offset = image.payload_offset + address - image.load_address
    return image.content[offset : offset + size]


def _range_predicates(
    requirement: InitializedRequirement | ZeroRequirement, image: LinkedImage
) -> tuple[list[PredicateResult], LinkedStorage | None]:
    address, size = requirement.address, requirement.size
    matches = [
        section
        for section in image.sections
        if section.flags & 2
        and section.address <= address
        and address + size <= section.address + section.size
    ]
    predicates = [
        _unknown(
            "placement_meaning",
            "placement string has no established comparison protocol",
            expected=requirement.placement,
        )
    ]
    if len(matches) != 1:
        predicates.append(
            PredicateResult(
                "fixed_range",
                "missing" if not matches else "failed",
                "required allocated range is absent or ambiguous",
                [address, address + size],
                [row.index for row in matches],
            )
        )
        return predicates, None
    section = matches[0]
    predicates.append(
        _predicate(
            "fixed_range",
            True,
            expected=[address, address + size],
            actual=[section.address, section.address + section.size],
        )
    )
    predicates.append(
        _predicate("allocation", bool(section.flags & 2), actual=section.flags)
    )
    predicates.append(
        _predicate(
            "alignment",
            not section.alignment or section.address % section.alignment == 0,
            actual=section.alignment,
        )
    )
    overlaps = [
        row.index
        for row in image.sections
        if row.index != section.index
        and row.flags & 2
        and row.size
        and row.address < address + size
        and address < row.address + row.size
    ]
    predicates.append(_predicate("overlap", not overlaps, actual=overlaps))
    symbols = [
        symbol
        for symbol in image.symbols
        if symbol.info & 15 == 1
        and symbol.section == section.index
        and symbol.value == address
    ]
    if len(symbols) == 1:
        predicates.append(
            _predicate(
                "object_extent",
                symbols[0].size == size,
                expected=size,
                actual=symbols[0].size,
            )
        )
    else:
        predicates.append(
            _unknown(
                "object_extent",
                "independent object identity is unavailable or ambiguous",
                expected=size,
                actual=[symbol.size for symbol in symbols],
            )
        )
    return predicates, section


def _contents_predicates(
    requirement: InitializedRequirement,
    image: LinkedImage,
    facts: OriginalAssociatedFacts,
) -> list[PredicateResult]:
    predicates, section = _range_predicates(requirement, image)
    if section is not None:
        if section.content is None:
            predicates.append(
                PredicateResult(
                    "contents",
                    "missing",
                    "required initialized bytes have no file payload",
                )
            )
        else:
            offset = requirement.address - section.address
            actual = section.content[offset : offset + requirement.size]
            first = next(
                (
                    index
                    for index, pair in enumerate(zip(requirement.expected, actual))
                    if pair[0] != pair[1]
                ),
                None,
            )
            predicates.append(
                _predicate(
                    "contents",
                    actual == requirement.expected,
                    expected=hashlib.sha256(requirement.expected).hexdigest(),
                    actual=dict(
                        bytes=len(actual),
                        sha256=hashlib.sha256(actual).hexdigest(),
                        first_difference=first,
                    ),
                )
            )
            segments = [
                segment
                for segment in image.segments
                if segment.address <= requirement.address
                and requirement.address + requirement.size
                <= segment.address + segment.file_size
                and segment.offset + requirement.address - segment.address
                == section.file_offset + offset
            ]
            predicates.append(
                _predicate(
                    "segment_containment",
                    len(segments) == 1,
                    actual=[segment.index for segment in segments],
                )
                if image.segments
                else _unknown(
                    "segment_containment", "linked image has no load-segment evidence"
                )
            )
    original = _original_range(
        facts, facts.member_target, requirement.address, requirement.size
    )
    predicates.append(
        _unknown(
            "original_contents",
            "required range is not mapped by a complete bound image",
        )
        if original is None
        else _predicate(
            "original_contents",
            original == requirement.expected,
            expected=hashlib.sha256(requirement.expected).hexdigest(),
            actual=hashlib.sha256(original).hexdigest(),
        )
    )
    return predicates


def _zero_predicates(
    requirement: ZeroRequirement, image: LinkedImage
) -> list[PredicateResult]:
    predicates, section = _range_predicates(requirement, image)
    if section is not None:
        predicates.append(_predicate("nobits", section.kind == 8, actual=section.kind))
        segments = [
            segment
            for segment in image.segments
            if segment.address + segment.file_size <= requirement.address
            and requirement.address + requirement.size
            <= segment.address + segment.memory_size
        ]
        conflicts = [
            segment.index
            for segment in image.segments
            if segment.file_size
            and segment.address < requirement.address + requirement.size
            and requirement.address < segment.address + segment.file_size
        ]
        predicates.append(
            _predicate(
                "elf_zero_fill",
                len(segments) == 1 and not conflicts,
                actual=dict(
                    segments=[row.index for row in segments], conflicts=conflicts
                ),
            )
        )
    predicates.append(
        _unknown(
            "original_loader",
            "ELF zero-fill does not prove original PSX initialization",
        )
    )
    return predicates


def _section_mapping(
    section: ObjectStorage, object_image: ObjectImage, linked_image: LinkedImage
) -> LinkedStorage | None:
    if len([row for row in object_image.sections if row.name == section.name]) != 1:
        return None
    matches = [
        row
        for row in linked_image.sections
        if row.name == section.name and row.size == section.size
    ]
    if len(matches) != 1:
        return None
    linked = matches[0]
    if section.kind != linked.kind or section.flags & 7 != linked.flags & 7:
        return None
    anchors = 0
    for symbol in object_image.symbols:
        if symbol.section != section.index or not symbol.name or not symbol.size:
            continue
        definitions = [row for row in linked_image.symbols if row.name == symbol.name]
        if (
            len(definitions) != 1
            or definitions[0].section != linked.index
            or definitions[0].value != linked.address + symbol.value
        ):
            return None
        if definitions[0].size != symbol.size:
            return None
        anchors += 1
    if not anchors:
        return None
    if section.content is not None and linked.content is not None:
        ignored = set()
        for relocation in object_image.relocations:
            if relocation.target_section != section.index:
                continue
            if relocation.storage_bytes is None:
                return None
            span = set(
                range(relocation.offset, relocation.offset + relocation.storage_bytes)
            )
            if ignored & span:
                return None
            ignored.update(span)
        if any(
            left != right
            for index, (left, right) in enumerate(zip(section.content, linked.content))
            if index not in ignored
        ):
            return None
    return linked


def _binding_predicates(
    requirement: BindingRequirement,
    object_image: ObjectImage,
    image: LinkedImage,
    facts: OriginalAssociatedFacts,
) -> list[PredicateResult]:
    symbols = [symbol for symbol in image.symbols if symbol.name == requirement.symbol]
    definitions = [symbol for symbol in symbols if symbol.section != 0]
    predicates = [
        _predicate(
            "unique_definition",
            len(symbols) == len(definitions) == 1,
            actual=[asdict(row) for row in symbols],
        )
    ]
    if not definitions:
        predicates.append(
            PredicateResult(
                "binding",
                "missing",
                "required external symbol has no linked definition",
            )
        )
    elif len(definitions) == 1:
        predicates.append(
            _predicate(
                "binding_address",
                definitions[0].value == requirement.address,
                expected=requirement.address,
                actual=definitions[0].value,
            )
        )
    object_symbols = [
        symbol for symbol in object_image.symbols if symbol.name == requirement.symbol
    ]
    predicates.append(
        _predicate(
            "object_reference",
            any(
                row.section == 0
                and row.binding in {1, 2}
                and row.type in {0, 1, 2}
                and not row.visibility & ~3
                for row in object_symbols
            ),
            actual=[asdict(row) for row in object_symbols],
        )
    )
    predicates.append(
        _predicate(
            "conflicting_definitions",
            not any(row.section != 0 for row in object_symbols),
            actual=[asdict(row) for row in object_symbols if row.section != 0],
        )
    )
    if any(
        row.binding not in {1, 2} or row.type not in {0, 1, 2} or row.visibility & ~3
        for row in object_symbols
    ):
        predicates.append(
            _unknown(
                "symbol_interpretation",
                "binding symbol type, binding or visibility is unsupported",
            )
        )
    original = _original_range(facts, requirement.target, requirement.address, 1)
    predicates.append(
        _predicate(
            "bound_target_address",
            original is not None,
            expected=dict(target=requirement.target, address=requirement.address),
            actual=[
                dict(target=row.target, image_id=row.image_id)
                for row in facts.images
                if row.target == requirement.target
            ],
        )
    )
    predicates.append(
        _unknown(
            "binding_target_provenance",
            "numeric ELF symbols do not encode the selected original target",
        )
    )
    predicates.append(
        _unknown("target_residency", "original target residency is not established")
    )
    return predicates


def _relocation_predicates(
    requirement: RelocationRequirement,
    requirements: MemberRequirements,
    object_image: ObjectImage,
    image: LinkedImage,
    facts: OriginalAssociatedFacts,
) -> list[PredicateResult]:
    matches = []
    unmapped = []
    for row in object_image.relocations:
        section = object_image.sections[row.target_section]
        linked = _section_mapping(section, object_image, image)
        if linked is None:
            if row.symbol_name == requirement.symbol:
                unmapped.append(row)
            continue
        if linked.address + row.offset == requirement.location:
            matches.append((row, section, linked))
    if len(matches) != 1:
        if not matches and unmapped:
            return [
                _unknown(
                    "relocation_mapping",
                    "required symbol has relocations with unproven section mapping",
                )
            ]
        return [
            PredicateResult(
                "relocation",
                "missing" if not matches else "failed",
                "independent relocation location is absent or ambiguous",
                requirement.location,
                [asdict(row[0]) for row in matches],
            )
        ]
    row, section, linked = matches[0]
    predicates = [
        _predicate(
            "relocation_symbol",
            row.symbol_name == requirement.symbol,
            expected=requirement.symbol,
            actual=row.symbol_name,
        )
    ]
    if requirement.kind != "R_MIPS_32":
        predicates.append(
            _unknown(
                "relocation_kind",
                "unsupported independent relocation kind",
                expected=requirement.kind,
                actual=row.kind,
            )
        )
        return predicates
    predicates.append(
        _predicate("relocation_kind", row.kind == 2, expected=2, actual=row.kind)
    )
    if row.kind != 2 or row.storage_bytes != 4:
        predicates.append(
            _unknown("resolution", "relocation storage or calculation is unsupported")
        )
        return predicates
    symbol = object_image.symbols[row.symbol_index]
    if symbol.binding == 0 and symbol.type == 3:
        predicates.append(
            _unknown(
                "resolution",
                "local section-symbol relocation requires distinct MIPS section interpretation",
            )
        )
        return predicates
    if (
        symbol.binding not in {1, 2}
        or symbol.type not in {0, 1, 2}
        or symbol.visibility & ~3
    ):
        predicates.append(
            _unknown("resolution", "relocation symbol interpretation is unsupported")
        )
        return predicates
    bindings = [
        item
        for item in requirements.known
        if isinstance(item, BindingRequirement) and item.symbol == requirement.symbol
    ]
    if len(bindings) != 1 or section.content is None or linked.content is None:
        predicates.append(
            _unknown(
                "resolution",
                "unique independent binding or relocation storage is unavailable",
            )
        )
        return predicates
    addend = (
        row.addend
        if row.addend is not None
        else int.from_bytes(section.content[row.offset : row.offset + 4], "little")
    )
    expected = (bindings[0].address + addend) & 0xFFFFFFFF
    actual_bytes = linked.content[row.offset : row.offset + 4]
    original = _original_range(facts, facts.member_target, requirement.location, 4)
    predicates.append(
        _predicate(
            "resolved_word",
            len(actual_bytes) == 4
            and int.from_bytes(actual_bytes, "little") == expected,
            expected=expected,
            actual=int.from_bytes(actual_bytes, "little"),
        )
    )
    definitions = [
        item
        for item in image.symbols
        if item.name == requirement.symbol and item.section != 0
    ]
    predicates.append(
        _predicate(
            "resolved_symbol",
            len(definitions) == 1 and definitions[0].value == bindings[0].address,
            expected=bindings[0].address,
            actual=[item.value for item in definitions],
        )
    )
    predicates.append(
        _unknown(
            "original_word",
            "relocation location is not mapped by the bound original image",
        )
        if original is None
        else _predicate(
            "original_word",
            int.from_bytes(original, "little") == expected,
            expected=expected,
            actual=int.from_bytes(original, "little"),
        )
    )
    pending = [
        record
        for record in image.relocations
        if record["offset"] == requirement.location
    ]
    predicates.append(_predicate("remaining_relocation", not pending, actual=pending))
    predicates.append(
        _unknown(
            "binding_target_provenance",
            "relocation value does not prove original target residency",
        )
    )
    return predicates


def measure_associated_requirements(
    requirements: MemberRequirements,
    *,
    object_image: ObjectImage,
    linked_image: LinkedImage | None,
    original_facts: OriginalAssociatedFacts,
    prior_failures: tuple[str, ...] = (),
    profile_id: str = "",
) -> ProfileRequirementsResult:
    rows = []
    for requirement in requirements.known:
        if isinstance(requirement, InitializedRequirement):
            category = "initialized_data"
        elif isinstance(requirement, ZeroRequirement):
            category = "zero_initialized"
        elif isinstance(requirement, RelocationRequirement):
            category = "relocations"
        else:
            category = "external_bindings"
        if linked_image is None:
            predicates = [
                _unknown(
                    "linked_artifact", "prior comparison did not produce a linked image"
                )
            ]
        elif isinstance(requirement, InitializedRequirement):
            predicates = _contents_predicates(requirement, linked_image, original_facts)
        elif isinstance(requirement, ZeroRequirement):
            predicates = _zero_predicates(requirement, linked_image)
        elif isinstance(requirement, RelocationRequirement):
            predicates = _relocation_predicates(
                requirement, requirements, object_image, linked_image, original_facts
            )
        else:
            predicates = _binding_predicates(
                requirement, object_image, linked_image, original_facts
            )
        rows.append(RequirementResult(requirement.id, category, tuple(predicates)))
    allocated = []
    section_census = []
    symbol_census = []
    for owner, image in (("object", object_image), ("linked", linked_image)):
        if image is None:
            continue
        for section in image.sections:
            record = dict(
                owner=owner,
                index=section.index,
                name=section.name,
                kind=section.kind,
                flags=section.flags,
                address=section.address,
                size=section.size,
                alignment=section.alignment,
                file_offset=section.file_offset,
                sha256=None
                if section.content is None
                else hashlib.sha256(section.content).hexdigest(),
            )
            section_census.append(record)
            if section.flags & 2:
                allocated.append(
                    dict(
                        owner=owner,
                        index=section.index,
                        name=section.name,
                        kind=section.kind,
                        flags=section.flags,
                        address=section.address,
                        size=section.size,
                        alignment=section.alignment,
                        file_offset=section.file_offset,
                        sha256=None
                        if section.content is None
                        else hashlib.sha256(section.content).hexdigest(),
                        authority="unaccounted",
                    )
                )
        symbol_census.extend(
            dict(owner=owner, **asdict(symbol)) for symbol in image.symbols
        )
    return ProfileRequirementsResult(
        profile_id,
        tuple(rows),
        requirements.unresolved,
        tuple(allocated),
        tuple(asdict(row) for row in object_image.relocations),
        prior_failures,
        symbols=tuple(symbol_census),
        sections=tuple(section_census),
        segments=()
        if linked_image is None
        else tuple(asdict(row) for row in linked_image.segments),
        linked_relocations=() if linked_image is None else linked_image.relocations,
    )
