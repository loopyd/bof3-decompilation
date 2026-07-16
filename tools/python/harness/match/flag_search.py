from __future__ import annotations

import json
import os
import re
import shlex
import subprocess
import tempfile
from pathlib import Path
from typing import Any

from .asm_diff import (
    AsmDiffRequest,
    matching_instruction_count,
    run_asm_diff_one,
)
from ._asm_disasm import extract_instructions, disassemble_linked
from ._asm_link import function_bytes_match
from ..io import RepoLayout


OPTIMIZATION_RE = re.compile(r"^-O(?:[0-3s]|fast)$")


def _compile_command(layout: RepoLayout, source: Path) -> tuple[list[str], Path]:
    database = layout.root / "compile_commands.json"
    if not database.is_file():
        raise FileNotFoundError(f"missing {database}; run `just build` first")
    rows = json.loads(database.read_text(encoding="utf-8"))
    resolved = source.resolve()
    matches = [row for row in rows if Path(row.get("file", "")).resolve() == resolved]
    if len(matches) != 1:
        raise ValueError(
            f"expected one compile command for {source}, found {len(matches)}"
        )
    row = matches[0]
    command = row.get("arguments")
    if command is None:
        command = shlex.split(row["command"])
    return list(command), Path(row["directory"])


def _with_candidate(command: list[str], flags: list[str], output: Path) -> list[str]:
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
    *, layout: RepoLayout, source: Path, catalog_path: Path
) -> dict[str, Any]:
    source = source.expanduser().resolve()
    baseline = run_asm_diff_one(AsmDiffRequest(source_path=source), layout=layout)
    original_path = Path(baseline["outputs"]["original"])
    original_size = baseline["original_size"]
    original_bytes = Path(baseline["outputs"]["original_bytes"]).read_bytes()
    address = int(baseline["address"], 16)
    original = [
        line for line in original_path.read_text(encoding="utf-8").splitlines() if line
    ]
    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    command, cwd = _compile_command(layout, source)
    objdump = os.environ.get(
        "PSX_OBJDUMP",
        str(layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-objdump"),
    )
    results: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="harness-flags-") as tmp:
        for index, raw_flags in enumerate(catalog["candidates"]):
            flags = [str(flag) for flag in raw_flags]
            object_path = Path(tmp) / f"candidate-{index}.o"
            compile_result = subprocess.run(
                _with_candidate(command, flags, object_path),
                cwd=cwd,
                capture_output=True,
                text=True,
            )
            if compile_result.returncode != 0:
                results.append(
                    {"flags": flags, "status": "compile_error", "match_percent": 0.0}
                )
                continue
            try:
                byte_match, _compiled = function_bytes_match(
                    object_path,
                    address=address,
                    size=original_size,
                    original_bytes=original_bytes,
                    symbols_c_path=source.parent / "symbols.c",
                    layout=layout,
                )
                linked_path = object_path.with_suffix(".linked.o")
                linked_dump = disassemble_linked(
                    objdump_path=Path(objdump), linked_path=linked_path
                )
                current = extract_instructions(linked_dump)
                matches = matching_instruction_count(original, current)
                percent = round(
                    (matches / max(len(original), len(current), 1)) * 100, 2
                )
                status = "exact_match" if byte_match else "different"
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
    results.sort(key=lambda row: (-row["match_percent"], row["flags"]))
    return {
        "schema": "harness.compiler-flag-search/v1",
        "source": str(source),
        "catalog": str(catalog_path),
        "exact_matches": [row for row in results if row["status"] == "exact_match"],
        "results": results,
    }
