from __future__ import annotations

from pathlib import Path
from typing import Any

from ..jsonio import read_json
from .config import HarnessConfig


def _program_path_from_payload(config: HarnessConfig, payload_path: str) -> str | None:
    path = Path(payload_path)
    try:
        relative = path.resolve().relative_to(config.root / "out" / "extracted" / "BIN")
    except ValueError:
        try:
            relative = path.resolve().relative_to(config.root / "out" / "extracted")
        except ValueError:
            return None
        if relative.as_posix() == "SLUS_004.22":
            return "/boot/SLUS_004.22"
        if relative.as_posix() == "LOGO/LOGO.EXE":
            return "/boot/LOGO.EXE"
        return None
    return "/bins/" + relative.as_posix()


def _program_path_from_manifest_entry(
    config: HarnessConfig, entry: dict[str, Any]
) -> str | None:
    folder = str(entry.get("project_folder_path") or "").rstrip("/")
    name = str(entry.get("program_name") or "")
    if folder and name:
        if folder == "/":
            return "/" + name.lstrip("/")
        return f"{folder}/{name}"

    payload_path = entry.get("payload_path") or entry.get("source")
    if not payload_path:
        return None
    return _program_path_from_payload(config, str(payload_path))


def expected_manifest_program_aliases(config: HarnessConfig) -> dict[str, set[str]]:
    if not (config.root / "out/ghidra-bof3/ghidra_import_manifest.json").is_file():
        return {}
    payload = read_json(config.root / "out/ghidra-bof3/ghidra_import_manifest.json")
    imports = payload.get("imports", [])
    if not isinstance(imports, list):
        return {}

    programs: dict[str, set[str]] = {}
    for entry in imports:
        if not isinstance(entry, dict):
            continue
        canonical = _program_path_from_manifest_entry(config, entry)
        if not canonical:
            continue

        aliases = programs.setdefault(canonical, {canonical})
        payload_path = entry.get("payload_path") or entry.get("source")
        if payload_path:
            raw_program_path = _program_path_from_payload(config, str(payload_path))
            if raw_program_path:
                aliases.add(raw_program_path)
    return programs


def expected_manifest_programs(config: HarnessConfig) -> set[str]:
    return set(expected_manifest_program_aliases(config))


def exported_ghidra_programs(config: HarnessConfig) -> set[str]:
    if not config.raw_ghidra_export.is_file():
        return set()
    payload = read_json(config.raw_ghidra_export)
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        return set()
    return {
        str(row.get("program_path"))
        for row in rows
        if isinstance(row, dict) and row.get("program_path")
    }


def build_ghidra_coverage(config: HarnessConfig) -> dict[str, Any]:
    expected = expected_manifest_program_aliases(config)
    exported = exported_ghidra_programs(config)
    matched = {
        canonical
        for canonical, aliases in expected.items()
        if aliases.intersection(exported)
    }
    known_aliases = {alias for aliases in expected.values() for alias in aliases}
    missing = sorted(set(expected) - matched)
    extra = sorted(exported - known_aliases)
    return {
        "schema": "rebof3-simple.harness-ghidra-coverage/v1",
        "expected_program_count": len(expected),
        "exported_program_count": len(exported),
        "matched_program_count": len(matched),
        "missing_program_count": len(missing),
        "extra_program_count": len(extra),
        "complete": not missing,
        "missing_programs": missing,
        "extra_programs": extra,
    }
