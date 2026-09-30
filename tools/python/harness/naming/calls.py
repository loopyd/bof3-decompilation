"""Decode bounded straight-line MIPS bodies from original words."""

from __future__ import annotations

import struct


def decode_selected_call(code: bytes, start: int, selected: int) -> dict:
    """Accept one direct call with a canonical stack/return transition only.

    This deliberately excludes branches, loops, indirect calls and load-derived
    arguments. Unknown instructions never establish an absence of guards.
    """
    if len(code) % 4 or not 32 <= len(code) <= 128:
        raise ValueError(
            "selected_call wrapper must contain 8..32 aligned instructions"
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
        or words[:2] != [0x27BD0000 | ((-frame) & 0xFFFF), 0xAFBF0000 | slot]
    ):
        raise ValueError("selected_call requires canonical saved-ra stack transition")
    registers = [{"kind": "entry_register", "register": number} for number in range(32)]
    registers[0] = {"kind": "constant", "value": 0}

    def step(word: int) -> None:
        if word == 0:
            return
        op, rs, rt, rd = (
            word >> 26,
            (word >> 21) & 31,
            (word >> 16) & 31,
            (word >> 11) & 31,
        )
        immediate = word & 0xFFFF
        if op == 9:
            destination_register = rt
            value = {
                "kind": "addiu",
                "base": registers[rs],
                "immediate": immediate - 0x10000 if immediate & 0x8000 else immediate,
            }
        elif op == 15 and rs == 0:
            destination_register = rt
            value = {"kind": "constant", "value": immediate << 16}
        elif op == 13:
            destination_register = rt
            value = {"kind": "ori", "base": registers[rs], "immediate": immediate}
        elif op == 0 and word & 63 == 0x21 and (word >> 6) & 31 == 0:
            destination_register = rd
            value = {"kind": "addu", "left": registers[rs], "right": registers[rt]}
        else:
            raise ValueError(
                "selected_call unresolved guard or argument: branch/loop or unsupported instruction"
            )
        if (
            destination_register not in range(2, 16)
            or rs in {29, 31}
            or (op == 0 and rt in {29, 31})
        ):
            raise ValueError("selected_call unsupported register dependency")
        # Keep expressions bounded even when arithmetic repeatedly doubles a DAG.
        import json

        if len(json.dumps(value)) > 4096:
            raise ValueError("selected_call argument expression exceeds decoder bound")
        registers[destination_register] = value

    for word in words[2:index]:
        step(word)
    step(words[index + 1])

    def instruction(offset: int) -> dict:
        return {
            "address": f"0x{start + offset * 4:08X}",
            "word": f"0x{words[offset]:08X}",
        }

    return {
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


def decode_owner_body(code: bytes, start: int) -> dict:
    """Describe only the direct effects of a completely decoded call wrapper."""
    if len(code) >= 16 and int.from_bytes(code[:4], "little") >> 26 == 15:
        return decode_owner_stores(code, start)
    if len(code) % 4 or not 32 <= len(code) <= 128:
        raise ValueError(
            "owner_body requires a canonical call wrapper or global-pointer zero-byte stores"
        )
    words = list(struct.unpack(f"<{len(code) // 4}I", code))
    calls = [index for index, word in enumerate(words) if word >> 26 == 3]
    if len(calls) != 1:
        raise ValueError("owner_body requires exactly one direct jal")
    index = calls[0]
    destination = ((start + index * 4 + 4) & 0xF0000000) | (
        (words[index] & 0x3FFFFFF) << 2
    )
    decoded = decode_selected_call(code, start, destination)
    return {
        "coverage": "complete straight-line call wrapper; direct effects only, not transitive callee effects",
        "range": {"start": f"0x{start:08X}", "end": f"0x{start + len(code):08X}"},
        "effects": {
            "stack_frame_bytes": decoded["transition"]["stack_restored_bytes"],
            "saved_register": "ra",
            "saved_register_stack_offset": words[1] & 0xFFFF,
            "argument_registers": decoded["argument_registers"],
            "result": decoded["result"],
            "transition": decoded["transition"],
        },
        "callees": [
            {key: decoded[key] for key in ("callee", "callsite", "delay_slot")}
        ],
        "globals": {
            "direct_accesses": [],
            "basis": "no direct globals observed within the decoded body",
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
