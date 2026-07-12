"""Lift/accept workflow for canonical target manifests."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import shutil
from typing import Any

from ..domain import load_target_manifests, normalize_target_id
from .context import build_context


def _target_output(root: Path, target_id: str, address: int) -> Path:
    return root / "out" / "lift" / target_id / f"func_{address:08x}"


def lift_function(
    root: Path,
    target: str,
    address: int,
    *,
    seed: str | None = None,
) -> dict[str, Any]:
    root = root.resolve()
    target_id = normalize_target_id(target)
    manifest = load_target_manifests(root).get(target_id.value)
    if manifest is None:
        raise ValueError(f"unknown target: {target}")
    binary = root / manifest.binary
    if not binary.is_file():
        raise FileNotFoundError(f"target binary not found: {binary}")
    if (
        address < manifest.load_address
        or address >= manifest.load_address + binary.stat().st_size
    ):
        raise ValueError(f"address 0x{address:08x} is outside {target_id.value}")
    output = _target_output(root, target_id.value, address)
    output.mkdir(parents=True, exist_ok=True)
    source = root / manifest.source_dir / f"func_{address:08x}.c"
    existing = source.read_text(encoding="utf-8") if source.is_file() else ""
    if not existing:
        existing = (
            '#include "internal.h"\n\n'
            "/* @behavior Pending analysis.\n"
            f" * @source 0x{address:08x} func_{address:08x}\n"
            " */\n"
            f"void func_{address:08x}(void) {{\n}}\n"
        )
    candidate = output / "candidate.c"
    candidate.write_text(existing, encoding="utf-8")
    binary_offset = address - manifest.load_address
    original = binary.read_bytes()[binary_offset : binary_offset + 4]
    (output / "original.s").write_text(
        f"/* raw bytes at 0x{address:08x}: {original.hex()} */\n", encoding="utf-8"
    )
    context = (
        build_context(root, target_id.value, address) if source.is_file() else None
    )
    (output / "context.c").write_text(
        (root / context["output"] / "prelude.c").read_text(encoding="utf-8")
        if context
        else existing,
        encoding="utf-8",
    )
    compile_script = output / "compile.sh"
    compile_script.write_text(
        "#!/bin/sh\nset -eu\n"
        f"# Harness profile: {manifest.profile}\n"
        "# Compiler invocation is resolved by the target build manifest.\n",
        encoding="utf-8",
    )
    compile_script.chmod(0o755)
    metadata = {
        "schema": "harness.lift/v1",
        "target": target_id.value,
        "disc_id": manifest.disc_id,
        "function": f"{target_id.value}@{address:08x}",
        "address": f"0x{address:08x}",
        "profile": manifest.profile,
        "seed": seed,
        "candidate": str(candidate.relative_to(root)),
        "binary_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
    }
    (output / "metadata.json").write_text(
        json.dumps(metadata, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return metadata


def accept_candidate(root: Path, candidate: Path, target: str, address: int) -> Path:
    root = root.resolve()
    target_id = normalize_target_id(target)
    manifest = load_target_manifests(root).get(target_id.value)
    if manifest is None:
        raise ValueError(f"unknown target: {target}")
    text = candidate.read_text(encoding="utf-8")
    if "Pending analysis" in text or "PERM" in text or "m2c" in text.lower():
        raise ValueError("candidate contains a lift/permuter placeholder")
    if not re.search(rf"\bfunc_{address:08x}\s*\(", text):
        raise ValueError("candidate does not define the requested function")
    definitions = re.findall(
        r"(?m)^\s*(?:[A-Za-z_]\w*\s+|[A-Za-z_]\w*\s*\*)+([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{",
        text,
    )
    if definitions != [f"func_{address:08x}"]:
        raise ValueError("candidate must contain exactly one requested function body")
    if "@behavior" not in text or "@source" not in text:
        raise ValueError(
            "candidate is missing factual @behavior/@source trace metadata"
        )
    destination = root / manifest.source_dir / f"func_{address:08x}.c"
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(candidate, destination)
    return destination
