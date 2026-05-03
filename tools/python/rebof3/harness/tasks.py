from __future__ import annotations

from pathlib import Path
import re
from typing import Any

from ..jsonio import read_json
from .config import HarnessConfig


SOURCE_ADDRESS_RE = re.compile(r"@source:\s*(0x[0-9a-fA-F]+|[0-9a-fA-F]{8})")
FUNC_NAME_RE = re.compile(r"func_([0-9a-fA-F]{8})")


def _parse_int(value: str) -> int:
    return int(value, 0) if value.lower().startswith("0x") else int(value, 16)


def _source_address(path: Path) -> int | None:
    text = path.read_text(encoding="utf-8")
    if match := SOURCE_ADDRESS_RE.search(text):
        return _parse_int(match.group(1))
    if match := FUNC_NAME_RE.search(path.stem):
        return int(match.group(1), 16)
    return None


def _source_program_path(relative_path: Path) -> str:
    parts = relative_path.parts
    if parts[:2] == ("src", "core") or parts[:2] == ("src", "boot"):
        return "/boot/SLUS_004.22"
    if parts[:2] == ("src", "logo") or parts[:3] == ("src", "modules", "logo"):
        return "/boot/LOGO/LOGO.EXE"
    if parts[:4] == ("src", "modules", "battle", "03"):
        return "/bins/BIN/BATTLE/BATTLE/03.bin"
    if parts[:4] == ("src", "modules", "battle", "15"):
        return "/bins/BATTLE/BATTLE/15.bin"
    return "/bins/" + "/".join(parts[2:-1] + (relative_path.stem + ".bin",))


def _source_hint(relative_path: Path) -> str | None:
    parts = relative_path.parts
    if parts[:4] == ("src", "modules", "battle", "03"):
        return "build/extracted/BIN/BATTLE/BATTLE.EMI#3"
    if parts[:4] == ("src", "modules", "battle", "15"):
        return "build/extracted/BIN/BATTLE/BATTLE.EMI#15"
    if parts[:2] == ("src", "core") or parts[:2] == ("src", "boot"):
        return "build/extracted/SLUS_004.22"
    if parts[:2] == ("src", "logo") or parts[:3] == ("src", "modules", "logo"):
        return "build/extracted/LOGO/LOGO.EXE"
    return None


def _source_binary_payload(
    config: HarnessConfig, relative_path: Path
) -> dict[str, Any]:
    parts = relative_path.parts
    if parts[:4] == ("src", "modules", "battle", "03"):
        return {
            "binary_path": str(config.root / "out/emi_raw/BIN/BATTLE/BATTLE/3.bin"),
            "load_address": 0x801D0C00,
        }
    if parts[:4] == ("src", "modules", "battle", "15"):
        return {
            "binary_path": str(config.root / "out/emi_raw/BIN/BATTLE/BATTLE/15.bin"),
            "load_address": 0x80096800,
        }
    return {}


def _source_function_size(
    config: HarnessConfig, *, program_path: str, address: int
) -> int | None:
    if not config.raw_ghidra_export.is_file():
        return None
    payload = read_json(config.raw_ghidra_export)
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        return None
    address_hex = f"{address:08x}"
    for row in rows:
        if not isinstance(row, dict):
            continue
        if row.get("kind") != "function":
            continue
        if str(row.get("program_path") or "") != program_path:
            continue
        if str(row.get("address") or "").lower().removeprefix("0x") != address_hex:
            continue
        body_min = str(row.get("body_min") or "").lower().removeprefix("0x")
        body_max = str(row.get("body_max") or "").lower().removeprefix("0x")
        if not body_min or not body_max:
            return None
        return int(body_max, 16) - int(body_min, 16) + 1
    return None


def source_function_payload(config: HarnessConfig, source_path: Path) -> dict[str, Any]:
    resolved = source_path if source_path.is_absolute() else config.root / source_path
    try:
        relative_path = resolved.resolve().relative_to(config.root / "bof3")
    except ValueError:
        return {}
    address = _source_address(resolved.resolve())
    program_path = _source_program_path(relative_path)
    payload = {
        "source_path": str(resolved.resolve().relative_to(config.root)),
        **_source_binary_payload(config, relative_path),
    }
    if address is not None:
        size = _source_function_size(
            config, program_path=program_path, address=address
        )
        if size is not None:
            payload["size"] = size
    return {
        **payload,
    }


def source_function_target_id(config: HarnessConfig, source_path: Path) -> str | None:
    resolved = source_path if source_path.is_absolute() else config.root / source_path
    try:
        relative_path = resolved.resolve().relative_to(config.root / "bof3")
    except ValueError:
        return None
    return f"func-src:{relative_path.as_posix()}"


def source_function_target_records(config: HarnessConfig) -> list[dict[str, Any]]:
    source_root = config.root / "bof3" / "src"
    if not source_root.is_dir():
        return []
    records: list[dict[str, Any]] = []
    for source_path in sorted(source_root.rglob("*.c")):
        address = _source_address(source_path)
        if address is None:
            continue
        relative_path = source_path.relative_to(config.root / "bof3")
        source_hint = _source_hint(relative_path)
        priority = 25 if source_hint and "BATTLE.EMI#3" in source_hint else 40
        payload = source_function_payload(config, source_path)
        records.append(
            {
                "id": f"func-src:{relative_path.as_posix()}",
                "type": "function",
                "status": "queued",
                "priority": priority,
                "summary": f"{relative_path.as_posix()} 0x{address:08x}",
                "source_hint": source_hint,
                "program_path": _source_program_path(relative_path),
                "entry_hex": f"0x{address:08x}",
                "payload": payload,
            }
        )
    return records


def function_target_records(config: HarnessConfig) -> list[dict[str, Any]]:
    records = source_function_target_records(config)
    seen = {
        (record.get("program_path"), record.get("entry_hex"))
        for record in records
        if record.get("program_path") and record.get("entry_hex")
    }
    if not config.function_index.is_file():
        return records
    payload = read_json(config.function_index)
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        return records

    for row in rows:
        if not isinstance(row, dict):
            continue
        program_path = str(row.get("program_path") or "")
        entry_hex = str(row.get("entry_hex") or row.get("entry") or "")
        if not program_path or not entry_hex:
            continue
        key = (program_path, entry_hex)
        if key in seen:
            continue
        seen.add(key)
        source_hint = row.get("source_hint")
        priority = 35 if source_hint and "BATTLE.EMI#3" in str(source_hint) else 60
        records.append(
            {
                "id": f"func:{program_path}@{entry_hex}",
                "type": "function",
                "status": "queued",
                "priority": priority,
                "summary": f"{program_path} {entry_hex} {row.get('name') or ''}".strip(),
                "source_hint": source_hint,
                "program_path": program_path,
                "entry_hex": entry_hex,
                "payload": dict(row),
            }
        )
    return records


def migration_target_records(config: HarnessConfig) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for target in config.migration_targets:
        records.append(
            {
                "id": f"migration:{target.id}",
                "type": "migration",
                "status": "queued",
                "priority": 20 if target.id == "battle_03" else 50,
                "summary": f"migrate {target.label}",
                "source_hint": target.source_hint,
                "program_path": target.program_path,
                "payload": {
                    "label": target.label,
                    "source_dir": str(target.source_dir),
                    "source_hint": target.source_hint,
                },
            }
        )
    return records


def compact_target_row(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "id": row["id"],
        "type": row["type"],
        "status": row["status"],
        "priority": row["priority"],
        "summary": row["summary"],
        "source_hint": row.get("source_hint"),
        "program_path": row.get("program_path"),
        "entry_hex": row.get("entry_hex"),
    }
