from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from ..domain.registry import SOURCE_TAG_RE
from ..io import read_json, RepoLayout

IMPLAUSIBLE_SIBLING_FUNCTION_SIZE = 0x1000
MIPS_JR_RA = 0x03E00008
PSX_EXE_MAGIC = b"PS-X EXE"
PSX_EXE_HEADER_SIZE = 0x800
# Centralized in domain.registry; alias retained for existing importers.
SOURCE_ADDRESS_RE = SOURCE_TAG_RE
FUNC_NAME_RE = re.compile(r"func_([0-9a-fA-F]{8})")


@dataclass(frozen=True)
class PsxExeInfo:
    load_address: int
    payload_size: int
    payload_offset: int = PSX_EXE_HEADER_SIZE


def parse_int(value: str | int) -> int:
    if isinstance(value, int):
        return value
    raw = value.strip()
    try:
        return int(raw, 0)
    except ValueError:
        return int(raw, 16)


def format_hex(value: int) -> str:
    return f"0x{value:08x}"


def read_u32le(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], byteorder="little")


def read_psx_exe_info(path: Path) -> PsxExeInfo | None:
    header = path.read_bytes()[:PSX_EXE_HEADER_SIZE]
    if not header.startswith(PSX_EXE_MAGIC):
        return None
    return PsxExeInfo(
        load_address=read_u32le(header, 0x18),
        payload_size=read_u32le(header, 0x1C),
    )


def parse_source_address(source_path: Path) -> int:
    text = source_path.read_text(encoding="utf-8")
    match = SOURCE_ADDRESS_RE.search(text)
    if match is not None:
        return int(match.group(1), 16)
    match = FUNC_NAME_RE.search(source_path.stem)
    if match is not None:
        return int(match.group(1), 16)
    raise ValueError(
        f"cannot infer original address from {source_path}; add @source or pass --address"
    )


def collect_source_addresses(source_dir: Path) -> list[tuple[Path, int]]:
    rows: list[tuple[Path, int]] = []
    for source_path in sorted(source_dir.glob("*.c")):
        try:
            rows.append((source_path, parse_source_address(source_path)))
        except ValueError:
            continue
    return sorted(rows, key=lambda row: (row[1], row[0].name))


def infer_size_from_sibling_sources(source_path: Path, address: int) -> int | None:
    for candidate_path, candidate_address in collect_source_addresses(
        source_path.parent
    ):
        if candidate_path == source_path:
            continue
        if candidate_address > address:
            size = candidate_address - address
            if size <= IMPLAUSIBLE_SIBLING_FUNCTION_SIZE:
                return size
            break
    return None


def infer_size_from_binary_return(
    binary_path: Path,
    *,
    address: int,
    load_address: int | None,
) -> int:
    binary_data = binary_path.read_bytes()
    psx_info = read_psx_exe_info(binary_path)
    if psx_info is None:
        if load_address is None:
            raise ValueError(
                f"{binary_path} is not a PS-X EXE; pass --load-address for raw binaries"
            )
        payload_offset = 0
        payload_load_address = load_address
        payload_size = len(binary_data)
    else:
        payload_offset = psx_info.payload_offset
        payload_load_address = psx_info.load_address
        payload_size = psx_info.payload_size

    relative_offset = address - payload_load_address
    if relative_offset < 0 or relative_offset >= payload_size:
        raise ValueError(
            f"{format_hex(address)} is outside {binary_path} loaded at {format_hex(payload_load_address)}"
        )

    for offset in range(
        payload_offset + relative_offset, payload_offset + payload_size, 4
    ):
        if read_u32le(binary_data, offset) == MIPS_JR_RA:
            return offset - payload_offset - relative_offset + 8
    raise ValueError(
        f"cannot infer original size for {format_hex(address)} from {binary_path}; pass --size"
    )


def infer_original_size(
    source_path: Path,
    *,
    address: int,
    binary_path: Path,
    load_address: int | None,
) -> int:
    try:
        return infer_size_from_binary_return(
            binary_path, address=address, load_address=load_address
        )
    except ValueError:
        sibling_size = infer_size_from_sibling_sources(source_path, address)
        if sibling_size is not None:
            return sibling_size
        raise


def source_function_name(source_path: Path, address: int) -> str:
    match = FUNC_NAME_RE.search(source_path.stem)
    if match is not None:
        return f"func_{match.group(1).upper()}"
    return f"func_{address:08X}"


def _source_relative_path(layout: RepoLayout, source_path: Path) -> Path:
    resolved_root = layout.root.expanduser().resolve()
    expanded_source = source_path.expanduser()
    resolved_source = (
        expanded_source.resolve()
        if expanded_source.is_absolute()
        else (resolved_root / expanded_source).resolve()
    )
    try:
        return resolved_source.relative_to(resolved_root)
    except ValueError as exc:
        raise ValueError(
            f"source path {source_path} is outside repository {resolved_root}"
        ) from exc


def object_path_for_source(layout: RepoLayout, source_path: Path) -> Path:
    source_rel = _source_relative_path(layout, source_path)
    return layout.build_dir / source_rel.with_suffix(".o")


def compiler_asm_path_for_object(object_path: Path) -> Path:
    return object_path.with_name(object_path.name + ".s")


def build_target_for_source(layout: RepoLayout, source_path: Path) -> str:
    object_path = object_path_for_source(layout, source_path)
    return object_path.relative_to(layout.root.resolve()).as_posix()


def default_binary_for_source(layout: RepoLayout, source_path: Path) -> Path:
    resolved_source = source_path.expanduser().resolve()
    try:
        source_rel = resolved_source.relative_to(layout.root).as_posix()
    except ValueError:
        source_rel = ""
    from ..domain.manifests import load_target_manifests

    source_dir = str(Path(source_rel).parent)
    for manifest in load_target_manifests(layout.root).values():
        if manifest.source_dir == source_dir:
            return layout.root / manifest.binary

    raise ValueError(f"cannot resolve original binary for overlay source: {source_rel}")


def overlay_load_address_for_source(
    layout: RepoLayout, source_path: Path
) -> int | None:
    resolved_source = source_path.expanduser().resolve()
    try:
        source_rel = resolved_source.relative_to(layout.root).as_posix()
    except ValueError:
        return None

    from ..domain.manifests import load_target_manifests

    source_dir = str(Path(source_rel).parent)
    for manifest in load_target_manifests(layout.root).values():
        if manifest.source_dir == source_dir:
            return (
                _catalog_load_address(layout, manifest.disc_id) or manifest.load_address
            )

    return None


def _catalog_load_address(layout: RepoLayout, disc_id: str) -> int | None:
    """Return the payload base, which may precede a target's first function."""
    catalog_path = layout.root / "out" / "catalog" / "emi.json"
    if not catalog_path.is_file():
        return None
    from ..emi.catalog_verify import resolve_entry

    try:
        return int(resolve_entry(read_json(catalog_path), disc_id)["load_address"])
    except (KeyError, ValueError):
        return None


def extract_original_bytes(
    binary_path: Path,
    *,
    address: int,
    size: int,
    load_address: int | None,
) -> bytes:
    binary_data = binary_path.read_bytes()
    psx_info = read_psx_exe_info(binary_path)
    if psx_info is None:
        if load_address is None:
            raise ValueError(
                f"{binary_path} is not a PS-X EXE; pass --load-address for raw binaries"
            )
        payload_offset = 0
        payload_load_address = load_address
        payload_size = len(binary_data)
    else:
        payload_offset = psx_info.payload_offset
        payload_load_address = psx_info.load_address
        payload_size = psx_info.payload_size

    relative_offset = address - payload_load_address
    if relative_offset < 0 or relative_offset + size > payload_size:
        raise ValueError(
            f"{format_hex(address)}..{format_hex(address + size)} is outside {binary_path} loaded at {format_hex(payload_load_address)}"
        )
    file_offset = payload_offset + relative_offset
    return binary_data[file_offset : file_offset + size]
