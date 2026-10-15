"""Decode complete straight-line MIPS stack-copy dispatch consumers."""

from __future__ import annotations

import struct
from dataclasses import dataclass, field

from harness.domain.mips import word_fields
from harness.naming.effects import _overlaps_instructions


def _constant(value: int) -> dict:
    return {"kind": "constant", "value": value & 0xFFFFFFFF}


def _entry(register: int) -> dict:
    return {"kind": "entry_register", "register": register}


def _add(left: dict, right: dict) -> dict:
    if left["kind"] == "constant" and right["kind"] == "constant":
        return _constant(left["value"] + right["value"])
    if right["kind"] == "constant":
        value = right["value"]
        offset = value - 0x100000000 if value & 0x80000000 else value
        if left == _entry(29):
            return {"kind": "entry_stack_offset", "offset_bytes": offset}
        if left["kind"] == "entry_stack_offset":
            return {
                "kind": "entry_stack_offset",
                "offset_bytes": left["offset_bytes"] + offset,
            }
        if left["kind"] == "indexed_stack":
            return {**left, "base_offset": left["base_offset"] + offset}
        if offset == 0:
            return left
    if left["kind"] == "entry_stack_offset" and right["kind"] == "scaled_selector":
        return {**right, "kind": "indexed_stack", "base_offset": left["offset_bytes"]}
    raise ValueError("dispatch unsupported address or integer expression")


@dataclass
class _DispatchState:
    start: int
    end: int
    selected: int
    registers: list[dict] = field(
        default_factory=lambda: [_constant(0)] + [_entry(i) for i in range(1, 32)]
    )
    pending: tuple[int, dict] | None = None
    frame: int | None = None
    restored: bool = False
    stack: dict[int, dict] = field(default_factory=dict)
    reads: list[dict] = field(default_factory=list)
    copies: list[dict] = field(default_factory=list)
    globals: list[dict] = field(default_factory=list)
    memory: list[dict] = field(default_factory=list)
    call: dict | None = None

    def settle(self, destination: int | None = None) -> None:
        if self.pending is not None:
            register, value = self.pending
            if register != destination:
                self.registers[register] = value
            self.pending = None

    def step(self, word: int, address: int) -> None:
        if word == 0:
            self.settle()
            return
        f = word_fields(word)
        left, right = self.registers[f.rs], self.registers[f.rt]
        destination, value, load = f.rt, None, False
        if f.opcode == 15 and f.rs == 0:
            value = _constant(f.immediate << 16)
        elif f.opcode == 13 and left["kind"] == "constant":
            value = _constant(left["value"] | f.immediate)
        elif f.opcode == 9:
            value = _add(left, _constant(f.signed_immediate))
            if destination == 29:
                if f.rs != 29:
                    raise ValueError("dispatch stack adjustment has an unresolved base")
                if self.frame is None and f.signed_immediate < 0:
                    self.frame = -f.signed_immediate
                    if self.frame % 8 or not 24 <= self.frame <= 4096:
                        raise ValueError("dispatch stack frame is unsupported")
                elif (
                    self.frame is not None
                    and f.signed_immediate == self.frame
                    and self.call is not None
                    and not self.restored
                ):
                    self.restored = True
                    value = _entry(29)
                else:
                    raise ValueError(
                        "dispatch stack adjustments are incomplete or repeated"
                    )
        elif f.opcode == 0 and f.funct == 0x21 and f.shift == 0:
            destination = f.rd
            value = _add(left, right)
        elif f.opcode == 0 and f.funct == 0 and f.rs == 0:
            destination = f.rd
            if right["kind"] != "unsigned_byte_read" or f.shift != 2:
                raise ValueError("dispatch shift is not a byte selector scaled by four")
            value = {"kind": "scaled_selector", "selector": right, "scale": 4}
        elif f.opcode in {0x23, 0x24}:
            load = True
            if f.opcode == 0x24:
                if left["kind"] != "word_read" or f.signed_immediate < 0:
                    raise ValueError(
                        "dispatch selector has no constant pointer-cell read"
                    )
                value = {
                    "kind": "unsigned_byte_read",
                    "instruction": f"0x{address:08X}",
                    "pointer_cell_read": left,
                    "offset_bytes": f.signed_immediate,
                }
                self.memory.append(
                    {
                        "access": "read",
                        "instruction": f"0x{address:08X}",
                        "address": {
                            "pointer_cell_read": left,
                            "offset_bytes": f.signed_immediate,
                        },
                        "width_bytes": 1,
                        "value": value,
                    }
                )
                self.settle()
                if destination in {0, 29, 31}:
                    raise ValueError("dispatch unsupported selector destination")
                self.pending = destination, value
                return
            effective = _add(left, _constant(f.signed_immediate))
            if effective["kind"] == "constant":
                absolute = effective["value"]
                if absolute % 4:
                    raise ValueError("dispatch unaligned word read")
                value = {
                    "kind": "word_read",
                    "instruction": f"0x{address:08X}",
                    "address": f"0x{absolute:08X}",
                    "width_bytes": 4,
                }
                if self.selected <= absolute < self.selected + 64:
                    value = {**value, "kind": "selected_word_read"}
                    self.reads.append(value)
            elif effective["kind"] == "entry_stack_offset":
                offset = effective["offset_bytes"]
                if offset not in self.stack or self.stack[offset] != _entry(31):
                    raise ValueError("dispatch stack read is not the saved entry ra")
                value = self.stack[offset]
            elif effective["kind"] == "indexed_stack":
                value = {
                    **effective,
                    "kind": "indexed_stack_word",
                    "instruction": f"0x{address:08X}",
                    "copied_slots": list(self.copies),
                }
            else:
                raise ValueError("dispatch unsupported word-read address")
            self.memory.append(
                {
                    "access": "read",
                    "instruction": f"0x{address:08X}",
                    "address": effective,
                    "width_bytes": 4,
                    "value": value,
                }
            )
        elif f.opcode == 0x2B:
            destination = None
            effective = _add(left, _constant(f.signed_immediate))
            if effective["kind"] == "entry_stack_offset":
                offset = effective["offset_bytes"]
                if (
                    self.frame is None
                    or offset % 4
                    or not -self.frame <= offset <= -4
                    or offset in self.stack
                    or self.call is not None
                    or right != _entry(31)
                    and right["kind"] != "selected_word_read"
                ):
                    raise ValueError(
                        "dispatch stack write is not a distinct saved ra or table copy"
                    )
                self.stack[offset] = right
                if right["kind"] == "selected_word_read":
                    self.copies.append(
                        {
                            "instruction": f"0x{address:08X}",
                            "stack_offset": offset,
                            "read": right,
                        }
                    )
            elif effective["kind"] == right["kind"] == "constant":
                absolute = effective["value"]
                if absolute % 4 or _overlaps_instructions(
                    absolute, 4, self.start, self.end
                ):
                    raise ValueError(
                        "dispatch unaligned or instruction-overlapping global store"
                    )
                self.globals.append(
                    {
                        "instruction": f"0x{address:08X}",
                        "address": f"0x{absolute:08X}",
                        "width_bytes": 4,
                        "value": right["value"],
                        "phase": "before_call" if self.call is None else "after_call",
                    }
                )
            else:
                raise ValueError("dispatch unsupported store address or value")
            self.memory.append(
                {
                    "access": "write",
                    "instruction": f"0x{address:08X}",
                    "address": effective,
                    "width_bytes": 4,
                    "value": right,
                }
            )
        else:
            raise ValueError("dispatch unsupported instruction or reserved encoding")
        if destination == 0 or destination == 31 and not load:
            raise ValueError("dispatch unsupported zero or return-register write")
        if destination == 29 and f.opcode != 9:
            raise ValueError("dispatch unsupported stack-register write")
        # Operands above sample old register values. A direct write cancels an
        # older delayed load; a following load keeps its own later commit.
        self.settle(None if load else destination)
        if destination is not None:
            if load:
                self.pending = destination, value
            else:
                self.registers[destination] = value


def decode_dispatch_body(code: bytes, start: int, selected: int) -> dict:
    """Explain every word of one complete straight-line stack-table dispatcher."""
    if (
        type(start) is not int
        or type(selected) is not int
        or start % 4
        or selected % 4
        or not 0 <= selected < 0x100000000
        or len(code) % 4
        or not 8 <= len(code) <= 65536
        or not 0 <= start < start + len(code) <= 0x100000000
    ):
        raise ValueError("dispatch invalid complete original range or selected address")
    words = struct.unpack(f"<{len(code) // 4}I", code)
    state = _DispatchState(start, start + len(code), selected)
    instructions = [
        {"address": f"0x{start + i * 4:08X}", "word": f"0x{w:08X}"}
        for i, w in enumerate(words)
    ]
    if words[-2] != 0x03E00008:
        raise ValueError("dispatch requires terminal jr ra and its ordinary slot")
    index = 0
    while index < len(words) - 2:
        f = word_fields(words[index])
        if f.opcode == 0 and f.funct == 9:
            if (
                f.rt
                or f.shift
                or f.rd != 31
                or state.call is not None
                or index + 1 >= len(words) - 2
                or words[index + 1] != 0
            ):
                raise ValueError(
                    "dispatch requires one canonical jalr ra with a nop slot"
                )
            target = state.registers[f.rs]
            if target["kind"] != "indexed_stack_word":
                raise ValueError(
                    "dispatch jalr target is not a settled indexed stack read"
                )
            state.call = {
                "instruction": instructions[index],
                "delay_slot": instructions[index + 1],
                "target": target,
            }
            state.settle(31)
            state.registers[31] = _constant(start + index * 4 + 8)
            state.step(words[index + 1], start + (index + 1) * 4)
            state.call["argument_registers"] = {
                f"a{i}": state.registers[4 + i] for i in range(4)
            }
            for register in (*range(1, 16), 24, 25, 26, 27, 31):
                state.registers[register] = {
                    "kind": "opaque_call_register",
                    "register": register,
                }
            index += 2
            continue
        state.step(words[index], start + index * 4)
        index += 1
    if state.registers[31] != _entry(31):
        raise ValueError("dispatch return target is not settled saved entry ra")
    state.settle()
    state.step(words[-1], start + (len(words) - 1) * 4)
    if (
        state.pending is not None
        or state.registers[29] != _entry(29)
        or not state.restored
        or state.call is None
    ):
        raise ValueError(
            "dispatch lacks restored sp, settled loads or complete indirect call"
        )
    copies = sorted(state.copies, key=lambda item: item["stack_offset"])
    if not 1 <= len(copies) <= 16 or len(state.reads) != len(copies):
        raise ValueError(
            "dispatch selected reads are not completely copied to stack slots"
        )
    base = copies[0]["stack_offset"]
    if any(
        c["stack_offset"] != base + i * 4
        or int(c["read"]["address"], 16) != selected + i * 4
        for i, c in enumerate(copies)
    ):
        raise ValueError(
            "dispatch selected words and copied stack slots are not contiguous"
        )
    target = state.call["target"]
    if (
        target["base_offset"] != base
        or target["scale"] != 4
        or target["copied_slots"] != state.copies
    ):
        raise ValueError("dispatch indexed read does not select the copied stack slots")
    return {
        "kind": "stack_copy_dispatch",
        "coverage": {"kind": "complete_original_body", "instructions": instructions},
        "observed_read_span": [selected, selected + len(copies) * 4],
        "selected_reads": state.reads,
        "stack_copies": copies,
        "global_stores": state.globals,
        "memory_accesses": state.memory,
        "selector": target["selector"],
        "selection": {
            "scale": 4,
            "entry_sp_offsets_by_byte_value": [base + i * 4 for i in range(256)],
            "copied_word_selector_values": list(range(len(copies))),
            "unresolved_selector_values": list(range(len(copies), 256)),
            "bounds_check": False,
        },
        "call": state.call,
        "transition": {
            "kind": "conditional_return_to_entry_ra",
            "instructions": instructions[-2:],
        },
        "assumptions": [
            "Entry has no uncommitted delayed load from an earlier instruction.",
            "Each memory access completes normally, with valid alignment and mapped storage.",
            "Captured instructions and standard PS1 RAM mappings remain stable, including through aliases and the indirect call.",
            "The loaded pointer cell and selected byte are live values; original file words do not establish current memory contents.",
            "The indirect call returns normally and preserves registers 16–23 and 28–30 (s0–s7, gp, sp, fp), and the saved entry-ra stack cell.",
            "The stack frame and copied slots do not alias the selected word reads, pointer cell, selector byte, global stores or captured instruction bytes.",
        ],
        "scope": "Observed word reads, stack copies and unsigned-byte indirect selection only. Values outside copied slots, object extent, total table length, lifetime, callee effects, gameplay meaning and a C signature or return contract remain unproven.",
    }
