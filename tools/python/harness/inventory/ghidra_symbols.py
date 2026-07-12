from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any

from ..jsonio import write_json


SLUG_CLEAN_RE = re.compile(r"[^0-9A-Za-z]+")
ENTRY_INDEX_RE = re.compile(r"_e(?P<entry>[0-9]+)(?:_|\.)", re.IGNORECASE)
RAW_BIN_NAME_RE = re.compile(r"^(?P<entry>[0-9]+)\.bin$", re.IGNORECASE)
SHADOW_BOOT_RE = re.compile(r"^/(SLUS_004\.22|LOGO\.EXE)(?:\.[0-9]+)?$", re.IGNORECASE)


def slugify(text: str) -> str:
    lowered = SLUG_CLEAN_RE.sub("_", text.strip().lower()).strip("_")
    return lowered or "program"


def infer_source_hint(program_path: str, folder: str, program_name: str) -> str | None:
    normalized_folder = str(folder or "").strip("/")
    normalized_program = str(program_name or "")
    if normalized_folder == "boot" and normalized_program == "SLUS_004.22":
        return "out/extracted/SLUS_004.22"
    if (
        normalized_folder in {"boot", "boot/logo"}
        and normalized_program.upper() == "LOGO.EXE"
    ):
        return "out/extracted/LOGO/LOGO.EXE"
    for prefix in ("bins/", "overlays/"):
        if not normalized_folder.startswith(prefix):
            continue
        archive_id = normalized_folder[len(prefix) :]
        match = ENTRY_INDEX_RE.search(normalized_program) or RAW_BIN_NAME_RE.match(
            normalized_program
        )
        if match is None or not archive_id:
            return None
        return f"out/extracted/{archive_id}.EMI#{int(match.group('entry'))}"
    _ = program_path
    return None


def canonical_program_path(program_path: str) -> str:
    match = SHADOW_BOOT_RE.match(program_path)
    if match is None:
        return program_path
    if match.group(1).upper() == "LOGO.EXE":
        return "/boot/LOGO.EXE"
    return "/boot/SLUS_004.22"


def disambiguate_program_slugs(programs: list[dict[str, Any]]) -> dict[str, str]:
    counts: dict[str, int] = {}
    slugs: dict[str, str] = {}
    for program in sorted(
        programs, key=lambda item: str(item.get("program_path") or "")
    ):
        key = str(
            program.get("program_path") or program.get("program_name") or "program"
        )
        base_slug = slugify(key)
        next_count = counts.get(base_slug, 0) + 1
        counts[base_slug] = next_count
        slugs[key] = base_slug if next_count == 1 else f"{base_slug}_{next_count}"
    return slugs


def parse_hexish(value: str) -> int:
    return int(value, 16) if value.lower().startswith("0x") else int(value, 16)


def format_hex(value: int) -> str:
    return f"0x{value:08x}"


def sort_functions(functions: list[dict[str, Any]]) -> list[dict[str, Any]]:
    return sorted(
        functions,
        key=lambda row: (
            parse_hexish(str(row.get("entry") or row.get("address") or "0")),
            str(row.get("name") or ""),
        ),
    )


def normalize_programs(payload: dict[str, Any]) -> list[dict[str, Any]]:
    raw_programs = payload.get("programs")
    if isinstance(raw_programs, list) and raw_programs:
        programs = []
        for program in raw_programs:
            if not isinstance(program, dict):
                continue
            program_path = canonical_program_path(
                str(program.get("program_path") or program.get("program_name") or "")
            )
            program_name = str(program.get("program_name") or Path(program_path).name)
            folder = str(
                program.get("folder") or Path(program_path).parent.as_posix() or "/"
            )
            if not folder.startswith("/"):
                folder = f"/{folder.strip('/')}"
            programs.append(
                {
                    "program_name": program_name,
                    "program_path": program_path,
                    "folder": folder,
                    "functions": sort_functions(list(program.get("functions") or [])),
                }
            )
        return deduplicate_programs(programs)

    rows = payload.get("rows")
    if not isinstance(rows, list):
        return []

    grouped: dict[str, dict[str, Any]] = {}
    for row in rows:
        if not isinstance(row, dict) or row.get("kind") != "function":
            continue
        program_path = canonical_program_path(str(row.get("program_path") or ""))
        folder = str(Path(program_path).parent.as_posix() or "/")
        program = grouped.setdefault(
            program_path,
            {
                "program_name": Path(program_path).name,
                "program_path": program_path,
                "folder": folder,
                "functions": [],
            },
        )
        program["functions"].append(
            {
                "entry": str(row.get("address") or row.get("entry") or "0"),
                "name": row.get("name"),
                "signature": row.get("type_spec") or row.get("signature"),
                "body_min": row.get("body_min"),
                "body_max": row.get("body_max"),
                "comment": row.get("comment"),
                "repeatable_comment": row.get("repeatable_comment"),
                "namespace": row.get("namespace"),
                "name_source": row.get("name_source"),
                "is_thunk": bool(row.get("is_thunk", False)),
                "parameters": row.get("parameters")
                if isinstance(row.get("parameters"), list)
                else [],
                "locals": row.get("locals")
                if isinstance(row.get("locals"), list)
                else [],
            }
        )
    programs = []
    for program in grouped.values():
        program["functions"] = sort_functions(program["functions"])
        programs.append(program)
    return deduplicate_programs(programs)


def deduplicate_programs(programs: list[dict[str, Any]]) -> list[dict[str, Any]]:
    deduped: dict[str, dict[str, Any]] = {}
    for program in programs:
        program_path = str(program["program_path"])
        existing = deduped.get(program_path)
        if existing is None:
            deduped[program_path] = program
            continue
        seen = {
            (str(function.get("entry") or ""), str(function.get("name") or ""))
            for function in existing["functions"]
        }
        for function in program["functions"]:
            key = (str(function.get("entry") or ""), str(function.get("name") or ""))
            if key not in seen:
                existing["functions"].append(function)
        existing["functions"] = sort_functions(existing["functions"])
    return [deduped[key] for key in sorted(deduped)]


def build_program_payload(
    program: dict[str, Any], program_slug: str, output_dir: Path
) -> dict[str, Any]:
    program_path = str(program["program_path"])
    folder = str(program["folder"])
    program_name = str(program["program_name"])
    functions = sort_functions(list(program["functions"]))
    source_hint = infer_source_hint(program_path, folder, program_name)
    output_path = output_dir / f"{program_slug}_ghidra_symbols.json"
    return {
        "folder": folder,
        "function_count": len(functions),
        "functions": functions,
        "output_path": str(output_path),
        "program_name": program_name,
        "program_path": program_path,
        "program_slug": program_slug,
        "source_hint": source_hint,
    }


def flatten_function_rows(programs: list[dict[str, Any]]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for program in programs:
        for function in program["functions"]:
            entry_text = str(function.get("entry") or "0")
            rows.append(
                {
                    "body_max": function.get("body_max"),
                    "body_min": function.get("body_min"),
                    "comment": function.get("comment"),
                    "entry": entry_text,
                    "entry_hex": format_hex(parse_hexish(entry_text)),
                    "folder": program["folder"],
                    "is_thunk": bool(function.get("is_thunk", False)),
                    "name": function.get("name"),
                    "name_source": function.get("name_source"),
                    "namespace": function.get("namespace"),
                    "program_name": program["program_name"],
                    "program_path": program["program_path"],
                    "program_slug": program["program_slug"],
                    "parameters": function.get("parameters")
                    if isinstance(function.get("parameters"), list)
                    else [],
                    "locals": function.get("locals")
                    if isinstance(function.get("locals"), list)
                    else [],
                    "repeatable_comment": function.get("repeatable_comment"),
                    "signature": function.get("signature"),
                    "source_hint": program["source_hint"],
                    "symbol_file": program["output_path"],
                }
            )
    rows.sort(
        key=lambda row: (
            row["program_path"],
            parse_hexish(row["entry"]),
            str(row.get("name") or ""),
        )
    )
    return rows


def render_function_index_tsv(rows: list[dict[str, Any]]) -> str:
    header = [
        "program_path",
        "program_name",
        "program_slug",
        "entry",
        "entry_hex",
        "name",
        "signature",
        "body_min",
        "body_max",
        "namespace",
        "name_source",
        "is_thunk",
        "source_hint",
    ]
    lines = ["\t".join(header)]
    for row in rows:
        lines.append("\t".join(str(row.get(column) or "") for column in header))
    return "\n".join(lines) + "\n"


def render_ghidra_symbols_markdown(index_payload: dict[str, Any]) -> str:
    lines = [
        "# Ghidra Symbols",
        "",
        "Machine-generated inventory of imported Ghidra function symbols.",
        "",
        f"- Project name: `{index_payload.get('project_name') or '?'}`",
        f"- Program count: {index_payload['program_count']}",
        f"- Function count: {index_payload['function_count']}",
        "",
        "## Programs",
        "",
    ]
    for program in index_payload["programs"]:
        source_hint = program.get("source_hint")
        suffix = f" source `{source_hint}`" if source_hint else ""
        lines.append(
            f"- `{program['program_path']}`: {program['function_count']} functions, output `{program['output_path']}`{suffix}"
        )
    return "\n".join(lines) + "\n"


def transform_export(
    payload: dict[str, Any],
    *,
    program_output_dir: Path,
) -> tuple[dict[str, Any], list[dict[str, Any]], str, list[dict[str, Any]]]:
    raw_programs = normalize_programs(payload)
    slug_map = disambiguate_program_slugs(raw_programs)
    programs = [
        build_program_payload(
            program,
            slug_map[str(program["program_path"])],
            program_output_dir,
        )
        for program in raw_programs
    ]
    rows = flatten_function_rows(programs)
    index_payload = {
        "schema": "harness.inventory-ghidra-symbols/v1",
        "project_name": payload.get("project_name"),
        "program_count": len(programs),
        "function_count": len(rows),
        "selected_programs": [
            str(path) for path in payload.get("selected_programs") or []
        ],
        "programs": [
            {key: value for key, value in program.items() if key != "functions"}
            for program in programs
        ],
    }
    return index_payload, rows, render_function_index_tsv(rows), programs


def write_program_symbol_files(programs: list[dict[str, Any]]) -> None:
    for program in programs:
        path = Path(str(program["output_path"]))
        path.parent.mkdir(parents=True, exist_ok=True)
        write_json(path, program)


def prune_program_symbol_files(
    programs: list[dict[str, Any]], output_dir: Path
) -> None:
    active_paths = {Path(str(program["output_path"])) for program in programs}
    if not output_dir.exists():
        return
    for path in output_dir.glob("*_ghidra_symbols.json"):
        if path not in active_paths:
            path.unlink()


def import_ghidra_symbols(
    *,
    input_path: Path,
    index_out: Path,
    function_index_out: Path,
    function_index_tsv_out: Path,
    md_out: Path,
    program_output_dir: Path,
) -> dict[str, Any]:
    payload = json.loads(input_path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError(f"expected JSON object in {input_path}")
    index_payload, rows, tsv_text, programs = transform_export(
        payload,
        program_output_dir=program_output_dir,
    )
    write_program_symbol_files(programs)
    if not payload.get("selected_programs"):
        prune_program_symbol_files(programs, program_output_dir)
    write_json(index_out, index_payload)
    write_json(
        function_index_out,
        {"rows": rows, "schema": "harness.inventory-ghidra-function-index/v1"},
    )
    function_index_tsv_out.parent.mkdir(parents=True, exist_ok=True)
    function_index_tsv_out.write_text(tsv_text, encoding="utf-8")
    md_out.parent.mkdir(parents=True, exist_ok=True)
    md_out.write_text(render_ghidra_symbols_markdown(index_payload), encoding="utf-8")
    return index_payload
