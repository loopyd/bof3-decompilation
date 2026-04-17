from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from ..common import ROOT as COMMON_ROOT, format_hex, parse_hexish


ROOT = COMMON_ROOT


def relative_to_root(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return str(path.resolve())


def normalize_repo_path(path_text: str | None) -> Path | None:
    if not path_text:
        return None
    path = Path(path_text)
    return path if path.is_absolute() else ROOT / path


def load_bundle(path: Path) -> dict[str, Any]:
    return dict(json.loads(path.read_text(encoding="utf-8")))


def baseline_from_bundle_json(path: Path) -> dict[str, Any] | None:
    if not path.exists():
        return None
    bundle = load_bundle(path)
    files = bundle.get("files") or {}
    function = bundle.get("function") or {}
    asm_path = normalize_repo_path(files.get("asm"))
    if asm_path is None or not asm_path.exists():
        return None

    entry_text = function.get("entry") or bundle.get("requested_address")
    body_min = function.get("body_min")
    body_max = function.get("body_max")

    return {
        "kind": "ghidra_decomp_function",
        "bundle_json": relative_to_root(path),
        "asm_source": relative_to_root(asm_path),
        "program_name": bundle.get("program_name"),
        "requested_address": bundle.get("requested_address"),
        "entry_hex": None
        if entry_text is None
        else format_hex(parse_hexish(str(entry_text))),
        "symbol_name": function.get("name"),
        "signature": function.get("signature"),
        "status": function.get("status"),
        "decompile_status": function.get("decompile_status"),
        "body_min_hex": None
        if body_min is None
        else format_hex(parse_hexish(str(body_min))),
        "body_max_hex": None
        if body_max is None
        else format_hex(parse_hexish(str(body_max))),
    }
