"""Build parser-safe context artifacts without changing compiler inputs."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from ..domain import load_target_manifests, normalize_target_id


def _hash_parts(parts: list[bytes]) -> str:
    digest = hashlib.sha256()
    for part in parts:
        digest.update(part)
        digest.update(b"\0")
    return digest.hexdigest()


def build_context(
    root: Path,
    target: str,
    address: int,
    *,
    mode: str = "prelude",
) -> dict[str, Any]:
    """Generate a cached context containing declarations and one function."""

    if mode not in {"prelude", "minimal", "full"}:
        raise ValueError("context mode must be prelude, minimal, or full")
    root = root.resolve()
    target_id = normalize_target_id(target)
    manifests = load_target_manifests(root)
    manifest = manifests.get(target_id.value)
    if manifest is None:
        raise ValueError(f"unknown target: {target}")
    source_dir = root / manifest.source_dir
    function_path = source_dir / f"func_{address:08x}.c"
    if not function_path.is_file():
        raise FileNotFoundError(f"lifted function source not found: {function_path}")
    header = source_dir / "internal.h"
    header_text = header.read_text(encoding="utf-8") if header.is_file() else ""
    function_text = function_path.read_text(encoding="utf-8")
    include_dir = root / "include"
    shared_headers = [
        path for path in sorted(include_dir.rglob("*.h")) if path.is_file()
    ]
    header_bytes = [
        mode.encode(),
        manifest.profile.encode(),
        header_text.encode(),
        function_text.encode(),
    ]
    header_bytes.extend(path.read_bytes() for path in shared_headers)
    context_hash = _hash_parts(header_bytes)
    output = root / "out" / "context" / target_id.value / manifest.profile
    output.mkdir(parents=True, exist_ok=True)
    prelude = header_text
    if prelude and not prelude.endswith("\n"):
        prelude += "\n"
    prelude += "\n" + function_text
    parser_context = prelude
    if mode == "minimal":
        parser_context = function_text
    elif mode == "full":
        parser_context = (
            "\n\n".join(path.read_text(encoding="utf-8") for path in shared_headers)
            + "\n\n"
            + prelude
        )
    (output / "prelude.c").write_text(prelude, encoding="utf-8")
    (output / "parser-context.c").write_text(parser_context, encoding="utf-8")
    (output / "translation-unit.i").write_text(prelude, encoding="utf-8")
    declarations = {
        "target": target_id.value,
        "function": f"{target_id.value}@{address:08x}",
        "headers": [str(path.relative_to(root)) for path in shared_headers],
        "source": str(function_path.relative_to(root)),
        "mode": mode,
    }
    (output / "declarations.json").write_text(
        json.dumps(declarations, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    metadata = {
        "schema": "harness.context/v1",
        "target": target_id.value,
        "profile": manifest.profile,
        "address": f"0x{address:08x}",
        "mode": mode,
        "context_hash": context_hash,
        "output": str(output.relative_to(root)),
        "compiler_faithful_input": str(
            (output / "translation-unit.i").relative_to(root)
        ),
        "transformations": [
            "target internal.h declarations retained",
            "shared include declarations concatenated for parser context",
            f"context mode applied: {mode}",
        ],
    }
    (output / "metadata.json").write_text(
        json.dumps(metadata, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return metadata
