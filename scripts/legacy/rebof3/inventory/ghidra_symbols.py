from __future__ import annotations

import argparse
import json
import sqlite3
from pathlib import Path
from typing import Any

from ..common import ROOT, format_hex, parse_hexish, relative_to_root
from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import (
    emit_output_summary,
    ensure_output_parents,
    write_json_output,
    write_markdown_output,
)
from ..program_identity import infer_source_hint, slugify
from .db.connection import connect_inventory_database
from .db.migrations import ensure_inventory_schema
from .layout import INVENTORY_SQLITE
from .repositories.metadata import MetadataRepository
from .repositories.programs import ProgramRepository
from ..models.inventory import InventoryFunctionRow, InventoryProgramRow


DEFAULT_RAW_EXPORT = ROOT / "tmp" / "ghidra_symbols" / "raw_project_export.json"


def canonical_program_path(program_path: str) -> str:
    normalized = "/" + str(program_path or "").strip("/")
    if normalized.startswith("/SLUS_004.22"):
        return "/boot/SLUS_004.22"
    if normalized in {"/LOGO.EXE", "/LOGO/LOGO.EXE"} or normalized.startswith(
        "/LOGO.EXE."
    ):
        return "/boot/LOGO/LOGO.EXE"
    if normalized.endswith(".bin.0") or normalized.endswith(".EXE.0"):
        return normalized[:-2]
    return normalized


def disambiguate_program_slugs(programs: list[dict[str, Any]]) -> dict[str, str]:
    counts: dict[str, int] = {}
    slugs: dict[str, str] = {}
    for program in sorted(
        programs, key=lambda item: str(item.get("program_path") or "")
    ):
        base_slug = slugify(
            str(program.get("program_path") or program.get("program_name") or "program")
        )
        next_count = counts.get(base_slug, 0) + 1
        counts[base_slug] = next_count
        slug = base_slug if next_count == 1 else f"{base_slug}_{next_count}"
        slugs[
            str(program.get("program_path") or program.get("program_name") or slug)
        ] = slug
    return slugs


def sort_functions(functions: list[dict[str, Any]]) -> list[dict[str, Any]]:
    return sorted(
        functions,
        key=lambda row: (
            parse_hexish(str(row.get("entry") or "0")),
            str(row.get("name") or ""),
        ),
    )


def build_program_payload(program: dict[str, Any], program_slug: str) -> dict[str, Any]:
    program_path = canonical_program_path(
        str(program.get("program_path") or program.get("program_name") or "")
    )
    folder = str(program.get("folder") or "/")
    if program_path:
        folder = str(Path(program_path).parent).replace("//", "/")
    program_name = str(program.get("program_name") or Path(program_path).name)
    functions = sort_functions(list(program.get("functions") or []))
    source_hint = infer_source_hint(program_path, folder, program_name)
    return {
        "program_name": program_name,
        "program_path": program_path,
        "program_slug": program_slug,
        "folder": folder,
        "source_hint": source_hint,
        "function_count": len(functions),
        "functions": functions,
    }


def regroup_function_rows_into_programs(
    rows: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    grouped: dict[str, dict[str, Any]] = {}
    seen_entries: set[tuple[str, str]] = set()
    for row in rows:
        if str(row.get("kind") or "") != "function":
            continue
        program_path = canonical_program_path(str(row.get("program_path") or ""))
        if not program_path:
            continue
        program = grouped.setdefault(
            program_path,
            {
                "program_name": str(Path(program_path).name),
                "program_path": program_path,
                "folder": str(Path(program_path).parent).replace("//", "/"),
                "functions": [],
            },
        )
        entry_text = str(
            row.get("entry")
            or row.get("entry_text")
            or row.get("address")
            or row.get("entry_hex")
            or ""
        )
        if not entry_text:
            continue
        dedupe_key = (program_path, format_hex(parse_hexish(entry_text)))
        if dedupe_key in seen_entries:
            continue
        seen_entries.add(dedupe_key)
        program["functions"].append(
            {
                "entry": entry_text,
                "name": row.get("name"),
                "signature": row.get("signature") or row.get("type_spec"),
                "body_min": row.get("body_min") or row.get("address"),
                "body_max": row.get("body_max"),
                "comment": row.get("comment"),
                "repeatable_comment": row.get("repeatable_comment"),
                "namespace": row.get("namespace"),
                "name_source": row.get("name_source"),
                "is_thunk": bool(row.get("is_thunk", False)),
            }
        )
    return [grouped[key] for key in sorted(grouped.keys())]


def flatten_symbol_rows(programs: list[dict[str, Any]]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for program in programs:
        for function in program["functions"]:
            rows.append(
                {
                    "program_name": program["program_name"],
                    "program_path": program["program_path"],
                    "program_slug": program["program_slug"],
                    "folder": program["folder"],
                    "source_hint": program["source_hint"],
                    "entry": str(function.get("entry") or ""),
                    "entry_hex": format_hex(
                        parse_hexish(str(function.get("entry") or "0"))
                    ),
                    "name": function.get("name"),
                    "signature": function.get("signature"),
                    "body_min": function.get("body_min"),
                    "body_max": function.get("body_max"),
                    "comment": function.get("comment"),
                    "repeatable_comment": function.get("repeatable_comment"),
                    "namespace": function.get("namespace"),
                    "name_source": function.get("name_source"),
                    "is_thunk": bool(function.get("is_thunk", False)),
                }
            )
    rows.sort(
        key=lambda row: (
            str(row["program_path"]),
            parse_hexish(str(row["entry"])),
            str(row.get("name") or ""),
        )
    )
    return rows


def render_symbols_tsv(rows: list[dict[str, Any]]) -> str:
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
        lines.append(
            "\t".join(
                [
                    str(row.get("program_path") or ""),
                    str(row.get("program_name") or ""),
                    str(row.get("program_slug") or ""),
                    str(row.get("entry") or ""),
                    str(row.get("entry_hex") or ""),
                    str(row.get("name") or ""),
                    str(row.get("signature") or ""),
                    str(row.get("body_min") or ""),
                    str(row.get("body_max") or ""),
                    str(row.get("namespace") or ""),
                    str(row.get("name_source") or ""),
                    str(row.get("is_thunk") or False),
                    str(row.get("source_hint") or ""),
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_markdown(index_payload: dict[str, Any]) -> str:
    lines = [
        "# Ghidra Symbols",
        "",
        "Machine-generated inventory of saved Ghidra function names and comments exported from the current BOF3 project.",
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
        source_suffix = f" source `{source_hint}`" if source_hint else ""
        lines.append(
            f"- `{program['program_path']}`: {program['function_count']} functions{source_suffix}"
        )

    return "\n".join(lines) + "\n"


def transform_export(
    payload: dict[str, Any],
) -> tuple[dict[str, Any], list[dict[str, Any]], str, list[dict[str, Any]]]:
    raw_programs = list(payload.get("programs") or [])
    if not raw_programs:
        raw_programs = regroup_function_rows_into_programs(
            list(payload.get("rows") or [])
        )
    slug_map = disambiguate_program_slugs(raw_programs)
    programs = [
        build_program_payload(
            program,
            slug_map[
                str(
                    program.get("program_path")
                    or program.get("program_name")
                    or "program"
                )
            ],
        )
        for program in sorted(
            raw_programs, key=lambda item: str(item.get("program_path") or "")
        )
    ]
    rows = flatten_symbol_rows(programs)
    index_payload = {
        "project_name": payload.get("project_name"),
        "program_count": len(programs),
        "function_count": len(rows),
        "selected_programs": list(payload.get("selected_programs") or []),
        "programs": [
            {key: value for key, value in program.items() if key != "functions"}
            for program in programs
        ],
    }
    return index_payload, rows, render_symbols_tsv(rows), programs


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("inventory", "ghidra-symbols"),
        description="Reshape raw Ghidra symbol exports into durable program and function indexes.",
    )
    add_logging_args(parser)
    parser.add_argument("input", nargs="?", type=Path, default=DEFAULT_RAW_EXPORT)
    parser.add_argument("--db", type=Path, default=INVENTORY_SQLITE)
    parser.add_argument(
        "--index-out", type=Path, default=None, help="optional JSON output"
    )
    parser.add_argument(
        "--symbols-out",
        type=Path,
        default=None,
        help="optional symbols JSON output",
    )
    parser.add_argument(
        "--symbols-tsv-out", type=Path, default=None, help="optional symbols TSV output"
    )
    parser.add_argument(
        "--md-out", type=Path, default=None, help="optional Markdown output"
    )
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def persist_programs_to_inventory(
    connection: sqlite3.Connection,
    *,
    programs: list[dict[str, Any]],
) -> tuple[int, int, int]:
    program_repo = ProgramRepository(connection)
    metadata_repo = MetadataRepository(connection)
    program_count = 0
    function_count = 0
    metadata_count = 0
    for program in programs:
        program_slug = str(program["program_slug"])
        program_path = str(program["program_path"])
        program_repo.upsert_program(
            InventoryProgramRow(
                program_slug=program_slug,
                program_name=str(program["program_name"]),
                program_path=program_path,
                folder=None
                if program.get("folder") is None
                else str(program.get("folder")),
                source_hint=(
                    None
                    if program.get("source_hint") is None
                    else str(program.get("source_hint"))
                ),
            )
        )
        program_count += 1
        for function in program["functions"]:
            entry_text = str(function.get("entry") or "0")
            entry_hex = format_hex(parse_hexish(entry_text))
            program_repo.upsert_function(
                InventoryFunctionRow(
                    program_slug=program_slug,
                    entry_address=parse_hexish(entry_text),
                    entry_hex=entry_hex,
                    name=str(function.get("name") or entry_hex),
                    signature=(
                        None
                        if function.get("signature") is None
                        else str(function.get("signature"))
                    ),
                    body_min=(
                        None
                        if function.get("body_min") in {None, ""}
                        else parse_hexish(str(function.get("body_min")))
                    ),
                    body_max=(
                        None
                        if function.get("body_max") in {None, ""}
                        else parse_hexish(str(function.get("body_max")))
                    ),
                    comment=(
                        None
                        if function.get("comment") is None
                        else str(function.get("comment"))
                    ),
                    repeatable_comment=(
                        None
                        if function.get("repeatable_comment") is None
                        else str(function.get("repeatable_comment"))
                    ),
                    namespace=(
                        None
                        if function.get("namespace") is None
                        else str(function.get("namespace"))
                    ),
                    name_source=(
                        None
                        if function.get("name_source") is None
                        else str(function.get("name_source"))
                    ),
                    is_thunk=bool(function.get("is_thunk", False)),
                    source_hint=(
                        None
                        if program.get("source_hint") is None
                        else str(program.get("source_hint"))
                    ),
                )
            )
            function_count += 1
            metadata_repo.upsert_row(
                row_key=f"{program_path}|function|{entry_hex}",
                program_path=program_path,
                kind="function",
                address_key=entry_hex[2:],
                address=parse_hexish(entry_text),
                entry_text=entry_text,
                path=None,
                name=None
                if function.get("name") is None
                else str(function.get("name")),
                comment=(
                    None
                    if function.get("comment") is None
                    else str(function.get("comment"))
                ),
                repeatable_comment=(
                    None
                    if function.get("repeatable_comment") is None
                    else str(function.get("repeatable_comment"))
                ),
                type_spec=(
                    None
                    if function.get("signature") is None
                    else str(function.get("signature"))
                ),
                source=(
                    None
                    if program.get("source_hint") is None
                    else str(program.get("source_hint"))
                ),
                confidence="high",
                tags=["ghidra", "function"],
                extra={
                    "namespace": function.get("namespace"),
                    "name_source": function.get("name_source"),
                },
            )
            metadata_count += 1
    return program_count, function_count, metadata_count


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "ghidra_symbols")
    if not args.input.exists():
        logger.error(f"input not found: {args.input}")
        return 1

    payload = json.loads(args.input.read_text(encoding="utf-8"))
    index_payload, rows, tsv_text, programs = transform_export(payload)
    connection = connect_inventory_database(args.db)
    ensure_inventory_schema(connection)
    db_programs, db_functions, db_metadata = persist_programs_to_inventory(
        connection,
        programs=programs,
    )
    connection.close()
    ensure_output_parents(
        args.index_out,
        args.symbols_out,
        args.symbols_tsv_out,
        args.md_out,
    )
    if args.index_out is not None:
        write_json_output(args.index_out, index_payload)
    if args.symbols_out is not None:
        write_json_output(args.symbols_out, rows)
    if args.symbols_tsv_out is not None:
        args.symbols_tsv_out.write_text(tsv_text, encoding="utf-8")
    if args.md_out is not None:
        write_markdown_output(args.md_out, render_markdown(index_payload))
    emit_output_summary(
        logger,
        summary=(
            f"ghidra symbol programs={index_payload['program_count']} "
            f"functions={index_payload['function_count']} "
            f"db_programs={db_programs} db_functions={db_functions} db_metadata={db_metadata}"
        ),
        json_path=args.index_out,
        md_path=args.md_out,
    )
    return 0
