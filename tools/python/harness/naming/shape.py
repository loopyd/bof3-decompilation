"""Shape equality for the ordinal licence.

Two reviewed bodies may share a name with an ordinal only when their instructions
agree and the difference sits in an immediate: a table base, a passed constant, a
carried address. The test is word by word: equal word, or equal opcode *and* equal
register operands with a differing immediate field.
"""

from __future__ import annotations

# Instructions whose immediate field is an address or constant the name may omit.
_IMMEDIATE_OPCODES = frozenset({0x09, 0x0C, 0x0D, 0x0F, 0x18, 0x19})


def _register_fields(word: int, opcode: int) -> tuple[int, ...]:
    if opcode == 0x0F:  # lui: rt only
        return ((word >> 16) & 0x1F,)
    if opcode in {0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29}:  # loads/stores
        return ((word >> 21) & 0x1F, (word >> 16) & 0x1F)
    return ((word >> 21) & 0x1F, (word >> 16) & 0x1F, (word >> 11) & 0x1F)


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
        if op_a != op_b or op_a not in _IMMEDIATE_OPCODES:
            return False
        if _register_fields(a, op_a) != _register_fields(b, op_b):
            return False
    return True
