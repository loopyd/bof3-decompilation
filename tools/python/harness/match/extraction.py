"""Read complete, symbol-qualified function images from linked PSX ELF files."""

from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.files import read_file

_HEADER = struct.Struct("<16sHHIIIIIHHHHHH")
_SECTION = struct.Struct("<IIIIIIIIII")
_SYMBOL = struct.Struct("<IIIBBH")
_MAX_FILE_SIZE = 64 * 1024 * 1024


@dataclass(frozen=True)
class Section:
    name_offset: int
    kind: int
    flags: int
    address: int
    offset: int
    size: int
    link: int
    info: int
    alignment: int
    entry_size: int


@dataclass(frozen=True)
class FunctionImage:
    name: str
    address: int
    section: str
    section_address: int
    section_size: int
    content: bytes


def _read_range(content: bytes, offset: int, size: int, label: str) -> bytes:
    if offset < 0 or size < 0 or offset + size > len(content):
        raise ValueError(f"ELF {label} exceeds file bounds")
    return content[offset : offset + size]


def _read_string(strings: bytes, offset: int) -> str:
    if offset >= len(strings):
        raise ValueError("ELF string offset exceeds string table")
    end = strings.find(b"\0", offset)
    if end < 0 or end - offset > 4096:
        raise ValueError("ELF string is unterminated or oversized")
    try:
        return strings[offset:end].decode("ascii")
    except UnicodeDecodeError as error:
        raise ValueError("ELF section name is not ASCII") from error


def _read_sections(content: bytes) -> tuple[list[Section], list[str]]:
    if len(content) < _HEADER.size:
        raise ValueError("truncated ELF header")
    header = _HEADER.unpack_from(content)
    identity, kind, machine, version = header[:4]
    if (
        identity[:7] != b"\x7fELF\x01\x01\x01"
        or kind != 2
        or machine != 8
        or version != 1
        or header[8] != _HEADER.size
    ):
        raise ValueError(
            "function extraction requires a linked ELF32 little-endian MIPS image"
        )
    section_offset, entry_size, count, names_index = (
        header[6],
        header[11],
        header[12],
        header[13],
    )
    if (
        entry_size != _SECTION.size
        or not 0 < count < 0xFF00
        or not 0 < names_index < count
        or section_offset < _HEADER.size
        or section_offset % 4
    ):
        raise ValueError("unsupported ELF section table layout")
    table = _read_range(content, section_offset, count * entry_size, "section table")
    metadata_ranges = [(0, _HEADER.size), (section_offset, section_offset + len(table))]
    if header[10]:
        if header[9] != 32 or header[5] < _HEADER.size or header[5] % 4:
            raise ValueError("unsupported ELF program header table")
        programs = _read_range(
            content, header[5], header[9] * header[10], "program headers"
        )
        metadata_ranges.append((header[5], header[5] + len(programs)))
    sections = [Section(*values) for values in _SECTION.iter_unpack(table)]
    if any(_SECTION.unpack_from(table)):
        raise ValueError("unsupported ELF null section or extended numbering")
    for section in sections:
        check_deadline()
        if section.alignment and section.alignment & (section.alignment - 1):
            raise ValueError("invalid ELF section alignment")
        if section.alignment and (
            (section.flags & 6 and section.address % section.alignment)
            or (section.kind not in {0, 8} and section.offset % section.alignment)
        ):
            raise ValueError("ELF section violates its declared alignment")
        if section.kind not in {0, 8}:
            _read_range(content, section.offset, section.size, "section payload")
            if section.size and any(
                section.offset < end and start < section.offset + section.size
                for start, end in metadata_ranges
            ):
                raise ValueError("ELF section payload overlaps header metadata")
        if section.flags & 6 and section.address + section.size > 2**32:
            raise ValueError("allocated ELF section exceeds the 32-bit address space")
    names_section = sections[names_index]
    if names_section.kind != 3:
        raise ValueError("ELF section names do not reference a string table")
    names = _read_range(
        content, names_section.offset, names_section.size, "section names"
    )
    if not names or names[0] != 0 or names[-1] != 0:
        raise ValueError("invalid ELF section-name string table")
    return sections, [_read_string(names, section.name_offset) for section in sections]


def _find_symbol(content: bytes, sections: list[Section], name: str) -> tuple:
    encoded = name.encode("ascii") + b"\0"
    matches = []
    for section in sections:
        if section.kind != 2:
            continue
        if (
            section.entry_size != _SYMBOL.size
            or section.offset % 4
            or section.size < _SYMBOL.size
            or section.size % _SYMBOL.size
            or not 0 < section.link < len(sections)
            or not 1 <= section.info <= section.size // _SYMBOL.size
        ):
            raise ValueError("unsupported ELF symbol table layout")
        strings_section = sections[section.link]
        if strings_section.kind != 3:
            raise ValueError("ELF symbols do not reference a string table")
        strings = _read_range(
            content, strings_section.offset, strings_section.size, "symbol names"
        )
        if not strings or strings[0] != 0 or strings[-1] != 0:
            raise ValueError("invalid ELF symbol-name string table")
        symbols = _read_range(content, section.offset, section.size, "symbols")
        if any(_SYMBOL.unpack_from(symbols)):
            raise ValueError("ELF reserved symbol entry must be all zero")
        seen_nonlocal = False
        for index, symbol in enumerate(_SYMBOL.iter_unpack(symbols)):
            check_deadline()
            local = symbol[3] >> 4 == 0
            if (
                (not local and index < section.info)
                or (local and seen_nonlocal)
                or (
                    local
                    and index >= section.info
                    and (symbol[3] & 15 not in {0, 4} or symbol[2] != 0)
                )
            ):
                raise ValueError(
                    "ELF symbol binding disagrees with the local partition"
                )
            seen_nonlocal = seen_nonlocal or not local
            if symbol[5] == 0xFFFF or len(sections) <= symbol[5] < 0xFF00:
                raise ValueError("ELF symbol has an unsupported section index")
            name_offset = symbol[0]
            if name_offset >= len(strings):
                raise ValueError("ELF symbol name exceeds its string table")
            if strings[name_offset : name_offset + len(encoded)] == encoded:
                matches.append(symbol)
    if len(matches) != 1:
        raise ValueError(f"expected exactly one defined function symbol: {name}")
    return matches[0]


def read_function_image(
    path: Path, *, function_name: str, address: int
) -> FunctionImage:
    """Select the full actual function extent; expected original length is not a bound."""
    check_deadline()
    if (
        not isinstance(function_name, str)
        or not function_name
        or len(function_name) > 4096
        or not function_name.isascii()
        or "\0" in function_name
        or type(address) is not int
        or not 0 <= address < 2**32
        or address % 4
    ):
        raise ValueError(
            "function extraction requires a name and aligned 32-bit address"
        )
    content = read_file(path.parent, path.name, max_bytes=_MAX_FILE_SIZE)
    if content is None:
        raise ValueError("missing linked ELF image")
    sections, names = _read_sections(content)
    symbol = _find_symbol(content, sections, function_name)
    _, value, size, info, visibility, section_index = symbol
    if (
        info & 15 != 2
        or info >> 4 not in {0, 1, 2}
        or visibility & ~3
        or not 0 < section_index < len(sections)
        or section_index >= 0xFF00
        or value != address
        or size <= 0
        or size % 4
        or value + size > 2**32
    ):
        raise ValueError(
            "function symbol has an unsupported type, definition, address or extent"
        )
    section = sections[section_index]
    relative = value - section.address
    if (
        section.kind != 1
        or section.flags & 6 != 6
        or section.flags & 0xC00
        or not names[section_index]
        or section.address % 4
        or section.offset % 4
        or relative < 0
        or relative + size > section.size
    ):
        raise ValueError(
            "function symbol is not contained in executable allocated file bytes"
        )
    for index, other in enumerate(sections):
        if (
            index != section_index
            and other.flags & 6
            and other.size
            and other.address < value + size
            and value < other.address + other.size
        ):
            raise ValueError(
                "function address overlaps another allocated or executable ELF section"
            )
        if other.kind in {4, 9} and other.size and other.info == section_index:
            raise ValueError("function section retains unsupported relocation records")
    selected = _read_range(content, section.offset + relative, size, "function")
    check_deadline()
    return FunctionImage(
        name=function_name,
        address=value,
        section=names[section_index],
        section_address=section.address,
        section_size=section.size,
        content=selected,
    )
