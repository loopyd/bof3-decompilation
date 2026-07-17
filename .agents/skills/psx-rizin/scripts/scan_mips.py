#!/usr/bin/env python3
"""Triage a raw little-endian PS1/MIPS binary.

The output is candidate evidence only. Random/compressed data can contain words
that resemble pointers, prologues, or jump instructions.
"""

from __future__ import annotations

import argparse
import json
import struct
from collections import Counter
from pathlib import Path
from typing import Any


def parse_int(value: str) -> int:
    return int(value, 0)


def physical_ram_offset(value: int, ram_size: int) -> int | None:
    physical = value & 0x1FFFFFFF
    if physical < ram_size and (value & 0xE0000000) in {0x80000000, 0xA0000000}:
        return physical
    return None


def decode_jump(word: int, pc: int) -> tuple[str, int] | None:
    opcode = word >> 26
    if opcode not in {2, 3}:
        return None
    target = ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
    return ("j" if opcode == 2 else "jal", target & 0xFFFFFFFF)


def scan(path: Path, base: int | None, ram_size: int, limit: int) -> dict[str, Any]:
    data = path.read_bytes()
    word_count = len(data) // 4
    pointers: list[dict[str, Any]] = []
    jumps: list[dict[str, Any]] = []
    prologues: list[dict[str, Any]] = []
    epilogues: list[dict[str, Any]] = []
    opcode_counts: Counter[int] = Counter()

    for offset in range(0, word_count * 4, 4):
        word = struct.unpack_from("<I", data, offset)[0]
        opcode_counts[word >> 26] += 1

        physical = physical_ram_offset(word, ram_size)
        if physical is not None and len(pointers) < limit:
            pointers.append(
                {
                    "file_offset": offset,
                    "word": word,
                    "virtual": f"0x{word:08x}",
                    "physical_candidate": f"0x{physical:08x}",
                }
            )

        # addiu sp, sp, negative immediate: 0x27bdxxxx
        if (word & 0xFFFF0000) == 0x27BD0000 and (word & 0x8000):
            frame = ((~word + 1) & 0xFFFF)
            if len(prologues) < limit:
                prologues.append(
                    {
                        "file_offset": offset,
                        "runtime": f"0x{base + offset:08x}" if base is not None else None,
                        "word": f"0x{word:08x}",
                        "candidate_frame_size": frame,
                    }
                )

        # jr ra, usually followed by a delay-slot instruction.
        if word == 0x03E00008 and len(epilogues) < limit:
            epilogues.append(
                {
                    "file_offset": offset,
                    "runtime": f"0x{base + offset:08x}" if base is not None else None,
                }
            )

        if base is not None:
            decoded = decode_jump(word, base + offset)
            if decoded is not None:
                mnemonic, target = decoded
                physical_target = physical_ram_offset(target, ram_size)
                if physical_target is not None and len(jumps) < limit:
                    jumps.append(
                        {
                            "file_offset": offset,
                            "runtime": f"0x{base + offset:08x}",
                            "mnemonic": mnemonic,
                            "target": f"0x{target:08x}",
                        }
                    )

    return {
        "path": str(path),
        "size": len(data),
        "base": f"0x{base:08x}" if base is not None else None,
        "ram_size": f"0x{ram_size:x}",
        "word_count": word_count,
        "candidate_counts": {
            "ram_pointer_words": sum(
                1
                for offset in range(0, word_count * 4, 4)
                if physical_ram_offset(struct.unpack_from("<I", data, offset)[0], ram_size)
                is not None
            ),
            "stack_frame_prologues": sum(
                1
                for offset in range(0, word_count * 4, 4)
                if (struct.unpack_from("<I", data, offset)[0] & 0xFFFF0000) == 0x27BD0000
                and (struct.unpack_from("<I", data, offset)[0] & 0x8000)
            ),
            "jr_ra": sum(
                1
                for offset in range(0, word_count * 4, 4)
                if struct.unpack_from("<I", data, offset)[0] == 0x03E00008
            ),
            "decoded_ram_jumps": len(jumps) if base is not None else None,
        },
        "samples_limited_to": limit,
        "ram_pointer_samples": pointers,
        "jump_call_samples": jumps,
        "prologue_samples": prologues,
        "epilogue_samples": epilogues,
        "opcode_histogram": {str(key): value for key, value in sorted(opcode_counts.items())},
        "warning": "Candidates are heuristic; validate with loader/runtime/static control-flow evidence.",
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("--base", type=parse_int, help="runtime base for decoding j/jal")
    parser.add_argument("--ram-size", type=parse_int, default=0x200000)
    parser.add_argument("--limit", type=int, default=200)
    parser.add_argument("--json", type=Path, help="write JSON to file instead of stdout")
    args = parser.parse_args()

    result = scan(args.input, args.base, args.ram_size, args.limit)
    encoded = json.dumps(result, indent=2, sort_keys=True)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(encoded + "\n", encoding="utf-8")
        print(args.json)
    else:
        print(encoded)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
