"""MIPS32 little-endian byte-level facts for executable payloads.

The domain owns raw executable-byte parsing: lui/%lo data-reference
decoding, static JAL decoding, and the trivial return-void classifier.
Analysis and command layers consume these typed facts; they never re-parse
instruction bytes.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass


@dataclass(frozen=True)
class NormalizedInstructionStream:
    """Instruction shape plus the only fields erased from that shape."""

    words: tuple[int, ...]
    parameters: tuple[tuple[int, str, int], ...]


# Only operand-immediate fields are erased. Branch displacements remain part of
# control-flow shape; register/opcode/funct fields always remain exact.
_IMMEDIATE_OPS = frozenset(range(0x08, 0x10)) | frozenset(range(0x20, 0x40))
_ADDRESS_OPS = frozenset({0x02, 0x03})


def normalized_instruction_stream(data: bytes) -> NormalizedInstructionStream:
    """Normalize immediate/address operands without erasing instruction shape."""

    if len(data) % 4:
        raise ValueError("MIPS instruction stream size must be a multiple of four")
    words: list[int] = []
    parameters: list[tuple[int, str, int]] = []
    for index in range(len(data) // 4):
        (word,) = struct.unpack_from("<I", data, index * 4)
        opcode = word >> 26
        if opcode in _ADDRESS_OPS:
            parameters.append((index, "address26", word & 0x03FFFFFF))
            words.append(word & 0xFC000000)
        elif opcode in _IMMEDIATE_OPS:
            parameters.append((index, "immediate16", word & 0xFFFF))
            words.append(word & 0xFFFF0000)
        else:
            words.append(word)
    return NormalizedInstructionStream(tuple(words), tuple(parameters))


# Internal control-shape check for the easy-to-regress boundary: arithmetic
# immediates normalize, branch displacements do not.
assert normalized_instruction_stream(b"\x01\x00\x08\x25").words == (0x25080000,)
assert normalized_instruction_stream(b"\x01\x00\x00\x11").words == (0x11000001,)

_LUI = 0x0F
# opcode -> immediate signedness for %lo uses of a lui-materialized register
_LO_OPS = {
    0x09: "s",  # addiu
    0x0D: "z",  # ori
    0x20: "s",
    0x21: "s",
    0x23: "s",
    0x24: "s",
    0x25: "s",  # lb/lh/lw/lbu/lhu
    0x28: "s",
    0x29: "s",
    0x2B: "s",  # sb/sh/sw
}
# SPECIAL functs that write rd
_SPECIAL_WRITES_RD = {
    0x00,
    0x02,
    0x03,
    0x04,
    0x08,
    0x09,
    0x0F,
    0x10,
    0x11,
    0x12,
    0x13,
    0x18,
    0x19,
    0x1A,
    0x1B,
    0x20,
    0x21,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x2A,
    0x2B,
}
_LUI_WINDOW = 12
_RETURN_VOID_BYTES = b"\x08\x00\xe0\x03\x00\x00\x00\x00"
_OPCODE_NAMES = {
    0x09: "addiu",
    0x0D: "ori",
    0x20: "lb",
    0x21: "lh",
    0x23: "lw",
    0x24: "lbu",
    0x25: "lhu",
    0x28: "sb",
    0x29: "sh",
    0x2B: "sw",
}


def trivial_kind(data: bytes) -> str | None:
    """Classify canonical return-void bodies; None for any other byte set."""

    if data == _RETURN_VOID_BYTES:
        return "return_void"
    if (
        len(data) == 8
        and data[:4] == b"\x08\x00\xe0\x03"
        and data[4:8] in {b"\x21\x10\x00\x00", b"\x25\x10\x00\x00"}
    ):
        return "constant_return"
    return None


def data_references(data: bytes) -> list[tuple[int, int, str, str]]:
    """Return source offsets, addresses, access kinds, and opcodes for lui/%lo pairs."""

    references: set[tuple[int, int, str, str]] = set()
    lui: dict[int, tuple[int, int]] = {}
    for index in range(len(data) // 4):
        (word,) = struct.unpack_from("<I", data, index * 4)
        op = word >> 26
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        rd = (word >> 11) & 31
        imm = word & 0xFFFF
        if op == _LUI:
            lui[rt] = (imm << 16, index)
            continue
        if op in _LO_OPS and rs in lui and index - lui[rs][1] <= _LUI_WINDOW:
            hi, _ = lui[rs]
            simm = (
                imm if _LO_OPS[op] == "z" else (imm - 0x10000 if imm & 0x8000 else imm)
            )
            access_kind = (
                "load"
                if 0x20 <= op <= 0x25
                else "store"
                if 0x28 <= op <= 0x2B
                else "address"
            )
            references.add(
                (index * 4, (hi + simm) & 0xFFFFFFFF, access_kind, _OPCODE_NAMES[op])
            )
        if op == 0 and (word & 0x3F) in _SPECIAL_WRITES_RD and rd in lui:
            del lui[rd]
        if op in _LO_OPS and rt in lui and op not in (0x28, 0x29, 0x2B):
            del lui[rt]
    return sorted(references)


def static_jals(data: bytes, base_address: int) -> list[tuple[int, int]]:
    """Decode static (direct) JALs: ``(absolute callsite, absolute target)`` pairs.

    Non-JAL words are skipped; a JAL target is PC-relative with the
    link-increment applied, so ``target = pc + 4 + ((imm & 0x03FFFFFF) << 2)``
    with the PC upper nibble preserved.
    """

    targets: list[tuple[int, int]] = []
    for index in range(len(data) // 4):
        (word,) = struct.unpack_from("<I", data, index * 4)
        if word >> 26 != 3:
            continue
        callsite = base_address + index * 4
        target = ((callsite + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
        targets.append((callsite, target))
    return targets


__all__ = [
    "NormalizedInstructionStream",
    "data_references",
    "normalized_instruction_stream",
    "static_jals",
    "trivial_kind",
]
