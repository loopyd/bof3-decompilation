"""Independent bounded MIPS-I return and unsigned-byte comparison schedules."""

from __future__ import annotations

from dataclasses import dataclass

from harness.domain.mips import word_fields


@dataclass(frozen=True)
class LeafInstruction:
    pc: int
    word: int
    operation: str
    reads: tuple[int, ...]
    writes: tuple[int, ...]
    constant_before: int | None
    constant_after: int | None
    delay_owner: int | None


ZERO_LEAF_CONDITIONS = (
    "frozen-entry-pc",
    "normal-instruction-mode",
    "outside-caller-delay-slot",
    "resident-unchanged-original-fetch-range",
    "no-pending-gpr-load",
    "valid-aligned-normal-continuation",
    "normal-synchronous-execution",
    "no-interrupt-dma-exception-overlay-transition",
)


@dataclass(frozen=True)
class ZeroReturnLeaf:
    member_key: str
    instructions: tuple[LeafInstruction, ...]
    return_pc: int
    delay_pc: int
    final_constant: int
    preserved_gprs: tuple[int, ...]
    preserved_banks: tuple[str, ...]
    conditions: tuple[str, ...]


@dataclass(frozen=True)
class EmptyReturnLeaf:
    member_key: str
    instructions: tuple[LeafInstruction, ...]
    return_pc: int
    delay_pc: int
    conditions: tuple[str, ...]


@dataclass(frozen=True)
class ByteComparisonLeaf:
    member_key: str
    instructions: tuple[LeafInstruction, ...]
    return_pc: int
    delay_pc: int
    read_pc: int
    address: int
    comparison: int
    base_register: int
    loaded_register: int
    xor_register: int
    preserved_gprs: tuple[int, ...]
    preserved_banks: tuple[str, ...]
    conditions: tuple[str, ...]


BYTE_LEAF_CONDITIONS = ZERO_LEAF_CONDITIONS + (
    "valid-live-readable-normal-ram-byte",
    "same-arbitrary-current-byte-stable-during-call",
)


@dataclass(frozen=True)
class LeafFacts:
    leaf: ZeroReturnLeaf | EmptyReturnLeaf | ByteComparisonLeaf | None
    expected_pcs: tuple[int, ...]
    covered_pcs: tuple[int, ...]
    unrecognized_pcs: tuple[int, ...]
    predicates: tuple


def derive_byte_comparison_leaf(member, source) -> LeafFacts:
    from harness.common.deadlines import check_deadline
    from harness.reconstruction.obligations import PredicateResult

    check_deadline()
    content = getattr(source, "instruction_bytes", getattr(source, "content", None))
    address = getattr(source, "pc", getattr(source, "address", None))
    pcs = tuple(range(member.start, member.end, 4))
    complete = (
        type(content) is bytes
        and address == member.start
        and len(content) == member.end - member.start == 24
        and member.start % 4 == 0
    )
    words = (
        tuple(int.from_bytes(content[i : i + 4], "little") for i in range(0, 24, 4))
        if complete
        else ()
    )
    fields = tuple(word_fields(word) for word in words)
    supported = False
    if complete:
        high, load, nop, xor, jump, compare = fields
        effective = ((high.immediate << 16) + load.signed_immediate) & 0xFFFFFFFF
        supported = (
            high.opcode == 15
            and high.rs == 0
            and high.rt not in {0, 31}
            and load.opcode == 36
            and load.rs == high.rt
            and load.rt not in {0, 31}
            and nop.word == 0
            and xor.opcode == 14
            and xor.rs == load.rt
            and xor.rt not in {0, 31}
            and xor.immediate <= 255
            and jump.word == 0x03E00008
            and compare.opcode == 11
            and compare.rs == xor.rt
            and compare.rt == 2
            and compare.signed_immediate == 1
            and 0x80000000 <= effective < 0x80200000
        )
    if not supported:
        return LeafFacts(
            None,
            pcs,
            pcs if complete else (),
            pcs,
            (
                PredicateResult(
                    "whole-effects",
                    "unknown",
                    "interval is outside the complete unsigned-byte comparison schedule",
                ),
            ),
        )
    operations = (
        ("lui-address", (0,), (high.rt,), None, high.immediate << 16),
        ("lbu-pending-byte", (load.rs,), (), high.immediate << 16, None),
        ("nop-commit-pending-byte", (0,), (load.rt,), None, None),
        ("xori-current-byte", (xor.rs,), (xor.rt,), None, None),
        ("jr-capture-ra", (31,), (), None, None),
        ("sltiu-equal-byte", (compare.rs,), (2,), None, None),
    )
    writes = {high.rt, load.rt, xor.rt, 2}
    leaf = ByteComparisonLeaf(
        member.key,
        tuple(
            LeafInstruction(
                pc,
                word,
                operation,
                reads,
                written,
                before,
                after,
                pcs[4] if index == 5 else None,
            )
            for index, (
                pc,
                word,
                (operation, reads, written, before, after),
            ) in enumerate(zip(pcs, words, operations, strict=True))
        ),
        pcs[4],
        pcs[5],
        pcs[1],
        effective,
        xor.immediate,
        high.rt,
        load.rt,
        xor.rt,
        tuple(reg for reg in range(32) if reg not in writes),
        ("hi", "lo", "cp0", "gte", "memory"),
        BYTE_LEAF_CONDITIONS,
    )
    return LeafFacts(
        leaf,
        pcs,
        pcs,
        (),
        (
            PredicateResult(
                "whole-effects",
                "matched",
                "complete unsigned-byte load-delay, symbolic comparison, return-delay and register/bank preservation schedule under explicit conditions",
            ),
        ),
    )


def derive_empty_return_leaf(member, source) -> LeafFacts:
    from harness.common.deadlines import check_deadline
    from harness.reconstruction.obligations import PredicateResult

    check_deadline()
    content = getattr(source, "instruction_bytes", getattr(source, "content", None))
    address = getattr(source, "pc", getattr(source, "address", None))
    expected = tuple(range(member.start, member.end, 4))
    complete = (
        type(content) is bytes
        and address == member.start
        and len(content) == member.end - member.start
        and len(content) % 4 == 0
    )
    if not complete:
        return LeafFacts(
            None,
            expected,
            (),
            (),
            (
                PredicateResult(
                    "whole-effects",
                    "failed",
                    "incomplete or displaced instruction interval",
                ),
            ),
        )
    words = tuple(
        int.from_bytes(content[i : i + 4], "little") for i in range(0, len(content), 4)
    )
    if words != (0x03E00008, 0):
        return LeafFacts(
            None,
            expected,
            expected,
            tuple(
                pc
                for pc, word in zip(expected, words, strict=True)
                if word not in {0x03E00008, 0}
            ),
            (
                PredicateResult(
                    "whole-effects",
                    "unknown",
                    "complete interval is outside empty-return-leaf/v1",
                    actual=list(words),
                ),
            ),
        )
    return LeafFacts(
        EmptyReturnLeaf(
            member.key,
            (
                LeafInstruction(
                    member.start, 0x03E00008, "jr-ra", (31,), (), None, None, None
                ),
                LeafInstruction(
                    member.start + 4, 0, "nop", (0,), (), None, None, member.start
                ),
            ),
            member.start,
            member.start + 4,
            ZERO_LEAF_CONDITIONS,
        ),
        expected,
        expected,
        (),
        (
            PredicateResult(
                "whole-effects",
                "matched",
                "complete JR RA and NOP schedule covered under explicit entry conditions",
            ),
        ),
    )


def derive_zero_return_leaf(member, source) -> LeafFacts:
    from harness.common.deadlines import check_deadline
    from harness.reconstruction.obligations import PredicateResult

    content = getattr(source, "instruction_bytes", getattr(source, "content", None))
    address = getattr(source, "pc", getattr(source, "address", None))
    expected = tuple(range(member.start, member.end, 4))
    if (
        type(content) is not bytes
        or address != member.start
        or len(content) % 4
        or len(content) != member.end - member.start
    ):
        return LeafFacts(
            None,
            expected,
            (),
            (),
            (
                PredicateResult(
                    "whole-effects",
                    "failed",
                    "incomplete or displaced instruction interval",
                ),
            ),
        )
    pcs = tuple(range(address, address + len(content), 4))
    words = tuple(
        int.from_bytes(content[i : i + 4], "little") for i in range(0, len(content), 4)
    )
    zeros = {
        0x24020000: "addiu-v0-zero",
        0x34020000: "ori-v0-zero",
        0x00001021: "addu-v0-zero-zero",
    }
    schedule = (
        len(words) == 2
        and words[0] == 0x03E00008
        and words[1] in zeros
        or len(words) == 3
        and words[0] in zeros
        and words[1:] == (0x03E00008, 0)
    )
    unrecognized = tuple(
        pc
        for pc, word in zip(pcs, words, strict=True)
        if word not in {0, 0x03E00008, *zeros}
    )
    if not schedule:
        return LeafFacts(
            None,
            pcs,
            pcs,
            unrecognized,
            (
                PredicateResult(
                    "whole-effects",
                    "unknown",
                    "complete interval is outside zero-return-leaf/v1",
                    actual=list(words),
                ),
            ),
        )
    return_pc = pcs[0] if len(words) == 2 else pcs[1]
    rows, constant = [], None
    for pc, word in zip(pcs, words, strict=True):
        check_deadline()
        fields = word_fields(word)
        before = constant
        if word in zeros:
            assert fields.rt == 2 if fields.opcode else fields.rd == 2
            reads = (0, 0) if word == 0x00001021 else (0,)
            writes, operation, constant = (2,), zeros[word], 0
        elif word == 0x03E00008:
            reads, writes, operation = (31,), (), "jr-ra"
        else:
            reads, writes, operation = (0,), (), "nop"
        rows.append(
            LeafInstruction(
                pc,
                word,
                operation,
                reads,
                writes,
                before,
                constant,
                return_pc if pc == return_pc + 4 else None,
            )
        )
    leaf = ZeroReturnLeaf(
        member.key,
        tuple(rows),
        return_pc,
        return_pc + 4,
        0,
        tuple(i for i in range(32) if i != 2),
        ("hi", "lo", "cp0", "gte", "memory"),
        ZERO_LEAF_CONDITIONS,
    )
    return LeafFacts(
        leaf,
        pcs,
        pcs,
        (),
        (
            PredicateResult(
                "whole-effects",
                "matched",
                "every instruction, control/delay and preservation effect covered under explicit entry conditions",
            ),
        ),
    )
