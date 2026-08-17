"""Search the compiler-flag catalog for one target-qualified function."""

from __future__ import annotations

import json
import os
import re
import shlex
import subprocess
import tempfile
from pathlib import Path
from typing import Any

from ..domain.symbols import load_target_symbols
from ..domain.claims import manifest_binding_sources
from ..domain.sources import owning_manifest
from ..io import RepoLayout
from ..toolchain.gcc_variants import EmptyCatalog, lookup_variant
from ._asm_disasm import extract_instructions, disassemble_linked
from ._asm_link import function_bytes_match
from ._asm_diff_payload import AsmDiffRequest, matching_instruction_count
from .asm_diff import run_asm_diff_one

OPTIMIZATION_RE = re.compile(r"^-O(?:[0-3s]|fast)$")
CMAKE_ENV_ASSIGNMENT_RE = re.compile(r"^[^=]+=.*$")


def _compile_command(layout: RepoLayout, source: Path) -> tuple[list[str], Path]:
    database = layout.root / "compile_commands.json"
    if not database.is_file():
        raise FileNotFoundError(f"missing {database}; run `just build` first")
    rows = json.loads(database.read_text(encoding="utf-8"))
    resolved = source.resolve()
    matches = [row for row in rows if Path(row.get("file", "")).resolve() == resolved]
    if len(matches) != 1:
        raise ValueError(
            f"expected 1 compile command for {source}, found {len(matches)}"
        )
    row = matches[0]
    command = row.get("arguments")
    if command is None:
        command = shlex.split(row["command"])
    return list(command), Path(row["directory"])


def _strip_embedded_psx_gcc(command: list[str]) -> list[str]:
    """Remove PSX_GCC only from the leading ``cmake -E env`` assignments."""
    if command[:3] != ["cmake", "-E", "env"]:
        return command
    index = 3
    env: list[str] = []
    while index < len(command) and CMAKE_ENV_ASSIGNMENT_RE.match(command[index]):
        if not command[index].startswith("PSX_GCC="):
            env.append(command[index])
        index += 1
    return [*command[:3], *env, *command[index:]]


def _resolve_bindings(
    layout: RepoLayout, source: Path
) -> tuple[Path, dict[str, int] | None]:
    """Resolve the WEAK_SYMBOL_AT binding file and canonical map for linking.

    Migrated targets: the claimed hand-maintained support binding source plus
    the composed target map (target-qualified, never path ancestry).  Legacy
    targets: ``source.parent/symbols.c`` with no canonical map.
    """

    manifest = owning_manifest(layout.root, source)
    if manifest is not None:
        binding_files = manifest_binding_sources(layout.root, manifest)
        symbols_c_path = (
            binding_files[0]
            if binding_files
            else layout.root / manifest.source_dir / "symbols.c"
        )
        canonical_bindings = {
            symbol.canonical_name: symbol.address
            for symbol in load_target_symbols(layout.root, manifest.id.value)
        }
        return symbols_c_path, canonical_bindings
    return source.parent / "symbols.c", None


def _with_candidate(command: list[str], flags: list[str], output: Path) -> list[str]:
    """Replace canonical -O flags with candidate flags and set output path."""
    result: list[str] = []
    skip = False
    for arg in command:
        if skip:
            skip = False
            continue
        if arg == "-o":
            skip = True
            continue
        if arg.startswith("-o") and len(arg) > 2:
            continue
        if OPTIMIZATION_RE.match(arg):
            continue
        result.append(arg)
    result.extend([*flags, "-o", str(output)])
    return result


def search_flags(
    *,
    layout: RepoLayout,
    source: Path,
    catalog_path: Path,
    compiler_id: str | None = None,
) -> dict[str, Any]:
    source = source.expanduser().resolve()
    baseline = run_asm_diff_one(AsmDiffRequest(source_path=source), layout=layout)
    original_path = Path(baseline["outputs"]["original"])
    original_size = baseline["original_size"]
    original_bytes = Path(baseline["outputs"]["original_bytes"]).read_bytes()
    original = original_path.read_text(encoding="utf-8").splitlines()
    address = int(baseline["address"], 16)
    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    objdump = os.environ.get(
        "PSX_OBJDUMP",
        str(layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-objdump"),
    )

    # Resolve variant environment when a compiler_id is given.
    variant_env: dict[str, str] = {}
    variant_label = "canonical"
    if compiler_id is not None:
        variant = lookup_variant(layout, compiler_id)
        if isinstance(variant, EmptyCatalog):
            raise ValueError(
                f"compiler variant {compiler_id!r} not available (empty catalog)"
            )
        variant.verify(layout)
        variant_env = {
            "PSX_GCC": str(variant.install_path(layout) / variant.executable_relpath)
        }
        variant_label = variant.label

    cmd, cmd_dir = _compile_command(layout, source)
    if compiler_id is not None:
        cmd = _strip_embedded_psx_gcc(cmd)
    symbols_c_path, canonical_bindings = _resolve_bindings(layout, source)
    results: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="harness-flags-") as tmp:
        work = Path(tmp)
        # candidates is a list of flag lists: [["-O0"], ["-O1"], ...]
        for flags in catalog.get("candidates", []):
            object_path = work / "candidate.o"
            candidate = _with_candidate(cmd, flags, object_path)
            try:
                compile_result = subprocess.run(
                    candidate,
                    cwd=str(cmd_dir),
                    capture_output=True,
                    text=True,
                    env={**os.environ, **variant_env},
                )
            except FileNotFoundError:
                results.append(
                    {"flags": flags, "status": "compile_error", "match_percent": 0.0}
                )
                continue
            if compile_result.returncode != 0:
                results.append(
                    {"flags": flags, "status": "compile_error", "match_percent": 0.0}
                )
                continue
            try:
                match_ok, compiled = function_bytes_match(
                    object_path=object_path,
                    address=address,
                    size=original_size,
                    original_bytes=original_bytes,
                    symbols_c_path=symbols_c_path,
                    canonical_bindings=canonical_bindings,
                    layout=layout,
                )
                linked_path = object_path.with_suffix(".linked.o")
                if match_ok:
                    linked_dump = disassemble_linked(
                        objdump_path=Path(objdump), linked_path=linked_path
                    )
                    current = extract_instructions(linked_dump)
                    matches = matching_instruction_count(original, current)
                    percent = round(
                        (matches / max(len(original), len(current), 1)) * 100, 2
                    )
                    status = "exact_match"
                else:
                    # Bytes differ; still compute instruction percentage
                    try:
                        linked_dump = disassemble_linked(
                            objdump_path=Path(objdump), linked_path=linked_path
                        )
                        current = extract_instructions(linked_dump)
                        matches = matching_instruction_count(original, current)
                        percent = round(
                            (matches / max(len(original), len(current), 1)) * 100, 2
                        )
                    except RuntimeError:
                        percent = 0.0
                    status = "different"
            except RuntimeError:
                results.append(
                    {"flags": flags, "status": "link_error", "match_percent": 0.0}
                )
                continue
            results.append(
                {
                    "flags": flags,
                    "status": status,
                    "match_percent": percent,
                }
            )

    results.sort(key=lambda row: (-row["match_percent"], str(row["flags"])))
    payload: dict[str, Any] = {
        "schema": "harness.compiler-flag-search/v1",
        "source": str(source),
        "catalog": str(catalog_path),
        "exact_matches": [r for r in results if r["status"] == "exact_match"],
        "results": results,
    }
    if compiler_id is not None:
        payload["compiler_id"] = compiler_id
        payload["variant_label"] = variant_label
        payload["address"] = address
        payload["size"] = original_size
    return payload
