from __future__ import annotations

import difflib
import os
import re
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Mapping

from ..domain.manifests import SectionPlacement

from ..build import build, cmake_target_for_source
from ..io import write_json, RepoLayout, repo_layout
from ._asm_disasm import (
    current_symbol_size,
    disassemble_linked,
    disassemble_original,
    extract_instructions,
    render_normalized,
)
from ._asm_link import extract_section_bytes, function_bytes_match
from ._asm_resolve import (
    PsxExeInfo,  # noqa: F401 — re-exported for backward compat
    build_target_for_source,
    collect_source_addresses,  # noqa: F401 — re-exported
    compiler_asm_path_for_object,
    default_binary_for_source,
    extract_original_bytes,
    format_hex,
    infer_original_size,
    infer_size_from_binary_return,  # noqa: F401 — re-exported
    infer_size_from_sibling_sources,  # noqa: F401 — re-exported
    object_path_for_source,
    overlay_load_address_for_source,
    parse_int,
    parse_source_address,
    read_psx_exe_info,  # noqa: F401 — re-exported
    read_u32le,  # noqa: F401 — re-exported
    source_function_name,
)

__all__ = [
    "AsmDiffRequest",
    "PsxExeInfo",
    "_asm_diff_compare",
    "_asm_diff_resolve",
    "build_result_payload",
    "build_target_for_source",
    "collect_source_addresses",
    "default_binary_for_source",
    "extract_original_bytes",
    "first_instruction_mismatch",
    "infer_original_size",
    "infer_size_from_sibling_sources",
    "matching_instruction_count",
    "object_path_for_source",
    "overlay_load_address_for_source",
    "parse_int",
    "parse_source_address",
    "read_psx_exe_info",
    "read_u32le",
    "run_asm_diff_one",
]

# -- data -------------------------------------------------------------------------------


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


# -- diff -------------------------------------------------------------------------------


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


# -- result -----------------------------------------------------------------------------


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


# -- orchestration ----------------------------------------------------------------------


def run_build_object(
    layout: RepoLayout, source_path: Path, build_log_path: Path | None
) -> None:
    target = cmake_target_for_source(layout.root, source_path)
    object_path = object_path_for_source(layout, source_path)
    if shutil.which("cmake") is None:
        raise FileNotFoundError(
            f"cmake executable not found in PATH; cannot build {target}"
        )

    result = build(layout.root, target)
    log_text = result.stdout
    if result.stderr:
        if log_text:
            log_text += "\n"
        log_text += result.stderr
    if build_log_path is not None:
        build_log_path.parent.mkdir(parents=True, exist_ok=True)
        build_log_path.write_text(log_text, encoding="utf-8")
    if result.returncode != 0:
        suffix = "" if build_log_path is None else f"; see {build_log_path}"
        raise RuntimeError(f"object build failed for {target}{suffix}")
    if (
        not object_path.is_file()
        or object_path.stat().st_mtime < source_path.stat().st_mtime
    ):
        raise RuntimeError(
            f"object build did not refresh {object_path}; see {build_log_path}"
        )


def _asm_diff_resolve(
    repo: RepoLayout, request: AsmDiffRequest
) -> dict[str, Any]:
    """Resolve and prepare every input needed by the comparison step.

    Returns a dict with keys:
      source_path, address, function_name, binary_path, load_address,
      original_size, object_path, output_dir

    This is extracted so the status-audit batch path can know what to build
    without duplicating resolution logic.
    """
    source_path = request.source_path.expanduser().resolve()
    address = (
        request.address
        if request.address is not None
        else parse_source_address(source_path)
    )
    function_name = source_function_name(source_path, address)
    binary_path = (
        request.binary_path.expanduser().resolve()
        if request.binary_path is not None
        else default_binary_for_source(repo, source_path)
    )
    load_address = request.load_address or overlay_load_address_for_source(
        repo, source_path
    )
    original_size = (
        request.size
        if request.size is not None
        else infer_original_size(
            source_path,
            address=address,
            binary_path=binary_path,
            load_address=load_address,
        )
    )
    object_path = object_path_for_source(repo, source_path)
    output_root = request.output_root or repo.out_dir / "matching"
    try:
        owner = source_path.parent.relative_to(repo.root).as_posix()
    except ValueError:
        owner = source_path.parent.name
    target_slug = re.sub(r"[^A-Za-z0-9_.-]+", "_", owner)
    output_dir = output_root / target_slug / function_name
    if request.diagnostics:
        if output_dir.is_dir():
            shutil.rmtree(output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
    return {
        "source_path": source_path,
        "address": address,
        "function_name": function_name,
        "binary_path": binary_path,
        "load_address": load_address,
        "original_size": original_size,
        "object_path": object_path,
        "output_dir": output_dir,
    }


def _asm_diff_compare(
    repo: RepoLayout,
    request: AsmDiffRequest,
    resolved: dict[str, Any],
) -> dict[str, Any]:
    """Run the link, byte-match, placement, size, and diagnostic steps.

    *Assumes* the object already exists (built by the caller).  Object
    freshness is verified via ``st_mtime``.
    """
    source_path = resolved["source_path"]
    address = resolved["address"]
    function_name = resolved["function_name"]
    binary_path = resolved["binary_path"]
    load_address = resolved["load_address"]
    original_size = resolved["original_size"]
    object_path = resolved["object_path"]
    output_dir = resolved["output_dir"]

    if not object_path.is_file():
        raise FileNotFoundError(f"expected object was not built: {object_path}")
    if not binary_path.is_file():
        raise FileNotFoundError(f"original binary not found: {binary_path}")

    objdump_path = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-objdump"
    nm_path = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-nm"
    if request.diagnostics and not os.access(objdump_path, os.X_OK):
        raise FileNotFoundError(f"missing executable {objdump_path}; run `just setup`")
    if not os.access(nm_path, os.X_OK):
        raise FileNotFoundError(f"missing executable {nm_path}; run `just setup`")

    original_bytes_path = output_dir / "original.bin"
    original_bytes = extract_original_bytes(
        binary_path,
        address=address,
        size=original_size,
        load_address=load_address,
    )
    if request.diagnostics:
        original_bytes_path.write_bytes(original_bytes)

    if request.section_placements is None:
        from ..domain.manifests import load_target_manifests

        source_directory = source_path.parent.relative_to(repo.root).as_posix()
        manifest = next(
            (
                value
                for value in load_target_manifests(repo.root).values()
                if value.source_dir == source_directory
            ),
            None,
        )
        placements = (
            () if manifest is None else manifest.section_placements.get(address, ())
        )
    else:
        placements = request.section_placements
    section_addresses = {
        placement.section: placement.address for placement in placements
    }

    current_compiler_asm = compiler_asm_path_for_object(object_path)
    if request.diagnostics and not current_compiler_asm.is_file():
        raise FileNotFoundError(
            f"expected compiler assembly was not written: {current_compiler_asm}"
        )

    byte_match, compiled_bytes = function_bytes_match(
        object_path,
        address=address,
        size=original_size,
        original_bytes=original_bytes,
        symbols_c_path=request.symbols_c_path or source_path.parent / "symbols.c",
        canonical_bindings=request.canonical_bindings,
        layout=repo,
        section_addresses=section_addresses,
    )
    linked_path = object_path.with_suffix(".linked.o")
    for placement in placements:
        linked_section = extract_section_bytes(
            linked_path, section=placement.section, layout=repo
        )
        original_section = extract_original_bytes(
            binary_path,
            address=placement.address,
            size=placement.size,
            load_address=load_address,
        )
        if len(linked_section) != placement.size or linked_section != original_section:
            raise RuntimeError(
                f"reviewed {placement.section} placement for {function_name} does not "
                "match original bytes"
            )
    current_size = current_symbol_size(nm_path, object_path, function_name)
    if not request.diagnostics:
        return {
            "schema": "harness.byte-match-one/v1",
            "status": "exact_match" if byte_match else "different",
            "exact_match": byte_match,
            "byte_match": byte_match,
            "source": str(source_path),
            "function": function_name,
            "address": format_hex(address),
            "original_size": original_size,
            "current_size": current_size,
            "size_delta": None if current_size is None else current_size - original_size,
            "original_binary": str(binary_path),
            "current_object": str(object_path),
            "outputs": {},
        }

    shutil.copyfile(current_compiler_asm, output_dir / "compiler.s")

    original_objdump = disassemble_original(
        objdump_path=objdump_path,
        original_bytes_path=original_bytes_path,
        address=address,
    )
    linked_objdump = disassemble_linked(
        objdump_path=objdump_path, linked_path=linked_path
    )

    original_lines = extract_instructions(original_objdump)
    current_lines = extract_instructions(linked_objdump)

    (output_dir / "original.s").write_text(
        render_normalized(original_lines), encoding="utf-8"
    )
    (output_dir / "current.s").write_text(
        render_normalized(current_lines), encoding="utf-8"
    )
    (output_dir / "linked.s").write_text(linked_objdump, encoding="utf-8")
    (output_dir / "diff.patch").write_text(
        render_diff(original_lines, current_lines), encoding="utf-8"
    )

    payload = build_result_payload(
        source_path=source_path,
        function_name=function_name,
        address=address,
        original_size=original_size,
        current_size=current_size,
        byte_match=byte_match,
        binary_path=binary_path,
        object_path=object_path,
        output_dir=output_dir,
        original_lines=original_lines,
        current_lines=current_lines,
        linked_path=linked_path,
    )
    write_json(output_dir / "summary.json", payload)
    return payload


def run_asm_diff_one(
    request: AsmDiffRequest,
    *,
    layout: RepoLayout | None = None,
) -> dict[str, Any]:
    repo = layout or repo_layout()
    resolved = _asm_diff_resolve(repo, request)
    run_build_object(
        repo,
        resolved["source_path"],
        resolved["output_dir"] / "build.log" if request.diagnostics else None,
    )
    return _asm_diff_compare(repo, request, resolved)
