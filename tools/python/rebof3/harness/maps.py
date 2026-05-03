from __future__ import annotations

from pathlib import Path
from typing import Any

from ..jsonio import read_json, write_json
from .binary import resolve_binary_pair
from .config import HarnessConfig
from .workspace import safe_name


def _rows(path: Path) -> list[dict[str, Any]]:
    if not path.is_file():
        return []
    payload = read_json(path)
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        return []
    return [dict(row) for row in rows if isinstance(row, dict)]


def function_rows(config: HarnessConfig) -> list[dict[str, Any]]:
    return _rows(config.function_index)


def raw_ghidra_rows(config: HarnessConfig) -> list[dict[str, Any]]:
    return _rows(config.raw_ghidra_export)


def _raw_module_metadata_path(compiled_bin: Path | None) -> Path | None:
    if compiled_bin is None:
        return None
    return compiled_bin.with_suffix(compiled_bin.suffix + ".json")


def _entry_hex(value: Any) -> str | None:
    if value is None:
        return None
    if isinstance(value, int):
        return f"0x{value:08x}"
    text = str(value).strip()
    if not text:
        return None
    try:
        return f"0x{int(text, 0):08x}"
    except ValueError:
        pass
    try:
        return f"0x{int(text, 16):08x}"
    except ValueError:
        return text.lower()


def _function_name_from_object(value: Any, fallback: str) -> str:
    name = Path(str(value or "")).name
    if name.endswith(".c.obj"):
        return name[: -len(".c.obj")]
    if name:
        return Path(name).stem
    return fallback


def raw_module_functions(compiled_bin: Path | None) -> list[dict[str, Any]]:
    metadata_path = _raw_module_metadata_path(compiled_bin)
    if metadata_path is None or not metadata_path.is_file():
        return []
    payload = read_json(metadata_path)
    placements = payload.get("placements", [])
    if not isinstance(placements, list):
        return []
    functions: list[dict[str, Any]] = []
    for placement in placements:
        if not isinstance(placement, dict):
            continue
        entry_hex = _entry_hex(placement.get("address"))
        if entry_hex is None:
            continue
        functions.append(
            {
                "address": entry_hex,
                "entry_hex": entry_hex,
                "kind": "function",
                "name": _function_name_from_object(
                    placement.get("object"), f"func_{entry_hex[2:]}"
                ),
                "object": placement.get("object"),
                "offset": placement.get("offset"),
                "original_size": placement.get("original_size"),
                "size": placement.get("size"),
                "source": "raw-module-metadata",
                "truncated": bool(placement.get("truncated")),
            }
        )
    return functions


def _same_target(row: dict[str, Any], target: dict[str, Any]) -> bool:
    source_hint = str(target.get("source_hint") or "")
    program_paths = _target_program_paths(target)
    return (
        bool(source_hint and str(row.get("source_hint") or "") == source_hint)
        or str(row.get("program_path") or "") in program_paths
    )


def _target_program_paths(target: dict[str, Any]) -> set[str]:
    paths = {str(target.get("program_path") or "")}
    payload = target.get("payload") if isinstance(target.get("payload"), dict) else {}
    archive_id = str(payload.get("archive_id") or "")
    entry_name = str(payload.get("entry_name") or "")
    if archive_id and entry_name:
        paths.add(f"/bins/{archive_id}/{entry_name}")
    return {path for path in paths if path}


def _symbol_row(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "address": row.get("address") or row.get("entry") or row.get("entry_hex"),
        "kind": row.get("kind") or "symbol",
        "name": row.get("name"),
        "program_path": row.get("program_path"),
        "source": "raw-ghidra-export",
    }


def _xref_row(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "from_address": row.get("from_address") or row.get("from"),
        "kind": row.get("kind") or "xref",
        "name": row.get("name"),
        "program_path": row.get("program_path"),
        "source": "raw-ghidra-export",
        "to_address": row.get("to_address") or row.get("to") or row.get("address"),
    }


def build_binary_map(config: HarnessConfig, target: dict[str, Any]) -> dict[str, Any]:
    pair = resolve_binary_pair(config, target)
    functions: list[dict[str, Any]] = [
        {
            "address": _entry_hex(row.get("entry_hex") or row.get("entry")),
            "entry": row.get("entry"),
            "entry_hex": _entry_hex(row.get("entry_hex") or row.get("entry")),
            "kind": "function",
            "name": row.get("name"),
            "program_path": row.get("program_path"),
            "signature": row.get("signature"),
            "source": "ghidra-function-index",
        }
        for row in function_rows(config)
        if _same_target(row, target)
    ]
    seen_functions = {
        entry
        for function in functions
        if (entry := _entry_hex(function.get("entry_hex") or function.get("address")))
    }
    for function in raw_module_functions(pair.compiled):
        entry = _entry_hex(function.get("entry_hex") or function.get("address"))
        if entry is None or entry in seen_functions:
            continue
        seen_functions.add(entry)
        functions.append(function)
    raw_rows = [row for row in raw_ghidra_rows(config) if _same_target(row, target)]
    symbols = [
        _symbol_row(row)
        for row in raw_rows
        if str(row.get("kind") or "").lower() not in {"xref", "reference"}
    ]
    xrefs = [
        _xref_row(row)
        for row in raw_rows
        if str(row.get("kind") or "").lower() in {"xref", "reference"}
        or row.get("from_address")
        or row.get("to_address")
    ]
    return {
        "schema": "rebof3-simple.harness-binary-map/v1",
        "function_count": len(functions),
        "functions": functions,
        "original_bin": None if pair.original is None else str(pair.original),
        "compiled_bin": None if pair.compiled is None else str(pair.compiled),
        "source_hint": target.get("source_hint"),
        "symbol_count": len(symbols),
        "symbols": symbols,
        "target_id": target["id"],
        "xref_count": len(xrefs),
        "xrefs": xrefs,
    }


def write_binary_map(
    config: HarnessConfig, target: dict[str, Any], *, output_root: Path
) -> tuple[dict[str, Any], Path]:
    payload = build_binary_map(config, target)
    output_dir = output_root / safe_name(str(target["id"]))
    output_dir.mkdir(parents=True, exist_ok=True)
    path = output_dir / "binary-map.json"
    write_json(path, payload)
    return payload, path
