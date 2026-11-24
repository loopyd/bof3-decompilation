"""Shape equality for the ordinal licence.

Two reviewed bodies may share a name with an ordinal only when their instructions
agree and the difference sits in an immediate: a table base, a passed constant, a
carried address. The test is word by word: equal word, or equal opcode *and* equal
register operands with a differing immediate field.
"""

from __future__ import annotations

# Opcodes whose difference may be nothing but an address or constant: the
# immediate arithmetic family, the loads and stores that carry a table base or a
# record offset, and the jumps that carry a code address.
_IMMEDIATE_OPCODES = frozenset(
    {
        0x08,  # addi
        0x09,  # addiu
        0x0A,  # slti
        0x0B,  # sltiu
        0x0C,  # andi
        0x0D,  # ori
        0x0E,  # xori
        0x0F,  # lui
        0x18,  # daddi
        0x19,  # daddiu
        0x20, 0x21, 0x23, 0x24, 0x25,  # lb..lhu
        0x28, 0x29, 0x2B,  # sb, sh, sw
    }
)
# Jumps carry a code address; two bodies that share one may sit at different
# addresses, so their target fields differ without a behavioural difference.
_JUMP_OPCODES = frozenset({0x02, 0x03})
_LOADS_AND_STORES = frozenset({0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B})


def _register_fields(word: int, opcode: int) -> tuple[int, ...]:
    """The register operands a differing immediate must leave untouched."""
    if opcode in _JUMP_OPCODES:
        return ()
    if opcode == 0x0F:  # lui: rt only, the rest is the immediate
        return ((word >> 16) & 0x1F,)
    # Every other licensed opcode is I-type: rs at 21-25, rt at 16-20, and the
    # immediate occupies 0-15. Bits 11-15 belong to the immediate, not to a `rd`.
    return ((word >> 21) & 0x1F, (word >> 16) & 0x1F)


def same_shape(left: bytes, right: bytes) -> bool:
    """True when two little-endian MIPS bodies differ only by immediates.

    A word pair matches when the words are equal, or when both carry one of the
    immediate opcodes with the same register operands. Any other difference - an
    opcode, a register, a length, a delay-slot change - means the bodies are
    different and the ordinal is not licensed.
    """
    if not isinstance(left, bytes) or not isinstance(right, bytes):
        raise ValueError("shape comparison requires immutable bytes")
    if len(left) != len(right) or len(left) % 4:
        return False
    for offset in range(0, len(left), 4):
        a = int.from_bytes(left[offset : offset + 4], "little")
        b = int.from_bytes(right[offset : offset + 4], "little")
        if a == b:
            continue
        op_a, op_b = a >> 26, b >> 26
        if op_a != op_b:
            return False
        if op_a in _JUMP_OPCODES:
            continue
        if op_a not in _IMMEDIATE_OPCODES:
            return False
        if _register_fields(a, op_a) != _register_fields(b, op_b):
            return False
    return True
