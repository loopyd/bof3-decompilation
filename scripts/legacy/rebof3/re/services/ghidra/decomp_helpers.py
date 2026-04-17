from __future__ import annotations

import re
from pathlib import Path
from typing import Any

from ....assets.emi_archive import EmiArchive
from ....common import infer_bin_metadata, parse_source_spec
from ....inventory.db.connection import inventory_db
from ....inventory.ghidra_symbols import canonical_program_path
from ..asm_normalize import (
    AddressSymbolResolver,
    normalize_commented_asm,
    normalize_function_symbol_name,
)
from ..bootstrap import default_inventory_db


def bundle_artifact_paths(artifacts_dir: Path) -> dict[str, Path]:
    return {
        "json": artifacts_dir / "func.json",
        "ghidra_c": artifacts_dir / "func.ghidra.c",
        "ghidra_asm": artifacts_dir / "func.ghidra.s",
        "spim_asm": artifacts_dir / "func.spim.s",
        "asm": artifacts_dir / "func.s",
        "m2c_context_source": artifacts_dir / "func.m2c.ctx.c",
        "m2c_context": artifacts_dir / "func.m2c.ctx.i",
        "m2c_asm": artifacts_dir / "func.m2c.s",
        "m2c_c": artifacts_dir / "func.m2c.c",
    }


def infer_source_base_addr(source_path: Path, entry_index: int | None) -> int | None:
    if entry_index is not None:
        archive = EmiArchive(source_path)
        return int(archive.entry(entry_index).load_arg)

    if source_path.suffix.lower() == ".bin":
        metadata = infer_bin_metadata(source_path)
        if metadata is not None:
            return int(metadata["load_address"])

    return None


def default_program_name(source_text: str, base_addr: int | None) -> str:
    source_spec = parse_source_spec(source_text)
    source_path, entry_index = source_spec.path, source_spec.entry_index
    if entry_index is not None:
        suffix = f"_e{entry_index:02d}"
        if base_addr is not None:
            suffix += f"_{base_addr:08x}"
        return f"{source_path.stem}{suffix}.bin"
    return source_path.name


def extract_decompiled_c(exported: list[dict[str, Any]]) -> str:
    if not exported:
        return ""
    return str(exported[0].get("c", ""))


def source_program_path(source_text: str) -> str | None:
    if source_text.startswith("/"):
        return canonical_program_path(source_text)

    source_spec = parse_source_spec(source_text)
    source_path = source_spec.path
    parts = source_path.parts
    if parts[-1:] == ("SLUS_004.22",):
        return "/boot/SLUS_004.22"
    if parts[-2:] == ("LOGO", "LOGO.EXE"):
        return "/boot/LOGO/LOGO.EXE"
    if source_spec.entry_index is None or source_path.suffix.upper() != ".EMI":
        return None
    try:
        extracted_index = parts.index("extracted")
    except ValueError:
        return None
    relative_parts = parts[extracted_index + 1 :]
    if not relative_parts:
        return None
    archive_name = Path(relative_parts[-1]).stem
    folder_parts = relative_parts[:-1]
    return canonical_program_path(
        "/bins/" + "/".join(folder_parts) + f"/{archive_name}/{source_spec.entry_index}.bin"
    )


def load_program_symbol_resolver(source_text: str) -> AddressSymbolResolver | None:
    inventory_path = default_inventory_db()
    if not inventory_path.exists():
        return None
    program_path = source_program_path(source_text)
    if not program_path:
        return None

    function_symbols: dict[int, str] = {}
    data_symbols: dict[int, str] = {}
    with inventory_db(inventory_path) as connection:
        for row in connection.execute(
            "SELECT entry_address, name FROM functions f JOIN programs p ON p.id = f.program_id WHERE p.program_path = ?",
            (program_path,),
        ).fetchall():
            address = row[0]
            name = str(row[1] or "").strip()
            if address is None:
                continue
            function_symbols[int(address)] = normalize_function_symbol_name(
                name or f"func_{int(address):08x}",
                int(address),
            )
        for row in connection.execute(
            "SELECT address, kind, name FROM metadata_rows WHERE program_path = ? AND kind IN ('data', 'label', 'symbol') AND address IS NOT NULL",
            (program_path,),
        ).fetchall():
            address = row[0]
            kind = str(row[1] or "")
            name = str(row[2] or "").strip()
            if address is None:
                continue
            address_int = int(address)
            candidate_name = name or (
                f"DAT_{address_int:08x}"
                if kind != "label"
                else f"LAB_{address_int:08x}"
            )
            existing = data_symbols.get(address_int)
            if existing is None or kind == "label":
                data_symbols[address_int] = candidate_name
    return AddressSymbolResolver(
        function_symbols=function_symbols,
        data_symbols=data_symbols,
    )


def rewrite_asm_for_m2c(
    text: str, *, resolver: AddressSymbolResolver | None = None
) -> str:
    return normalize_commented_asm(text, resolver=resolver)


def bundle_function_metadata(
    function_payload: dict[str, Any] | None,
) -> dict[str, Any] | None:
    if function_payload is None:
        return None

    bundled = dict(function_payload)
    bundled.pop("c", None)
    return bundled


__all__ = [
    "rewrite_asm_for_m2c",
    "load_program_symbol_resolver",
    "source_program_path",
    "bundle_artifact_paths",
    "bundle_function_metadata",
    "default_program_name",
    "extract_decompiled_c",
    "infer_bin_metadata",
    "infer_source_base_addr",
    "parse_source_spec",
]
