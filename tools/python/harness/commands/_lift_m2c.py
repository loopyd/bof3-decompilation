"""m2ctx/m2c command handlers.

Input preparation (splat assembly lookup, deterministic context rendering,
and glabel extraction) lives in ``toolchain.m2c``; selector resolution lives
in ``commands._common``. This module only adapts parsed arguments.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys

from ..io import repo_layout
from ..toolchain.m2c import (
    M2cToolchain,
    asm_label,
    context_path,
    render_context,
    splat_assembly,
)
from ._common import resolve_function_selector


def run_m2ctx(args: argparse.Namespace) -> int:
    function, manifest, _ = resolve_function_selector(args.function)
    destination = Path(args.out) if args.out else context_path(function)
    if not destination.is_absolute():
        destination = repo_layout().root / destination
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(render_context(function, manifest), encoding="utf-8")
    if args.json:
        print(
            json.dumps(
                {
                    "target": manifest.id.value,
                    "address": f"0x{function.address:08X}",
                    "context": str(destination),
                },
                sort_keys=True,
            )
        )
    else:
        print(destination)
    return 0


def run_m2c(args: argparse.Namespace) -> int:
    function, manifest, _ = resolve_function_selector(args.function)
    assembly = splat_assembly(manifest, function.address)
    context = context_path(function)
    context.parent.mkdir(parents=True, exist_ok=True)
    context.write_text(render_context(function, manifest), encoding="utf-8")
    root = repo_layout().root
    m2c = M2cToolchain(root)
    arguments: list[str] = [
        "-t",
        "mipsel-gcc-c",
        "-f",
        asm_label(assembly),
        "--globals",
        "used",
        "--valid-syntax",
        "--knr",
        "--deterministic-vars",
        "--comment-style",
        "multiline",
        "--context",
        str(context),
    ]
    for extra in args.context:
        arguments.extend(("--context", extra))
    if args.void:
        arguments.append("--void")
    arguments.append(str(assembly))
    result = m2c.execute(arguments, capture_output=True, text=True)
    seed = re.sub(
        r"\bfunc_([0-9A-Fa-f]{8})\b",
        lambda match: f"func_{match.group(1).upper()}",
        result.stdout,
    )
    if args.out is not None:
        output = Path(args.out)
        if not output.is_absolute():
            output = root / output
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(seed, encoding="utf-8")
    else:
        print(seed, end="")
    if result.stderr:
        print(result.stderr, end="", file=sys.stderr)
    if result.returncode:
        return result.returncode
    return 0
