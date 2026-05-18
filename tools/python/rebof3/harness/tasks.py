from __future__ import annotations

from pathlib import Path
import re
from typing import Any

from ..jsonio import read_json
from .config import HarnessConfig


SOURCE_ADDRESS_RE = re.compile(r"@source:\s*(0x[0-9a-fA-F]+|[0-9a-fA-F]{8})")
FUNC_NAME_RE = re.compile(r"func_([0-9a-fA-F]{8})")
STAGED_EMI_PROGRAM_RE = re.compile(
    r"^(?P<archive>.+)_e(?P<entry>[0-9]+)_[0-9a-fA-F]{8}\.bin$"
)
FUNCTION_ALIAS_RE = re.compile(
    r"^func:(?P<archive>[^@#]+(?:/[^@#]+)*)#(?P<entry>[0-9]+)@(?P<addr>0x[0-9a-fA-F]+|[0-9a-fA-F]{8})$"
)


def _parse_int(value: str) -> int:
    return int(value, 0) if value.lower().startswith("0x") else int(value, 16)


def _row_address(row: dict[str, Any]) -> int | None:
    value = row.get("entry_hex") or row.get("entry") or row.get("address")
    if value is None:
        return None
    try:
        return _parse_int(str(value))
    except ValueError:
        return None


def is_reverse_function_row(row: dict[str, Any]) -> bool:
    address = _row_address(row)
    if address is None or address < 0x80000000:
        return False
    if str(row.get("name_source") or "").upper() == "IMPORTED":
        return False
    return True


def _function_row_index(config: HarnessConfig) -> dict[int, list[dict[str, Any]]]:
    if not config.function_index.is_file():
        return {}
    payload = read_json(config.function_index)
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        return {}
    index: dict[int, list[dict[str, Any]]] = {}
    for row in rows:
        if not isinstance(row, dict) or not is_reverse_function_row(row):
            continue
        address = _row_address(row)
        if address is None:
            continue
        index.setdefault(address, []).append(dict(row))
    return index


def _source_index_row(
    address: int | None,
    row_index: dict[int, list[dict[str, Any]]] | None,
    source_hint: str | None = None,
) -> dict[str, Any] | None:
    if address is None or row_index is None:
        return None
    rows = row_index.get(address, [])
    if source_hint is not None:
        for row in rows:
            if row.get("source_hint") == source_hint:
                return row
    return rows[0] if len(rows) == 1 else None


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
        return "/boot/LOGO.EXE"
    if parts[:4] == ("src", "modules", "battle", "03"):
        return "/bins/BATTLE/BATTLE/3.bin"
    if parts[:4] == ("src", "modules", "battle", "15"):
        return "/bins/BATTLE/BATTLE/15.bin"
    return "/bins/" + "/".join(parts[2:-1] + (relative_path.stem + ".bin",))


def _source_hint(relative_path: Path) -> str | None:
    parts = relative_path.parts
    if parts[:4] == ("src", "modules", "game", "00"):
        return "output/extracted/BIN/ETC/GAME.EMI#0"
    if parts[:4] == ("src", "modules", "game", "01"):
        return "output/extracted/BIN/ETC/GAME.EMI#1"
    if parts[:4] == ("src", "modules", "battle", "03"):
        return "output/extracted/BIN/BATTLE/BATTLE.EMI#3"
    if parts[:4] == ("src", "modules", "battle", "15"):
        return "output/extracted/BIN/BATTLE/BATTLE.EMI#15"
    if parts[:4] == ("src", "modules", "bate", "03"):
        return "output/extracted/BIN/ETC/BATE.EMI#3"
    if parts[:4] == ("src", "modules", "batl_re2", "01"):
        return "output/extracted/BIN/BATTLE/BATL_RE2.EMI#1"
    if parts[:4] == ("src", "modules", "commu00", "00"):
        return "output/extracted/BIN/ETC/COMMU00.EMI#0"
    if parts[:4] == ("src", "modules", "shop", "00"):
        return "output/extracted/BIN/ETC/SHOP.EMI#0"
    if parts[:4] == ("src", "modules", "sisyou", "00"):
        return "output/extracted/BIN/ETC/SISYOU.EMI#0"
    if parts[:4] == ("src", "modules", "scena00", "00"):
        return "output/extracted/BIN/SCENARIO/SCENA00.EMI#0"
    if parts[:4] == ("src", "modules", "scena16", "00"):
        return "output/extracted/BIN/SCENARIO/SCENA16.EMI#0"
    if parts[:4] == ("src", "modules", "sce10eff", "00"):
        return "output/extracted/BIN/SCENARIO/SCE10EFF.EMI#0"
    if parts[:3] == ("src", "modules", "world00") and len(parts) >= 5:
        return f"output/extracted/BIN/WORLD00/{parts[3].upper()}.EMI#{int(parts[4], 10)}"
    if parts[:2] == ("src", "core") or parts[:2] == ("src", "boot"):
        return "output/extracted/SLUS_004.22"
    if parts[:2] == ("src", "logo") or parts[:3] == ("src", "modules", "logo"):
        return "output/extracted/LOGO/LOGO.EXE"
    return None


def _staged_program_parts(program_path: str) -> tuple[tuple[str, ...], int, int] | None:
    path = Path(program_path)
    match = STAGED_EMI_PROGRAM_RE.match(path.name)
    if not match:
        return None
    parts = tuple(Path(program_path.removeprefix("/bins/")).parent.parts)
    load_address = int(
        match.group(0).removesuffix(".bin").rsplit("_", 1)[1], 16
    )
    return parts, int(match.group("entry"), 10), load_address


def _source_path_for_program(program_path: str, entry_hex: str) -> str | None:
    parsed = _staged_program_parts(program_path)
    if parsed is None:
        if program_path == "/boot/LOGO.EXE":
            return f"bof3/src/modules/logo/func_{entry_hex.removeprefix('0x').lower()}.c"
        if program_path == "/boot/SLUS_004.22":
            return f"bof3/src/core/func_{entry_hex.removeprefix('0x').lower()}.c"
        return None

    parts, entry, _load_address = parsed
    addr = entry_hex.removeprefix("0x").lower()
    if parts == ("ETC", "GAME"):
        return f"bof3/src/modules/game/{entry:02d}/func_{addr}.c"
    if parts == ("BATTLE", "BATTLE"):
        return f"bof3/src/modules/battle/{entry:02d}/func_{addr}.c"
    if len(parts) == 2 and parts[0] == "WORLD00":
        return f"bof3/src/modules/world00/{parts[1].lower()}/{entry:02d}/func_{addr}.c"
    if parts:
        module_parts = "/".join(part.lower() for part in parts)
        return f"bof3/src/modules/{module_parts}/{entry:02d}/func_{addr}.c"
    return None


def _source_hint_for_program(program_path: str) -> str | None:
    parsed = _staged_program_parts(program_path)
    if parsed is None:
        if program_path == "/boot/LOGO.EXE":
            return "output/extracted/LOGO/LOGO.EXE"
        if program_path == "/boot/SLUS_004.22":
            return "output/extracted/SLUS_004.22"
        return None
    parts, entry, _load_address = parsed
    if parts:
        return f"output/extracted/BIN/{'/'.join(parts)}.EMI#{entry}"
    return None


def _binary_payload_for_program(config: HarnessConfig, program_path: str) -> dict[str, Any]:
    if program_path == "/boot/LOGO.EXE":
        return {"binary_path": str(config.root / "output/extracted/LOGO/LOGO.EXE")}
    if program_path == "/boot/SLUS_004.22":
        return {"binary_path": str(config.root / "output/extracted/SLUS_004.22")}
    if program_path.startswith("/bins/"):
        binary_path = _binary_path_from_program(config, program_path)
        load_address = _load_address_from_manifest(binary_path)
        if load_address is None:
            parsed = _staged_program_parts(program_path)
            if parsed is not None:
                load_address = parsed[2]
        return {
            "binary_path": str(binary_path),
            **({} if load_address is None else {"load_address": load_address}),
        }
    return {}


def _binary_path_from_source_hint(config: HarnessConfig, source_hint: str) -> Path | None:
    if "#" in source_hint:
        emi_path, entry = source_hint.rsplit("#", 1)
        bin_dir = emi_path.removesuffix(".EMI")
        return config.root / bin_dir / f"{entry}.bin"
    return config.root / source_hint


def _source_binary_payload(
    config: HarnessConfig, relative_path: Path, program_path: str | None = None
) -> dict[str, Any]:
    hint = _source_hint(relative_path)
    if not hint:
        return {}
    binary_path = _binary_path_from_source_hint(config, hint)
    if not binary_path:
        return {}
    load_address = _load_address_from_manifest(binary_path)
    payload: dict[str, Any] = {"binary_path": str(binary_path)}
    if load_address is not None:
        payload["load_address"] = load_address
    return payload


def _binary_path_from_program(config: HarnessConfig, program_path: str) -> Path:
    parts = list(Path(program_path.removeprefix("/bins/")).parts)
    if parts and parts[0] == "BIN":
        parts = parts[1:]
    raw_path = config.root / "output/extracted/BIN" / Path(*parts)
    if raw_path.is_file() or not parts:
        return raw_path
    match = STAGED_EMI_PROGRAM_RE.match(parts[-1])
    if match:
        return config.root / "output/extracted/BIN" / Path(*parts[:-1]) / (
            f"{int(match.group('entry'), 10)}.bin"
        )
    return raw_path


def _load_address_from_manifest(binary_path: Path) -> int | None:
    manifest = binary_path.parent / "emi.json"
    if not manifest.is_file():
        return None
    payload = read_json(manifest)
    entries = payload.get("entries", [])
    if not isinstance(entries, list):
        return None
    for entry in entries:
        if not isinstance(entry, dict):
            continue
        if str(entry.get("name") or f"{entry.get('index', '')}.bin") == binary_path.name:
            ram_ptr = entry.get("ram_ptr")
            return int(ram_ptr) if ram_ptr is not None else None
    return None


def _function_size_index(config: HarnessConfig) -> dict[tuple[str, int], int]:
    if not config.raw_ghidra_export.is_file():
        return {}
    payload = read_json(config.raw_ghidra_export)
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        return {}
    index: dict[tuple[str, int], int] = {}
    for row in rows:
        if not isinstance(row, dict):
            continue
        if row.get("kind") != "function":
            continue
        program_path = str(row.get("program_path") or "")
        address = _row_address(row)
        if not program_path or address is None:
            continue
        body_min = str(row.get("body_min") or "").lower().removeprefix("0x")
        body_max = str(row.get("body_max") or "").lower().removeprefix("0x")
        if body_min and body_max:
            index[(program_path, address)] = int(body_max, 16) - int(body_min, 16) + 1
    return index


def source_function_payload(
    config: HarnessConfig,
    source_path: Path,
    *,
    size_index: dict[tuple[str, int], int] | None = None,
    row_index: dict[int, list[dict[str, Any]]] | None = None,
) -> dict[str, Any]:
    resolved = source_path if source_path.is_absolute() else config.root / source_path
    try:
        relative_path = resolved.resolve().relative_to(config.root / "bof3")
    except ValueError:
        return {}
    address = _source_address(resolved.resolve())
    rows_by_address = _function_row_index(config) if row_index is None else row_index
    inferred_source_hint = _source_hint(relative_path)
    index_row = _source_index_row(address, rows_by_address, inferred_source_hint)
    program_path = (
        str(index_row.get("program_path"))
        if index_row and index_row.get("program_path")
        else _source_program_path(relative_path)
    )
    payload = {
        "source_path": str(resolved.resolve().relative_to(config.root)),
        "program_path": program_path,
        **_source_binary_payload(config, relative_path, program_path),
    }
    if index_row and index_row.get("source_hint"):
        payload["source_hint"] = index_row.get("source_hint")
    if address is not None:
        sizes = _function_size_index(config) if size_index is None else size_index
        size = sizes.get((program_path, address))
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
    size_index = _function_size_index(config)
    row_index = _function_row_index(config)
    for source_path in sorted(source_root.rglob("*.c")):
        address = _source_address(source_path)
        if address is None:
            continue
        relative_path = source_path.relative_to(config.root / "bof3")
        payload = source_function_payload(
            config, source_path, size_index=size_index, row_index=row_index
        )
        source_hint = payload.get("source_hint") or _source_hint(relative_path)
        program_path = payload.get("program_path") or _source_program_path(relative_path)
        priority = 25 if source_hint and "BATTLE.EMI#3" in source_hint else 40
        records.append(
            {
                "id": f"func-src:{relative_path.as_posix()}",
                "type": "function",
                "status": "queued",
                "priority": priority,
                "summary": f"{relative_path.as_posix()} 0x{address:08x}",
                "source_hint": source_hint,
                "program_path": program_path,
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
        if not is_reverse_function_row(row):
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


def analysis_function_target_records(
    config: HarnessConfig, rows: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for row in rows:
        if not is_reverse_function_row(row):
            continue
        if int(row.get("is_thunk") or 0):
            continue
        program_path = str(row.get("program_path") or "")
        entry_hex = str(row.get("address") or row.get("entry_hex") or "")
        if not program_path or not entry_hex:
            continue
        if not entry_hex.startswith("0x"):
            entry_hex = f"0x{entry_hex.lower()}"
        source_path = _source_path_for_program(program_path, entry_hex)
        source_hint = _source_hint_for_program(program_path)
        body_min = str(row.get("body_min") or "")
        body_max = str(row.get("body_max") or "")
        payload = {
            **dict(row),
            **_binary_payload_for_program(config, program_path),
        }
        if source_path is not None:
            payload["source_path"] = source_path
        if body_min and body_max:
            payload["size"] = _parse_int(body_max) - _parse_int(body_min) + 1
        priority = 50
        if source_path and (config.root / source_path).is_file():
            priority = 35
        if source_hint and "BATTLE/BATTLE.EMI#3" in source_hint:
            priority = 25
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
                "payload": payload,
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
        "alias": function_target_alias(row),
        "type": row["type"],
        "status": row["status"],
        "priority": row["priority"],
        "summary": row["summary"],
        "source_hint": row.get("source_hint"),
        "program_path": row.get("program_path"),
        "entry_hex": row.get("entry_hex"),
    }


def function_target_alias(row: dict[str, Any]) -> str | None:
    if row.get("type") != "function":
        return None
    program_path = str(row.get("program_path") or "")
    entry_hex = str(row.get("entry_hex") or "")
    if not program_path.startswith("/bins/") or not entry_hex:
        return None
    parts = list(Path(program_path.removeprefix("/bins/")).parts)
    if parts and parts[0] == "BIN":
        parts = parts[1:]
    if not parts:
        return None
    filename = parts[-1]
    if filename.endswith(".bin") and filename.removesuffix(".bin").isdigit():
        return f"func:{'/'.join(parts[:-1])}#{filename.removesuffix('.bin')}@{entry_hex}"
    match = STAGED_EMI_PROGRAM_RE.match(filename)
    if match:
        return f"func:{'/'.join(parts[:-1])}#{int(match.group('entry'), 10)}@{entry_hex}"
    return None


def resolve_function_target_alias(
    rows: list[dict[str, Any]], target_id: str
) -> dict[str, Any] | None:
    if FUNCTION_ALIAS_RE.match(target_id) is None:
        return None
    for row in rows:
        if function_target_alias(row) == target_id:
            return row
    return None


def _module_fragments(module: str) -> set[str]:
    raw = module.strip()
    fragments = {raw}
    if raw.upper() == "GAME#0":
        fragments.update(_module_fragments("ETC/GAME#0"))
    elif raw.upper() == "GAME#1":
        fragments.update(_module_fragments("ETC/GAME#1"))
    elif raw.upper().startswith("BATTLE#"):
        fragments.update(_module_fragments(f"BATTLE/BATTLE#{raw.split('#', 1)[1]}"))
    if raw.startswith("emi:"):
        archive_entry = raw.removeprefix("emi:")
    else:
        archive_entry = raw
    if "#" in archive_entry:
        archive, entry = archive_entry.rsplit("#", 1)
        archive_name = Path(archive).name
        try:
            entry_int = int(entry)
        except ValueError:
            return fragments
        fragments.add(archive_entry)
        fragments.add(f"{archive}.EMI#{entry_int}")
        fragments.add(f"/bins/{archive}/{entry_int}.bin")
        fragments.add(f"/bins/{archive}/{entry_int:02d}.bin")
        fragments.add(f"/bins/{archive}/{archive_name}_e{entry_int:02d}_")
        fragments.add(f"/bins/BIN/{archive}/{entry_int}.bin")
        fragments.add(f"/bins/BIN/{archive}/{entry_int:02d}.bin")
    return fragments


def target_matches_module(row: dict[str, Any], module: str | None) -> bool:
    if not module:
        return True
    haystack = "\n".join(
        str(value or "")
        for value in (
            row.get("id"),
            row.get("summary"),
            row.get("source_hint"),
            row.get("program_path"),
        )
    )
    return any(fragment and fragment in haystack for fragment in _module_fragments(module))
