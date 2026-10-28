"""Capture complete original function ranges and target bindings from pinned inputs."""

from __future__ import annotations

from pathlib import Path

from harness.common.inputs import InputBatch
from harness.common.paths import leaf_stat
from harness.domain.claims import collect_manifest_source_addresses
from harness.domain.layout import parse_splat_text
from harness.domain.manifests import _parse_manifest
from harness.domain.psx import is_psx_exe, payload_for, validate_psx_header
from harness.domain.symbols import load_target_symbols, parse_map
from harness.domain.functions import parse_function_records
from harness.domain.sources import owning_manifest, compiled_symbol_name

_LIMIT = 64 * 1024 * 1024


def _capture_data(manifest, payload, binary: bytes, functions: list[dict]) -> dict:
    sections = {}
    for member in functions:
        for placement in manifest.section_placements.get(member["address"], ()):
            if not any(
                placement.section == name or placement.section.startswith(name + ".")
                for name in (".data", ".rodata", ".rdata", ".sdata")
            ):
                raise ValueError("grouped data requires a reviewed initialized section")
            address, size = placement.address, placement.size
            if (
                not payload.load_address
                <= address
                < address + size
                <= payload.payload_end
            ):
                raise ValueError(
                    "grouped initialized data exceeds the original payload"
                )
            first = payload.binary_offset + address - payload.load_address
            section = {
                "section": placement.section,
                "address": address,
                "file_start": first,
                "original": binary[first : first + size],
            }
            previous = sections.get(placement.section)
            if previous is not None and any(
                previous[key] != value for key, value in section.items()
            ):
                raise ValueError("grouped members disagree on shared data placement")
            if previous is None:
                sections[placement.section] = {**section, "owners": []}
            sections[placement.section]["owners"].append(member["selector"])
    ranges = [
        (
            member["address"],
            member["address"] + len(member["original"]),
            member["symbol"],
        )
        for member in functions
    ] + [
        (section["address"], section["address"] + len(section["original"]), name)
        for name, section in sections.items()
    ]
    ranges.sort()
    for previous, following in zip(ranges, ranges[1:]):
        if previous[1] > following[0]:
            raise ValueError("grouped original function or data ranges overlap")
    return sections


def capture_members(root: Path, source: str) -> dict:
    """Read complete current members from explicit ownership, metadata and Splat."""
    owner = owning_manifest(root, root / source)
    if owner is None or source not in owner.sources + owner.support_sources:
        raise ValueError("grouped source requires one explicit lift owner")
    inputs = {}
    with InputBatch(root) as batch:

        def read(name: str, *, required: bool = True) -> bytes:
            before = leaf_stat(root, name)
            if before is not None and before.st_nlink != 1:
                raise ValueError(f"comparison input has multiple links: {name}")
            state, content = batch.read(root / name, max_bytes=_LIMIT)
            after = leaf_stat(root, name)
            if after is not None and after.st_nlink != 1:
                raise ValueError(f"comparison input has multiple links: {name}")
            if required and content is None:
                raise ValueError(f"comparison input is absent: {name}")
            inputs[name] = state
            return content if content is not None else b""

        manifest_name = f"config/targets/{owner.id.value}/target.toml"
        manifest = _parse_manifest(root / manifest_name, read(manifest_name))
        if manifest != owner:
            raise ValueError("grouped manifest changed during member discovery")
        records = parse_function_records(read(source).decode("utf-8"))
        binary = read(manifest.binary)
        if is_psx_exe(binary):
            validate_psx_header(
                binary, manifest.load_address, binary_name=manifest.binary
            )
        payload = payload_for(
            binary, manifest.load_address, binary_name=manifest.binary
        )
        if payload.payload_end > 0x100000000:
            raise ValueError("comparison target payload overflows 32-bit addresses")
        layout = parse_splat_text(
            read(manifest.splat).decode("utf-8"),
            manifest.load_address,
            origin=root / manifest.splat,
        )
        local_name = f"config/targets/{manifest.id.value}/symbols.txt"
        local = parse_map(read(local_name).decode("utf-8"), source=local_name)
        symbols = load_target_symbols(
            root,
            manifest.id.value,
            psyq_space=manifest.psyq_space,
            read_file=lambda path: read(
                path.relative_to(root).as_posix(),
                required=path != root / "config/targets/shared/symbols.txt",
            ),
        )
        local_by_name = {symbol.canonical_name: symbol.address for symbol in local}
        functions = []
        singleton_claims = set()
        selectors, names = set(), set()
        for record in records:
            address = record.address
            selector = f"{manifest.id.value}@0x{address:08X}"
            symbol = compiled_symbol_name(
                root,
                root / source,
                address=address,
                manifest=manifest,
                layout=layout,
                symbols=symbols,
            )
            if (
                selector in selectors
                or symbol in names
                or local_by_name.get(symbol) != address
                or record.kind == "function"
                and symbol != record.spelling
            ):
                raise ValueError("comparison member identity is duplicate or unbound")
            boundaries = [
                boundary
                for boundary in layout.boundaries
                if boundary.virtual_start == address
            ]
            if len(boundaries) != 1:
                raise ValueError(
                    "comparison member requires one exact reviewed boundary"
                )
            boundary = boundaries[0]
            singleton_source = (
                boundary.source is None
                and len(records) == 1
                and record.kind == "function"
                and boundary.name == symbol
                and [
                    path
                    for path, claimed_address in collect_manifest_source_addresses(
                        root,
                        manifest,
                        read_source=lambda path: read(
                            path.relative_to(root).as_posix()
                        ).decode("utf-8"),
                    )
                    if claimed_address == address
                ]
                == [root / source]
            )
            if singleton_source:
                singleton_claims.add(address)
            size = boundary.virtual_size
            if (
                boundary.kind != "c"
                or boundary.source != source
                and not singleton_source
                or size is None
                or size < 8
                or size % 4
                or address % 4
                or boundary.file_size != size
                or not payload.load_address
                <= address
                < address + size
                <= payload.payload_end
                or boundary.file_start
                != payload.binary_offset + address - payload.load_address
                or boundary.file_end != boundary.file_start + size
            ):
                raise ValueError("comparison member has an invalid original byte range")
            original = binary[boundary.file_start : boundary.file_end]
            if len(original) != size:
                raise ValueError("comparison original bytes are truncated")
            selectors.add(selector)
            names.add(symbol)
            functions.append(
                {
                    "selector": selector,
                    "symbol": symbol,
                    "address": address,
                    "file_start": boundary.file_start,
                    "original": original,
                }
            )
        claimed = singleton_claims | {
            boundary.virtual_start
            for boundary in layout.boundaries
            if boundary.kind == "c" and boundary.source == source
        }
        if not functions or claimed != {member["address"] for member in functions}:
            raise ValueError("comparison must cover every reviewed source group member")
        data_sections = _capture_data(manifest, payload, binary, functions)
    return {
        "inputs": inputs,
        "target": manifest.id.value,
        "binary": manifest.binary,
        "load_address": manifest.load_address,
        "section_placements": {
            member["address"]: manifest.section_placements.get(member["address"], ())
            for member in functions
        },
        "bindings": {symbol.canonical_name: symbol.address for symbol in symbols},
        "data_sections": data_sections,
        "functions": sorted(functions, key=lambda member: member["selector"]),
    }
