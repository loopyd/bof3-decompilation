#!/usr/bin/env python3
"""Inspect and extract Sony PlayStation PS-X EXE files.

This parser intentionally handles only the conventional 0x800-byte PS-X EXE
header and raw payload mapping. It does not claim to parse arbitrary overlays,
CPE files, or packed executables.
"""

from __future__ import annotations

import argparse
import json
import struct
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import NoReturn

HEADER_SIZE = 0x800
MAGIC = b"PS-X EXE"


class PsxExeError(ValueError):
    pass


@dataclass(frozen=True)
class PsxExeHeader:
    path: str
    file_size: int
    magic: str
    initial_pc: int
    initial_gp: int
    text_address: int
    text_size: int
    data_address: int
    data_size: int
    bss_address: int
    bss_size: int
    stack_address: int
    stack_size: int
    payload_file_offset: int
    payload_available_size: int
    text_end: int
    text_size_fits_file: bool


def fail(message: str, exit_code: int = 2) -> NoReturn:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(exit_code)


def parse_int(value: str) -> int:
    try:
        return int(value, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"invalid integer: {value}") from exc


def u32(header: bytes, offset: int) -> int:
    return struct.unpack_from("<I", header, offset)[0]


def read_header(path: Path) -> PsxExeHeader:
    try:
        file_size = path.stat().st_size
    except OSError as exc:
        raise PsxExeError(f"cannot stat {path}: {exc}") from exc

    if file_size < HEADER_SIZE:
        raise PsxExeError(
            f"{path} is {file_size} bytes; a conventional PS-X EXE needs at least 0x800 bytes"
        )

    try:
        with path.open("rb") as stream:
            header = stream.read(HEADER_SIZE)
    except OSError as exc:
        raise PsxExeError(f"cannot read {path}: {exc}") from exc

    if header[: len(MAGIC)] != MAGIC:
        observed = header[: len(MAGIC)]
        raise PsxExeError(f"missing PS-X EXE magic; observed {observed!r}")

    text_address = u32(header, 0x18)
    text_size = u32(header, 0x1C)
    available = file_size - HEADER_SIZE

    return PsxExeHeader(
        path=str(path),
        file_size=file_size,
        magic=MAGIC.decode("ascii"),
        initial_pc=u32(header, 0x10),
        initial_gp=u32(header, 0x14),
        text_address=text_address,
        text_size=text_size,
        data_address=u32(header, 0x20),
        data_size=u32(header, 0x24),
        bss_address=u32(header, 0x28),
        bss_size=u32(header, 0x2C),
        stack_address=u32(header, 0x30),
        stack_size=u32(header, 0x34),
        payload_file_offset=HEADER_SIZE,
        payload_available_size=available,
        text_end=(text_address + text_size) & 0xFFFFFFFF,
        text_size_fits_file=(text_size <= available),
    )


def hexify(data: dict[str, object]) -> dict[str, object]:
    address_fields = {
        "initial_pc",
        "initial_gp",
        "text_address",
        "text_size",
        "data_address",
        "data_size",
        "bss_address",
        "bss_size",
        "stack_address",
        "stack_size",
        "payload_file_offset",
        "payload_available_size",
        "text_end",
        "file_size",
    }
    result: dict[str, object] = {}
    for key, value in data.items():
        if key in address_fields and isinstance(value, int):
            result[key] = f"0x{value:08x}"
        else:
            result[key] = value
    return result


def command_inspect(args: argparse.Namespace) -> int:
    header = read_header(args.input)
    raw = asdict(header)
    if args.json:
        print(json.dumps(raw, indent=2, sort_keys=True))
        return 0

    pretty = hexify(raw)
    width = max(len(key) for key in pretty)
    for key, value in pretty.items():
        print(f"{key:<{width}} : {value}")
    if not header.text_size_fits_file:
        print(
            "warning: header text_size exceeds bytes available after the 0x800-byte header",
            file=sys.stderr,
        )
    return 0


def extraction_size(header: PsxExeHeader, extract_all: bool) -> int:
    if extract_all or header.text_size == 0:
        return header.payload_available_size
    if header.text_size > header.payload_available_size:
        raise PsxExeError(
            f"header text_size 0x{header.text_size:x} exceeds available payload "
            f"0x{header.payload_available_size:x}; use --all only after investigating"
        )
    return header.text_size


def command_extract(args: argparse.Namespace) -> int:
    header = read_header(args.input)
    size = extraction_size(header, args.all)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    try:
        with args.input.open("rb") as source:
            source.seek(HEADER_SIZE)
            payload = source.read(size)
        if len(payload) != size:
            raise PsxExeError(f"short read: expected {size}, got {len(payload)}")
        args.output.write_bytes(payload)
    except OSError as exc:
        raise PsxExeError(f"extract failed: {exc}") from exc
    print(
        json.dumps(
            {
                "input": str(args.input),
                "output": str(args.output),
                "bytes": size,
                "runtime_base": f"0x{header.text_address:08x}",
            },
            indent=2,
        )
    )
    return 0


def command_offset_to_addr(args: argparse.Namespace) -> int:
    header = read_header(args.input)
    offset = args.offset
    if offset < HEADER_SIZE:
        raise PsxExeError("offset is inside the PS-X EXE header, not the mapped text payload")
    payload_offset = offset - HEADER_SIZE
    if payload_offset >= header.payload_available_size:
        raise PsxExeError("offset is beyond the available payload")
    address = (header.text_address + payload_offset) & 0xFFFFFFFF
    print(f"0x{address:08x}")
    return 0


def command_addr_to_offset(args: argparse.Namespace) -> int:
    header = read_header(args.input)
    address = args.address
    if address < header.text_address:
        raise PsxExeError("address precedes text load address")
    payload_offset = address - header.text_address
    if payload_offset >= header.payload_available_size:
        raise PsxExeError("address is beyond the available mapped payload")
    print(f"0x{HEADER_SIZE + payload_offset:x}")
    return 0


def command_aliases(args: argparse.Namespace) -> int:
    physical = args.address & 0x1FFFFFFF
    result = {
        "input": f"0x{args.address:08x}",
        "physical_candidate": f"0x{physical:08x}",
        "cached_kseg0_candidate": f"0x{physical | 0x80000000:08x}",
        "uncached_kseg1_candidate": f"0x{physical | 0xA0000000:08x}",
    }
    print(json.dumps(result, indent=2))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)

    inspect = sub.add_parser("inspect", help="inspect the conventional PS-X EXE header")
    inspect.add_argument("input", type=Path)
    inspect.add_argument("--json", action="store_true", help="emit numeric JSON")
    inspect.set_defaults(handler=command_inspect)

    extract = sub.add_parser("extract", help="extract the mapped text payload")
    extract.add_argument("input", type=Path)
    extract.add_argument("-o", "--output", type=Path, required=True)
    extract.add_argument(
        "--all",
        action="store_true",
        help="extract every byte after the header instead of header text_size",
    )
    extract.set_defaults(handler=command_extract)

    to_addr = sub.add_parser("offset-to-addr", help="convert PS-X EXE file offset to runtime address")
    to_addr.add_argument("input", type=Path)
    to_addr.add_argument("offset", type=parse_int)
    to_addr.set_defaults(handler=command_offset_to_addr)

    to_offset = sub.add_parser("addr-to-offset", help="convert runtime address to PS-X EXE file offset")
    to_offset.add_argument("input", type=Path)
    to_offset.add_argument("address", type=parse_int)
    to_offset.set_defaults(handler=command_addr_to_offset)

    aliases = sub.add_parser("aliases", help="show candidate physical/KSEG aliases")
    aliases.add_argument("address", type=parse_int)
    aliases.set_defaults(handler=command_aliases)
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    try:
        return int(args.handler(args))
    except PsxExeError as exc:
        fail(str(exc))


if __name__ == "__main__":
    raise SystemExit(main())
