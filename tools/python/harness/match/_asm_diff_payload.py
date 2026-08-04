"""Diff payload construction: request model, rendering, and match metrics."""

from __future__ import annotations

import difflib
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Mapping

from ..domain.manifests import SectionPlacement

from ._asm_resolve import format_hex

@dataclass(frozen=True)
class AsmDiffRequest:
    source_path: Path
    address: int | None = None
    size: int | None = None
    binary_path: Path | None = None
    load_address: int | None = None
    output_root: Path | None = None
    symbols_c_path: Path | None = None
    canonical_bindings: Mapping[str, int] | None = None
    section_placements: tuple[SectionPlacement, ...] | None = None
    diagnostics: bool = True

def render_diff(original_lines: list[str], current_lines: list[str]) -> str:
    diff_lines = difflib.unified_diff(
        original_lines,
        current_lines,
        fromfile="original",
        tofile="current",
        lineterm="",
    )
    return "\n".join(diff_lines) + "\n"

def matching_instruction_count(
    original_lines: list[str], current_lines: list[str]
) -> int:
    matcher = difflib.SequenceMatcher(a=original_lines, b=current_lines, autojunk=False)
    return sum(block.size for block in matcher.get_matching_blocks())

def first_instruction_mismatch(
    original_lines: list[str], current_lines: list[str]
) -> dict[str, Any] | None:
    matcher = difflib.SequenceMatcher(a=original_lines, b=current_lines, autojunk=False)
    for (
        tag,
        original_start,
        original_end,
        current_start,
        current_end,
    ) in matcher.get_opcodes():
        if tag == "equal":
            continue
        original_index = original_start if original_start < original_end else None
        current_index = current_start if current_start < current_end else None
        return {
            "original_index": original_index,
            "current_index": current_index,
            "original_offset": (None if original_index is None else original_index * 4),
            "current_offset": None if current_index is None else current_index * 4,
            "original": (
                None if original_index is None else original_lines[original_index]
            ),
            "current": None if current_index is None else current_lines[current_index],
        }
    return None

def build_result_payload(
    *,
    source_path: Path,
    function_name: str,
    address: int,
    original_size: int,
    current_size: int | None,
    byte_match: bool | None = None,
    binary_path: Path,
    object_path: Path,
    output_dir: Path,
    original_lines: list[str],
    current_lines: list[str],
    linked_path: Path | None = None,
) -> dict[str, Any]:
    exact_match = (
        byte_match if byte_match is not None else original_lines == current_lines
    )
    status = "exact_match" if exact_match else "different"
    matching_count = matching_instruction_count(original_lines, current_lines)
    denominator = max(len(original_lines), len(current_lines), 1)
    match_percent = (
        100.0 if byte_match else round((matching_count / denominator) * 100, 2)
    )
    payload: dict[str, Any] = {
        "schema": "harness.asm-diff-one/v2",
        "status": status,
        "exact_match": exact_match,
        "byte_match": byte_match,
        "source": str(source_path),
        "function": function_name,
        "address": format_hex(address),
        "original_size": original_size,
        "current_size": current_size,
        "size_delta": None if current_size is None else current_size - original_size,
        "original_binary": str(binary_path),
        "current_object": str(object_path),
        "instruction_count": {
            "original": len(original_lines),
            "current": len(current_lines),
            "matching": matching_count,
            "match_percent": match_percent,
        },
        "first_mismatch": first_instruction_mismatch(original_lines, current_lines),
        "outputs": {
            "directory": str(output_dir),
            "summary": str(output_dir / "summary.json"),
            "diff": str(output_dir / "diff.patch"),
            "original": str(output_dir / "original.s"),
            "current": str(output_dir / "current.s"),
            "compiler": str(output_dir / "compiler.s"),
            "original_bytes": str(output_dir / "original.bin"),
            "build_log": str(output_dir / "build.log"),
        },
    }
    if linked_path is not None:
        payload["outputs"]["linked"] = str(output_dir / "linked.s")
    return payload
