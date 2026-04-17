from __future__ import annotations

import shutil
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ...assets.emi_archive import EmiArchive
from ...common import (
    format_hex,
    infer_bin_metadata,
    parse_hexish,
    parse_source_spec,
    relative_to_root,
    run_command,
    write_text_output,
)
from ...config import ROOT
from .asm_normalize import (
    AddressSymbolResolver,
    normalize_asm_symbol_name,
    normalize_function_symbol_name,
)


@dataclass(frozen=True, slots=True)
class SpimdisasmSlice:
    source_path: Path
    source_kind: str
    load_address: int
    start_address: int
    end_address: int
    slice_path: Path
    size: int


def _resolve_source_path(source_text: str) -> tuple[Path, int | None]:
    source_spec = parse_source_spec(source_text)
    source_path = source_spec.path
    if not source_path.is_absolute():
        source_path = (ROOT / source_path).resolve()
    return source_path, source_spec.entry_index


def parse_psx_exe_header(path: Path) -> dict[str, int]:
    data = path.read_bytes()
    if len(data) < 0x20 or data[:8] != b"PS-X EXE":
        raise ValueError(f"not a PS-X EXE: {path}")
    return {
        "text_addr": struct.unpack_from("<I", data, 0x18)[0],
        "text_size": struct.unpack_from("<I", data, 0x1C)[0],
    }


def resolve_source_payload(source_text: str) -> tuple[bytes, int, str, Path]:
    source_path, entry_index = _resolve_source_path(source_text)
    if entry_index is not None:
        archive = EmiArchive(source_path)
        entry = archive.entry(entry_index)
        return archive.payload(entry_index), int(entry.load_arg), "emi", source_path

    try:
        header = parse_psx_exe_header(source_path)
    except ValueError:
        header = None
    if header is not None:
        data = source_path.read_bytes()
        text_size = int(header["text_size"])
        text_start = 0x800
        return (
            data[text_start : text_start + text_size],
            int(header["text_addr"]),
            "psx-exe",
            source_path,
        )

    metadata = infer_bin_metadata(source_path)
    if metadata is None:
        raise ValueError(f"could not infer load address for source: {source_text}")
    return (
        source_path.read_bytes(),
        int(metadata["load_address"]),
        "bin",
        source_path,
    )


def function_bounds(function_payload: dict[str, Any]) -> tuple[int, int]:
    start_address = parse_hexish(str(function_payload["body_min"]))
    end_address = parse_hexish(str(function_payload["body_max"]))
    if end_address < start_address:
        raise ValueError(
            f"invalid function bounds: {format_hex(start_address)}..{format_hex(end_address)}"
        )
    return start_address, end_address


def slice_function_binary(
    *,
    source_text: str,
    function_payload: dict[str, Any],
    slice_path: Path,
) -> SpimdisasmSlice:
    payload, load_address, source_kind, source_path = resolve_source_payload(
        source_text
    )
    start_address, end_address = function_bounds(function_payload)
    start_offset = start_address - load_address
    end_offset = end_address - load_address + 1
    if start_offset < 0 or end_offset > len(payload):
        raise ValueError(
            "function slice falls outside source payload: "
            f"{format_hex(start_address)}..{format_hex(end_address)} "
            f"(load={format_hex(load_address)}, size=0x{len(payload):x})"
        )

    slice_bytes = payload[start_offset:end_offset]
    slice_path.parent.mkdir(parents=True, exist_ok=True)
    slice_path.write_bytes(slice_bytes)
    return SpimdisasmSlice(
        source_path=source_path,
        source_kind=source_kind,
        load_address=load_address,
        start_address=start_address,
        end_address=end_address,
        slice_path=slice_path,
        size=len(slice_bytes),
    )


def write_spim_symbol_addrs(
    path: Path, resolver: AddressSymbolResolver | None
) -> Path | None:
    if resolver is None:
        return None

    lines: list[str] = []
    seen_names: set[str] = set()
    for address, name in sorted(resolver.function_symbols.items()):
        normalized_name = normalize_asm_symbol_name(
            normalize_function_symbol_name(name, address), "func", address
        )
        if normalized_name in seen_names:
            continue
        seen_names.add(normalized_name)
        lines.append(f"{normalized_name} = {format_hex(address)}; // type:func")

    function_addresses = set(resolver.function_symbols)
    for address, name in sorted(resolver.data_symbols.items()):
        if address in function_addresses:
            continue
        normalized_name = normalize_asm_symbol_name(name, "DAT", address)
        if normalized_name in seen_names:
            continue
        seen_names.add(normalized_name)
        if normalized_name.startswith("LAB_"):
            lines.append(f"{normalized_name} = {format_hex(address)}; // type:label")
        else:
            lines.append(f"{normalized_name} = {format_hex(address)};")

    if not lines:
        return None

    write_text_output(path, "\n".join(lines) + "\n")
    return path


def build_spimdisasm_command(
    *,
    slice_path: Path,
    output_path: Path,
    slice_vram: int,
    slice_size: int,
    symbol_addrs_path: Path | None = None,
) -> list[str]:
    command = [
        sys.executable,
        "-m",
        "spimdisasm",
        "singleFileDisasm",
        str(slice_path),
        str(output_path),
        "--start",
        "0x0",
        "--end",
        hex(slice_size),
        "--vram",
        format_hex(slice_vram),
        "--instr-category",
        "r3000gte",
        "--compiler",
        "PSYQ",
        "--endian",
        "little",
        "--abi",
        "O32",
        "--arch-level",
        "MIPS1",
        "--named-registers",
        "--pseudo-instr",
        "--no-glabel-count",
        "--no-asm-generated-by",
    ]
    if symbol_addrs_path is not None:
        command.extend(["--symbol-addrs", str(symbol_addrs_path)])
    return command


def run_spimdisasm_function_asm(
    *,
    source_text: str,
    function_payload: dict[str, Any],
    output_path: Path,
    resolver: AddressSymbolResolver | None = None,
) -> dict[str, Any]:
    slice_path = output_path.with_suffix(".bin")
    symbol_addrs_path = output_path.with_suffix(".symbol_addrs.txt")
    tool_output_path = output_path.parent / f"{output_path.name}.dir"
    slice_info = slice_function_binary(
        source_text=source_text,
        function_payload=function_payload,
        slice_path=slice_path,
    )
    written_symbol_addrs = write_spim_symbol_addrs(symbol_addrs_path, resolver)
    command = build_spimdisasm_command(
        slice_path=slice_info.slice_path,
        output_path=tool_output_path,
        slice_vram=slice_info.start_address,
        slice_size=slice_info.size,
        symbol_addrs_path=written_symbol_addrs,
    )
    if tool_output_path.exists():
        if tool_output_path.is_dir():
            shutil.rmtree(tool_output_path)
        else:
            tool_output_path.unlink()
    result = run_command(command)
    metadata: dict[str, Any] = {
        "status": "ok" if result.returncode == 0 else "failed",
        "command": command,
        "slice_path": relative_to_root(slice_info.slice_path),
        "symbol_addrs_path": (
            None
            if written_symbol_addrs is None
            else relative_to_root(written_symbol_addrs)
        ),
        "output_path": relative_to_root(output_path),
        "source_kind": slice_info.source_kind,
        "slice_start": format_hex(slice_info.start_address),
        "slice_end": format_hex(slice_info.end_address),
    }
    if result.returncode == 0:
        if tool_output_path.is_file():
            output_path.parent.mkdir(parents=True, exist_ok=True)
            output_path.write_text(
                tool_output_path.read_text(encoding="utf-8"), encoding="utf-8"
            )
        elif tool_output_path.is_dir():
            asm_files = sorted(tool_output_path.rglob("*.s"))
            if not asm_files:
                metadata["status"] = "failed"
                metadata["stderr"] = "spimdisasm produced no asm output files"
            else:
                output_path.parent.mkdir(parents=True, exist_ok=True)
                output_path.write_text(
                    asm_files[0].read_text(encoding="utf-8"), encoding="utf-8"
                )
        else:
            metadata["status"] = "failed"
            metadata["stderr"] = "spimdisasm produced no output artifact"
    if metadata["status"] != "ok":
        if metadata.get("stderr") is None:
            metadata["stderr"] = (result.stderr or result.stdout).strip() or None
    return metadata


__all__ = [
    "SpimdisasmSlice",
    "build_spimdisasm_command",
    "function_bounds",
    "parse_psx_exe_header",
    "resolve_source_payload",
    "run_spimdisasm_function_asm",
    "slice_function_binary",
    "write_spim_symbol_addrs",
]
