"""Decode direct memory effects of complete acyclic original MIPS leaf bodies."""

from __future__ import annotations

import json
import struct
from dataclasses import dataclass, replace


@dataclass(frozen=True)
class _LeafPath:
    registers: tuple[dict, ...]
    pending_load: tuple[int, dict] | None = None
    guards: tuple[dict, ...] = ()
    accesses: tuple[dict, ...] = ()
    covered: frozenset[int] = frozenset()


def _signed(value: int, bits: int) -> int:
    value &= (1 << bits) - 1
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def _expression(operation: str, left: dict, right: dict) -> dict:
    if left["kind"] == right["kind"] == "constant":
        a, b = left["value"], right["value"]
        value = {
            "add": lambda: (a + b) & 0xFFFFFFFF,
            "and": lambda: (a & b) & 0xFFFFFFFF,
            "signed_less_than": lambda: int(_signed(a, 32) < _signed(b, 32)),
        }[operation]()
        return {"kind": "constant", "value": value}
    if operation == "add" and right == {"kind": "constant", "value": 0}:
        return left
    if operation == "add" and left == {"kind": "constant", "value": 0}:
        return right
    if (
        operation == "add"
        and left["kind"] in {"entry_register", "entry_register_offset"}
        and left["register"] == 29
        and right["kind"] == "constant"
    ):
        offset = _signed(left.get("offset_bytes", 0) + right["value"], 32)
        return (
            {"kind": "entry_register_offset", "register": 29, "offset_bytes": offset}
            if offset
            else {"kind": "entry_register", "register": 29}
        )
    value = {"kind": operation, "left": left, "right": right, "width_bits": 32}
    if len(json.dumps(value)) > 4096:
        raise ValueError(
            "selected_body expression exceeds decoder representation bound"
        )
    return value


def _settle_load(
    path: _LeafPath, reads: set[int], destination: int | None
) -> _LeafPath:
    if path.pending_load is None:
        return path
    register, value = path.pending_load
    if register in reads or register == destination:
        raise ValueError("selected_body unresolved MIPS-I load-delay dependency")
    registers = list(path.registers)
    registers[register] = value
    return replace(path, registers=tuple(registers), pending_load=None)


def ram_offset(value: int) -> int | None:
    """Return the main-RAM mirror offset of a KUSEG/KSEG0/KSEG1 address, else None."""
    if value & 0xE0000000 in {0, 0x80000000, 0xA0000000}:
        physical = value & 0x1FFFFFFF
        if physical < 0x800000:
            return physical % 0x200000
    return None


def _overlaps_instructions(address: int, width: int, start: int, end: int) -> bool:
    if address < end and address + width > start:
        return True
    first, last = ram_offset(start), ram_offset(end - 1)
    if first is None or last != (first + end - start - 1) % 0x200000:
        return False
    return any(
        offset is not None and (offset - first) % 0x200000 < end - start
        for offset in (ram_offset(address + byte) for byte in range(width))
    )


def _step(path: _LeafPath, word: int, address: int, start: int, end: int) -> _LeafPath:
    if word == 0:
        return _settle_load(path, set(), None)
    op, rs, rt, rd = word >> 26, (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31
    immediate = _signed(word, 16)
    reads, destination = {rs}, rt
    value = None
    if op == 0 and word & 63 in {0x21, 0x24} and (word >> 6) & 31 == 0:
        reads.add(rt)
        destination = rd
        value = _expression(
            "add" if word & 63 == 0x21 else "and",
            path.registers[rs],
            path.registers[rt],
        )
    elif op in {9, 10, 12}:
        value = _expression(
            {9: "add", 10: "signed_less_than", 12: "and"}[op],
            path.registers[rs],
            {"kind": "constant", "value": word & 0xFFFF if op == 12 else immediate},
        )
    elif op == 15 and rs == 0:
        reads = set()
        value = {"kind": "constant", "value": (word & 0xFFFF) << 16}
    elif op in {33, 37, 40, 41}:
        is_read, width = op in {33, 37}, 1 if op == 40 else 2
        if not is_read:
            reads.add(rt)
            destination = None
        base = path.registers[rs]
        if base["kind"] != "constant":
            raise ValueError("selected_body unresolved memory address")
        effective = (base["value"] + immediate) & 0xFFFFFFFF
        if effective % width:
            raise ValueError("selected_body unaligned memory access")
        if not is_read and _overlaps_instructions(effective, width, start, end):
            raise ValueError("selected_body store overlaps captured instruction bytes")
        access = {
            "kind": "read" if is_read else "store",
            "instruction": f"0x{address:08X}",
            "address": f"0x{effective:08X}",
            "width_bytes": width,
        }
        if is_read:
            access.update(signed=op == 33, extension_bits=32)
            value = {**access, "kind": "memory_read"}
        else:
            access.update(value=path.registers[rt], stored_low_bits=width * 8)
        path = replace(path, accesses=path.accesses + (access,))
    else:
        raise ValueError("selected_body unsupported instruction or reserved encoding")
    if destination is not None and (
        destination in {0, 31} or destination == 29 and not (op == 9 and rs == 29)
    ):
        raise ValueError(
            "selected_body unsupported zero, stack or return-register write"
        )
    path = _settle_load(path, reads, destination)
    if destination is None:
        return path
    if op in {33, 37}:
        return replace(path, pending_load=(destination, value))
    registers = list(path.registers)
    registers[destination] = value
    return replace(path, registers=tuple(registers))


def decode_leaf_effects(code: bytes, start: int) -> dict:
    """Enumerate supported complete paths without assuming memory-read stability."""
    if (
        type(start) is not int
        or start % 4
        or len(code) % 4
        or not 8 <= len(code) <= 65536
        or not 0 <= start < start + len(code) <= 0x100000000
    ):
        raise ValueError("selected_body invalid complete original range")
    words = struct.unpack(f"<{len(code) // 4}I", code)
    tail, end = len(words) - 2, start + len(code)
    if words[tail] != 0x03E00008:
        raise ValueError("selected_body requires terminal jr ra and its ordinary slot")

    def instruction(index: int) -> dict:
        return {
            "address": f"0x{start + index * 4:08X}",
            "word": f"0x{words[index]:08X}",
        }

    transfers = {
        index for index, word in enumerate(words[:tail]) if word >> 26 in {2, 4, 5}
    }
    slots = {index + 1 for index in transfers} | {tail + 1}
    if any(index + 1 >= tail or index in slots for index in transfers):
        raise ValueError("selected_body transfer lacks an ordinary delay slot")
    edges = {}
    for index in transfers:
        word = words[index]
        target = (
            (((start + index * 4 + 4) & 0xF0000000) | ((word & 0x3FFFFFF) << 2))
            if word >> 26 == 2
            else (start + index * 4 + 4 + _signed(word, 16) * 4) & 0xFFFFFFFF
        )
        offset = target - start
        if offset % 4 or not index + 1 < offset // 4 <= tail or offset // 4 in slots:
            raise ValueError(
                "selected_body backward, escaping or delay-slot control edge"
            )
        edges[index] = offset // 4
    registers = tuple(
        {"kind": "entry_register", "register": i}
        if i
        else {"kind": "constant", "value": 0}
        for i in range(32)
    )
    pending = [(0, _LeafPath(registers))]
    completed = []
    while pending:
        if len(pending) + len(completed) > 4096:
            raise ValueError(
                "selected_body control graph exceeds decoder representation bound"
            )
        index, path = pending.pop()
        if not 0 <= index <= tail or index in slots:
            raise ValueError("selected_body unsupported control entry")
        path = replace(path, covered=path.covered | {index})
        word = words[index]
        if index == tail:
            path = _settle_load(path, {31}, None)
            path = _step(path, words[index + 1], start + (index + 1) * 4, start, end)
            if (
                path.pending_load is not None
                or path.registers[29] != registers[29]
                or path.registers[31] != registers[31]
            ):
                raise ValueError(
                    "selected_body return lacks settled loads or restored entry sp/ra"
                )
            completed.append(replace(path, covered=path.covered | {index + 1}))
            continue
        if index not in transfers:
            path = _step(path, word, start + index * 4, start, end)
            pending.append((index + 1, path))
            continue
        op, rs, rt = word >> 26, (word >> 21) & 31, (word >> 16) & 31
        left, right = path.registers[rs], path.registers[rt]
        path = _settle_load(path, {rs, rt} if op in {4, 5} else set(), None)
        # The branch samples operands before the ordinary slot changes registers.
        path = _step(path, words[index + 1], start + (index + 1) * 4, start, end)
        path = replace(path, covered=path.covered | {index + 1})
        if op == 2:
            pending.append((edges[index], path))
            continue
        for taken in (False, True):
            equal = taken == (op == 4)
            if left == right and not equal:
                continue
            if left["kind"] == right["kind"] == "constant" and equal != (
                left["value"] == right["value"]
            ):
                continue
            guard = {
                "branch": instruction(index),
                "delay_slot": instruction(index + 1),
                "relation": "equal" if equal else "not_equal",
                "left": left,
                "right": right,
            }
            pending.append(
                (
                    edges[index] if taken else index + 2,
                    replace(path, guards=path.guards + (guard,)),
                )
            )
    if not completed or set().union(*(path.covered for path in completed)) != set(
        range(len(words))
    ):
        raise ValueError(
            "selected_body original range contains unexplained or unreachable instructions"
        )
    return {
        "kind": "acyclic_leaf_memory_effects",
        "coverage": {
            "kind": "complete_original_body",
            "instructions": [instruction(i) for i in range(len(words))],
        },
        "paths": [
            {"guards": list(path.guards), "ordered_accesses": list(path.accesses)}
            for path in completed
        ],
        "transition": {
            "kind": "return_to_entry_ra",
            "stack_pointer": "restored_entry_sp",
            "instructions": [instruction(tail), instruction(tail + 1)],
        },
        "assumptions": [
            "Entry has no uncommitted delayed load from an earlier instruction.",
            "Each observed memory access completes normally at a mapped address.",
            "Standard PS1 two-megabyte RAM mirrors apply; address mappings and captured instruction bytes remain unchanged, including by this body's own stores.",
            "Instruction bytes and register/control state are not changed by external execution during this body.",
        ],
        "scope": "Direct architectural path effects only. Reads remain distinct, including repeated addresses. Path feasibility, object extent, gameplay meaning, asynchronous effects and a C return contract are not established.",
    }
