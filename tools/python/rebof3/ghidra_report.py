from __future__ import annotations

import hashlib
import json
import re
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from .jsonio import read_json
from .paths import RepoLayout


PSX_EXE_MAGIC = b"PS-X EXE"
PSX_EXE_HEADER_SIZE = 0x800
STAGED_EMI_PROGRAM_RE = re.compile(
    r"^(?P<archive>.+)_e(?P<entry>[0-9]+)_[0-9a-fA-F]{8}\.bin$"
)
INTERNAL_IDENTIFIER_RE = re.compile(
    r"(?:typedef\s+(?:struct\s+)?(?P<typedef>[A-Za-z_][A-Za-z0-9_]*)|"
    r"#define\s+(?P<define>[A-Za-z_][A-Za-z0-9_]*)|"
    r"(?P<symbol>(?:DAT|PTR|FUN|func)_[0-9A-Za-z_]+))\b"
)


@dataclass(frozen=True)
class BinaryInfo:
    path: Path
    load_address: int
    payload_offset: int
    payload_size: int


def parse_int(value: Any) -> int | None:
    if value is None:
        return None
    text = str(value).strip().lower()
    if not text:
        return None
    try:
        return int(text, 0) if text.startswith("0x") else int(text, 16)
    except ValueError:
        return None


def format_hex(value: int | None) -> str | None:
    return None if value is None else f"0x{value:08x}"


def normalize_address(value: Any) -> str | None:
    parsed = parse_int(value)
    return format_hex(parsed)


def is_reverse_function(row: dict[str, Any]) -> bool:
    address = parse_int(row.get("entry_hex") or row.get("entry") or row.get("address"))
    if address is None or address < 0x80000000:
        return False
    return str(row.get("name_source") or "").upper() != "IMPORTED"


def row_entry(row: dict[str, Any]) -> str | None:
    return normalize_address(row.get("entry_hex") or row.get("entry") or row.get("address"))


def row_body_bounds(row: dict[str, Any]) -> tuple[int, int] | None:
    body_min = parse_int(row.get("body_min"))
    body_max = parse_int(row.get("body_max"))
    if body_min is None or body_max is None or body_max < body_min:
        return None
    return body_min, body_max


def read_psx_exe_info(path: Path) -> tuple[int, int, int] | None:
    header = path.read_bytes()[:PSX_EXE_HEADER_SIZE]
    if not header.startswith(PSX_EXE_MAGIC):
        return None
    load_address = int.from_bytes(header[0x18:0x1C], "little")
    payload_size = int.from_bytes(header[0x1C:0x20], "little")
    return load_address, payload_size, PSX_EXE_HEADER_SIZE


def binary_path_for_program(layout: RepoLayout, program_path: str) -> Path | None:
    if program_path == "/boot/SLUS_004.22":
        return layout.slus_path
    if program_path in {"/boot/LOGO.EXE", "/boot/LOGO/LOGO.EXE"}:
        return layout.logo_path
    if not program_path.startswith("/bins/"):
        return None

    parts = list(Path(program_path.removeprefix("/bins/")).parts)
    if parts and parts[0] == "BIN":
        parts = parts[1:]
    raw_path = layout.emi_root / Path(*parts)
    if raw_path.is_file() or not parts:
        return raw_path

    match = STAGED_EMI_PROGRAM_RE.match(parts[-1])
    if match is None:
        return raw_path
    return layout.emi_root / Path(*parts[:-1]) / f"{int(match.group('entry'), 10)}.bin"


def load_address_from_emi_manifest(binary_path: Path) -> int | None:
    manifest = binary_path.parent / "emi.json"
    if not manifest.is_file():
        return None
    payload = read_json(manifest)
    for entry in payload.get("entries", []):
        if not isinstance(entry, dict):
            continue
        name = str(entry.get("name") or f"{entry.get('index', '')}.bin")
        if name == binary_path.name and entry.get("ram_ptr") is not None:
            return int(entry["ram_ptr"])
    return None


def resolve_binary(layout: RepoLayout, program_path: str) -> BinaryInfo | None:
    path = binary_path_for_program(layout, program_path)
    if path is None or not path.is_file():
        return None
    psx = read_psx_exe_info(path)
    if psx is not None:
        load_address, payload_size, payload_offset = psx
        return BinaryInfo(path, load_address, payload_offset, payload_size)
    load_address = load_address_from_emi_manifest(path)
    if load_address is None:
        return None
    return BinaryInfo(path, load_address, 0, path.stat().st_size)


def extract_original_body(layout: RepoLayout, row: dict[str, Any]) -> bytes | None:
    bounds = row_body_bounds(row)
    if bounds is None:
        return None
    body_min, body_max = bounds
    binary = resolve_binary(layout, str(row.get("program_path") or ""))
    if binary is None:
        return None
    size = body_max - body_min + 1
    offset = body_min - binary.load_address + binary.payload_offset
    if offset < 0 or offset + size > binary.payload_offset + binary.payload_size:
        return None
    return binary.path.read_bytes()[offset : offset + size]


def source_key(row: dict[str, Any]) -> tuple[str, str]:
    return (str(row.get("source_hint") or ""), str(row_entry(row) or ""))


def load_function_rows(layout: RepoLayout) -> list[dict[str, Any]]:
    if not layout.inventory_ghidra_function_index_path.is_file():
        return []
    payload = read_json(layout.inventory_ghidra_function_index_path)
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        return []
    deduped: dict[tuple[str, str], dict[str, Any]] = {}
    for row in rows:
        if not isinstance(row, dict) or not is_reverse_function(row):
            continue
        key = source_key(row)
        if key not in deduped:
            deduped[key] = dict(row)
    return sorted(deduped.values(), key=lambda row: source_key(row))


def load_raw_rows(layout: RepoLayout) -> list[dict[str, Any]]:
    path = layout.inventory_artifacts_dir / "raw_ghidra_export.json"
    if not path.is_file():
        return []
    payload = read_json(path)
    rows = payload.get("rows", [])
    return [dict(row) for row in rows if isinstance(row, dict)] if isinstance(rows, list) else []


def build_symbol_index(rows: list[dict[str, Any]]) -> dict[tuple[str, str], dict[str, Any]]:
    symbols: dict[tuple[str, str], dict[str, Any]] = {}
    for row in rows:
        if row.get("kind") != "symbol":
            continue
        address = normalize_address(row.get("address"))
        if address:
            symbols[(str(row.get("program_path") or ""), address)] = row
    return symbols


def build_source_status_index(layout: RepoLayout) -> dict[tuple[str, str], dict[str, Any]]:
    path = layout.out_dir / "source-status-full.json"
    if not path.is_file():
        return {}
    payload = read_json(path)
    statuses: dict[tuple[str, str], dict[str, Any]] = {}
    for module in payload.get("modules", []):
        if not isinstance(module, dict):
            continue
        for status in module.get("merged_function_statuses", []):
            if not isinstance(status, dict):
                continue
            source_hint = str(status.get("source_hint") or "")
            address = str(status.get("address") or "")
            if source_hint and address:
                statuses[(source_hint, address)] = status
    return statuses


def function_size(row: dict[str, Any]) -> int | None:
    bounds = row_body_bounds(row)
    if bounds is None:
        return None
    return bounds[1] - bounds[0] + 1


def function_hash(layout: RepoLayout, row: dict[str, Any]) -> str | None:
    body = extract_original_body(layout, row)
    if body is None:
        return None
    return hashlib.sha256(body).hexdigest()


def build_duplicate_groups(layout: RepoLayout) -> dict[str, Any]:
    rows = load_function_rows(layout)
    source_status = build_source_status_index(layout)
    by_hash: dict[str, list[dict[str, Any]]] = defaultdict(list)
    missing = 0
    for row in rows:
        raw_hash = function_hash(layout, row)
        if raw_hash is None:
            missing += 1
            continue
        entry = row_entry(row)
        member = {
            "address": entry,
            "body_max": normalize_address(row.get("body_max")),
            "body_min": normalize_address(row.get("body_min")),
            "function": row.get("name"),
            "lift_status": source_status.get((str(row.get("source_hint") or ""), str(entry or "")), {}).get("status", "missing"),
            "program_path": row.get("program_path"),
            "signature": row.get("signature"),
            "size": function_size(row),
            "source_hint": row.get("source_hint"),
        }
        by_hash[raw_hash].append(member)

    groups: list[dict[str, Any]] = []
    for raw_hash, members in by_hash.items():
        if len(members) < 2:
            continue
        ordered = sorted(
            members,
            key=lambda member: (
                0 if member.get("lift_status") == "exact" else 1,
                0 if member.get("lift_status") == "lifted" else 1,
                str(member.get("source_hint") or ""),
                str(member.get("address") or ""),
            ),
        )
        groups.append(
            {
                "group_id": f"fnhash_{raw_hash[:12]}",
                "kind": "raw_body_hash",
                "raw_body_hash": raw_hash,
                "function_count": len(ordered),
                "representative": ordered[0],
                "families": sorted(
                    {
                        str(member.get("source_hint") or "")
                        .removeprefix("output/extracted/")
                        .split("/", 1)[0]
                        for member in ordered
                        if str(member.get("source_hint") or "").startswith("output/extracted/")
                    }
                ),
                "members": ordered,
                "recommended_action": duplicate_action(ordered),
            }
        )
    groups.sort(
        key=lambda group: (
            -int(group["function_count"]),
            -(group["representative"].get("size") or 0),
            str(group["group_id"]),
        )
    )
    return {
        "schema": "rebof3-simple.ghidra-function-duplicates/v1",
        "function_count": len(rows),
        "hashed_function_count": sum(len(members) for members in by_hash.values()),
        "missing_hash_count": missing,
        "duplicate_group_count": len(groups),
        "duplicated_function_count": sum(int(group["function_count"]) for group in groups),
        "groups": groups,
    }


def duplicate_action(members: list[dict[str, Any]]) -> str:
    if any(member.get("lift_status") == "exact" for member in members):
        return "reuse exact representative as duplicate evidence; verify aliases independently"
    if any(member.get("lift_status") == "lifted" for member in members):
        return "prioritize matching the lifted representative before aliases"
    return "lift one representative first; defer aliases until representative matches"


def duplicate_group_by_member(duplicates: dict[str, Any]) -> dict[tuple[str, str], dict[str, Any]]:
    index: dict[tuple[str, str], dict[str, Any]] = {}
    for group in duplicates.get("groups", []):
        if not isinstance(group, dict):
            continue
        for member in group.get("members", []):
            if not isinstance(member, dict):
                continue
            source_hint = str(member.get("source_hint") or "")
            address = str(member.get("address") or "")
            if source_hint and address:
                index[(source_hint, address)] = group
    return index


def refs_for_function(
    raw_rows: list[dict[str, Any]],
    row: dict[str, Any],
    *,
    symbols: dict[tuple[str, str], dict[str, Any]],
) -> dict[str, Any]:
    bounds = row_body_bounds(row)
    program_path = str(row.get("program_path") or "")
    if bounds is None:
        return {"calls": [], "data_refs": [], "incoming": []}
    body_min, body_max = bounds
    calls: list[dict[str, Any]] = []
    data_refs: list[dict[str, Any]] = []
    incoming: list[dict[str, Any]] = []
    for ref in raw_rows:
        if ref.get("kind") != "xref" or str(ref.get("program_path") or "") != program_path:
            continue
        from_addr = parse_int(ref.get("from_address"))
        to_addr = parse_int(ref.get("to_address"))
        ref_type = str(ref.get("reference_type") or "")
        to_hex = format_hex(to_addr)
        target_symbol = symbols.get((program_path, str(to_hex or "")), {})
        ref_record = {
            "from": format_hex(from_addr),
            "to": to_hex,
            "type": ref_type,
            "symbol": target_symbol.get("name"),
        }
        if from_addr is not None and body_min <= from_addr <= body_max:
            if "CALL" in ref_type.upper():
                calls.append(ref_record)
            else:
                data_refs.append(ref_record)
        if to_addr is not None and body_min <= to_addr <= body_max and from_addr not in (None, row_entry(row)):
            incoming.append(ref_record)
    return {
        "calls": calls[:24],
        "call_count": len(calls),
        "data_refs": data_refs[:32],
        "data_ref_count": len(data_refs),
        "incoming": incoming[:24],
        "incoming_count": len(incoming),
    }


def find_function(layout: RepoLayout, address: str, source_hint: str | None = None) -> dict[str, Any] | None:
    wanted = normalize_address(address)
    for row in load_function_rows(layout):
        if row_entry(row) != wanted:
            continue
        if source_hint and str(row.get("source_hint") or "") != source_hint:
            continue
        return row
    return None


def function_report(layout: RepoLayout, address: str, source_hint: str | None = None) -> dict[str, Any]:
    row = find_function(layout, address, source_hint)
    if row is None:
        raise LookupError(f"unknown function: {address}")
    raw_rows = load_raw_rows(layout)
    symbols = build_symbol_index(raw_rows)
    duplicates = build_duplicate_groups(layout)
    duplicate_index = duplicate_group_by_member(duplicates)
    entry = row_entry(row)
    source_status = build_source_status_index(layout).get((str(row.get("source_hint") or ""), str(entry or "")))
    binary = resolve_binary(layout, str(row.get("program_path") or ""))
    body_min, body_max = row_body_bounds(row) or (None, None)
    file_offset = None
    if binary is not None and body_min is not None:
        file_offset = body_min - binary.load_address + binary.payload_offset
    group = duplicate_index.get((str(row.get("source_hint") or ""), str(entry or "")))
    return {
        "schema": "rebof3-simple.ghidra-function-report/v1",
        "function": row.get("name"),
        "signature": row.get("signature"),
        "source_hint": row.get("source_hint"),
        "program_path": row.get("program_path"),
        "entry": entry,
        "body_min": format_hex(body_min),
        "body_max": format_hex(body_max),
        "file_offset": format_hex(file_offset),
        "load_address": None if binary is None else format_hex(binary.load_address),
        "size": function_size(row),
        "raw_body_hash": function_hash(layout, row),
        "duplicate_group": None if group is None else group["group_id"],
        "duplicate_representative": None if group is None else group["representative"],
        "source_status": None if source_status is None else source_status.get("status"),
        "source_path": None if source_status is None else source_status.get("source"),
        "comments": [
            text for text in (row.get("comment"), row.get("repeatable_comment")) if text
        ],
        "parameters": row.get("parameters") if isinstance(row.get("parameters"), list) else [],
        "locals": row.get("locals") if isinstance(row.get("locals"), list) else [],
        "refs": refs_for_function(raw_rows, row, symbols=symbols),
        "next_action": next_action(source_status),
    }


def next_action(source_status: dict[str, Any] | None) -> str:
    if source_status and source_status.get("source"):
        return f"bin/harness verify function bof3/{source_status['source']}"
    return "bin/harness lift <target-id>"


def queue_report(layout: RepoLayout, *, limit: int) -> dict[str, Any]:
    source_status = build_source_status_index(layout)
    duplicates = duplicate_group_by_member(build_duplicate_groups(layout))
    candidates: list[dict[str, Any]] = []
    for key, status in source_status.items():
        if status.get("status") == "exact" or not status.get("source"):
            continue
        source_hint, address = key
        row = find_function(layout, address, source_hint)
        if row is None:
            continue
        group = duplicates.get(key)
        size = function_size(row) or 0
        candidates.append(
            {
                "address": address,
                "duplicate_group": None if group is None else group.get("group_id"),
                "function": row.get("name"),
                "score": size,
                "size": size,
                "source": f"bof3/{status['source']}",
                "source_hint": source_hint,
                "status": status.get("status"),
                "verify": f"bin/harness verify function bof3/{status['source']}",
            }
        )
    candidates.sort(key=lambda item: (item["size"], item["source_hint"], item["address"]))
    return {
        "schema": "rebof3-simple.ghidra-lift-queue/v1",
        "candidate_count": len(candidates),
        "tasks": candidates[:limit],
    }


def module_report(layout: RepoLayout, source_hint: str) -> dict[str, Any]:
    rows = [row for row in load_function_rows(layout) if row.get("source_hint") == source_hint]
    source_status = build_source_status_index(layout)
    return {
        "schema": "rebof3-simple.ghidra-module-report/v1",
        "source_hint": source_hint,
        "function_count": len(rows),
        "functions": [
            {
                "address": row_entry(row),
                "function": row.get("name"),
                "size": function_size(row),
                "status": source_status.get((source_hint, str(row_entry(row) or "")), {}).get("status", "missing"),
            }
            for row in rows
        ],
    }


def context_gaps(layout: RepoLayout) -> dict[str, Any]:
    context_path = layout.bof3_dir / "include" / "bof3" / "context.h"
    context_text = context_path.read_text(encoding="utf-8") if context_path.is_file() else ""
    context_names = set(identifier_names(context_text))
    modules: list[dict[str, Any]] = []
    for internal in sorted((layout.bof3_dir / "src").rglob("internal.h")):
        text = internal.read_text(encoding="utf-8", errors="ignore")
        names = sorted(set(identifier_names(text)))
        missing = [name for name in names if name not in context_names]
        duplicate = [name for name in names if name in context_names]
        if not names:
            status = "empty-removable"
        elif missing and duplicate:
            status = "mixed"
        elif missing:
            status = "needed-private"
        else:
            status = "duplicate-context"
        modules.append(
            {
                "internal": str(internal.relative_to(layout.root)),
                "status": status,
                "definition_count": len(names),
                "duplicate_count": len(duplicate),
                "missing_count": len(missing),
                "missing_samples": missing[:16],
            }
        )
    return {"schema": "rebof3-simple.context-gaps/v1", "modules": modules}


def identifier_names(text: str) -> list[str]:
    names: list[str] = []
    for match in INTERNAL_IDENTIFIER_RE.finditer(text):
        name = match.group("typedef") or match.group("define") or match.group("symbol")
        if name:
            names.append(name)
    return names


def render_markdown(payload: dict[str, Any]) -> str:
    schema = str(payload.get("schema") or "")
    if schema.endswith("function-report/v1"):
        return render_function_markdown(payload)
    if schema.endswith("function-duplicates/v1"):
        return render_duplicates_markdown(payload)
    if schema.endswith("lift-queue/v1"):
        return render_queue_markdown(payload)
    if schema.endswith("module-report/v1"):
        return render_module_markdown(payload)
    if schema.endswith("context-gaps/v1"):
        return render_context_gaps_markdown(payload)
    return json.dumps(payload, indent=2, sort_keys=True) + "\n"


def render_function_markdown(payload: dict[str, Any]) -> str:
    lines = [
        f"# {payload['function']} {payload['entry']}",
        "",
        f"- source: `{payload['source_hint']}`",
        f"- program: `{payload['program_path']}`",
        f"- signature: `{payload.get('signature') or ''}`",
        f"- body: `{payload['body_min']}`..`{payload['body_max']}` size `{payload['size']}`",
        f"- file offset: `{payload['file_offset']}` load `{payload['load_address']}`",
        f"- raw hash: `{payload['raw_body_hash']}`",
        f"- source status: `{payload.get('source_status') or 'missing'}`",
    ]
    if payload.get("duplicate_group"):
        lines.append(f"- duplicate group: `{payload['duplicate_group']}`")
    if payload.get("comments"):
        lines.append(f"- comments: {'; '.join(payload['comments'])}")
    if payload.get("parameters"):
        lines.append(f"- parameters: {len(payload['parameters'])}")
    if payload.get("locals"):
        lines.append(f"- locals: {len(payload['locals'])}")
    refs = payload["refs"]
    lines.extend([
        "",
        "## Evidence",
        "",
        f"- calls: {refs['call_count']}",
        f"- data refs: {refs['data_ref_count']}",
        f"- incoming refs: {refs['incoming_count']}",
    ])
    for label, key in (("Calls", "calls"), ("Data Refs", "data_refs"), ("Incoming", "incoming")):
        rows = refs.get(key) or []
        if rows:
            lines.extend(["", f"## {label}", ""])
            for row in rows[:12]:
                symbol = f" `{row['symbol']}`" if row.get("symbol") else ""
                lines.append(f"- `{row['from']}` -> `{row['to']}` `{row['type']}`{symbol}")
    lines.extend(["", f"next: `{payload['next_action']}`", ""])
    return "\n".join(lines)


def render_duplicates_markdown(payload: dict[str, Any]) -> str:
    lines = [
        "# Function Duplicates",
        "",
        f"- functions: {payload['function_count']}",
        f"- duplicate groups: {payload['duplicate_group_count']}",
        f"- duplicated functions: {payload['duplicated_function_count']}",
        "",
    ]
    for group in payload["groups"][:40]:
        rep = group["representative"]
        lines.append(
            f"- `{group['group_id']}`: {group['function_count']} funcs, rep `{rep['source_hint']}` `{rep['address']}` size `{rep.get('size')}`"
        )
    return "\n".join(lines) + "\n"


def render_queue_markdown(payload: dict[str, Any]) -> str:
    lines = ["# Lift Queue", ""]
    for task in payload["tasks"]:
        duplicate = f" duplicate `{task['duplicate_group']}`" if task.get("duplicate_group") else ""
        lines.append(
            f"- `{task['source']}` `{task['address']}` size `{task['size']}`{duplicate}: `{task['verify']}`"
        )
    return "\n".join(lines) + "\n"


def render_module_markdown(payload: dict[str, Any]) -> str:
    lines = [f"# {payload['source_hint']}", "", f"- functions: {payload['function_count']}", ""]
    for row in payload["functions"][:200]:
        lines.append(f"- `{row['address']}` `{row['function']}` size `{row['size']}` status `{row['status']}`")
    return "\n".join(lines) + "\n"


def render_context_gaps_markdown(payload: dict[str, Any]) -> str:
    lines = ["# Context Gaps", ""]
    for row in payload["modules"]:
        lines.append(
            f"- `{row['internal']}`: `{row['status']}` missing `{row['missing_count']}` duplicate `{row['duplicate_count']}`"
        )
    return "\n".join(lines) + "\n"
