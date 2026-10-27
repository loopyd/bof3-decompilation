"""Decode supported MIPS call paths and direct effects from original words."""

from __future__ import annotations

import json
import struct
from dataclasses import dataclass, replace

from harness.naming.effects import _overlaps_instructions, ram_offset

CALLER_MIN_BYTES = 32
CALLER_MAX_BYTES = 2048
CALLER_INSTRUCTION_RANGE = "8..512"


def decode_selected_call(code: bytes, start: int, selected: int) -> dict:
    """Decode a wrapper or a complete forward BEQ/BNE caller with opaque calls."""
    if len(code) % 4 or not CALLER_MIN_BYTES <= len(code) <= CALLER_MAX_BYTES:
        raise ValueError(
            f"selected_call requires {CALLER_INSTRUCTION_RANGE} aligned instructions"
        )
    words = struct.unpack(f"<{len(code) // 4}I", code)
    if any(word >> 26 in {4, 5} for word in words):
        return _decode_branch_calls(words, start, selected)
    return _decode_call_wrapper(code, start, selected)


@dataclass(frozen=True)
class _CallPath:
    registers: tuple[dict, ...]
    pending_load: tuple[int, dict] | None = None
    frame_allocated: bool = False
    ra_saved: bool = False
    guards: tuple[dict, ...] = ()
    calls: tuple[dict, ...] = ()
    covered: frozenset[int] = frozenset()
    arguments: dict | None = None


def _signed_immediate(word: int) -> int:
    value = word & 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def _commit_call_load(path: _CallPath, reads: set[int]) -> _CallPath:
    if path.pending_load is None:
        return path
    register, value = path.pending_load
    if register in reads:
        raise ValueError("selected_call unresolved MIPS-I load-delay dependency")
    registers = list(path.registers)
    registers[register] = value
    return replace(path, registers=tuple(registers), pending_load=None)


def _call_expression(kind: str, left: dict, right: dict) -> dict:
    if left["kind"] == right["kind"] == "constant":
        a, b = left["value"], right["value"]
        return {
            "kind": "constant",
            "value": (a | b if kind == "ori" else a + b) & 0xFFFFFFFF,
        }
    value = {"kind": kind, "left": left, "right": right, "width_bits": 32}
    if len(json.dumps(value)) > 4096:
        raise ValueError(
            "selected_call expression exceeds decoder representation bound"
        )
    return value


def _step_call_path(
    path: _CallPath, word: int, address: int, frame: int, slot: int
) -> _CallPath:
    """Decode one ordinary instruction; transfers and their slots have separate owners."""
    if word == 0:
        return _commit_call_load(path, set())
    if path.calls:
        raise ValueError(
            "selected_call unsupported post-call register or memory dependency"
        )
    if word == 0x27BD0000 | ((-frame) & 0xFFFF):
        if path.frame_allocated:
            raise ValueError("selected_call repeated frame allocation")
        return replace(_commit_call_load(path, set()), frame_allocated=True)
    if word == 0xAFBF0000 | slot:
        if not path.frame_allocated or path.ra_saved:
            raise ValueError(
                "selected_call saved-ra store lacks a unique allocated frame"
            )
        return replace(_commit_call_load(path, set()), ra_saved=True)
    op, rs, rt, rd = word >> 26, (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31
    reads = {rs}
    destination = rt
    if op == 0 and word & 63 == 0x21 and (word >> 6) & 31 == 0:
        reads.add(rt)
        destination = rd
        value = _call_expression("addu", path.registers[rs], path.registers[rt])
    elif op == 9:
        value = _call_expression(
            "addiu",
            path.registers[rs],
            {"kind": "constant", "value": _signed_immediate(word)},
        )
    elif op == 13:
        value = _call_expression(
            "ori", path.registers[rs], {"kind": "constant", "value": word & 0xFFFF}
        )
    elif op == 15 and rs == 0:
        reads = set()
        value = {"kind": "constant", "value": (word & 0xFFFF) << 16}
    elif op == 37:
        base = path.registers[rs]
        if base["kind"] != "constant":
            raise ValueError("selected_call unresolved halfword-read address")
        effective = (base["value"] + _signed_immediate(word)) & 0xFFFFFFFF
        if effective % 2:
            raise ValueError("selected_call unaligned halfword read")
        value = {
            "kind": "unsigned_halfword_read",
            "address": f"0x{effective:08X}",
            "instruction": f"0x{address:08X}",
            "width": 2,
        }
    else:
        raise ValueError("selected_call unsupported instruction or reserved encoding")
    if destination not in range(2, 16) or reads & {29, 31}:
        raise ValueError("selected_call unsupported register dependency")
    if path.pending_load is not None and path.pending_load[0] == destination:
        raise ValueError("selected_call unresolved overlapping load/register write")
    path = _commit_call_load(path, reads)
    if op == 37:
        return replace(path, pending_load=(destination, value))
    registers = list(path.registers)
    registers[destination] = value
    return replace(path, registers=tuple(registers))


def _decode_branch_calls(words: tuple[int, ...], start: int, selected: int) -> dict:
    """Visit every supported acyclic path, retaining opaque-call assumptions."""
    if (
        type(start) is not int
        or type(selected) is not int
        or start % 4
        or selected % 4
        or not 0 <= selected <= 0xFFFFFFFF
        or not 0 <= start < start + len(words) * 4 <= 0x100000000
    ):
        raise ValueError("selected_call invalid original range or callee")
    tail_index = len(words) - 4
    tail = words[tail_index:]
    if tail[2] != 0x03E00008 or not (tail[1] == 0 or tail[3] == 0):
        raise ValueError("selected_call undecodable return transition")
    restore = tail[3] if tail[1] == 0 else tail[1]
    frame, slot = restore & 0xFFFF, tail[0] & 0xFFFF
    if (
        restore >> 16 != 0x27BD
        or not 8 <= frame < 0x8000
        or frame % 8
        or tail[0] >> 16 != 0x8FBF
        or slot % 4
        or not 0 <= slot <= frame - 4
    ):
        raise ValueError("selected_call requires canonical saved-ra stack transition")

    def instruction(index: int) -> dict:
        return {
            "address": f"0x{start + index * 4:08X}",
            "word": f"0x{words[index]:08X}",
        }

    def destination(index: int) -> int:
        return ((start + index * 4 + 4) & 0xF0000000) | (
            (words[index] & 0x3FFFFFF) << 2
        )

    transfers = {
        index
        for index, word in enumerate(words[:tail_index])
        if word >> 26 in {2, 3, 4, 5}
    }
    slots = {index + 1 for index in transfers} | {len(words) - 1}
    if any(index + 1 >= tail_index or index in slots for index in transfers):
        raise ValueError("selected_call transfer has no ordinary delay slot")
    selected_sites = [
        index
        for index in transfers
        if words[index] >> 26 == 3 and destination(index) == selected
    ]
    if len(selected_sites) != 1:
        raise ValueError("selected_call requires one selected direct callsite")
    selected_site = selected_sites[0]

    def edge(index: int, target: int) -> int:
        offset = target - start
        if (
            offset % 4
            or not index + 1 < offset // 4 <= tail_index
            or offset // 4 in slots
        ):
            raise ValueError(
                "selected_call backward, escaping or delay-slot control edge"
            )
        return offset // 4

    # Validate even edges that later prove infeasible; no partial decode is evidence.
    edges = {}
    for index in transfers:
        op = words[index] >> 26
        if op in {4, 5}:
            target = (
                start + index * 4 + 4 + _signed_immediate(words[index]) * 4
            ) & 0xFFFFFFFF
            edges[index] = edge(index, target)
        elif op == 2:
            edges[index] = edge(index, destination(index))

    registers = tuple(
        {"kind": "entry_register", "register": i}
        if i
        else {"kind": "constant", "value": 0}
        for i in range(32)
    )
    pending = [(0, _CallPath(registers=registers))]
    completed = []
    while pending:
        index, path = pending.pop()
        if len(pending) + len(completed) > 4096:
            raise ValueError(
                "selected_call control graph exceeds decoder representation bound"
            )
        if index == tail_index:
            if (
                not path.frame_allocated
                or not path.ra_saved
                or path.pending_load is not None
            ):
                raise ValueError(
                    "selected_call return lacks settled load or saved entry-ra frame"
                )
            completed.append(
                replace(
                    path,
                    covered=path.covered | frozenset(range(tail_index, len(words))),
                )
            )
            continue
        if not 0 <= index < tail_index or index in slots:
            raise ValueError("selected_call unsupported control entry")
        path = replace(path, covered=path.covered | {index})
        word, op = words[index], words[index] >> 26
        if index not in transfers:
            path = _step_call_path(path, word, start + index * 4, frame, slot)
            pending.append((index + 1, path))
            continue
        delay = words[index + 1]
        if op in {2, 3} and delay != 0:
            raise ValueError("selected_call direct jump/call requires a nop delay slot")
        if op in {4, 5} and delay not in {0, 0xAFBF0000 | slot}:
            raise ValueError("selected_call unsupported conditional delay slot")
        path = replace(path, covered=path.covered | {index + 1})
        if op in {4, 5}:
            if path.calls:
                raise ValueError("selected_call unresolved post-call branch state")
            rs, rt = (word >> 21) & 31, (word >> 16) & 31
            if {rs, rt} & {29, 31}:
                raise ValueError("selected_call unsupported branch register dependency")
            left, right = path.registers[rs], path.registers[rt]
            path = _commit_call_load(path, {rs, rt})
            path = _step_call_path(path, delay, start + (index + 1) * 4, frame, slot)
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
            continue
        path = _commit_call_load(path, set())
        if op == 2:
            pending.append((edges[index], path))
            continue
        if not path.frame_allocated or not path.ra_saved:
            raise ValueError(
                "selected_call frame allocation and saved entry-ra must dominate every call"
            )
        if index == selected_site:
            if path.calls:
                raise ValueError("selected_call arguments follow an opaque prior call")
            path = replace(
                path, arguments={f"a{i - 4}": path.registers[i] for i in range(4, 8)}
            )
        call = {
            "callsite": instruction(index),
            "callee": f"0x{destination(index):08X}",
            "delay_slot": instruction(index + 1),
        }
        path = replace(path, calls=path.calls + (call,))
        pending.append((index + 2, path))

    if not completed or set().union(*(path.covered for path in completed)) != set(
        range(len(words))
    ):
        raise ValueError(
            "selected_call original body has unexplained or unreachable instructions"
        )
    selected_paths = [path for path in completed if path.arguments is not None]
    if not selected_paths or any(
        path.arguments != selected_paths[0].arguments for path in selected_paths
    ):
        raise ValueError("selected_call unresolved selected-call argument join")
    continuations = []
    for path in selected_paths:
        later_calls = list(path.calls[1:])
        continuations.append(
            {
                "guards": list(path.guards),
                "later_calls": later_calls,
                "local_reads_before_later_call": [],
                "outgoing_registers": {
                    "kind": "unresolved_after_subsequent_call"
                    if later_calls
                    else "body_local_forwarding",
                    "registers": ["v0", "v1"],
                },
            }
        )
    return {
        "callsite": instruction(selected_site),
        "callee": f"0x{selected:08X}",
        "delay_slot": instruction(selected_site + 1),
        "argument_registers": selected_paths[0].arguments,
        "guards": {
            "kind": "conditional_paths",
            "paths": [list(path.guards) for path in selected_paths],
            "scope": "path predicates; feasibility is not established",
        },
        "result": {
            "kind": "uninterpreted_call_registers",
            "registers": ["v0", "v1"],
            "continuations": continuations,
            "scope": "local register flow only; no C return contract or transitive callee effects",
        },
        "transition": {
            "kind": "conditional_return_to_caller",
            "stack_restored_bytes": frame,
            "assumptions": [
                "observed memory operations complete normally",
                "all opaque calls return normally and preserve sp and the caller's saved-entry-ra stack cell",
            ],
            "instructions": [instruction(i) for i in range(tail_index, len(words))],
            "paths": [
                {"guards": list(path.guards), "calls": list(path.calls)}
                for path in completed
            ],
            "coverage": {
                "kind": "complete_acyclic_body",
                "instructions": [instruction(i) for i in range(len(words))],
            },
        },
    }


def _decode_call_wrapper(code: bytes, start: int, selected: int) -> dict:
    """Accept one direct call with a canonical stack/return transition only.

    This deliberately excludes branches, loops, indirect calls and load-derived
    arguments. Fixed-address SB/SH stores are retained as ordered direct effects.
    Unknown instructions never establish an absence of guards.
    """
    if len(code) % 4 or not CALLER_MIN_BYTES <= len(code) <= CALLER_MAX_BYTES:
        raise ValueError(
            f"selected_call wrapper must contain {CALLER_INSTRUCTION_RANGE} aligned instructions"
        )
    words = list(struct.unpack(f"<{len(code) // 4}I", code))
    calls = [index for index, word in enumerate(words) if word >> 26 == 3]
    if len(calls) != 1:
        raise ValueError("selected_call requires exactly one direct jal")
    index = calls[0]
    if index < 2:
        raise ValueError("selected_call precedes the saved-ra prologue")
    site = start + index * 4
    destination = ((site + 4) & 0xF0000000) | ((words[index] & 0x3FFFFFF) << 2)
    if destination != selected or index + 1 >= len(words):
        raise ValueError("selected_call has no matching direct call and delay slot")
    # Restore ra and sp, respecting the MIPS-I load and return delay slots.
    tail = words[index + 2 :]
    if len(tail) != 4 or tail[2] != 0x03E00008 or not (tail[1] == 0 or tail[3] == 0):
        raise ValueError(
            "selected_call undecodable transition; branch/loop or non-wrapper body"
        )
    restore = tail[3] if tail[1] == 0 else tail[1]
    frame = restore & 0xFFFF
    slot = tail[0] & 0xFFFF
    if (
        restore >> 16 != 0x27BD
        or not 8 <= frame < 0x8000
        or frame % 8
        or slot % 4
        or not 0 <= slot <= frame - 4
        or tail[0] >> 16 != 0x8FBF
    ):
        raise ValueError("selected_call requires canonical saved-ra stack transition")
    # The canonical allocation and saved-ra pair must dominate the call in order.
    allocate = [
        offset
        for offset, word in enumerate(words)
        if word == 0x27BD0000 | ((-frame) & 0xFFFF)
    ]
    save = [offset for offset, word in enumerate(words) if word == 0xAFBF0000 | slot]
    if len(allocate) != 1 or len(save) != 1 or not allocate[0] < save[0] < index:
        raise ValueError("selected_call requires canonical saved-ra stack transition")
    registers = [{"kind": "entry_register", "register": number} for number in range(32)]
    registers[0] = {"kind": "constant", "value": 0}
    stores: list[dict] = []

    def step(word: int, address: int, allow_store: bool) -> None:
        if word == 0:
            return
        op, rs, rt, rd = (
            word >> 26,
            (word >> 21) & 31,
            (word >> 16) & 31,
            (word >> 11) & 31,
        )
        immediate = word & 0xFFFF
        offset = immediate - 0x10000 if immediate & 0x8000 else immediate
        if op in {40, 41}:
            if not allow_store:
                raise ValueError("selected_call unsupported delay-slot store")
            if rs in {29, 31} or rt in {29, 31}:
                raise ValueError("selected_call unsupported store register dependency")
            width = 1 if op == 40 else 2
            base, source = registers[rs], registers[rt]
            if base["kind"] != "constant" or source["kind"] != "constant":
                raise ValueError("selected_call unresolved store base or value")
            effective = (base["value"] + offset) & 0xFFFFFFFF
            if effective % width:
                raise ValueError("selected_call unaligned store")
            if ram_offset(effective) is None:
                raise ValueError("selected_call unsupported store address class")
            if _overlaps_instructions(effective, width, start, start + len(code)):
                raise ValueError(
                    "selected_call store overlaps captured instruction bytes"
                )
            if _overlaps_instructions(effective, width, selected, selected + 4):
                raise ValueError(
                    "selected_call store overwrites the selected callee entry"
                )
            stores.append(
                {
                    "kind": "store",
                    "instruction": f"0x{address:08X}",
                    "address": f"0x{effective:08X}",
                    "width_bytes": width,
                    "value": source["value"] & ((1 << (width * 8)) - 1),
                    "stored_low_bits": width * 8,
                }
            )
            return
        if op == 9:
            destination_register, base = rt, registers[rs]
            value = (
                {"kind": "constant", "value": (base["value"] + offset) & 0xFFFFFFFF}
                if base["kind"] == "constant"
                else {"kind": "addiu", "base": base, "immediate": offset}
            )
        elif op == 15 and rs == 0:
            destination_register = rt
            value = {"kind": "constant", "value": immediate << 16}
        elif op == 13:
            destination_register, base = rt, registers[rs]
            value = (
                {"kind": "constant", "value": (base["value"] | immediate) & 0xFFFFFFFF}
                if base["kind"] == "constant"
                else {"kind": "ori", "base": base, "immediate": immediate}
            )
        elif op == 0 and word & 63 == 0x21 and (word >> 6) & 31 == 0:
            destination_register, left, right = rd, registers[rs], registers[rt]
            value = (
                {
                    "kind": "constant",
                    "value": (left["value"] + right["value"]) & 0xFFFFFFFF,
                }
                if left["kind"] == right["kind"] == "constant"
                else {"kind": "addu", "left": left, "right": right}
            )
        else:
            raise ValueError(
                "selected_call unresolved guard or argument: branch/loop or unsupported instruction"
            )
        # `at` carries only constant address materialisation, never call operands.
        if (
            destination_register not in (range(1, 16) if op == 15 else range(2, 16))
            or rs in {29, 31}
            or (op == 0 and rt in {29, 31})
        ):
            raise ValueError("selected_call unsupported register dependency")
        # Keep expressions bounded even when arithmetic repeatedly doubles a DAG.
        import json

        if len(json.dumps(value)) > 4096:
            raise ValueError("selected_call argument expression exceeds decoder bound")
        registers[destination_register] = value

    for position in range(index):
        if position in (allocate[0], save[0]):
            continue
        step(words[position], start + position * 4, True)
    step(words[index + 1], start + (index + 1) * 4, False)

    def instruction(offset: int) -> dict:
        return {
            "address": f"0x{start + offset * 4:08X}",
            "word": f"0x{words[offset]:08X}",
        }

    decoded = {
        "callsite": instruction(index),
        "callee": f"0x{selected:08X}",
        "delay_slot": instruction(index + 1),
        "argument_registers": {
            f"a{number - 4}": registers[number] for number in range(4, 8)
        },
        "guards": {"kind": "none", "basis": "fully decoded straight-line wrapper"},
        "result": {
            "kind": "forwarded_registers",
            "registers": ["v0", "v1"],
            "meaning": "callee return values, not interpreted",
        },
        "transition": {
            "kind": "return_to_caller",
            "stack_restored_bytes": frame,
            "instructions": [
                instruction(offset) for offset in range(index + 2, len(words))
            ],
        },
    }
    if stores:
        decoded["direct_effects"] = {
            "kind": "ordered_fixed_address_stores",
            "stores": stores,
            "assumptions": [
                "entry sp is unknown, so the fixed store addresses are premises rather than proven exclusions: they are assumed disjoint from the caller's live stack frame and its saved-entry-ra cell",
                "stores are proven not to cover the captured caller instructions or the selected callee entry word; aliasing with any other executing code, mapped object or live datum remains assumed away",
                "each direct store completes normally while its ordinary main-RAM address remains mapped",
            ],
            "scope": "ordered direct stores only: instruction, fixed address, width and width-truncated constant value; current memory contents, object extent, lifetime, readers, gameplay meaning and callee effects remain unproven",
        }
    return decoded


def decode_owner_body(code: bytes, start: int) -> dict:
    """Describe only the direct effects of a completely decoded call wrapper."""
    if len(code) % 4:
        raise ValueError("owner_body requires aligned original instructions")
    words = list(struct.unpack(f"<{len(code) // 4}I", code))
    calls = [index for index, word in enumerate(words) if word >> 26 == 3]
    if not calls and len(code) >= 16 and words[0] >> 26 == 15:
        return decode_owner_stores(code, start)
    if not CALLER_MIN_BYTES <= len(code) <= CALLER_MAX_BYTES:
        raise ValueError(
            "owner_body requires a canonical call wrapper or global-pointer zero-byte stores"
        )
    if len(calls) != 1:
        raise ValueError("owner_body requires exactly one direct jal")
    index = calls[0]
    destination = ((start + index * 4 + 4) & 0xF0000000) | (
        (words[index] & 0x3FFFFFF) << 2
    )
    decoded = _decode_call_wrapper(code, start, destination)
    stores = decoded.get("direct_effects", {}).get("stores", [])
    # The wrapper decoder proved one saved-entry-ra store; it need not be word 1.
    saved = [word for word in words if word >> 16 == 0xAFBF]
    body = {
        "coverage": "complete straight-line call wrapper; direct effects only, not transitive callee effects",
        "range": {"start": f"0x{start:08X}", "end": f"0x{start + len(code):08X}"},
        "effects": {
            "stack_frame_bytes": decoded["transition"]["stack_restored_bytes"],
            "saved_register": "ra",
            "saved_register_stack_offset": saved[0] & 0xFFFF,
            "argument_registers": decoded["argument_registers"],
            "result": decoded["result"],
            "transition": decoded["transition"],
        },
        "callees": [
            {key: decoded[key] for key in ("callee", "callsite", "delay_slot")}
        ],
        "globals": {
            "direct_accesses": [],
            "basis": (
                "no direct global reads observed; fixed-address stores are recorded under direct_effects"
                if stores
                else "no direct globals observed within the decoded body"
            ),
        },
        "tables": {
            "direct_accesses": [],
            "basis": "no direct tables observed within the decoded body",
        },
        "consumers": [
            {
                "kind": "return_to_caller",
                "registers": ["v0", "v1"],
                "scope": "body-local forwarding only; external callers not analyzed",
            }
        ],
        "guards": decoded["guards"],
    }
    if stores:
        body["direct_effects"] = decoded["direct_effects"]
    return body


def decode_owner_stores(code: bytes, start: int) -> dict:
    """Decode repeated lui/lw/nop/sb-zero groups with a return-delay store."""
    if len(code) % 16 or not 16 <= len(code) <= 65536:
        raise ValueError(
            "owner_body zero-store body requires complete four-word groups"
        )
    words = list(struct.unpack(f"<{len(code) // 4}I", code))
    effects, globals_read = [], []
    for index in range(0, len(words), 4):
        lui, load, gap, store = words[index : index + 4]
        register = (lui >> 16) & 31
        final = index + 4 == len(words)
        if (
            lui >> 21 != 0x1E0
            or register not in range(2, 16)
            or load >> 26 != 35
            or (load >> 21) & 31 != register
            or (load >> 16) & 31 != register
            or gap != (0x03E00008 if final else 0)
            or store >> 26 != 40
            or (store >> 21) & 31 != register
            or (store >> 16) & 31 != 0
        ):
            raise ValueError(
                "owner_body unsupported zero-store group or unresolved base/load-delay"
            )

        def signed(word: int) -> int:
            value = word & 0xFFFF
            return value - 0x10000 if value & 0x8000 else value

        address = (((lui & 0xFFFF) << 16) + signed(load)) & 0xFFFFFFFF
        if address % 4:
            raise ValueError("owner_body unaligned global pointer read")
        load_site = f"0x{start + (index + 1) * 4:08X}"
        globals_read.append(
            {
                "kind": "global_pointer_read",
                "address": f"0x{address:08X}",
                "width": 4,
                "instruction": load_site,
            }
        )
        effects.append(
            {
                "kind": "zero_byte_store",
                "instruction": f"0x{start + (index + 3) * 4:08X}",
                "base": {
                    "kind": "loaded_pointer",
                    "load_instruction": load_site,
                    "global_address": f"0x{address:08X}",
                    "register": register,
                },
                "offset": signed(store),
                "width": 1,
                "value": 0,
                "load_delay": {"intervening_instructions": 1, "word": f"0x{gap:08X}"},
                "return_delay_slot": final,
            }
        )
    return {
        "coverage": "complete straight-line global-pointer zero-byte stores; direct effects only",
        "range": {"start": f"0x{start:08X}", "end": f"0x{start + len(code):08X}"},
        "effects": effects,
        "callees": [],
        "globals": globals_read,
        "tables": {
            "direct_accesses": [],
            "basis": "no direct table indexing observed within the decoded body",
        },
        "consumers": [
            {
                "kind": "store_base",
                "load_instruction": effect["base"]["load_instruction"],
                "store_instruction": effect["instruction"],
            }
            for effect in effects
        ],
        "guards": {"kind": "none", "basis": "fully decoded straight-line stores"},
        "transition": {
            "kind": "return_to_caller",
            "stack_restored_bytes": 0,
            "instructions": [
                {
                    "address": f"0x{start + (len(words) - 2 + i) * 4:08X}",
                    "word": f"0x{word:08X}",
                }
                for i, word in enumerate(words[-2:])
            ],
        },
    }
