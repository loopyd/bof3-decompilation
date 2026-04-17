from __future__ import annotations

import re
import shutil
from pathlib import Path
from typing import Any

from ...common import format_hex, relative_to_root, run_command, write_text_output
from ...config import PSN00B_TOOLCHAIN_BIN, PSYQ_ORIGINAL_40_ROOT, ROOT
from ...match import source_map
from .ghidra.decomp_helpers import source_program_path


DEFAULT_PSYQ_CONTEXT_HEADERS = (
    "libcd.h",
    "libapi.h",
    "libetc.h",
    "libpad.h",
    "libsnd.h",
    "libspu.h",
)
REPO_SYMBOL_RE = re.compile(r"\b(?:func|FUN|LAB)_(?P<addr>[0-9a-fA-F]{8})\b")


def build_m2c_context_source(
    *,
    source_text: str,
    requested_address: int,
    selected_asm_text: str,
    program_name: str | None = None,
) -> tuple[str, dict[str, Any]]:
    current_mapping = resolve_source_mapping(
        source_text=source_text,
        address=requested_address,
        program_name=program_name,
    )
    internal_header = (
        None if current_mapping is None else mapping_internal_header(current_mapping)
    )
    prototype_lines = referenced_repo_prototype_lines(
        selected_asm_text,
        source_text=source_text,
        requested_address=requested_address,
        program_name=program_name,
    )

    lines = [
        '#include "bof3/defines.h"',
        '#include "bof3/psyq_compat.h"',
    ]
    for header in DEFAULT_PSYQ_CONTEXT_HEADERS:
        lines.append(f"#include <{header}>")
    if internal_header is not None:
        lines.append(f'#include "{internal_header}"')
    if prototype_lines:
        lines.append("")
        lines.append("/* inferred repo prototypes */")
        lines.extend(prototype_lines)

    text = "\n".join(lines) + "\n"
    metadata: dict[str, Any] = {
        "status": "ok",
        "current_source_file": (
            None
            if current_mapping is None
            else str(current_mapping.get("source_file") or "")
        ),
        "internal_header": internal_header,
        "prototype_count": len(prototype_lines),
        "psyq_headers": list(DEFAULT_PSYQ_CONTEXT_HEADERS),
    }
    return text, metadata


def resolve_source_mapping(
    *,
    source_text: str,
    address: int,
    program_name: str | None = None,
) -> dict[str, Any] | None:
    return source_map.find_source_mapping(
        format_hex(address),
        program_path=source_program_path(source_text),
        program_name=program_name,
        source_hint=program_name,
    )


def mapping_internal_header(mapping: dict[str, Any]) -> str | None:
    source_file = str(mapping.get("source_file") or "")
    if not source_file:
        return None
    source_path = ROOT / source_file
    internal_header = source_path.parent / "internal.h"
    if not internal_header.exists():
        return None
    return relative_to_root(internal_header)


def referenced_repo_prototype_lines(
    asm_text: str,
    *,
    source_text: str,
    requested_address: int,
    program_name: str | None = None,
) -> list[str]:
    prototypes: dict[int, str] = {}
    for match in REPO_SYMBOL_RE.finditer(asm_text):
        address = int(match.group("addr"), 16)
        if address == requested_address or address in prototypes:
            continue
        mapping = resolve_source_mapping(
            source_text=source_text,
            address=address,
            program_name=program_name,
        )
        if mapping is None:
            continue
        signature = str(mapping.get("source_signature") or "").strip()
        if not signature:
            continue
        prototypes[address] = f"{signature};"
    return [prototypes[address] for address in sorted(prototypes)]


def resolve_m2c_preprocessor() -> str:
    preferred = PSN00B_TOOLCHAIN_BIN / "mipsel-none-elf-cpp"
    if preferred.exists():
        return str(preferred)
    discovered = shutil.which("mipsel-none-elf-cpp")
    if discovered:
        return discovered
    return shutil.which("cpp") or "cpp"


def build_m2c_context_preprocess_command(
    *,
    source_path: Path,
    output_path: Path,
) -> list[str]:
    command = [
        resolve_m2c_preprocessor(),
        "-P",
        "-I",
        str(ROOT / "bof3" / "include"),
        "-I",
        str(ROOT),
    ]
    psyq_include = PSYQ_ORIGINAL_40_ROOT / "include"
    if psyq_include.exists():
        command.extend(["-I", str(psyq_include)])
    command.extend(["-o", str(output_path), str(source_path)])
    return command


def generate_m2c_context_artifacts(
    *,
    source_text: str,
    requested_address: int,
    selected_asm_text: str,
    context_source_path: Path,
    context_preprocessed_path: Path,
    program_name: str | None = None,
) -> dict[str, Any]:
    context_source, metadata = build_m2c_context_source(
        source_text=source_text,
        requested_address=requested_address,
        selected_asm_text=selected_asm_text,
        program_name=program_name,
    )
    write_text_output(context_source_path, context_source)
    command = build_m2c_context_preprocess_command(
        source_path=context_source_path,
        output_path=context_preprocessed_path,
    )
    result = run_command(command)
    metadata.update(
        {
            "command": command,
            "source_path": relative_to_root(context_source_path),
            "path": relative_to_root(context_preprocessed_path),
            "attempted": True,
        }
    )
    if result.returncode != 0:
        metadata["status"] = "failed"
        metadata["stderr"] = (result.stderr or result.stdout).strip() or None
        return metadata
    metadata["status"] = "ok"
    metadata["stderr"] = None
    return metadata


__all__ = [
    "DEFAULT_PSYQ_CONTEXT_HEADERS",
    "build_m2c_context_preprocess_command",
    "build_m2c_context_source",
    "generate_m2c_context_artifacts",
    "mapping_internal_header",
    "referenced_repo_prototype_lines",
    "resolve_m2c_preprocessor",
    "resolve_source_mapping",
]
