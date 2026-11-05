"""Object build and assembly resolve/compare execution for asm-diff."""

from __future__ import annotations

import hashlib
import os
import re
import shutil
from collections.abc import Callable
from pathlib import Path
from typing import Any, Mapping

from ..build.compilation import compile_object
from ..common.files import read_file
from ..domain.psx import payload_for
from ..io import write_json, RepoLayout
from ._asm_disasm import (
    disassemble_linked,
    disassemble_original,
    extract_instructions,
    render_normalized,
)
from ._asm_link import _target_map_bindings, extract_section_bytes, function_bytes_match
from ._asm_resolve import (
    compiler_asm_path_for_object,
    default_binary_for_source,
    extract_original_bytes,
    format_hex,
    infer_original_size,
    object_path_for_source,
    overlay_load_address_for_source,
)

from ..domain.sources import compiled_symbol_name, owning_manifest, source_address
from ..domain.manifests import TargetManifest
from ..domain.tags import require_single_function
from ..domain.policy import validate_matching_source

from ._asm_diff_payload import (
    AsmDiffRequest,
    EmittedRange,
    build_result_payload,
    render_diff,
)
from .extraction import read_function_bytes, read_linked_image


# One process compares many members of the same translation unit - a grouped
# source with thirty functions would otherwise be compiled thirty times. The
# cache is per process, so it cannot outlive the command that populated it, and
# it is keyed by the source's identity as well as its path.
_BUILT: set[tuple[Path, int, int]] = set()


def run_build_object(
    layout: RepoLayout, source_path: Path, build_log_path: Path | None
) -> None:
    object_path = object_path_for_source(layout, source_path)
    stat = source_path.stat()
    key = (source_path, stat.st_mtime_ns, stat.st_size)
    if (
        key in _BUILT
        and object_path.is_file()
        and object_path.stat().st_mtime >= stat.st_mtime
    ):
        return
    result = compile_object(layout.root, source_path, object_path)
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
        raise RuntimeError(f"object build failed for {source_path}{suffix}")
    if (
        not object_path.is_file()
        or object_path.stat().st_mtime < source_path.stat().st_mtime
    ):
        raise RuntimeError(
            f"object build did not refresh {object_path}; see {build_log_path}"
        )
    _BUILT.add(key)


def _asm_diff_resolve(
    repo: RepoLayout,
    request: AsmDiffRequest,
    *,
    manifests: Mapping[str, TargetManifest] | None = None,
) -> dict[str, Any]:
    """Resolve and prepare every input needed by the comparison step.

    Returns a dict with keys:
      source_path, address, function_name, binary_path, load_address,
      original_size, object_path, output_dir

    This is extracted so the status-audit batch path can know what to build
    without duplicating resolution logic.
    """
    source_path = request.source_path.expanduser().resolve()
    validate_matching_source(repo.root, source_path)
    require_single_function(source_path.read_text(encoding="utf-8"))
    address = (
        request.address if request.address is not None else source_address(source_path)
    )
    manifest = (
        owning_manifest(repo.root, source_path)
        if manifests is None
        else owning_manifest(repo.root, source_path, manifests=manifests)
    )
    function_name = compiled_symbol_name(
        repo.root, source_path, address, manifest=manifest
    )
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
            root=repo.root,
        )
    )
    object_path = object_path_for_source(repo, source_path)
    output_root = request.output_root or repo.out_dir / "matching"
    if manifest is not None:
        target_slug = manifest.id.value.replace("/", "_")
    else:
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
    *,
    manifests: Mapping[str, TargetManifest] | None = None,
    emitted: list[EmittedRange] | None = None,
    on_linked: Callable[[Path], None] | None = None,
) -> dict[str, Any]:
    """Run the link, byte-match, placement, size, and diagnostic steps.

    The caller must freshly produce the object before comparison. An existing
    object or a recent timestamp alone does not establish production freshness.

    ``manifests`` lets a caller that already holds a validated catalog (the
    status-audit batch path resolves it once per target) reuse it instead of
    re-enumerating every claim file for each comparison.
    """
    source_path = resolved["source_path"]
    validate_matching_source(repo.root, source_path)
    require_single_function(source_path.read_text(encoding="utf-8"))
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

    objdump_path = repo.psyqo_toolchain_root / "bin" / "mipsel-none-elf-objdump"
    nm_path = repo.psyqo_toolchain_root / "bin" / "mipsel-none-elf-nm"
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

    manifest = owning_manifest(repo.root, source_path, manifests=manifests)
    if request.section_placements is None:
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

    if request.symbols_c_path is not None:
        symbols_c_path = request.symbols_c_path
    elif manifest is not None:
        symbols_c_path = repo.root / manifest.source_dir / "symbols.c"
    else:
        symbols_c_path = source_path.parent / "symbols.c"
    canonical_bindings = request.canonical_bindings
    if canonical_bindings is None:
        canonical_bindings = _target_map_bindings(
            repo, symbols_c_path, manifest=manifest
        )
    byte_match, compiled_bytes = function_bytes_match(
        object_path,
        function_name=function_name,
        address=address,
        size=original_size,
        original_bytes=original_bytes,
        symbols_c_path=symbols_c_path,
        canonical_bindings=canonical_bindings,
        layout=repo,
        section_addresses=section_addresses,
        on_linked=on_linked,
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
    if emitted is not None and byte_match:
        if manifest is None:
            raise ValueError("emitted ranges require an explicit target owner")
        content = read_file(
            repo.root,
            linked_path.relative_to(repo.root).as_posix(),
            max_bytes=64 * 1024 * 1024,
        )
        image = read_linked_image(content, limit=64 * 1024 * 1024)
        function = read_function_bytes(
            content, function_name=function_name, address=address
        )
        if function.content != compiled_bytes:
            raise ValueError("linked function changed during emitted-byte capture")
        allowed = {function.section, *section_addresses, ".reginfo", ".MIPS.abiflags"}
        if any(
            section.flags & 2 and section.size and section.name not in allowed
            for section in image.sections
        ):
            raise ValueError("linked runtime data has no reviewed placement")
        selector = f"{manifest.id.value}@0x{address:08X}"
        source_name = source_path.relative_to(repo.root).as_posix()
        binary = read_file(repo.root, manifest.binary, max_bytes=64 * 1024 * 1024)
        original_payload = payload_for(
            binary, load_address, binary_name=manifest.binary
        )
        ranges = [
            EmittedRange(
                source=source_name,
                address=address,
                original_file_start=original_payload.binary_offset
                + address
                - load_address,
                content=compiled_bytes,
                owners=(selector,),
                symbol=function_name,
            )
        ]
        for placement in placements:
            sections = [row for row in image.sections if row.name == placement.section]
            if len(sections) != 1:
                raise ValueError("linked image omits or duplicates reviewed data")
            section = sections[0]
            if (
                section.kind != 1
                or not section.flags & 2
                or section.flags & (4 | 0xC00)
                or section.address != placement.address
                or section.size != placement.size
                or section.content is None
                or any(
                    row["target_section"] == section.index for row in image.relocations
                )
            ):
                raise ValueError("linked initialized data has unsupported placement")
            ranges.append(
                EmittedRange(
                    source=source_name,
                    address=section.address,
                    original_file_start=original_payload.binary_offset
                    + section.address
                    - load_address,
                    content=section.content,
                    owners=(selector,),
                    section=section.name,
                )
            )
        emitted.extend(ranges)
    current_size = len(compiled_bytes)
    image_evidence = {
        "original_sha256": hashlib.sha256(original_bytes).hexdigest(),
        "current_sha256": hashlib.sha256(compiled_bytes).hexdigest(),
    }
    if not request.diagnostics:
        payload = {
            "schema": "harness.byte-match-one/v1",
            "status": "exact_match" if byte_match else "different",
            "exact_match": byte_match,
            "byte_match": byte_match,
            "source": str(source_path),
            "function": function_name,
            "address": format_hex(address),
            "original_size": original_size,
            "current_size": current_size,
            "size_delta": None
            if current_size is None
            else current_size - original_size,
            "original_binary": str(binary_path),
            "current_object": str(object_path),
            "outputs": {},
        }
        if request.image_evidence:
            payload["function_image"] = image_evidence
        return payload

    shutil.copyfile(current_compiler_asm, output_dir / "compiler.s")

    original_objdump = disassemble_original(
        objdump_path=objdump_path,
        original_bytes_path=original_bytes_path,
        address=address,
    )
    linked_objdump = disassemble_linked(
        objdump_path=objdump_path,
        linked_path=linked_path,
        address=address,
        size=len(compiled_bytes),
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
    if request.image_evidence:
        payload["function_image"] = image_evidence
    write_json(output_dir / "summary.json", payload)
    return payload
