"""Conservative MIPS-I reachability checks for isolated function sections."""

from __future__ import annotations

_TABLE_LIMIT = 4096


def _table_dispatch(
    words: list[int], index: int, address: int
) -> tuple[int, int, int] | None:
    """Recognize one guarded MIPS-I absolute jump-table dispatch.

    The leading nop settles incoming loads before the bound reads the index.
    """
    barrier, bound, branch, shift, high, add, load, load_slot, jump, jump_slot = words[
        index : index + 10
    ]
    original_index = (bound >> 21) & 31
    predicate = (bound >> 16) & 31
    count = bound & 0xFFFF
    scaled = (shift >> 11) & 31
    base = (high >> 16) & 31
    pointer = (load >> 16) & 31
    if (
        barrier
        or bound >> 26 != 11
        or not 1 <= count <= _TABLE_LIMIT
        or not original_index
        or not predicate
        or predicate == original_index
        or branch >> 26 != 4
        or ((branch >> 21) & 31, (branch >> 16) & 31)
        not in {(predicate, 0), (0, predicate)}
        or not scaled
        or shift != (original_index << 16) | (scaled << 11) | (2 << 6)
        or high >> 26 != 15
        or (high >> 21) & 31
        or not base
        or base == scaled
        or add
        not in {
            (base << 21) | (scaled << 16) | (base << 11) | 33,
            (scaled << 21) | (base << 16) | (base << 11) | 33,
        }
        or load >> 26 != 35
        or (load >> 21) & 31 != base
        or pointer in {0, 31}
        or load_slot
        or jump != (pointer << 21) | 8
        or jump_slot
    ):
        return None
    low = load & 0xFFFF
    if low & 0x8000:
        low -= 0x10000
    table = (((high & 0xFFFF) << 16) + low) & 0xFFFFFFFF
    _, default = _classify_transfer(branch, address + index * 4 + 8)
    assert default is not None
    return table, count, default


def _table_targets(
    dispatch: tuple[int, int, int],
    readonly_ranges: tuple[tuple[int, bytes], ...],
    address: int,
    end: int,
) -> tuple[int, ...]:
    table, count, _ = dispatch
    ranges = [
        (start, content)
        for start, content in readonly_ranges
        if start <= table < table + count * 4 <= start + len(content)
    ]
    if table % 4 or len(ranges) != 1:
        raise ValueError(
            "grouped jump table lacks one complete reviewed readonly range"
        )
    start, content = ranges[0]
    offset = table - start
    targets = tuple(
        int.from_bytes(content[position : position + 4], "little")
        for position in range(offset, offset + count * 4, 4)
    )
    if any(target % 4 or not address <= target < end for target in targets):
        raise ValueError("grouped jump-table target leaves the complete function range")
    return targets


def _classify_transfer(word: int, address: int) -> tuple[str, int | None]:
    opcode = word >> 26
    source = (word >> 21) & 31
    target = (word >> 16) & 31
    if opcode in {2, 3}:
        destination = ((address + 4) & 0xF0000000) | ((word & 0x3FFFFFF) << 2)
        return ("jump" if opcode == 2 else "call", destination)
    if opcode == 0:
        operation = word & 63
        if operation == 8 and word & 0x1FFFFF == 8 and source == 31:
            return "return", None
        if operation == 9 and word & 0x1FFFFF == 0xF809:
            return "call", None
        if operation not in {
            0,
            2,
            3,
            4,
            6,
            7,
            16,
            17,
            18,
            19,
            24,
            25,
            26,
            27,
            32,
            33,
            34,
            35,
            36,
            37,
            38,
            39,
            42,
            43,
        }:
            raise ValueError("unsupported special control flow in grouped function")
        return "next", None
    if opcode in {1, 4, 5, 6, 7}:
        displacement = word & 0xFFFF
        if displacement & 0x8000:
            displacement -= 0x10000
        destination = (address + 4 + displacement * 4) & 0xFFFFFFFF
        if opcode == 1:
            if target not in {0, 1}:
                raise ValueError("unsupported REGIMM branch in grouped function")
            always = source == 0 and target == 1
            never = source == 0 and target == 0
        elif opcode in {6, 7}:
            if target:
                raise ValueError("invalid grouped branch encoding")
            always = source == 0 and opcode == 6
            never = source == 0 and opcode == 7
        else:
            always = source == target and opcode == 4
            never = source == target and opcode == 5
        return ("jump" if always else "skip" if never else "branch"), destination
    if opcode in {
        8,
        9,
        10,
        11,
        12,
        13,
        14,
        15,
        32,
        33,
        34,
        35,
        36,
        37,
        38,
        40,
        41,
        42,
        43,
        46,
        50,
        58,
    }:
        return "next", None
    if opcode == 18 and (source in {0, 2, 4, 6} or source >= 16):
        return "next", None
    raise ValueError("unsupported instruction flow in grouped function")


def validate_function_flow(
    content: bytes,
    address: int,
    *,
    readonly_ranges: tuple[tuple[int, bytes], ...] = (),
) -> None:
    """Reject reachable sequential escape and unproved indirect/exception transfers.

    Explicit external branches are tails; calls may return to PC+8. Delay-slot
    execution never marks that address visited as an ordinary instruction entry.
    Guarded table dispatches require actual reviewed readonly bytes and cannot be
    entered after their bound. This proves isolation only, not termination, ABI,
    runtime immutability, or original-byte fidelity.
    """

    if not content or len(content) % 4 or address % 4:
        raise ValueError("function flow requires aligned nonempty instructions")
    end = address + len(content)
    if not 0 <= address < end <= 0x100000000:
        raise ValueError("function flow exceeds the 32-bit address space")
    words = [
        int.from_bytes(content[offset : offset + 4], "little")
        for offset in range(0, len(content), 4)
    ]
    dispatches = {}
    interiors = set()
    for index in range(len(words) - 9):
        dispatch = _table_dispatch(words, index, address)
        if dispatch is not None:
            current = address + index * 4
            dispatches[current] = dispatch
            interiors.update(range(current + 4, current + 40, 4))
    pending = [address]
    visited = set()
    while pending:
        current = pending.pop()
        if not address <= current < end:
            raise ValueError("reachable function fallthrough leaves its section")
        if current in interiors:
            raise ValueError("reachable entry bypasses the grouped jump-table bound")
        if current in visited:
            continue
        visited.add(current)
        if current in dispatches:
            dispatch = dispatches[current]
            pending.extend(_table_targets(dispatch, readonly_ranges, address, end))
            default = dispatch[2]
            if address <= default < end:
                pending.append(default)
            continue
        kind, destination = _classify_transfer(words[(current - address) // 4], current)
        if kind == "next":
            pending.append(current + 4)
            continue
        if current + 4 >= end:
            raise ValueError("function transfer has no owned delay slot")
        slot, _ = _classify_transfer(words[(current - address) // 4 + 1], current + 4)
        if slot != "next":
            raise ValueError("unsupported transfer in a function delay slot")
        if kind in {"call", "branch", "skip"}:
            pending.append(current + 8)
        if kind in {"call", "branch", "jump"} and destination is not None:
            if address <= destination < end:
                pending.append(destination)
