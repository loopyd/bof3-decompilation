"""Byte-faithful native output decoding; captured source grants no semantic authority."""

from __future__ import annotations

import hashlib
import json
import math
import re
from dataclasses import dataclass
from enum import Enum
from typing import TYPE_CHECKING

from harness.common.artifacts import encode_json
from harness.reconstruction import callcontracts, calltargets, proofvalues, provenance

if TYPE_CHECKING:
    from harness.reconstruction.machinecode import LeafFacts
    from harness.reconstruction.obligations import (
        OriginalAssociatedFacts,
        PredicateResult,
    )

TOKEN_LIMIT = 8 << 20
SOURCE_LIMIT = 4 << 20
CONFIG_LIMIT = 2 << 20
STATUS_LIMIT = 256 << 10
TOKEN_COUNT = 131072
CONFIG_SHA256 = "6297157463f15283a86e5ed177bef059e0b7a360724b2741cdd920099334ad3b"
TOKEN_KINDS = frozenset(
    "nl ws punc op i_gvar i_lvar i_mem i_lab i_fnc i_arg keyw type preproc inc "
    "l_bool l_int l_fp l_str l_sym l_ptr cmnt".split()
)
STATUS_KEYS = frozenset(
    "schema status reason request_bytes_hex request_byte_count request_sha256 "
    "configuration_bytes_hex configuration_byte_count configuration_sha256 "
    "release_configuration_sha256 operation requested_range invoked_range "
    "range_convention retdec_invoked retdec_returned retdec_failed backend_ranges "
    "backend_selected_ranges initial_input_invariants".split()
) | frozenset(
    "tokens_bytes tokens_sha256 inferred_configuration_bytes inferred_configuration_sha256 "
    "tokens_complete inferred_configuration_complete partial_retention_attempted".split()
)
BINDING_STATUS_KEYS = frozenset(
    "bindings_attempted bindings_complete bindings_bytes bindings_sha256".split()
)
INVARIANT_COUNTS = frozenset(
    "imported_functions imported_globals imported_types static_signatures "
    "user_static_signatures library_types crypto_patterns abi_assets ordinal_assets".split()
)
RANGE_CONVENTION = "requested/invoked/selected ranges are half-open; function ranges are backend observations"


class NativeOutputRefused(RuntimeError):
    """Complete native observations failed a fixed parser or identity predicate."""


class SourceForm(str, Enum):
    SIGNED_NARROW_CAST = "signed-narrow-cast"
    UNSIGNED_MASK_SUBTRACT = "unsigned-mask-subtract"


def parse_source_form(value: object) -> SourceForm:
    _require(type(value) is str, "native source form must be a string")
    try:
        return SourceForm(value)
    except ValueError as error:
        raise NativeOutputRefused("unsupported native source form") from error


@dataclass(frozen=True)
class SourceSpan:
    token_index: int
    start: int
    end: int
    original_pc: int
    relation: str


@dataclass(frozen=True)
class CapturedUnit:
    """Immutable producer observations, never a declaration or admission capability."""

    member: provenance.BoundMember
    request: bytes
    status: bytes
    tokens: bytes
    configuration: bytes
    source: bytes
    spans: tuple[SourceSpan, ...]
    entry_symbol: str | None
    endpoint: str
    source_form: SourceForm | None = None
    backing: OriginalAssociatedFacts | None = None


@dataclass(frozen=True)
class LiteralZeroUnit:
    entry_symbol: str
    declaration: str
    decimal_value: int
    translation_unit: tuple[int, int]
    prototype: tuple[int, int]
    definition: tuple[int, int]
    return_expression: tuple[int, int]


@dataclass(frozen=True)
class EmptyVoidUnit:
    entry_symbol: str
    translation_unit: tuple[int, int]
    prototype: tuple[int, int]
    definition: tuple[int, int]


@dataclass(frozen=True)
class ByteComparisonUnit:
    entry_symbol: str
    symbol: str
    address: int
    read_pc: int
    comparison: int
    translation_unit: tuple[int, int]
    prototype: tuple[int, int]
    definition: tuple[int, int]
    return_expression: tuple[int, int]


@dataclass(frozen=True)
class SourceFacts:
    parsed: LiteralZeroUnit | EmptyVoidUnit | ByteComparisonUnit | None
    uncovered_tokens: tuple[int, ...]
    uncovered_source_ranges: tuple[tuple[int, int], ...]
    predicates: tuple[PredicateResult, ...]


@dataclass(frozen=True)
class EntryDomainFacts:
    member_key: str
    conditions: tuple[str, ...]
    uncovered_original_code: tuple[tuple[str, int, int], ...]
    predicates: tuple[PredicateResult, ...]


@dataclass(frozen=True)
class RepresentationFacts:
    original: LeafFacts
    compiled: LeafFacts
    entry: EntryDomainFacts
    conditional_agreement: tuple[PredicateResult, ...]
    whole_schedule_agreement: tuple[PredicateResult, ...]
    prerequisite_closure: tuple[PredicateResult, ...]


def derive_source_facts(
    unit,
    projected,
    generated: bytes,
    compiler_source: bytes,
    tokens: bytes,
    header: bytes,
    integer_facts,
) -> SourceFacts:
    from harness.reconstruction.obligations import PredicateResult
    from harness.reconstruction.prelude import STDINT_PRELUDE

    predicates = []
    unchanged = generated == compiler_source == unit.source and tokens == unit.tokens
    predicates.append(
        PredicateResult(
            "unchanged-source",
            "matched" if unchanged else "failed",
            "native, generated and compiler-consumed full source/token identities",
        )
    )
    prelude_ok = header == STDINT_PRELUDE
    predicates.append(
        PredicateResult(
            "uniform-prelude",
            "matched" if prelude_ok else "failed",
            "entire staged stdint.h equals the answer-free uniform prelude",
        )
    )
    source = unit.source
    stream = _keys(
        parse_json(tokens, TOKEN_LIMIT), {"tokens", "language"}, "source token stream"
    )
    _require(
        stream["language"] == "C"
        and type(stream["tokens"]) is list
        and len(stream["tokens"]) <= TOKEN_COUNT,
        "source token language/count",
    )
    reproduced = bytearray()
    for token in stream["tokens"]:
        if set(token) == {"addr"}:
            _require(
                type(token["addr"]) is str
                and (
                    token["addr"] == ""
                    or re.fullmatch(r"0x[0-9a-fA-F]{1,8}", token["addr"])
                ),
                "source marker must use the validated native PC grammar",
            )
            continue
        _keys(token, {"kind", "val"}, "source text token")
        _require(
            token["kind"] in TOKEN_KINDS and type(token["val"]) is str,
            "source token shape",
        )
        reproduced.extend(token["val"].encode("utf-8", "strict"))
    _require(bytes(reproduced) == source, "whole source/token concatenation differs")
    clean_tokens = []

    token_pattern = re.compile(
        rb"[A-Za-z_][A-Za-z_0-9]*|[0-9]+|<stdint\.h>|==|[{}();#-]"
    )
    cursor, lexical_ok = 0, all(0 < char < 128 for char in source)
    while lexical_ok and cursor < len(source):
        char = source[cursor]
        if char in b" \t\r\n\f\v":
            cursor += 1
            continue
        if source[cursor : cursor + 2] == b"/*":
            end = source.find(b"*/", cursor + 2)
            if end < 0 or b"/*" in source[cursor + 2 : end]:
                lexical_ok = False
                break
            cursor = end + 2
            continue
        match = token_pattern.match(source, cursor)
        if match is None:
            lexical_ok = False
            break
        clean_tokens.append((match.group(), cursor, cursor + len(match.group())))
        cursor += len(match.group())
    body = isinstance(projected, callcontracts.BodyProposalFacts)
    byte_match = None
    byte_address = None
    byte_comparison = None
    if (
        not body
        and len(projected.accesses) == 1
        and projected.accesses[0][1:] == ("read", 8)
    ):
        byte_match = re.fullmatch(
            rb"# include <stdint\.h> int32_t ([A-Za-z_][A-Za-z_0-9]*) \( void \) ; "
            rb"extern uint8_t ([A-Za-z_][A-Za-z_0-9]*) ; int32_t \1 \( void \) \{ "
            rb"return \( int32_t \) \( \( uint8_t \) \( \2 \) == "
            rb"\( uint8_t \) \( (- )?(?:0|[1-9][0-9]{0,2}) \) \) ; \}",
            b" ".join(row[0] for row in clean_tokens),
        )
        if byte_match:
            selected = [
                row
                for row in parse_json(unit.configuration, CONFIG_LIMIT)["functions"]
                if row.get("name") == unit.entry_symbol
            ]
            _require(len(selected) == 1, "byte source selected function differs")
            views = calltargets._address_views(selected[0])
            symbol = byte_match[2].decode("ascii")
            value = int(clean_tokens[-5][0])
            if byte_match[3]:
                value = -value
            if (
                byte_match[1].decode("ascii") == projected.entry_symbol
                and len(views) == 1
                and symbol in views
                and views[symbol][1] == 1
                and -128 <= value <= 255
                and not projected.parameter_registers
                and not projected.calls
            ):
                byte_address, byte_comparison = views[symbol][0], value & 255
            else:
                byte_match = None
    expected = (
        [
            b"void",
            projected.entry_symbol.encode(),
            b"(",
            b"void",
            b")",
            b";",
            b"void",
            projected.entry_symbol.encode(),
            b"(",
            b"void",
            b")",
            b"{",
            b"}",
        ]
        if body
        else [
            b"#",
            b"include",
            b"<stdint.h>",
            b"int32_t",
            projected.entry_symbol.encode(),
            b"(",
            b"void",
            b")",
            b";",
            b"int32_t",
            projected.entry_symbol.encode(),
            b"(",
            b"void",
            b")",
            b"{",
            b"return",
            b"0",
            b";",
            b"}",
        ]
    )
    grammar = (
        lexical_ok
        and ([row[0] for row in clean_tokens] == expected or byte_match is not None)
        and (body or not projected.parameter_registers)
    )
    if grammar and body:
        grammar = all(
            clean_tokens[first][2] < clean_tokens[last][1]
            for first, last in ((0, 1), (6, 7))
        )
    elif grammar:
        between = source[clean_tokens[1][2] : clean_tokens[2][1]]
        after_include = source[clean_tokens[2][2] : clean_tokens[3][1]]
        between = re.sub(rb"/\*.*?\*/", b" ", between, flags=re.S)
        after_include = re.sub(rb"/\*.*?\*/", b" ", after_include, flags=re.S)
        grammar = (
            bool(between)
            and all(c in b" \t" for c in between)
            and b"\n" not in source[clean_tokens[0][1] : clean_tokens[2][2]]
        )
        grammar = grammar and b"\n" in after_include
        for first, last in (
            ((3, 4), (9, 10), (13, 14)) if byte_match else ((3, 4), (9, 10), (15, 16))
        ):
            grammar = grammar and clean_tokens[first][2] < clean_tokens[last][1]
    prohibited = any(
        any(item[0] == word for item in clean_tokens)
        for word in (
            b"asm",
            b"__asm",
            b"__asm__",
            b"register",
            b"__attribute__",
            b"__declspec",
        )
    )
    policy_state = (
        "failed"
        if prohibited
        else "matched"
        if grammar and unchanged and prelude_ok
        else "unknown"
    )
    compatible_body = (
        body
        and grammar
        and unchanged
        and prelude_ok
        and integer_facts.c_dialect == "c89"
        and callcontracts.project_validated_body(unit) == projected
    )
    predicates.extend(
        (
            PredicateResult(
                "clean-c",
                policy_state,
                "complete empty-void/v1 translation unit"
                if grammar and body
                else "complete unsigned-byte extern comparison translation unit"
                if grammar and byte_match
                else "complete literal-zero-i32/v1 translation unit"
                if grammar
                else "source is outside the complete leaf grammar or contains a prohibited aid",
            ),
            PredicateResult(
                "type-semantics",
                "matched"
                if compatible_body
                or not body
                and grammar
                and unchanged
                and prelude_ok
                and integer_facts.status == "verified"
                and integer_facts.c_dialect == "c89"
                and integer_facts.int_bits == 32
                and (byte_match is None or integer_facts.char_bits == 8)
                else "unknown",
                "compatible void(void) for this member, unchanged source and C89 build under explicit machine-entry conditions; original and callee declarations remain unknown"
                if compatible_body
                else "native body proposal retains an unknown original function declaration"
                if body
                else "conditional int32_t(void) and unsigned 8-bit character build compatibility under explicit readable/stable-byte conditions; original function/object declaration and runtime byte remain unknown"
                if byte_match
                and integer_facts.status == "verified"
                and integer_facts.int_bits == 32
                and integer_facts.char_bits == 8
                else integer_facts.reason,
                actual={
                    "declaration": "void(void)",
                    "member_key": unit.member.key,
                    "range_sha256": unit.member.range_sha256,
                    "source_sha256": hashlib.sha256(source).hexdigest(),
                    "profile_identity": integer_facts.profile_identity,
                    "ordinal": integer_facts.ordinal,
                    "compiler_sha256": integer_facts.compiler.sha256,
                    "recipe_sha256": integer_facts.recipe.sha256,
                    "machine_entry_condition": projected.pending_entry_claim,
                }
                if compatible_body
                else None,
            ),
        )
    )
    parsed = None
    if grammar and unchanged and prelude_ok and body:
        parsed = EmptyVoidUnit(
            projected.entry_symbol,
            (0, len(source)),
            (clean_tokens[0][1], clean_tokens[5][2]),
            (clean_tokens[6][1], clean_tokens[12][2]),
        )
    elif grammar and unchanged and prelude_ok and byte_match:
        parsed = ByteComparisonUnit(
            projected.entry_symbol,
            byte_match[2].decode("ascii"),
            byte_address,
            projected.accesses[0][0],
            byte_comparison,
            (0, len(source)),
            (clean_tokens[3][1], clean_tokens[8][2]),
            (clean_tokens[13][1], clean_tokens[-1][2]),
            (clean_tokens[20][1], clean_tokens[-3][2]),
        )
    elif grammar and unchanged and prelude_ok:
        parsed = LiteralZeroUnit(
            projected.entry_symbol,
            "int32_t(void)",
            0,
            (0, len(source)),
            (clean_tokens[3][1], clean_tokens[8][2]),
            (clean_tokens[9][1], clean_tokens[18][2]),
            (clean_tokens[16][1], clean_tokens[16][2]),
        )
    return SourceFacts(
        parsed,
        () if parsed else tuple(range(len(stream["tokens"]))),
        () if parsed else ((0, len(source)),),
        tuple(predicates),
    )


def derive_entry_domain_facts(
    member, original_leaf: LeafFacts, compiled_leaf: LeafFacts
) -> EntryDomainFacts:
    from harness.reconstruction.machinecode import (
        ZERO_LEAF_CONDITIONS,
        BYTE_LEAF_CONDITIONS,
        ByteComparisonLeaf,
        EmptyReturnLeaf,
        ZeroReturnLeaf,
    )
    from harness.reconstruction.obligations import PredicateResult

    conditions = original_leaf.leaf.conditions if original_leaf.leaf else ()
    expected_pcs = tuple(range(member.start, member.end, 4))
    compatible = (
        type(original_leaf.leaf)
        in (EmptyReturnLeaf, ZeroReturnLeaf, ByteComparisonLeaf)
        and type(compiled_leaf.leaf) is type(original_leaf.leaf)
        and original_leaf.leaf == compiled_leaf.leaf
        and bool(expected_pcs)
        and all(
            facts.leaf.member_key == member.key
            and facts.expected_pcs == expected_pcs
            and facts.covered_pcs == expected_pcs
            and not facts.unrecognized_pcs
            and tuple(item.pc for item in facts.leaf.instructions) == expected_pcs
            and facts.leaf.conditions
            == (
                BYTE_LEAF_CONDITIONS
                if isinstance(facts.leaf, ByteComparisonLeaf)
                else ZERO_LEAF_CONDITIONS
            )
            and bool(facts.predicates)
            and all(predicate.status == "matched" for predicate in facts.predicates)
            for facts in (original_leaf, compiled_leaf)
        )
    )
    return EntryDomainFacts(
        member.key,
        conditions,
        () if compatible else ((member.target, member.start, member.end),),
        (
            PredicateResult(
                "entry-domain",
                "matched" if compatible else "unknown",
                "complete original and compiled leaf schedules agree under the same explicit machine conditions without proving incoming-path validity or an original C declaration"
                if compatible
                else "complete supported original and compiled leaves do not prove relative entry compatibility under the same explicit conditions",
            ),
        ),
    )


def derive_representation_facts(
    source, original, compiled, entry, projected
) -> RepresentationFacts:
    from harness.reconstruction.obligations import PredicateResult
    from harness.reconstruction.machinecode import (
        ByteComparisonLeaf,
        EmptyReturnLeaf,
        ZeroReturnLeaf,
    )

    body = isinstance(projected, callcontracts.BodyProposalFacts)
    byte = isinstance(source.parsed, ByteComparisonUnit)
    supported = (
        isinstance(
            source.parsed,
            EmptyVoidUnit if body else ByteComparisonUnit if byte else LiteralZeroUnit,
        )
        and isinstance(
            original.leaf,
            EmptyReturnLeaf if body else ByteComparisonLeaf if byte else ZeroReturnLeaf,
        )
        and isinstance(
            compiled.leaf,
            EmptyReturnLeaf if body else ByteComparisonLeaf if byte else ZeroReturnLeaf,
        )
        and (
            not byte
            or all(
                leaf.address == source.parsed.address
                and leaf.read_pc == source.parsed.read_pc
                and leaf.comparison == source.parsed.comparison
                for leaf in (original.leaf, compiled.leaf)
            )
        )
    )
    native_covered = (
        original.leaf is not None
        and projected.instructions
        == tuple((item.pc, item.word) for item in original.leaf.instructions)
        and projected.exits == ((original.leaf.return_pc, original.leaf.delay_pc),)
        and (
            body
            or not projected.calls
            and projected.accesses
            == (((original.leaf.read_pc, "read", 8),) if byte else ())
        )
    )
    conditional = PredicateResult(
        "conditional-representation",
        "matched"
        if supported and native_covered and (not byte or original.leaf == compiled.leaf)
        else "unknown",
        "empty source and complete original/native/compiled schedules agree under explicit entry conditions without recovering an original declaration"
        if supported and native_covered and body
        else "byte comparison result agrees under live-readable/stable-byte and C integer prerequisites; whole-effects separately checks all scratch/preservation and instruction effects"
        if supported and native_covered and byte and original.leaf == compiled.leaf
        else "byte comparison result agrees under live-readable/stable-byte and C integer prerequisites, while decoded scratch/preservation or instruction schedules differ"
        if supported and native_covered and byte
        else "literal zero and complete original/native/compiled effects agree under explicit entry and C integer prerequisites"
        if supported and native_covered
        else "finite source/effect/native census is not complete",
    )
    schedule = supported and original.leaf.instructions == compiled.leaf.instructions
    whole = PredicateResult(
        "whole-effects",
        "matched"
        if schedule and native_covered
        else "failed"
        if supported and native_covered
        else "unknown",
        "complete transient original/compiled PC/word/effect schedule comparison",
    )
    return RepresentationFacts(
        original, compiled, entry, (conditional,), (whole,), entry.predicates
    )


def _require(condition: object, message: str) -> None:
    if not condition:
        raise NativeOutputRefused(message)


def _pairs(entries: list) -> dict:
    result = {}
    for key, value in entries:
        _require(key not in result, "duplicate native JSON key")
        result[key] = value
    return result


def _reject_constant(value: str):
    raise NativeOutputRefused(f"nonfinite native JSON constant: {value}")


def _float(value: str) -> float:
    result = float(value)
    _require(math.isfinite(result), "nonfinite native JSON number")
    return result


def parse_json(data: bytes, limit: int) -> object:
    """Parse complete strict UTF-8 JSON with fixed nesting and duplicate-key bounds."""

    _require(type(data) is bytes and 0 < len(data) <= limit, "native JSON byte bound")
    try:
        text = data.decode("utf-8", "strict")
        depth = 0
        quoted = escaped = False
        for char in text:
            _require(char != "\x00", "native JSON NUL")
            if quoted:
                if escaped:
                    escaped = False
                elif char == "\\":
                    escaped = True
                elif char == '"':
                    quoted = False
            elif char == '"':
                quoted = True
            elif char in "[{":
                depth += 1
                _require(depth <= 32, "native JSON nesting bound")
            elif char in "]}":
                depth -= 1
                _require(depth >= 0, "native JSON nesting underflow")
        _require(depth == 0 and not quoted, "native JSON incomplete")
        parsed = json.loads(
            text,
            object_pairs_hook=_pairs,
            parse_constant=_reject_constant,
            parse_float=_float,
        )
        pending = [parsed]
        while pending:
            value = pending.pop()
            if isinstance(value, dict):
                pending.extend(value.keys())
                pending.extend(value.values())
            elif isinstance(value, list):
                pending.extend(value)
            elif isinstance(value, str):
                value.encode("utf-8", "strict")
        return parsed
    except (UnicodeError, ValueError, RecursionError) as error:
        raise NativeOutputRefused(f"native JSON cannot be parsed: {error}") from error


def _keys(value: object, expected: frozenset | set, subject: str) -> dict:
    _require(type(value) is dict and set(value) == expected, f"closed {subject} shape")
    return value


def _rows(value: object, limit: int, subject: str) -> list:
    _require(type(value) is list and len(value) <= limit, f"{subject} count")
    return value


def validate_entry_observation_descriptor(value):
    _keys(
        value,
        {"function", "image_id", "image_sha256", "entry_pc", "state", "files"},
        "entry observation",
    )
    _require(
        type(value["function"]) is str
        and 0 < len(value["function"]) <= 256
        and type(value["image_id"]) is str
        and 0 < len(value["image_id"]) <= 256
        and type(value["entry_pc"]) is int
        and 0 <= value["entry_pc"] <= 0xFFFFFFFF
        and value["entry_pc"] % 4 == 0,
        "entry observation identity differs",
    )
    _keys(value["state"], {"bytes", "sha256"}, "entry state identity")
    limits = {
        "entry-receipt.json": 2 << 20,
        "cpu-receipt.json": 2 << 20,
        "registers.json": 64 << 10,
        "cpu.json": 64 << 10,
        "ram.bin": 2 << 20,
    }
    _keys(value["files"], set(limits), "entry captures")
    hashes = [value["image_sha256"], value["state"]["sha256"]]
    _require(
        type(value["state"]["bytes"]) is int
        and 0 < value["state"]["bytes"] <= 64 << 20,
        "entry state byte bound",
    )
    for name, limit in limits.items():
        row = _keys(value["files"][name], {"path", "bytes", "sha256"}, "entry file")
        _require(
            row["path"] == f"input/entry-{name}"
            and type(row["bytes"]) is int
            and 0 < row["bytes"] <= limit,
            "entry capture path or byte bound",
        )
        hashes.append(row["sha256"])
    _require(
        value["files"]["ram.bin"]["bytes"] == 2 << 20
        and all(
            type(sha) is str and re.fullmatch(r"[0-9a-f]{64}", sha) for sha in hashes
        ),
        "entry capture digest or RAM extent",
    )
    return value


def validate_status(data: bytes, request: bytes, configuration: bytes) -> dict:
    """Bind the command's closed status to the exact supplied request/configuration bytes."""

    wanted = parse_json(request, 64 << 10)
    _require(type(wanted) is dict, "native status request object")
    forms = wanted.get("schema") == "retdec118.native-request/v4" or (
        wanted.get("schema")
        in {"retdec118.native-request/v5", "retdec118.native-request/v6"}
        and "source_form" in wanted
    )
    catalogued = forms or wanted.get("schema") in {
        "retdec118.native-request/v3",
        "retdec118.native-request/v5",
        "retdec118.native-request/v6",
    }
    fields = STATUS_KEYS | BINDING_STATUS_KEYS if catalogued else STATUS_KEYS
    if forms:
        fields |= {"source_form"}
    status = _keys(
        parse_json(data, STATUS_LIMIT),
        fields,
        "native status",
    )
    _require(
        status["schema"]
        == f"retdec118.native-status/v{4 if forms else 3 if catalogued else 2}",
        "native status schema",
    )
    if forms:
        selected = parse_source_form(wanted.get("source_form"))
        _require(
            wanted.get("operation") == "generate", "source form requires generation"
        )
        _require(
            status["source_form"] == selected.value
            or (
                status["source_form"] is None
                and status["status"] == "refused"
                and status["operation"] is None
                and status["retdec_invoked"] is False
            ),
            "native source form status echo differs",
        )
    if catalogued:
        _require(
            type(status["bindings_attempted"])
            is type(status["bindings_complete"])
            is bool,
            "binding retention booleans",
        )
        if status["bindings_complete"]:
            _require(
                status["bindings_attempted"]
                and type(status["bindings_bytes"]) is int
                and 0 < status["bindings_bytes"] <= calltargets.BINDINGS_LIMIT
                and type(status["bindings_sha256"]) is str
                and re.fullmatch(r"[0-9a-f]{64}", status["bindings_sha256"]),
                "binding retained artifact metadata",
            )
        else:
            _require(
                status["bindings_bytes"] is status["bindings_sha256"] is None,
                "incomplete binding artifact metadata",
            )
        _require(
            not status["retdec_invoked"] or status["bindings_complete"],
            "backend invoked before binding retention",
        )
    _require(
        status["status"]
        in {
            "refused",
            "generation-failed",
            "self-check-complete",
            "generation-complete",
        },
        "native status value",
    )
    _require(
        type(status["reason"]) is str and len(status["reason"].encode("utf-8")) <= 4096,
        "native reason bound",
    )
    for prefix, expected in (("request", request), ("configuration", configuration)):
        failure = f"native consumed {prefix} identity differs"
        if prefix == "configuration" and status["status"] == "refused":
            reason = json.dumps(status["reason"], ensure_ascii=True)
            failure += f"; native reported refusal: {reason}"
        _require(
            status[prefix + "_bytes_hex"] == expected.hex()
            and type(status[prefix + "_byte_count"]) is int
            and status[prefix + "_byte_count"] == len(expected)
            and status[prefix + "_sha256"] == hashlib.sha256(expected).hexdigest(),
            failure,
        )
    _require(
        status["release_configuration_sha256"] == CONFIG_SHA256,
        "native release configuration identity",
    )
    for name in (
        "retdec_invoked retdec_returned tokens_complete "
        "inferred_configuration_complete partial_retention_attempted"
    ).split():
        _require(type(status[name]) is bool, f"native {name} boolean")
    if status["status"] in {"self-check-complete", "generation-complete"}:
        _require(
            status["partial_retention_attempted"] is False,
            "successful native status entered failure retention",
        )
    _require(
        status["retdec_failed"] is None or type(status["retdec_failed"]) is bool,
        "native failure boolean",
    )
    return status


def _clean_input(status: dict) -> None:
    _require(
        status["configuration_sha256"] == CONFIG_SHA256,
        "successful native configuration is not this release",
    )
    invariants = _keys(
        status["initial_input_invariants"],
        INVARIANT_COUNTS | {"checked", "cache_used", "rizin_database_used"},
        "native initial input",
    )
    _require(invariants["checked"] is True, "native clean configuration unverified")
    _require(
        all(
            type(invariants[key]) is int and invariants[key] == 0
            for key in INVARIANT_COUNTS
        ),
        "native input imports/support",
    )
    _require(
        invariants["cache_used"] is False
        and invariants["rizin_database_used"] is False,
        "native cache/database input",
    )


def validate_bindings(data, observed, request, contexts, payload):
    _require(
        observed["bindings_attempted"] is observed["bindings_complete"] is True
        and type(observed["bindings_bytes"]) is int
        and observed["bindings_bytes"] == len(data)
        and observed["bindings_sha256"] == hashlib.sha256(data).hexdigest(),
        "native binding artifact identity differs",
    )
    document = parse_json(data, calltargets.BINDINGS_LIMIT)
    wanted = parse_json(request, 64 << 10)
    try:
        calltargets.validate_native_bindings(
            document=document,
            request=wanted,
            request_sha256=hashlib.sha256(request).hexdigest(),
            contexts=contexts,
            original_payload=payload,
            source_form=parse_source_form(wanted["source_form"])
            if "source_form" in wanted
            else None,
        )
    except (proofvalues.CallContractRefused, KeyError, TypeError, IndexError) as error:
        raise NativeOutputRefused(str(error)) from error
    return document


def validate_self_check(data: bytes, request: bytes, configuration: bytes) -> dict:
    """Require observed clean dispatch without any RetDec invocation or original input."""

    status = validate_status(data, request, configuration)
    wanted = parse_json(request, 16 << 10)
    _require(
        type(wanted) is dict
        and "source_form" not in wanted
        and wanted.get("operation") == "self-check"
        and wanted.get("member") is None
        and wanted.get("image") is None,
        "self-check request",
    )
    _clean_input(status)
    _require(
        status["status"] == "self-check-complete"
        and status["operation"] == "self-check",
        "self-check binding",
    )
    _require(
        status["retdec_invoked"] is False
        and status["retdec_returned"] is False
        and status["retdec_failed"] is None,
        "self-check invoked RetDec",
    )
    _require(
        status["requested_range"] is None
        and status["invoked_range"] is None
        and status["backend_ranges"] == []
        and status["backend_selected_ranges"] == [],
        "self-check consumed range",
    )
    _require(
        status["tokens_complete"] is False
        and status["inferred_configuration_complete"] is False
        and all(
            status[key] is None
            for key in (
                "tokens_bytes",
                "tokens_sha256",
                "inferred_configuration_bytes",
                "inferred_configuration_sha256",
            )
        ),
        "self-check emitted native artifacts",
    )
    return status


def validate_generation(
    *,
    member: provenance.BoundMember,
    image: provenance.BoundImage,
    request: bytes,
    configuration: bytes,
    status: bytes,
    tokens: bytes,
    inferred: bytes,
    payload: bytes,
    require_success: bool = True,
    expected_scope: dict | None = None,
    bindings: dict | None = None,
    contexts: calltargets.NativeContexts | None = None,
    entry_captures: dict | None = None,
) -> tuple[dict, str, tuple | None]:
    """Validate identity-bound native configuration on success or retained failure."""

    _require(
        type(member) is provenance.BoundMember and type(image) is provenance.BoundImage,
        "bound native subject types",
    )
    _require(
        type(payload) is bytes and len(payload) == image.payload_size,
        "complete verified payload bytes",
    )
    _require(member.image_id == image.image_id, "native subject image ownership")
    observed = validate_status(status, request, configuration)
    wanted = parse_json(request, 64 << 10)
    _require(type(wanted) is dict, "native generation request object")
    scoped = wanted.get("schema") == "retdec118.native-request/v6"
    overlays = scoped or wanted.get("schema") == "retdec118.native-request/v5"
    forms = wanted.get("schema") == "retdec118.native-request/v4" or (
        overlays and "source_form" in wanted
    )
    catalogued = (
        overlays or forms or wanted.get("schema") == "retdec118.native-request/v3"
    )
    revised = catalogued or wanted.get("schema") == "retdec118.native-request/v2"
    _require(
        len(request) <= (64 << 10 if revised else 16 << 10),
        "native request version byte bound",
    )
    request_fields = set(
        "schema operation request_id config_sha256 member image".split()
    )
    if revised:
        request_fields |= {"problem_sha256", "callees"}
    if catalogued:
        request_fields.add("image_catalog_sha256")
    if forms:
        request_fields.add("source_form")
    if overlays:
        request_fields.add("overlay_mounts")
    if scoped:
        request_fields.add("entry_observation")
        descriptor = validate_entry_observation_descriptor(wanted["entry_observation"])
        _require(
            entry_captures is not None
            and set(entry_captures) == set(descriptor["files"]),
            "native entry captures missing",
        )
        for name, record in descriptor["files"].items():
            raw = entry_captures[name]
            _require(
                type(raw) is bytes
                and len(raw) == record["bytes"]
                and hashlib.sha256(raw).hexdigest() == record["sha256"],
                "native entry capture differs",
            )
    _keys(wanted, request_fields, "native generation request")
    _require(
        wanted["schema"]
        in {
            "retdec118.native-request/v1",
            "retdec118.native-request/v2",
            "retdec118.native-request/v3",
            "retdec118.native-request/v4",
            "retdec118.native-request/v5",
            "retdec118.native-request/v6",
        }
        and wanted["config_sha256"] == CONFIG_SHA256
        and (type(wanted["request_id"]) is str)
        and re.fullmatch("[0-9a-f]{64}", wanted["request_id"]),
        "native generation request identity",
    )
    source_form = parse_source_form(wanted["source_form"]) if forms else None
    if forms or overlays:
        body = {key: value for key, value in wanted.items() if key != "request_id"}
        _require(
            wanted["request_id"]
            == hashlib.sha256(encode_json(body, path="native")).hexdigest()
            and request == encode_json(wanted, path="native"),
            "native request canonical identity differs",
        )
    _require(
        type(wanted) is dict and wanted.get("operation") == "generate",
        "generation request",
    )
    _require(
        wanted.get("member")
        == {
            "key": member.key,
            "target": member.target,
            "image_id": member.image_id,
            "start": member.start,
            "end": member.end,
            "range_sha256": member.range_sha256,
        },
        "generation member metadata differs from original binding",
    )
    original_image = wanted.get("image")
    _keys(
        original_image,
        set(
            "file_sha256 file_bytes payload_sha256 payload_offset payload_bytes load_address".split()
        ),
        "generation image",
    )
    _require(
        all(
            original_image[key] == value
            for key, value in {
                "file_sha256": image.file_sha256,
                "file_bytes": image.bytes,
                "payload_offset": image.payload_offset,
                "payload_bytes": image.payload_size,
                "load_address": image.load_address,
            }.items()
        ),
        "generation image metadata differs from original binding",
    )
    _require(
        original_image["payload_sha256"] == hashlib.sha256(payload).hexdigest(),
        "generation complete payload identity differs",
    )
    first, last = member.start - image.load_address, member.end - image.load_address
    _require(
        0 <= first < last <= len(payload)
        and first % 4 == last % 4 == 0
        and hashlib.sha256(payload[first:last]).hexdigest() == member.range_sha256,
        "generation selected original payload range differs",
    )
    artifacts = [("inferred_configuration", inferred)]
    if require_success or observed["tokens_complete"]:
        artifacts.append(("tokens", tokens))
    for prefix, data in artifacts:
        _require(
            observed[prefix + "_complete"] is True
            and type(observed[prefix + "_bytes"]) is int
            and observed[prefix + "_bytes"] == len(data)
            and observed[prefix + "_sha256"] == hashlib.sha256(data).hexdigest(),
            f"native {prefix} artifact identity differs",
        )
    _require(
        observed["operation"] == "generate" and observed["retdec_invoked"] is True,
        "native generation invocation",
    )
    if require_success:
        _require(
            observed["status"] == "generation-complete"
            and observed["retdec_returned"] is True
            and observed["retdec_failed"] is False,
            "native API did not succeed",
        )
    else:
        _require(observed["status"] == "generation-failed", "native refusal outcome")
    _clean_input(observed)
    expected_range = {"start": member.start, "end": member.end}
    _require(
        observed["requested_range"] == expected_range
        and observed["invoked_range"] == expected_range,
        "native requested/invoked range differs",
    )
    _require(
        observed["range_convention"] == RANGE_CONVENTION, "native range convention"
    )
    parsed_config = parse_json(inferred, CONFIG_LIMIT)
    _require(type(parsed_config) is dict, "inferred configuration object")
    for function in parsed_config.get("functions", []):
        restoration = function.get("originalCallSummary", {}).get("scopedRestoration")
        if restoration is not None:
            descriptor = wanted.get("entry_observation")
            _require(
                scoped and descriptor is not None,
                "scoped restoration lacks entry capture input",
            )
            _require(
                function["originalCallSummary"]["function"] == descriptor["function"]
                and restoration["entryPc"] == descriptor["entry_pc"]
                and restoration["stateHash"] == descriptor["state"]["sha256"]
                and restoration["entrySp"]
                == parse_json(entry_captures["registers.json"], 64 << 10)["registers"][
                    29
                ]
                and restoration["observation"]
                == hashlib.sha256(
                    json.dumps(
                        descriptor, sort_keys=True, separators=(",", ":")
                    ).encode()
                ).hexdigest(),
                "scoped restoration entry input differs",
            )
    backend = _rows(observed["backend_ranges"], 4096, "backend function")
    entries = []
    for row in backend:
        _keys(row, {"symbol", "start", "end"}, "backend function")
        _require(
            type(row["symbol"]) is str and len(row["symbol"].encode("utf-8")) <= 4096,
            "backend symbol bound",
        )
        for key in ("start", "end"):
            _require(
                row[key] is None
                or (type(row[key]) is int and 0 <= row[key] <= 1 << 32),
                "backend function address",
            )
        if row["start"] == member.start:
            entries.append(row["symbol"])
    _require(
        len(entries) == 1 and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]{0,255}", entries[0]),
        "strict selected symbol is missing or ambiguous",
    )
    symbol = entries[0]
    selected = _rows(
        observed["backend_selected_ranges"], 4096, "backend selected range"
    )
    for row in selected:
        _keys(row, {"start", "end"}, "backend selected range")
        _require(
            all(
                value is None or (type(value) is int and 0 <= value <= 1 << 32)
                for value in row.values()
            ),
            "backend selected address",
        )
    _bind_configuration(parsed_config, backend, selected)
    _require(selected == [expected_range], "strict native selected range differs")
    parameters = parsed_config["decompParams"]
    if forms:
        _require(
            parse_source_form(parameters.get("originalSourceForm")) is source_form,
            "inferred native source form differs",
        )
    else:
        _require(
            "originalSourceForm" not in parameters
            and all(
                "originalSourceForms" not in function
                for function in parsed_config["functions"]
            ),
            "legacy generation acquired a source form selection/census",
        )
    _require(
        parameters.get("originalOnlyReturnRecovery") is True,
        "strict native recovery mode missing",
    )
    _require(
        proofvalues._address(parameters.get("entryPoint"), refusal=NativeOutputRefused)
        == member.start,
        "strict native entry differs",
    )
    rows = [
        row
        for row in parsed_config.get("functions", [])
        if proofvalues._address(row.get("startAddr"), refusal=NativeOutputRefused)
        == member.start
    ]
    _require(
        len(rows) == 1
        and rows[0].get("name") == symbol
        and proofvalues._address(rows[0].get("endAddr"), refusal=NativeOutputRefused)
        == member.end,
        "strict selected configuration row differs",
    )
    row = rows[0]
    graph = None
    try:
        if catalogued:
            _require(
                contexts is not None and bindings is not None,
                "catalogued generation lacks original binding evidence",
            )
            calltargets.validate_native_bindings(
                document=bindings,
                request=wanted,
                request_sha256=hashlib.sha256(request).hexdigest(),
                contexts=contexts,
                original_payload=payload,
                source_form=source_form,
            )
        if revised:
            graph = calltargets.validate_contract_graph(
                parsed_config,
                wanted,
                request,
                expected_scope,
                payload,
                image.load_address,
                release_configuration_sha256=CONFIG_SHA256,
                bindings=bindings,
                contexts=contexts,
                entry_observation=scoped,
            )
        else:
            _require(
                expected_scope is None and "originalCallScope" not in parameters,
                "legacy request acquired graph scope",
            )
            _require(
                "originalCallSummary" not in row, "legacy return acquired graph summary"
            )
        callcontracts.validate_return(
            row,
            expected_range,
            revised=revised,
            entry_observation=scoped if revised else None,
        )
        candidate = revised and callcontracts.validate_body_candidate(row)
        _require(
            revised or "originalBodyCandidate" not in row,
            "legacy request acquired a body proposal",
        )
        if forms:
            _validate_source_forms(parsed_config, require_success=require_success)
        if observed["status"] == "generation-complete":
            if revised:
                ordinary = (
                    candidate
                    and row.get("originalBodyCandidate") == "mips-ordinary-body/v1"
                )
                for function in parsed_config["functions"]:
                    _require(
                        function is row or "originalBodyCandidate" not in function,
                        "native body proposal tagged a callee",
                    )
                    if ordinary and "originalCallSummary" not in function:
                        _require(
                            not function.get("parameters", []),
                            "native body proposal acquired callee parameters",
                        )
                    if "originalCallSummary" in function:
                        if ordinary:
                            if not proofvalues._complete(
                                function["originalCallSummary"]["declaration"]
                            ):
                                _require(
                                    not function.get("parameters", [])
                                    and "returnType" not in function
                                    and "returnStorage" not in function,
                                    "native body proposal acquired an original declaration",
                                )
                        elif not callcontracts.validate_body_candidate(function):
                            callcontracts.validate_projected_declaration(function)
            if not candidate:
                _validate_admission(row)
    except (
        proofvalues.CallContractRefused,
        KeyError,
        TypeError,
        IndexError,
        AttributeError,
    ) as error:
        raise NativeOutputRefused(str(error)) from error
    return row, symbol, graph


def _external_prototype_end(tokens, start, function):
    state = "return-type"
    for index in range(start, len(tokens)):
        token = _keys(tokens[index], {"kind", "val"}, "external prototype token")
        kind, value = token["kind"], token["val"]
        _require(
            type(kind) is str and type(value) is str,
            "external prototype token values",
        )
        if kind in {"ws", "nl"}:
            _require(not value or value.isspace(), "external prototype whitespace")
            continue
        scalar = kind == "type" and (
            value in {"void", "char", "short", "int", "long", "float", "double"}
            or re.fullmatch(r"u?int(?:8|16|32|64)_t", value)
        )
        punctuation = kind == "punc"
        if state == "return-type":
            _require(scalar, "external prototype return type is not scalar")
            state = "function"
        elif state == "function":
            _require(
                kind == "i_fnc" and value == function,
                "external prototype function differs from its original call",
            )
            state = "open"
        elif state == "open":
            _require(punctuation and value == "(", "external prototype opening")
            state = "first-argument"
        elif state in {"first-argument", "argument"}:
            if state == "first-argument" and punctuation and value == ")":
                state = "semicolon"
                continue
            _require(scalar, "external prototype parameter type is not scalar")
            state = "close" if value == "void" else "parameter"
        elif state == "parameter" and kind in {"i_lvar", "i_arg"}:
            _require(
                re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", value)
                and not re.match(
                    r"(?:__pseudo|__undefined|undefined_|unknown_)", value
                ),
                "external prototype parameter identifier differs",
            )
            state = "separator"
        elif state in {"parameter", "separator", "close"}:
            _require(
                punctuation and (value == ")" or (value == "," and state != "close")),
                "external prototype parameter list is not balanced",
            )
            state = "semicolon" if value == ")" else "argument"
        else:
            _require(punctuation and value == ";", "external prototype terminator")
            return index
    raise NativeOutputRefused("external prototype is incomplete")


def decode_unit(
    *,
    member: provenance.BoundMember,
    image: provenance.BoundImage,
    request: bytes,
    configuration: bytes,
    status: bytes,
    tokens: bytes,
    inferred: bytes,
    payload: bytes,
    expected_scope: dict | None = None,
    bindings: dict | None = None,
    contexts: calltargets.NativeContexts | None = None,
    entry_captures: dict | None = None,
) -> CapturedUnit:
    """Capture unedited native C and its contract or body-proposal evidence."""
    selected, symbol, graph = validate_generation(
        member=member,
        image=image,
        request=request,
        configuration=configuration,
        status=status,
        tokens=tokens,
        inferred=inferred,
        payload=payload,
        expected_scope=expected_scope,
        bindings=bindings,
        contexts=contexts,
        entry_captures=entry_captures,
    )
    stream = _keys(
        parse_json(tokens, TOKEN_LIMIT), {"tokens", "language"}, "native token document"
    )
    _require(
        stream["language"] == "C"
        and type(stream["tokens"]) is list
        and len(stream["tokens"]) <= TOKEN_COUNT,
        "native token language/count",
    )
    ordinary = selected.get("originalBodyCandidate") == "mips-ordinary-body/v1"
    stack_word = callcontracts.is_stack_word_body_candidate(selected)
    registers, functions, direct_functions = set(), set(), {}
    if ordinary or stack_word:
        parsed_config = parse_json(inferred, CONFIG_LIMIT)
        for register in _rows(
            parsed_config.get("registers", []), 4096, "native registers"
        ):
            _require(
                type(register) is dict and type(register.get("name")) is str,
                "native register name",
            )
            registers.add(register["name"])
        functions = (
            {symbol}
            if stack_word
            else {function["name"] for function in parsed_config["functions"]}
        )
        if ordinary:
            direct_functions = calltargets.original_direct_functions(
                parsed_config["functions"], selected
            )
    source = bytearray()
    spans = []
    current = None
    prototype_end = None
    prototype_reset = body_started = False
    for index, token in enumerate(stream["tokens"]):
        _require(type(token) is dict, "native token object")
        if set(token) == {"addr"}:
            address = token["addr"]
            _require(
                type(address) is str
                and (address == "" or re.fullmatch(r"0x[0-9a-fA-F]{1,8}", address)),
                "native PC spelling",
            )
            _require(prototype_end is None, "external prototype contains a PC marker")
            if prototype_reset:
                _require(address == "", "external prototype lacks its empty PC reset")
                prototype_reset = False
            current = int(address, 16) if address else None
            if current is not None and not (
                image.load_address <= current < image.load_address + image.payload_size
            ):
                _require(
                    ordinary
                    and not body_started
                    and current in direct_functions.values(),
                    "native PC escapes original payload",
                )
                function = next(
                    name
                    for name, target in direct_functions.items()
                    if target == current
                )
                prototype_end = _external_prototype_end(
                    stream["tokens"], index + 1, function
                )
        else:
            _keys(token, {"kind", "val"}, "native text token")
            _require(
                type(token["kind"]) is str
                and token["kind"] in TOKEN_KINDS
                and type(token["val"]) is str,
                "native text token values",
            )
            if prototype_reset:
                _require(
                    token["kind"] in {"ws", "nl"}
                    and (not token["val"] or token["val"].isspace()),
                    "external prototype PC suppression escapes its declaration",
                )
            if ordinary or stack_word:
                spelling = token["val"]
                _require(
                    spelling not in {"asm", "__asm", "__asm__", "register"}
                    and not (token["kind"] == "i_gvar" and spelling in registers)
                    and not (token["kind"] == "i_fnc" and spelling not in functions)
                    and not (
                        token["kind"] in {"i_fnc", "i_lvar", "i_gvar"}
                        and (
                            re.match(r"(?:__pseudo|__undefined|undefined_)", spelling)
                            or (
                                spelling.startswith("unknown_")
                                and not (
                                    token["kind"] == "i_fnc"
                                    and spelling in direct_functions
                                )
                            )
                        )
                    ),
                    "native body proposal retains register or helper machinery",
                )
            try:
                data = token["val"].encode("utf-8", "strict")
            except UnicodeError as error:
                raise NativeOutputRefused(
                    "native source contains invalid Unicode"
                ) from error
            _require(
                b"\x00" not in data and len(source) + len(data) <= SOURCE_LIMIT,
                "native source byte bound/NUL",
            )
            start = len(source)
            source.extend(data)
            if (
                prototype_end is None
                and not prototype_reset
                and current is not None
                and data
            ):
                relation = (
                    "selected-range"
                    if member.start <= current < member.end
                    else "image-context"
                )
                spans.append(SourceSpan(index, start, len(source), current, relation))
            if token["kind"] == "punc" and token["val"] == "{":
                body_started = True
            if index == prototype_end:
                prototype_end = None
                prototype_reset = True
    _require(
        prototype_end is None and not prototype_reset,
        "external prototype capture lacks a complete declaration and reset",
    )
    _require(bool(source), "native source is empty")
    if stack_word:
        _validate_stack_word_signature(stream["tokens"], selected, symbol)
    endpoint = (
        "off-range"
        if any(row.relation != "selected-range" for row in spans)
        else "observed-within-range"
        if spans
        else "no-associated-pcs"
    )
    wanted = parse_json(request, 64 << 10)
    backing = None
    if graph is not None and graph[0] is not None:
        from harness.reconstruction.obligations import (
            OriginalAssociatedFacts,
            OriginalImageFacts,
        )

        context, payloads = graph
        backing = OriginalAssociatedFacts(
            member.target,
            tuple(
                OriginalImageFacts(
                    mount["target"],
                    mount["image_id"],
                    payloads[mount["image_id"]][1],
                    0,
                    payloads[mount["image_id"]][0],
                )
                for mount in context["mounts"]
            ),
        )
    return CapturedUnit(
        member,
        request,
        status,
        tokens,
        inferred,
        bytes(source),
        tuple(spans),
        symbol,
        endpoint,
        parse_source_form(wanted["source_form"]) if "source_form" in wanted else None,
        backing,
    )


def _validate_stack_word_signature(tokens, selected, symbol):
    significant = [
        token
        for token in tokens
        if "kind" in token and token["kind"] not in {"ws", "nl", "cmnt"}
    ]
    definitions = declarations = 0
    for index, token in enumerate(significant):
        if token["kind"] != "i_fnc":
            continue
        _require(
            token["val"] == symbol
            and index > 0
            and significant[index - 1]["kind"] == "type"
            and significant[index - 1]["val"] in {"int32_t", "uint32_t"},
            "stack word C result carrier differs",
        )
        cursor = index + 1

        def consume(kinds, values):
            nonlocal cursor
            _require(
                cursor < len(significant)
                and significant[cursor]["kind"] in kinds
                and significant[cursor]["val"] in values,
                "stack word C carrier signature differs",
            )
            cursor += 1

        consume({"punc"}, {"("})
        for position, parameter in enumerate(selected["parameters"]):
            _require(
                parameter["name"] == f"arg{position + 1}",
                "stack word config carrier name differs",
            )
            consume({"type"}, {"int32_t", "uint32_t"})
            consume({"i_arg", "i_lvar"}, {f"a{position + 1}"})
            consume({"op", "punc"}, {","} if position < 4 else {")"})
        _require(cursor < len(significant), "stack word C declaration is incomplete")
        end = significant[cursor]
        _require(
            end["kind"] == "punc" and end["val"] in {";", "{"},
            "stack word C declaration terminator differs",
        )
        definitions += end["val"] == "{"
        declarations += end["val"] == ";"
    _require(
        definitions == 1 and declarations <= 1,
        "stack word C entry definition or declaration is not unique",
    )


def _validate_source_forms(configuration: dict, *, require_success: bool) -> None:
    for function in _rows(configuration["functions"], 4096, "source form function"):
        witnesses = _rows(
            function.get("originalSourceExpressions", []), 4096, "source expression"
        )
        if "originalSourceExpressions" in function:
            _require(bool(witnesses), "empty serialized source expression inventory")
        seen = set()
        for witness in witnesses:
            _keys(
                witness,
                {
                    "relation",
                    "machineWidth",
                    "viewWidth",
                    "entryRegister",
                    "pcs",
                    "definitions",
                },
                "source expression witness",
            )
            _require(
                witness["relation"] == "sign-extend-low-bits"
                and type(witness["machineWidth"]) is int
                and witness["machineWidth"] == 32
                and type(witness["viewWidth"]) is int
                and witness["viewWidth"] in {8, 16}
                and type(witness["entryRegister"]) is int
                and 4 <= witness["entryRegister"] <= 7,
                "source expression relation/width/register",
            )
            pcs = proofvalues._references(witness["pcs"], (1 << 32) - 1)
            ids = proofvalues._references(witness["definitions"], 4096)
            _require(
                len(pcs) == len(ids) == 2 and pcs[0] < pcs[1],
                "source expression references",
            )
            identity = (
                tuple(pcs),
                tuple(ids),
                witness["entryRegister"],
                witness["viewWidth"],
            )
            _require(identity not in seen, "duplicate source expression witness")
            seen.add(identity)
            native, summary = (
                function["returnDisposition"],
                function["originalCallSummary"],
            )
            evidence = native["evidence"]
            entry, shift = witness["entryRegister"], 32 - witness["viewWidth"]
            _require(
                native["kind"] == "value"
                and summary["declaration"] == {"kind": "complete", "remaining": []}
                and summary["parameterRegisters"].count(entry) == 1
                and evidence["decodeComplete"] is evidence["analysisComplete"] is True
                and evidence["symbol"] == function["name"]
                and evidence["start"] <= pcs[0] < pcs[1] < evidence["end"],
                "source expression original declaration/extent",
            )
            instructions, definitions = [], []
            for pc, definition_id in zip(pcs, ids):
                matching = [row for row in evidence["instructions"] if row["pc"] == pc]
                bound = [
                    row for row in evidence["definitions"] if row["id"] == definition_id
                ]
                _require(
                    len(matching) == len(bound) == 1,
                    "source expression original association",
                )
                instruction, definition = matching[0], bound[0]
                word = instruction["word"]
                _require(
                    word >> 26 == 0
                    and (word >> 21) & 31 == 0
                    and (word >> 6) & 31 == shift
                    and definition["pc"] == pc
                    and definition["register"] == (word >> 11) & 31
                    and definition["register"] != 0
                    and definition["width"] == 32
                    and definition["delayed"] is False
                    and definition["bad"] == []
                    and definition["origin"] == {"kind": "instruction"}
                    and definition["operation"] == "shift-immediate"
                    and definition["entries"] == definition["formals"] == [entry]
                    and instruction["owner"] == evidence["symbol"]
                    and instruction["bytes"] == 4
                    and instruction["mapped"] is True
                    and instruction["undef"] is False
                    and definition["register"] in instruction["fullWidthRegisters"],
                    "source expression original instruction/definition",
                )
                instructions.append(instruction)
                definitions.append(definition)
            left, right = instructions
            _require(
                left["word"] & 63 == 0
                and right["word"] & 63 == 3
                and (left["word"] >> 16) & 31 == entry
                and (right["word"] >> 16) & 31 == definitions[0]["register"]
                and definitions[0]["inputs"] == []
                and definitions[1]["inputs"] == [ids[0]]
                and sum(
                    row["pc"] == pcs[1]
                    and row["register"] == definitions[1]["register"]
                    for row in evidence["definitions"]
                )
                == 1,
                "source expression shift chain",
            )
            for pc in range(pcs[0], pcs[1], 4):
                edges = [edge for edge in evidence["edges"] if edge["from"] == pc]
                _require(
                    edges
                    == [
                        {
                            "from": pc,
                            "to": pc + 4,
                            "delayOwner": None,
                            "kind": "fallthrough",
                        }
                    ],
                    "source expression straight fallthrough",
                )
            _require(
                all(
                    edge["delayOwner"] is None
                    and (edge["to"] == pcs[0] or edge["from"] + 4 == edge["to"])
                    for edge in evidence["edges"]
                    if pcs[0] <= edge["to"] <= pcs[1]
                )
                and not any(
                    pcs[0] <= call["pc"] <= pcs[1] for call in evidence["calls"]
                ),
                "source expression incoming edge/call exclusion",
            )
        census = function.get("originalSourceForms")
        if census is None:
            _require(
                not require_success and "originalSourceForms" not in function,
                "source form census missing",
            )
            continue
        _keys(census, {"status", "remaining", "applications"}, "source form census")
        remaining = proofvalues._reasons(census["remaining"])
        applications = _rows(census["applications"], 4096, "source form application")
        indexes, unresolved = [], bool(remaining)
        for application in applications:
            _require(type(application) is dict, "source form application object")
            ready = application.get("status") == "ready"
            _keys(
                application,
                {"witnessIndex", "status", "occurrences" if ready else "reason"},
                "source form application",
            )
            index = application["witnessIndex"]
            _require(
                type(index) is int and 0 <= index < len(witnesses),
                "source form witness index",
            )
            indexes.append(index)
            if ready:
                _require(
                    type(application["occurrences"]) is int
                    and 1 <= application["occurrences"] <= 8192,
                    "source form occurrence count",
                )
            else:
                _require(
                    application["status"] == "unresolved",
                    "source form application status",
                )
                proofvalues._reasons([application["reason"]], nonempty=True)
                unresolved = True
        _require(
            indexes == list(range(len(witnesses))),
            "source form complete witness census",
        )
        _require(
            census["status"]
            == (
                "unresolved"
                if unresolved
                else "census"
                if applications
                else "no-bound-sites"
            ),
            "source form census status differs from applications",
        )
        _require(
            not require_success or not unresolved,
            "successful source form census remains unresolved",
        )


def _validate_admission(row: dict) -> None:
    native = row["returnDisposition"]
    kind, evidence = native["kind"], native["evidence"]
    lanes = set()
    for parameter in _rows(row.get("parameters", []), 4, "native parameter"):
        _require(
            type(parameter) is dict
            and type(parameter.get("type")) is dict
            and parameter["type"].get("llvmIr") == "i32",
            "native i32 parameter missing",
        )
        storage = _keys(
            parameter.get("storage"),
            {"type", "value", "registerNumber"},
            "native parameter storage",
        )
        register = storage["registerNumber"]
        _require(
            type(register) is int
            and 6 <= register <= 9
            and storage["type"] == "register"
            and storage["value"] == f"a{register - 6}"
            and register - 2 not in lanes,
            "native argument lane is invalid or duplicate",
        )
        lanes.add(register - 2)
    formals = {
        formal
        for category in ("definitions", "exits")
        for item in evidence[category]
        for formal in item["formals"]
    }
    _require(
        formals <= lanes, "native formal dependency has no matching parameter lane"
    )
    _require(
        kind == "value"
        and all(
            not call["required"] or (call["qualified"] and not call["unresolved"])
            for call in evidence["calls"]
        ),
        "native selected return/call contract unresolved",
    )


def _bind_configuration(configuration: dict, functions: list, selected: list) -> None:
    parameters = configuration.get("decompParams")
    _require(type(parameters) is dict, "inferred decompilation parameters missing")
    for data, expected, start_key, end_key in (
        (parameters.get("selectedRanges", []), selected, "start", "end"),
        (configuration.get("functions", []), functions, "startAddr", "endAddr"),
    ):
        normalized = []
        for row in _rows(data, 4096, "inferred range"):
            _require(type(row) is dict, "inferred range shape")
            observed = {
                "start": proofvalues._address(
                    row.get(start_key), refusal=NativeOutputRefused
                ),
                "end": proofvalues._address(
                    row.get(end_key), refusal=NativeOutputRefused
                ),
            }
            if start_key == "start":
                _require(set(row) <= {"start", "end"}, "inferred selected range shape")
            else:
                _require(type(row.get("name", "")) is str, "inferred function shape")
                observed["symbol"] = row.get("name", "")
            normalized.append(observed)
        _require(normalized == expected, "inferred ranges differ from native status")
