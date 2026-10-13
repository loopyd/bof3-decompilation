"""Bounded inspection of the returned ELF32 MIPS relocatable object.

Task 2.2 must report the symbols of the object it actually received, so this
module parses that object directly instead of sourcing names from the request or
the candidate source.  ``harness.match.extraction.read_function_image`` cannot
be reused: it requires a linked ``ET_EXEC`` image and explicitly rejects
relocatable objects, and weakening its verifier predicates is not permitted.

Every table read is bounded by the declaring section's own extent, not merely by
the object size: section headers, the symbol table and the linked string table
each carry their declared size and type, the symbol entry size must be exact,
every emitted name must lie inside its declared string table, and the requested
entry must be a unique defined ``FUNC`` symbol in a file-backed executable
section. Complete extraction retains every declared relocation without resolving
it or using an expected original length. Placement, semantic admission and
independent original comparison remain separate obligations.
"""

from __future__ import annotations

import hashlib
import struct
from dataclasses import dataclass, fields

from harness.reconstruction import contract
from harness.common.deadlines import check_deadline

ELF_MAGIC = b"\x7fELF"
ELFCLASS32 = 1
ELFDATA2LSB = 1
ET_REL = 1
EM_MIPS = 8
SHT_SYMTAB = 2
SHT_STRTAB = 3
SHT_RELA = 4
SHT_NOBITS = 8
SHT_REL = 9
SHT_RELR = 19
UNSUPPORTED_RELOCATION_TABLES = frozenset({SHT_RELR, 0x60000001, 0x60000002})
R_MIPS_32 = 2
SHF_EXECINSTR = 0x4
SHF_ALLOC = 0x2
SHN_LORESERVE = 0xFF00
SECTION_HEADER_BYTES = 40
SYMBOL_BYTES = 16
STB_GLOBAL = 1
STB_WEAK = 2
STT_NOTYPE = 0
STT_FUNC = 2
STT_SECTION = 3
STT_FILE = 4
MAX_SECTIONS = 4096
MAX_SYMBOLS = 65536
MAX_RELOCATIONS = 65536
#: One decoded symbol or section name, and the aggregate budget for all of them.
MAX_NAME_BYTES = 4096
MAX_NAME_BYTES_TOTAL = 4 << 20
SYMBOL_TYPES = {
    STT_NOTYPE: "notype",
    1: "object",
    STT_FUNC: "func",
    STT_SECTION: "section",
    STT_FILE: "file",
}


class ObjectRefused(RuntimeError):
    """The returned bytes are not a reviewed relocatable object."""


@dataclass(frozen=True)
class ObjectInspection:
    """What the received object actually declares about its entry symbol."""

    machine: int
    object_type: int
    sections: int
    symbols: int
    emitted_symbols: tuple[str, ...]
    entry_defined: bool
    entry_section: str | None
    entry_size: int
    entry_type: str | None


@dataclass(frozen=True)
class Relocation:
    """One actual relocation, including unresolved symbol and target-section identity."""

    section_index: int
    target_section: int
    offset: int
    symbol_index: int
    symbol_name: str | None
    symbol_section: int
    kind: int
    addend: int | None
    storage_bytes: int | None


@dataclass(frozen=True)
class FunctionImage:
    """Complete actual symbol bytes, with whole-object relocation inventory.

    ``value`` is section-relative for this relocatable object, never an original
    runtime address. This record asserts neither placement nor compatibility.
    """

    symbol: str
    object_sha256: str
    section_index: int
    section_name: str
    value: int
    file_offset: int
    data: bytes
    sha256: str
    relocations: tuple[Relocation, ...]


@dataclass(frozen=True)
class ObjectSection:
    """Actual section metadata; allocation flags do not prove runtime placement."""

    index: int
    kind: int
    flags: int
    size: int
    file_offset: int | None
    alignment: int


@dataclass(frozen=True)
class ObjectSymbol:
    index: int
    name: str | None
    value: int
    size: int
    binding: int
    type: int
    visibility: int
    section: int


@dataclass(frozen=True)
class ObjectStorage:
    index: int
    name: str
    kind: int
    flags: int
    address: int
    size: int
    alignment: int
    file_offset: int | None
    content: bytes | None


@dataclass(frozen=True)
class ObjectContainerLayout:
    elf_header: tuple[int, int]
    section_table: tuple[int, int]
    program_table: tuple[int, int]
    name_section: int
    flags: int
    entry: int
    sections: tuple[tuple[int, ...], ...]
    padding: tuple[tuple[int, int, bool], ...]


@dataclass(frozen=True)
class ObjectImage:
    sha256: str
    sections: tuple[ObjectStorage, ...]
    symbols: tuple[ObjectSymbol, ...]
    relocations: tuple[Relocation, ...]
    layout: ObjectContainerLayout | None = None


def _require(condition: object, message: str) -> None:
    if not condition:
        raise ObjectRefused(message)


def _text(blob: bytes, offset: int, limit: int, subject: str) -> str:
    """One bounded name read: never past the table, never past the name bound."""

    _require(0 <= offset < limit, f"{subject} escapes its string table")
    end = blob.find(b"\0", offset, min(limit, offset + MAX_NAME_BYTES))
    _require(end >= 0, f"{subject} is unterminated or exceeds the name bound")
    return blob[offset:end].decode("utf-8", "surrogateescape")


def _name_reader(data: bytes, strings: dict):
    """A cached, aggregate-bounded resolver for string-table names."""

    cache: dict[int, str] = {}
    spent = [0]

    def resolve(offset: int, subject: str) -> str:
        if offset in cache:
            return cache[offset]
        text = _text(
            data,
            strings["offset"] + offset,
            strings["offset"] + strings["size"],
            subject,
        )
        spent[0] += len(text)
        _require(
            spent[0] <= MAX_NAME_BYTES_TOTAL,
            "aggregate symbol names exceed the parser budget",
        )
        cache[offset] = text
        return text

    return resolve


def _sections(data: bytes) -> list[dict]:
    (offset,) = struct.unpack_from("<I", data, 0x20)
    (entry_bytes,) = struct.unpack_from("<H", data, 0x2E)
    (count,) = struct.unpack_from("<H", data, 0x30)
    _require(entry_bytes == SECTION_HEADER_BYTES, "section header size is not 40 bytes")
    _require(0 < count <= MAX_SECTIONS, f"section count is out of range: {count}")
    _require(
        offset + count * SECTION_HEADER_BYTES <= len(data),
        "section header table escapes the object",
    )
    rows = []
    for index in range(count):
        base = offset + index * SECTION_HEADER_BYTES
        name, kind, flags, address, body, size, link, info, alignment, entry = (
            struct.unpack_from("<IIIIIIIIII", data, base)
        )
        if kind != SHT_NOBITS:
            _require(
                body + size <= len(data),
                f"section {index} escapes the object",
            )
        rows.append(
            {
                "name": name,
                "type": kind,
                "flags": flags,
                "address": address,
                "offset": body,
                "size": size,
                "link": link,
                "info": info,
                "alignment": alignment,
                "entry": entry,
            }
        )
    return rows


def _symbols(data: bytes, sections: list[dict], table: dict) -> list[dict]:
    _require(
        table["entry"] == SYMBOL_BYTES,
        "symbol table entry size is not 16 bytes",
    )
    _require(
        table["size"] % SYMBOL_BYTES == 0,
        "symbol table size is not a whole number of entries",
    )
    _require(0 < table["link"] < len(sections), "symbol table string table is invalid")
    strings = sections[table["link"]]
    _require(strings["type"] == SHT_STRTAB, "symbol table does not link a string table")
    count = table["size"] // SYMBOL_BYTES
    _require(count <= MAX_SYMBOLS, f"symbol count is out of range: {count}")
    resolve = _name_reader(data, strings)
    rows = []
    for index in range(count):
        base = table["offset"] + index * SYMBOL_BYTES
        name, value, size, info, visibility, section = struct.unpack_from(
            "<IIIBBH", data, base
        )
        text = resolve(name, f"symbol {index} name") if name else None
        rows.append(
            {
                "index": index,
                "name": text,
                "value": value,
                "size": size,
                "binding": info >> 4,
                "type": info & 0xF,
                "section": section,
                "visibility": visibility,
            }
        )
    return rows


def inspect_relocatable_object(
    data: bytes, entry_symbol: str, *, limit: int = 0
) -> ObjectInspection:
    """Parse one ELF32 MIPS relocatable object and locate its entry symbol."""

    _require(limit == 0 or len(data) <= limit, "object exceeds the transfer bound")
    _require(len(data) >= 52, "object is shorter than an ELF32 header")
    _require(data[:4] == ELF_MAGIC, "object is not an ELF file")
    _require(data[4] == ELFCLASS32, "object is not ELF32")
    _require(data[5] == ELFDATA2LSB, "object is not little-endian")
    (object_type,) = struct.unpack_from("<H", data, 16)
    (machine,) = struct.unpack_from("<H", data, 18)
    _require(object_type == ET_REL, f"object is not relocatable: {object_type}")
    _require(machine == EM_MIPS, f"object is not MIPS: {machine}")
    sections = _sections(data)
    tables = [row for row in sections if row["type"] == SHT_SYMTAB]
    _require(len(tables) == 1, "object does not declare exactly one symbol table")
    symbols = _symbols(data, sections, tables[0])
    names: set[str] = set()
    matches: list[dict] = []
    for row in symbols:
        section = row["section"]
        defined = 0 < section < len(sections)
        if (
            row["name"]
            and defined
            and row["binding"] in {STB_GLOBAL, STB_WEAK}
            and contract.TOKEN.fullmatch(row["name"])
        ):
            names.add(row["name"])
        if row["name"] == entry_symbol:
            matches.append(row)
    _require(
        len(matches) == 1,
        f"entry symbol must be declared exactly once: {entry_symbol} "
        f"({len(matches)} matches)",
    )
    entry = matches[0]
    _require(
        0 < entry["section"] < len(sections),
        f"entry symbol is not defined in a section: {entry_symbol}",
    )
    section = sections[entry["section"]]
    _require(
        entry["type"] == STT_FUNC,
        f"entry symbol is not a function: {entry_symbol}",
    )
    _require(entry["size"] > 0, f"entry symbol has no size: {entry_symbol}")
    _require(
        section["type"] != SHT_NOBITS and section["flags"] & SHF_EXECINSTR,
        f"entry symbol is not in a file-backed executable section: {entry_symbol}",
    )
    _require(
        entry["value"] + entry["size"] <= section["size"],
        f"entry symbol is not contained in its section: {entry_symbol}",
    )
    (shstrndx,) = struct.unpack_from("<H", data, 0x32)
    _require(shstrndx < len(sections), "section name table index is out of range")
    header = sections[shstrndx]
    _require(header["type"] == SHT_STRTAB, "section name table is not a string table")
    section_name = _text(
        data,
        header["offset"] + section["name"],
        header["offset"] + header["size"],
        "entry section name",
    )
    return ObjectInspection(
        machine=machine,
        object_type=object_type,
        sections=len(sections),
        symbols=len(symbols),
        emitted_symbols=tuple(sorted(names)),
        entry_defined=True,
        entry_section=section_name,
        entry_size=int(entry["size"]),
        entry_type=SYMBOL_TYPES.get(entry["type"]),
    )


def _relocations(data: bytes, sections: list[dict], symbols: list[dict]) -> tuple:
    records = []
    symbol_index = next(
        index for index, row in enumerate(sections) if row["type"] == SHT_SYMTAB
    )
    for index, table in enumerate(sections):
        _require(
            table["type"] not in UNSUPPORTED_RELOCATION_TABLES,
            "unsupported packed relocation table format",
        )
        if table["type"] not in {SHT_REL, SHT_RELA}:
            continue
        width = 8 if table["type"] == SHT_REL else 12
        _require(table["entry"] == width, "relocation entry size is invalid")
        _require(table["size"] % width == 0, "relocation table has a partial entry")
        _require(table["link"] == symbol_index, "relocation symbol table is invalid")
        target = table["info"]
        _require(0 < target < len(sections), "relocation target section is invalid")
        count = table["size"] // width
        _require(
            len(records) + count <= MAX_RELOCATIONS,
            "relocation inventory exceeds the parser budget",
        )
        for position in range(count):
            base = table["offset"] + position * width
            offset, info = struct.unpack_from("<II", data, base)
            reference = info >> 8
            _require(reference < len(symbols), "relocation symbol index is invalid")
            _require(
                offset < sections[target]["size"],
                "relocation offset escapes its target section",
            )
            kind = info & 0xFF
            storage_bytes = 4 if kind == R_MIPS_32 else None
            if storage_bytes is not None:
                _require(
                    offset + storage_bytes <= sections[target]["size"],
                    "known relocation storage escapes its target section",
                )
            addend = (
                struct.unpack_from("<i", data, base + 8)[0]
                if table["type"] == SHT_RELA
                else None
            )
            symbol = symbols[reference]
            records.append(
                Relocation(
                    section_index=index,
                    target_section=target,
                    offset=offset,
                    symbol_index=reference,
                    symbol_name=symbol["name"],
                    symbol_section=symbol["section"],
                    kind=kind,
                    addend=addend,
                    storage_bytes=storage_bytes,
                )
            )
    return tuple(records)


def extract_function(data: bytes, entry_symbol: str, *, limit: int) -> FunctionImage:
    """Extract a whole actual ELF symbol with bounded relocation inventory.

    The caller supplies an artifact-size ceiling, never a desired function size.
    All symbol and table predicates of the existing object inspector apply.
    Unknown relocation storage spans stay ``None``, explicitly unvalidated;
    no relocation is treated as resolved. Unsupported packed tables refuse.
    """

    _require(type(data) is bytes, "object input must be immutable bytes")
    _require(type(limit) is int and limit > 0, "object transfer bound must be positive")
    inspection = inspect_relocatable_object(data, entry_symbol, limit=limit)
    sections = _sections(data)
    table = next(row for row in sections if row["type"] == SHT_SYMTAB)
    symbols = _symbols(data, sections, table)
    entry = next(row for row in symbols if row["name"] == entry_symbol)
    _require(
        entry["value"] % 4 == 0 and entry["size"] % 4 == 0,
        "MIPS function extent is not word aligned",
    )
    section = sections[entry["section"]]
    start = section["offset"] + entry["value"]
    body = data[start : start + entry["size"]]
    _require(len(body) == entry["size"], "actual function extent is incomplete")
    return FunctionImage(
        symbol=entry_symbol,
        object_sha256=hashlib.sha256(data).hexdigest(),
        section_index=entry["section"],
        section_name=inspection.entry_section,
        value=entry["value"],
        file_offset=start,
        data=body,
        sha256=hashlib.sha256(body).hexdigest(),
        relocations=_relocations(data, sections, symbols),
    )


def verify_extraction(
    record: FunctionImage, data: bytes, entry_symbol: str, *, limit: int
) -> FunctionImage:
    """Re-derive complete extraction from raw ELF and reject changed copied records.

    No claimed extent or digest controls the independent extraction. Equality
    preserves exact field types, including booleans versus numeric identities.
    This checks correspondence to the object, never original-byte acceptance.
    """

    _require(type(record) is FunctionImage, "extraction must be a FunctionImage")
    expected = extract_function(data, entry_symbol, limit=limit)
    for field in fields(FunctionImage):
        actual_value = getattr(record, field.name)
        expected_value = getattr(expected, field.name)
        _require(
            type(actual_value) is type(expected_value),
            f"extraction field type differs from the actual object: {field.name}",
        )
        if field.name == "relocations":
            _require(
                len(actual_value) == len(expected_value),
                "extraction relocation count differs from the actual object",
            )
            for actual_row, expected_row in zip(actual_value, expected_value):
                _require(
                    type(actual_row) is Relocation, "relocation record type is invalid"
                )
                for column in fields(Relocation):
                    left = getattr(actual_row, column.name)
                    right = getattr(expected_row, column.name)
                    _require(
                        type(left) is type(right) and left == right,
                        "extraction relocation differs from the actual object",
                    )
        else:
            _require(
                actual_value == expected_value,
                f"extraction field differs from the actual object: {field.name}",
            )
    return expected


def inspect_sections(
    data: bytes, entry_symbol: str, *, limit: int
) -> tuple[ObjectSection, ...]:
    """Expose bounded ELF sections after validating the actual object and entry.

    This supports relocation target classification. No section name, allocation
    flag or file extent supplies an original runtime address or exclusion proof.
    """

    _require(type(data) is bytes, "object input must be immutable bytes")
    _require(type(limit) is int and limit > 0, "object transfer bound must be positive")
    inspect_relocatable_object(data, entry_symbol, limit=limit)
    return tuple(
        ObjectSection(
            index=index,
            kind=section["type"],
            flags=section["flags"],
            size=section["size"],
            file_offset=section["offset"] if section["type"] != SHT_NOBITS else None,
            alignment=section["alignment"],
        )
        for index, section in enumerate(_sections(data))
    )


def read_object_image(data: bytes, entry_symbol: str, *, limit: int) -> ObjectImage:
    _require(
        type(data) is bytes and type(limit) is int and limit > 0,
        "object requires immutable bytes and a positive bound",
    )
    inspect_relocatable_object(data, entry_symbol, limit=limit)
    _require(
        data[6] == 1 and struct.unpack_from("<I", data, 20)[0] == 1,
        "unsupported ELF object version",
    )
    _require(
        struct.unpack_from("<H", data, 40)[0] == 52,
        "unsupported ELF object header size",
    )
    sections = _sections(data)
    _require(
        all(row["type"] != 11 for row in sections),
        "associated object census does not support SHT_DYNSYM",
    )
    _require(not any(sections[0].values()), "unsupported ELF null section")
    table_offset = struct.unpack_from("<I", data, 32)[0]
    _require(
        table_offset >= 52 and table_offset % 4 == 0,
        "invalid object section table offset",
    )
    names_index = struct.unpack_from("<H", data, 50)[0]
    _require(0 < names_index < len(sections), "invalid section-name table")
    strings = sections[names_index]
    _require(strings["type"] == SHT_STRTAB, "invalid section-name string storage")
    resolve = _name_reader(data, strings)
    storage = []
    for index, row in enumerate(sections):
        check_deadline()
        alignment = row["alignment"]
        _require(
            not alignment or alignment & (alignment - 1) == 0,
            "invalid object section alignment",
        )
        _require(
            row["address"] + row["size"] <= 2**32,
            "object section exceeds the address space",
        )
        if row["type"] not in {0, SHT_NOBITS} and row["size"]:
            _require(
                row["offset"] >= 52
                and not (
                    row["offset"] < table_offset + len(sections) * SECTION_HEADER_BYTES
                    and table_offset < row["offset"] + row["size"]
                ),
                "object section payload overlaps header metadata",
            )
            _require(
                not alignment or row["offset"] % alignment == 0,
                "object section payload violates declared alignment",
            )
        storage.append(
            ObjectStorage(
                index,
                resolve(row["name"], "section name"),
                row["type"],
                row["flags"],
                row["address"],
                row["size"],
                alignment,
                None if row["type"] == SHT_NOBITS else row["offset"],
                None
                if row["type"] == SHT_NOBITS
                else data[row["offset"] : row["offset"] + row["size"]],
            )
        )
    table = next(row for row in sections if row["type"] == SHT_SYMTAB)
    _require(
        table["size"] >= SYMBOL_BYTES
        and not any(data[table["offset"] : table["offset"] + SYMBOL_BYTES]),
        "object reserved symbol is not zero",
    )
    symbols = _symbols(data, sections, table)
    for symbol in symbols:
        check_deadline()
        section = symbol["section"]
        _require(
            section < len(sections) or section in {0xFFF1, 0xFFF2},
            "unsupported object symbol section",
        )
        if 0 < section < len(sections):
            _require(
                symbol["value"] + symbol["size"] <= sections[section]["size"],
                "object symbol exceeds its section",
            )
    physical_ranges = [
        (0, 52),
        (table_offset, table_offset + len(sections) * SECTION_HEADER_BYTES),
    ]
    program_offset = struct.unpack_from("<I", data, 28)[0]
    program_size = (
        struct.unpack_from("<H", data, 42)[0] * struct.unpack_from("<H", data, 44)[0]
    )
    _require(
        not program_size
        or 52 <= program_offset <= program_offset + program_size <= len(data),
        "object program metadata exceeds physical file",
    )
    if program_size:
        physical_ranges.append((program_offset, program_offset + program_size))
    physical_ranges.extend(
        (row["offset"], row["offset"] + row["size"])
        for row in sections
        if row["type"] not in {0, SHT_NOBITS} and row["size"]
    )
    padding, last = [], 0
    for first, end in sorted(physical_ranges):
        if first > last:
            padding.append((last, first - last, not any(data[last:first])))
        last = max(last, end)
    if last < len(data):
        padding.append((last, len(data) - last, not any(data[last:])))
    return ObjectImage(
        hashlib.sha256(data).hexdigest(),
        tuple(storage),
        tuple(ObjectSymbol(**row) for row in symbols),
        _relocations(data, sections, symbols),
        ObjectContainerLayout(
            (0, 52),
            (table_offset, len(sections) * SECTION_HEADER_BYTES),
            (
                struct.unpack_from("<I", data, 28)[0],
                struct.unpack_from("<H", data, 42)[0]
                * struct.unpack_from("<H", data, 44)[0],
            ),
            names_index,
            struct.unpack_from("<I", data, 36)[0],
            struct.unpack_from("<I", data, 24)[0],
            tuple(
                tuple(
                    row[name]
                    for name in (
                        "name",
                        "type",
                        "flags",
                        "address",
                        "offset",
                        "size",
                        "link",
                        "info",
                        "alignment",
                        "entry",
                    )
                )
                for row in sections
            ),
            tuple(padding),
        ),
    )
