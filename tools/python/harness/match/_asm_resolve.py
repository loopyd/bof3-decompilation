from __future__ import annotations

import json
import re
import shlex
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..jsonio import read_json
from ..paths import RepoLayout, repo_layout

IMPLAUSIBLE_SIBLING_FUNCTION_SIZE = 0x1000
MIPS_JR_RA = 0x03E00008
PSX_EXE_MAGIC = b"PS-X EXE"
PSX_EXE_HEADER_SIZE = 0x800
SOURCE_ADDRESS_RE = re.compile(r"@source\s+(0x[0-9a-fA-F]+|[0-9a-fA-F]{8})")
FUNC_NAME_RE = re.compile(r"func_([0-9a-fA-F]{8})")
CMAKE_TARGET_RE = re.compile(r"(?:^|/)CMakeFiles/(?P<target>[^/]+)\.dir/")


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
        return parse_int(match.group(1))
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


def _infer_size_from_function_index(source_path: Path, address: int) -> int:
    layout = repo_layout()
    func_index = layout.out_dir / "inventory" / "ghidra_function_index.json"
    if func_index.is_file():
        payload = read_json(func_index)
        rows = payload.get("rows", [])
        entry_hex = f"0x{address:08x}"
        for row in rows:
            if row.get("entry_hex") != entry_hex:
                continue
            body_min = row.get("body_min", "")
            body_max = row.get("body_max", "")
            if body_min and body_max:
                return int(body_max, 16) - int(body_min, 16) + 1
    raise ValueError(
        f"cannot infer original size for {source_path}; pass --size or add the next source in the same directory"
    )


def infer_original_size(
    source_path: Path,
    *,
    address: int,
    binary_path: Path,
    load_address: int | None,
) -> int:
    sibling_size = infer_size_from_sibling_sources(source_path, address)
    if sibling_size is not None:
        return sibling_size
    try:
        return _infer_size_from_function_index(source_path, address)
    except ValueError:
        return infer_size_from_binary_return(
            binary_path, address=address, load_address=load_address
        )


def source_function_name(source_path: Path, address: int) -> str:
    match = FUNC_NAME_RE.search(source_path.stem)
    if match is not None:
        return f"func_{match.group(1).lower()}"
    return f"func_{address:08x}"


def object_path_for_source(layout: RepoLayout, source_path: Path) -> Path:
    command = compile_command_for_source(layout, source_path)
    return Path(command["directory"]) / command["output"]


def compiler_asm_path_for_object(object_path: Path) -> Path:
    return object_path.with_name(object_path.name + ".s")


def build_target_for_source(layout: RepoLayout, source_path: Path) -> str:
    source_rel = source_path.expanduser().resolve().relative_to(layout.root.resolve())
    return source_rel.with_suffix(".obj").as_posix()


def compile_command_for_source(layout: RepoLayout, source_path: Path) -> dict[str, str]:
    commands_path = layout.build_dir / "default" / "compile_commands.json"
    if not commands_path.is_file():
        raise FileNotFoundError(f"missing {commands_path}; run `just build` first")
    payload: Any = json.loads(commands_path.read_text(encoding="utf-8"))
    if not isinstance(payload, list):
        raise ValueError(f"expected a JSON array in {commands_path}")
    resolved_source = source_path.expanduser().resolve()
    matches = [
        row
        for row in payload
        if isinstance(row, dict)
        and Path(str(row.get("file", ""))).resolve() == resolved_source
        and isinstance(row.get("directory"), str)
        and isinstance(row.get("output"), str)
    ]
    if len(matches) != 1:
        raise ValueError(
            f"expected one CMake compile command for {source_path}, found {len(matches)}"
        )
    result = {
        "directory": str(matches[0]["directory"]),
        "output": str(matches[0]["output"]),
    }
    command = matches[0].get("command")
    arguments = matches[0].get("arguments")
    if isinstance(command, str):
        result["command"] = command
    elif isinstance(arguments, list) and all(
        isinstance(argument, str) for argument in arguments
    ):
        result["command"] = shlex.join(arguments)
    return result


def default_binary_for_source(layout: RepoLayout, source_path: Path) -> Path:
    resolved_source = source_path.expanduser().resolve()
    try:
        source_rel = resolved_source.relative_to(layout.harness_dir).as_posix()
    except ValueError:
        source_rel = ""
    from ..domain import load_target_manifests

    source_dir = str(Path(source_rel).parent)
    for manifest in load_target_manifests(layout.root).values():
        if manifest.source_dir == source_dir:
            return layout.root / manifest.binary

    artifact = _artifact_overlay_for_source(layout, source_path)
    if artifact is not None:
        return artifact[0]
    raise ValueError(f"cannot resolve original binary for overlay source: {source_rel}")


def overlay_load_address_for_source(
    layout: RepoLayout, source_path: Path
) -> int | None:
    resolved_source = source_path.expanduser().resolve()
    try:
        source_rel = resolved_source.relative_to(layout.harness_dir).as_posix()
    except ValueError:
        return None

    from ..domain import load_target_manifests

    source_dir = str(Path(source_rel).parent)
    for manifest in load_target_manifests(layout.root).values():
        if manifest.source_dir == source_dir:
            return manifest.load_address

    artifact = _artifact_overlay_for_source(layout, source_path)
    return artifact[1] if artifact is not None else None


def _artifact_overlay_for_source(
    layout: RepoLayout, source_path: Path
) -> tuple[Path, int] | None:
    try:
        output = compile_command_for_source(layout, source_path)["output"]
    except (FileNotFoundError, ValueError):
        return None
    target_match = CMAKE_TARGET_RE.search(output)
    if target_match is None:
        return None
    manifest_path = (
        layout.build_dir / "default" / "artifacts" / "metadata" / "artifacts.json"
    )
    catalog_path = layout.root / "out" / "catalog" / "emi.json"
    if not manifest_path.is_file():
        return None
    target = target_match.group("target")
    manifest = read_json(manifest_path)
    rows = [row for row in manifest.get("artifacts", []) if row.get("target") == target]
    if len(rows) != 1:
        return None
    source_hint = str(rows[0].get("source_hint", ""))
    if ".EMI#" not in source_hint.upper():
        return None
    from ..domain import load_target_manifests

    for target_manifest in load_target_manifests(layout.root).values():
        if target_manifest.id.value == target:
            return (
                layout.root / target_manifest.binary,
                target_manifest.load_address,
            )
    # Keep extracted-catalog fallback for isolated fixtures and workspaces
    # created before a target manifest was promoted.
    if not catalog_path.is_file():
        return None
    from ..binaries import resolve_entry

    normalized_hint = source_hint.replace("\\", "/")
    marker = "BIN/"
    marker_offset = normalized_hint.upper().find(marker)
    if marker_offset >= 0:
        normalized_hint = normalized_hint[marker_offset:]
    entry = resolve_entry(read_json(catalog_path), normalized_hint)
    return Path(entry["payload_path"]), int(entry["load_address"])


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
