"""Deterministic PsyQ archive-to-target fingerprint correlation.

This deliberately reads the small, standard ar/ELF surface emitted by the
staged PsyQ SDK instead of treating a disassembler label as provenance.  A
raw byte match is useful evidence; a relocation-masked match remains a review
candidate and is never imported automatically.
"""

from __future__ import annotations

from collections import defaultdict
import hashlib
from pathlib import Path
import re
import struct
from typing import Any, Iterator

from ..domain import load_target_manifests
from .fingerprints import relocation_masked_hash


_AR_MAGIC = b"!<arch>\n"
_ELF_MAGIC = b"\x7fELF"
_SHT_SYMTAB = 2
_SHT_RELA = 4
_SHT_REL = 9
_STT_FUNC = 2
_STB_LOCAL = 0
_STB_GLOBAL = 1
_STB_WEAK = 2


def _sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _ar_members(path: Path) -> Iterator[tuple[str, bytes]]:
    """Yield normal and BSD long-name ar members without shelling out."""

    data = path.read_bytes()
    if not data.startswith(_AR_MAGIC):
        raise ValueError(f"not an ar archive: {path}")
    offset = len(_AR_MAGIC)
    string_table = b""
    while offset < len(data):
        header = data[offset : offset + 60]
        if len(header) != 60 or header[58:60] != b"`\n":
            raise ValueError(f"invalid ar member header in {path}")
        raw_name = header[:16].decode("ascii", errors="replace").rstrip()
        try:
            size = int(header[48:58].decode("ascii").strip())
        except ValueError as exc:
            raise ValueError(f"invalid ar member size in {path}") from exc
        body_start = offset + 60
        body_end = body_start + size
        body = data[body_start:body_end]
        if len(body) != size:
            raise ValueError(f"truncated ar member in {path}")
        offset = body_end + (size & 1)
        if raw_name == "//":
            string_table = body
            continue
        if raw_name in {"/", "/SYM64/"}:
            continue
        if raw_name.startswith("#1/"):
            name_size = int(raw_name[3:])
            name = body[:name_size].decode("utf-8", errors="replace")
            body = body[name_size:]
        elif raw_name.startswith("/") and raw_name[1:].isdigit():
            start = int(raw_name[1:])
            end = string_table.find(b"/\n", start)
            if end < 0:
                raise ValueError(f"invalid ar string-table name in {path}")
            name = string_table[start:end].decode("utf-8", errors="replace")
        else:
            name = raw_name.removesuffix("/")
        yield name, body


def _elf_functions(data: bytes, *, source: str) -> list[dict[str, Any]]:
    """Read ELF32 little-endian MIPS function bodies and relocation sites."""

    if len(data) < 52 or data[:4] != _ELF_MAGIC or data[4] != 1 or data[5] != 1:
        return []
    try:
        shoff = struct.unpack_from("<I", data, 0x20)[0]
        shentsize = struct.unpack_from("<H", data, 0x2E)[0]
        shnum = struct.unpack_from("<H", data, 0x30)[0]
        shstrndx = struct.unpack_from("<H", data, 0x32)[0]
    except struct.error as exc:
        raise ValueError(f"invalid ELF header: {source}") from exc
    if shentsize < 40 or shnum == 0 or shoff + shentsize * shnum > len(data):
        raise ValueError(f"invalid ELF section table: {source}")
    sections = [
        struct.unpack_from("<IIIIIIIIII", data, shoff + index * shentsize)
        for index in range(shnum)
    ]
    if shstrndx >= len(sections):
        raise ValueError(f"invalid ELF section-name table: {source}")
    string_section = sections[shstrndx]
    strings = data[string_section[4] : string_section[4] + string_section[5]]

    def string_at(table: bytes, offset: int) -> str:
        if offset >= len(table):
            return ""
        end = table.find(b"\0", offset)
        if end < 0:
            end = len(table)
        return table[offset:end].decode("utf-8", errors="replace")

    section_names = [string_at(strings, section[0]) for section in sections]
    relocations: dict[int, list[tuple[int, int]]] = defaultdict(list)
    for section in sections:
        (
            _name,
            section_type,
            _flags,
            _addr,
            body_offset,
            body_size,
            _link,
            info,
            _align,
            entry_size,
        ) = section
        if section_type not in {_SHT_REL, _SHT_RELA} or info >= len(sections):
            continue
        row_size = entry_size or (12 if section_type == _SHT_RELA else 8)
        if row_size < 8 or body_offset + body_size > len(data):
            continue
        for offset in range(body_offset, body_offset + body_size, row_size):
            if offset + 8 > len(data):
                break
            relocation_offset = struct.unpack_from("<I", data, offset)[0]
            # MIPS relocations target one encoded instruction word in these
            # archive members.  Masking exactly that word avoids pretending
            # the relocation-aware digest is an exact byte identity.
            relocations[info].append((relocation_offset, relocation_offset + 4))

    functions: list[dict[str, Any]] = []
    for section_index, section in enumerate(sections):
        (
            _name,
            section_type,
            _flags,
            _addr,
            body_offset,
            body_size,
            link,
            _info,
            _align,
            entry_size,
        ) = section
        if section_type != _SHT_SYMTAB or link >= len(sections):
            continue
        symbol_strings_section = sections[link]
        symbol_strings = data[
            symbol_strings_section[4] : symbol_strings_section[4]
            + symbol_strings_section[5]
        ]
        row_size = entry_size or 16
        if row_size < 16 or body_offset + body_size > len(data):
            continue
        symbols: list[dict[str, Any]] = []
        for offset in range(body_offset, body_offset + body_size, row_size):
            if offset + 16 > len(data):
                break
            name_offset, value, size, info, _other, section_ref = struct.unpack_from(
                "<IIIBBH", data, offset
            )
            symbol_type = info & 0x0F
            binding = info >> 4
            name = string_at(symbol_strings, name_offset)
            is_code = (
                section_ref < len(sections)
                and bool(sections[section_ref][2] & 0x4)  # SHF_EXECINSTR
            )
            # Converted PsyQ archives commonly preserve function symbols as
            # ELF STT_NOTYPE.  The executable section plus a real C spelling
            # is the conservative compatible representation; FILE/SECTION
            # pseudo-symbols and linker-local '$' labels are excluded.
            is_function = symbol_type == _STT_FUNC or (
                symbol_type == 0 and is_code and re.fullmatch(r"[A-Za-z_]\w*", name)
            )
            if not is_function or not name:
                continue
            symbols.append(
                {
                    "name": name,
                    "value": value,
                    "size": size,
                    "section": section_ref,
                    "visibility": (
                        "local"
                        if binding == _STB_LOCAL
                        else "weak"
                        if binding == _STB_WEAK
                        else "public"
                        if binding == _STB_GLOBAL
                        else "other"
                    ),
                }
            )
        by_section: dict[int, list[dict[str, Any]]] = defaultdict(list)
        for symbol in symbols:
            by_section[symbol["section"]].append(symbol)
        for function_section, rows in by_section.items():
            if function_section >= len(sections):
                continue
            target_section = sections[function_section]
            section_offset, section_size = target_section[4], target_section[5]
            if section_offset + section_size > len(data):
                continue
            rows.sort(key=lambda item: (item["value"], item["name"]))
            for index, row in enumerate(rows):
                next_value = (
                    rows[index + 1]["value"] if index + 1 < len(rows) else section_size
                )
                size = row["size"] or max(0, next_value - row["value"])
                if size < 8 or row["value"] + size > section_size:
                    continue
                payload = data[
                    section_offset + row["value"] : section_offset + row["value"] + size
                ]
                ranges = [
                    (max(0, start - row["value"]), min(size, end - row["value"]))
                    for start, end in relocations.get(function_section, [])
                    if start < row["value"] + size and end > row["value"]
                ]
                functions.append(
                    {
                        "name": row["name"],
                        "size": size,
                        "visibility": row["visibility"],
                        "section": section_names[function_section],
                        "raw_hash": _sha256(payload),
                        "relocation_hash": relocation_masked_hash(payload, ranges),
                        "payload": payload,
                        "relocations": ranges,
                    }
                )
    return functions


def _archive_functions(root: Path) -> list[dict[str, Any]]:
    libraries = sorted((root / "toolchains" / "psyq").glob("*/lib/*"))
    functions: list[dict[str, Any]] = []
    for archive in libraries:
        if not archive.is_file() or archive.suffix.lower() not in {".a", ".lib"}:
            continue
        version = archive.parents[1].name
        for member, body in _ar_members(archive):
            for function in _elf_functions(body, source=f"{archive}:{member}"):
                function.update(
                    {
                        "version": version,
                        "archive": archive.name,
                        "member": member,
                    }
                )
                functions.append(function)
    return functions


def _matches(payload: bytes, function: dict[str, Any]) -> Iterator[tuple[int, str]]:
    """Yield aligned exact and bounded relocation-aware offsets."""

    needle = function["payload"]
    start = 0
    while True:
        offset = payload.find(needle, start)
        if offset < 0:
            break
        if offset % 4 == 0:
            yield offset, "raw_exact"
        start = offset + 1
    if not function["relocations"]:
        return
    masked = bytearray(needle)
    for start, end in function["relocations"]:
        masked[start:end] = b"\0" * (end - start)
    # An unrelocated eight-byte anchor bounds candidate generation without
    # turning an archive scan into a quadratic full-image masked search.
    best_start = best_end = 0
    current_start: int | None = None
    for index, value in enumerate(masked + b"\0"):
        if value and current_start is None:
            current_start = index
        if (not value) and current_start is not None:
            if index - current_start > best_end - best_start:
                best_start, best_end = current_start, index
            current_start = None
    if best_end - best_start < 8:
        return
    anchor = bytes(masked[best_start:best_end])
    start = 0
    while True:
        anchor_offset = payload.find(anchor, start)
        if anchor_offset < 0:
            break
        offset = anchor_offset - best_start
        if (
            offset >= 0
            and offset % 4 == 0
            and offset + len(needle) <= len(payload)
            and payload[offset : offset + len(needle)] != needle
            and relocation_masked_hash(
                payload[offset : offset + len(needle)], function["relocations"]
            )
            == function["relocation_hash"]
        ):
            yield offset, "relocation_aware"
        start = anchor_offset + 1


def discover(root: Path, targets: list[str] | None = None) -> dict[str, Any]:
    """Return reproducible PsyQ evidence; it does not mutate maps or sources."""

    manifests = load_target_manifests(root)
    selected = set(targets or manifests)
    unknown = sorted(selected - set(manifests))
    if unknown:
        raise ValueError(f"unknown target: {unknown[0]}")
    sdk_functions = _archive_functions(root)
    results: list[dict[str, Any]] = []
    names_per_target: dict[tuple[str, str], int] = defaultdict(int)
    pending: list[tuple[dict[str, Any], str, int, str]] = []
    for target in sorted(selected):
        manifest = manifests[target]
        path = root / manifest.binary
        if not path.is_file():
            continue
        payload = path.read_bytes()
        for function in sdk_functions:
            for offset, kind in _matches(payload, function):
                address = manifest.load_address + offset
                pending.append((function, target, address, kind))
                names_per_target[(target, function["name"])] += 1
    for function, target, address, kind in pending:
        name = str(function["name"])
        if names_per_target[(target, name)] > 1:
            library = Path(str(function["archive"])).stem.removeprefix("lib")
            member = re.sub(r"[^A-Za-z0-9_]", "_", Path(str(function["member"])).stem)
            name = f"{library}_{member}_{name}"
        exact = kind == "raw_exact"
        results.append(
            {
                "target": target,
                "address": f"0x{address:08X}",
                "name": name,
                "original_name": function["name"],
                "size": function["size"],
                "version": function["version"],
                "archive": function["archive"],
                "member": function["member"],
                "visibility": function["visibility"],
                "raw_hash": function["raw_hash"],
                "relocation_hash": function["relocation_hash"],
                "match": kind,
                "confidence": "exact" if exact else "candidate",
                "external": True,
                "evidence": (
                    ["raw exact archive-member bytes"]
                    if exact
                    else [
                        "relocation-aware hash",
                        "requires reviewed size, prototype, and call shape",
                    ]
                ),
            }
        )
    # Tiny stubs are common in PsyQ.  Exact bytes alone cannot assign a
    # unique public identity when several SDK exports have that same body at
    # one target address, so retain them as evidence but require review.
    names_at_address: dict[tuple[str, str], set[str]] = defaultdict(set)
    for row in results:
        names_at_address[(str(row["target"]), str(row["address"]))].add(
            str(row["original_name"])
        )
    for row in results:
        names = names_at_address[(str(row["target"]), str(row["address"]))]
        if len(names) > 1:
            row["confidence"] = "candidate"
            row["evidence"] = [
                "raw exact archive-member bytes",
                "ambiguous identical SDK stub; review identity before import",
            ]
    results.sort(
        key=lambda row: (
            row["target"],
            int(str(row["address"]), 16),
            row["name"],
            row["version"],
        )
    )
    return {
        "schema": "bof3.psyq-find/v1",
        "archives": len({(row["version"], row["archive"]) for row in results}),
        "functions_scanned": len(sdk_functions),
        "targets_scanned": sorted(selected),
        "matches": results,
    }
