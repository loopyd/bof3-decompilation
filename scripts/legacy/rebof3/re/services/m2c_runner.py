from __future__ import annotations

import sys
from pathlib import Path
from typing import Any

from ...common import relative_to_root, run_command, write_text_output
from ...config import M2C_SCRIPT
from .asm_normalize import AddressSymbolResolver
from .ghidra.decomp_helpers import rewrite_asm_for_m2c


def build_m2c_command(
    asm_path: Path,
    *,
    context_paths: list[Path] | None = None,
    global_decls: str = "used",
    passes: int = 3,
) -> list[str]:
    command = [
        sys.executable,
        str(M2C_SCRIPT),
        "-t",
        "mipsel-gcc-c",
        "--globals",
        global_decls,
        "--passes",
        str(passes),
    ]
    for context_path in context_paths or []:
        command.extend(["--context", str(context_path)])
    command.append(str(asm_path))
    return command


def run_m2c_sidecar(
    *,
    asm_text: str,
    rewritten_asm_path: Path,
    output_path: Path,
    resolver: AddressSymbolResolver | None = None,
    context_paths: list[Path] | None = None,
) -> dict[str, Any]:
    rewritten_asm = rewrite_asm_for_m2c(asm_text, resolver=resolver)
    write_text_output(rewritten_asm_path, rewritten_asm)

    result = run_command(build_m2c_command(rewritten_asm_path, context_paths=context_paths))
    metadata: dict[str, Any] = {
        "attempted": True,
        "path": None,
        "status": "pending",
        "stderr": None,
    }
    if result.returncode == 0:
        write_text_output(output_path, result.stdout)
        metadata["path"] = relative_to_root(output_path)
        metadata["status"] = "ok"
        return metadata

    metadata["status"] = "failed"
    metadata["stderr"] = (result.stderr or result.stdout).strip() or None
    return metadata


__all__ = ["build_m2c_command", "run_m2c_sidecar"]
