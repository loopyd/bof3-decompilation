from __future__ import annotations

import argparse
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import (
    ROOT,
    parse_source_spec,
    relative_to_root,
    write_json_output,
    write_markdown_output,
    write_text_output,
)
from ..config import DEFAULT_MATCH_ROOT, DEFAULT_PSX_PROFILE
from . import scoreboard as scoreboard_lib


def default_output_paths(match_root: Path, profile: str) -> tuple[Path, Path, Path]:
    output_dir = match_root / "_reports"
    slug = profile.replace("-", "_")
    return (
        output_dir / f"enhanced_binary_report_{slug}.json",
        output_dir / f"enhanced_binary_report_{slug}.tsv",
        output_dir / f"enhanced_binary_report_{slug}.md",
    )


def default_summary_md_path(match_root: Path, profile: str) -> Path:
    output_dir = match_root / "_reports"
    slug = profile.replace("-", "_")
    return output_dir / f"enhanced_binary_report_summary_{slug}.md"


def binary_group_key(*, source_hint: str | None, program_path: str | None) -> str:
    source_text = str(source_hint or "").strip()
    if source_text:
        source_spec = parse_source_spec(source_text)
        source_path = source_spec.path
        if not source_path.is_absolute():
            source_path = ROOT / source_path
        return relative_to_root(source_path)
    program_text = canonical_program_path(str(program_path or ""))
    if not program_text:
        return ""
    if program_text.startswith("/bins/"):
        parts = [part for part in program_text.strip("/").split("/") if part]
        if len(parts) >= 5:
            folder_parts = parts[1:-1]
            archive_name = folder_parts[-1] + ".EMI"
            return str(
                Path("build/extracted").joinpath(*folder_parts[:-1], archive_name)
            )
    if program_text.startswith("/boot/LOGO/"):
        return "build/extracted/LOGO/LOGO.EXE"
    if program_text.startswith("/boot/"):
        return str(Path("build/extracted") / Path(program_text).name)
    return program_text


def canonical_program_path(program_path: str) -> str:
    text = str(program_path or "")
    if text.endswith(".bin.0"):
        return text[:-2]
    if text.endswith(".EXE.0") or text.endswith(".22.0"):
        return text[:-2]
    return text


def function_display_name(row: dict[str, Any]) -> str:
    return str(
        row.get("source_function")
        or row.get("name")
        or row.get("entry_hex")
        or "<unknown>"
    )


def match_value(row: dict[str, Any]) -> float:
    value = row.get("objdiff_match_percent")
    return 0.0 if value in (None, "") else float(value)


def function_category(row: dict[str, Any]) -> str:
    if match_value(row) > 0.0:
        return "matching"
    if bool(row.get("has_source_mapping")):
        return "lifted"
    return "missing"


def dedupe_rows(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    deduped: dict[tuple[str, str], dict[str, Any]] = {}
    for row in rows:
        key = (
            canonical_program_path(str(row.get("program_path") or "")),
            str(row.get("entry_hex") or ""),
        )
        current = deduped.get(key)
        if current is None:
            deduped[key] = row
            continue
        current_has_hint = bool(current.get("source_hint"))
        candidate_has_hint = bool(row.get("source_hint"))
        if candidate_has_hint and not current_has_hint:
            deduped[key] = row
            continue
        current_path = str(current.get("program_path") or "")
        candidate_path = str(row.get("program_path") or "")
        if current_path.endswith(".0") and not candidate_path.endswith(".0"):
            deduped[key] = row
    return list(deduped.values())


def build_slot_row(program_path: str, rows: list[dict[str, Any]]) -> dict[str, Any]:
    rows = dedupe_rows(rows)
    matching_names: list[str] = []
    lifted_names: list[str] = []
    missing_names: list[str] = []
    exact_functions = 0
    asm_exact_functions = 0
    families = sorted(
        {str(row.get("family") or "") for row in rows if str(row.get("family") or "")}
    )
    positive_matches: list[float] = []
    for row in rows:
        name = function_display_name(row)
        value = match_value(row)
        if value > 0.0:
            matching_names.append(name)
            positive_matches.append(value)
            if value >= 100.0:
                exact_functions += 1
        elif bool(row.get("has_source_mapping")):
            lifted_names.append(name)
        else:
            missing_names.append(name)
        asm_exact_functions += int(bool(row.get("asm_exact")))
    total = len(rows)
    matching = len(matching_names)
    lifted = len(lifted_names)
    missing = len(missing_names)
    completed = matching + lifted
    return {
        "program_path": program_path,
        "family": ", ".join(families),
        "total_functions": total,
        "matching_functions": matching,
        "exact_functions": exact_functions,
        "asm_exact_functions": asm_exact_functions,
        "lifted_functions": lifted,
        "missing_functions": missing,
        "completion_percent": 0.0
        if total == 0
        else round((float(completed) / float(total)) * 100.0, 3),
        "matching_percent": 0.0
        if total == 0
        else round((float(matching) / float(total)) * 100.0, 3),
        "highest_match_percent": None
        if not positive_matches
        else max(positive_matches),
        "lowest_match_percent": None if not positive_matches else min(positive_matches),
        "matching_function_names": sorted(matching_names),
        "lifted_function_names": sorted(lifted_names),
        "missing_function_names": sorted(missing_names),
    }


def build_binary_row(binary_path: str, rows: list[dict[str, Any]]) -> dict[str, Any]:
    rows = dedupe_rows(rows)
    slot_groups: dict[str, list[dict[str, Any]]] = {}
    for row in rows:
        slot_groups.setdefault(
            canonical_program_path(str(row.get("program_path") or "")), []
        ).append(row)
    slot_rows = [
        build_slot_row(program_path, slot_rows)
        for program_path, slot_rows in slot_groups.items()
    ]
    slot_rows.sort(key=lambda item: str(item.get("program_path") or ""))
    families = sorted(
        {str(row.get("family") or "") for row in rows if str(row.get("family") or "")}
    )
    positive_matches = [match_value(row) for row in rows if match_value(row) > 0.0]
    matching_names: list[str] = []
    lifted_names: list[str] = []
    missing_names: list[str] = []
    exact_functions = 0
    asm_exact_functions = 0
    for row in rows:
        name = function_display_name(row)
        value = match_value(row)
        if value > 0.0:
            matching_names.append(name)
            if value >= 100.0:
                exact_functions += 1
        elif bool(row.get("has_source_mapping")):
            lifted_names.append(name)
        else:
            missing_names.append(name)
        asm_exact_functions += int(bool(row.get("asm_exact")))
    total = len(rows)
    matching = len(matching_names)
    lifted = len(lifted_names)
    missing = len(missing_names)
    completed = matching + lifted
    return {
        "binary_path": binary_path,
        "family": ", ".join(families),
        "program_count": len(slot_rows),
        "programs": slot_rows,
        "total_functions": total,
        "matching_functions": matching,
        "exact_functions": exact_functions,
        "asm_exact_functions": asm_exact_functions,
        "lifted_functions": lifted,
        "missing_functions": missing,
        "completion_percent": 0.0
        if total == 0
        else round((float(completed) / float(total)) * 100.0, 3),
        "matching_percent": 0.0
        if total == 0
        else round((float(matching) / float(total)) * 100.0, 3),
        "highest_match_percent": None
        if not positive_matches
        else max(positive_matches),
        "lowest_match_percent": None if not positive_matches else min(positive_matches),
        "matching_function_names": sorted(matching_names),
        "lifted_function_names": sorted(lifted_names),
        "missing_function_names": sorted(missing_names),
    }


def build_views(binary_rows: list[dict[str, Any]]) -> dict[str, list[dict[str, Any]]]:
    most_complete = sorted(
        binary_rows,
        key=lambda row: (
            -float(row.get("completion_percent") or 0.0),
            -int(row.get("matching_functions") or 0),
            -int(row.get("lifted_functions") or 0),
            str(row.get("binary_path") or ""),
        ),
    )
    biggest_gaps = sorted(
        binary_rows,
        key=lambda row: (
            -int(row.get("missing_functions") or 0),
            -int(row.get("total_functions") or 0),
            str(row.get("binary_path") or ""),
        ),
    )
    progressed = [
        row
        for row in most_complete
        if int(row.get("matching_functions") or 0)
        or int(row.get("lifted_functions") or 0)
    ]
    return {
        "most_complete": list(most_complete),
        "biggest_gaps": list(biggest_gaps),
        "progressed": progressed,
    }


def build_binary_report_payload(
    scoreboard_payload: dict[str, Any], *, profile: str = DEFAULT_PSX_PROFILE
) -> dict[str, Any]:
    grouped: dict[str, list[dict[str, Any]]] = {}
    for row in list(scoreboard_payload.get("functions") or []):
        key = binary_group_key(
            source_hint=str(row.get("source_hint") or "") or None,
            program_path=str(row.get("program_path") or "") or None,
        )
        grouped.setdefault(key, []).append(row)

    binary_rows = [
        build_binary_row(binary_path, rows) for binary_path, rows in grouped.items()
    ]
    binary_rows.sort(
        key=lambda row: (
            -float(row.get("completion_percent") or 0.0),
            -int(row.get("matching_functions") or 0),
            -int(row.get("lifted_functions") or 0),
            str(row.get("binary_path") or ""),
        )
    )

    summary = {
        "binary_count": len(binary_rows),
        "progressed_binary_count": sum(
            1
            for row in binary_rows
            if int(row.get("matching_functions") or 0)
            or int(row.get("lifted_functions") or 0)
        ),
        "total_functions": sum(
            int(row.get("total_functions") or 0) for row in binary_rows
        ),
        "matching_functions": sum(
            int(row.get("matching_functions") or 0) for row in binary_rows
        ),
        "lifted_functions": sum(
            int(row.get("lifted_functions") or 0) for row in binary_rows
        ),
        "missing_functions": sum(
            int(row.get("missing_functions") or 0) for row in binary_rows
        ),
        "exact_functions": sum(
            int(row.get("exact_functions") or 0) for row in binary_rows
        ),
        "asm_exact_functions": sum(
            int(row.get("asm_exact_functions") or 0) for row in binary_rows
        ),
    }
    views = build_views(binary_rows)
    return {
        "generated_at": scoreboard_payload.get("generated_at"),
        "profile": profile,
        "inventory_db": scoreboard_payload.get("inventory_db"),
        "match_root": scoreboard_payload.get("match_root"),
        "source_root": scoreboard_payload.get("source_root"),
        "artifact_root": scoreboard_payload.get("artifact_root"),
        "summary": summary,
        "binaries": binary_rows,
        "views": views,
    }


def render_binary_table(
    rows: list[dict[str, Any]], *, limit: int | None = None
) -> list[str]:
    if limit is not None:
        rows = rows[: max(limit, 0)]
    lines = [
        "| Family | Binary | Slots | Total | Objdiff Exact | Asm Exact | Lifted | Missing | Complete % | Matching % |",
        "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for row in rows:
        lines.append(
            f"| {row.get('family') or ''} | `{row.get('binary_path') or ''}` | {int(row.get('program_count') or 0)} | {int(row.get('total_functions') or 0)} | {int(row.get('exact_functions') or 0)} | {int(row.get('asm_exact_functions') or 0)} | {int(row.get('lifted_functions') or 0)} | {int(row.get('missing_functions') or 0)} | {float(row.get('completion_percent') or 0.0):.3f} | {float(row.get('matching_percent') or 0.0):.3f} |"
        )
    return lines


def render_tsv(binary_rows: list[dict[str, Any]]) -> str:
    header = [
        "family",
        "binary_path",
        "program_path",
        "binary_total_functions",
        "binary_matching_functions",
        "binary_lifted_functions",
        "binary_missing_functions",
        "binary_completion_percent",
        "slot_total_functions",
        "slot_matching_functions",
        "slot_exact_functions",
        "slot_lifted_functions",
        "slot_missing_functions",
        "slot_completion_percent",
        "slot_matching_percent",
        "matching_function_names",
        "lifted_function_names",
        "missing_function_names",
    ]
    lines = ["\t".join(header)]
    for binary in binary_rows:
        for slot in list(binary.get("programs") or []):
            lines.append(
                "\t".join(
                    [
                        str(slot.get("family") or binary.get("family") or ""),
                        str(binary.get("binary_path") or ""),
                        str(slot.get("program_path") or ""),
                        str(binary.get("total_functions") or 0),
                        str(binary.get("matching_functions") or 0),
                        str(binary.get("lifted_functions") or 0),
                        str(binary.get("missing_functions") or 0),
                        str(binary.get("completion_percent") or 0.0),
                        str(slot.get("total_functions") or 0),
                        str(slot.get("matching_functions") or 0),
                        str(slot.get("exact_functions") or 0),
                        str(slot.get("lifted_functions") or 0),
                        str(slot.get("missing_functions") or 0),
                        str(slot.get("completion_percent") or 0.0),
                        str(slot.get("matching_percent") or 0.0),
                        "; ".join(
                            str(name)
                            for name in slot.get("matching_function_names") or []
                        ),
                        "; ".join(
                            str(name)
                            for name in slot.get("lifted_function_names") or []
                        ),
                        "; ".join(
                            str(name)
                            for name in slot.get("missing_function_names") or []
                        ),
                    ]
                )
            )
    return "\n".join(lines) + "\n"


def render_markdown(
    payload: dict[str, Any], *, view: str = "full", table_limit: int = 15
) -> str:
    summary = payload.get("summary") or {}
    binary_rows = list(payload.get("binaries") or [])
    views = payload.get("views") or {}
    most_complete = list(views.get("most_complete") or [])
    biggest_gaps = list(views.get("biggest_gaps") or [])
    progressed = list(views.get("progressed") or [])
    lines = [
        "# Enhanced Binary Report",
        "",
        f"- Generated: {payload.get('generated_at') or 'unknown'}",
        f"- Profile: {payload.get('profile') or DEFAULT_PSX_PROFILE}",
        f"- Binary groups: {summary.get('binary_count') or 0}",
        f"- Progressed binaries: {summary.get('progressed_binary_count') or 0}",
        f"- Matching functions: {summary.get('matching_functions') or 0}",
        f"- Asm-differ exact functions: {summary.get('asm_exact_functions') or 0}",
        f"- Lifted functions: {summary.get('lifted_functions') or 0}",
        f"- Missing functions: {summary.get('missing_functions') or 0}",
        "",
        "## Most Complete Binaries",
        "",
    ]
    lines.extend(render_binary_table(most_complete, limit=table_limit))
    lines.extend(
        [
            "",
            "## Biggest Remaining Gaps",
            "",
            *render_binary_table(biggest_gaps, limit=table_limit),
            "",
            "## Progressed Binaries",
            "",
            *render_binary_table(
                progressed, limit=None if view == "full" else table_limit
            ),
            "",
        ]
    )
    if view == "summary":
        return "\n".join(lines)
    for row in progressed:
        lines.extend(
            [
                f"## `{row.get('binary_path') or ''}`",
                "",
                f"- Family: {row.get('family') or ''}",
                f"- Slot programs: {int(row.get('program_count') or 0)}",
                f"- Total functions: {int(row.get('total_functions') or 0)}",
                f"- Matching: {int(row.get('matching_functions') or 0)}",
                f"- Objdiff exact: {int(row.get('exact_functions') or 0)}",
                f"- Asm-differ exact: {int(row.get('asm_exact_functions') or 0)}",
                f"- Lifted: {int(row.get('lifted_functions') or 0)}",
                f"- Missing: {int(row.get('missing_functions') or 0)}",
                f"- Complete %: {float(row.get('completion_percent') or 0.0):.3f}",
                f"- Matching %: {float(row.get('matching_percent') or 0.0):.3f}",
                "",
                "| Slot | Total | Objdiff Exact | Asm Exact | Matching | Lifted | Missing | Complete % | Matching % |",
                "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
            ]
        )
        for slot in list(row.get("programs") or []):
            lines.append(
                f"| `{slot.get('program_path') or ''}` | {int(slot.get('total_functions') or 0)} | {int(slot.get('exact_functions') or 0)} | {int(slot.get('asm_exact_functions') or 0)} | {int(slot.get('matching_functions') or 0)} | {int(slot.get('lifted_functions') or 0)} | {int(slot.get('missing_functions') or 0)} | {float(slot.get('completion_percent') or 0.0):.3f} | {float(slot.get('matching_percent') or 0.0):.3f} |"
            )
            lines.append(
                f"Matching: {', '.join(str(name) for name in slot.get('matching_function_names') or []) or 'None'}  "
            )
            lines.append(
                f"Lifted: {', '.join(str(name) for name in slot.get('lifted_function_names') or []) or 'None'}  "
            )
            lines.append(
                f"Missing: {', '.join(str(name) for name in slot.get('missing_function_names') or []) or 'None'}  "
            )
            lines.append("")
    return "\n".join(lines)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "enhanced-report"),
        description="Emit per-binary decomp coverage grouped by owning binary/artifact with slot-level metrics.",
    )
    add_logging_args(parser)
    parser.add_argument(
        "--inventory-db", type=Path, default=scoreboard_lib.DEFAULT_INVENTORY_DB
    )
    parser.add_argument("--match-root", type=Path, default=DEFAULT_MATCH_ROOT)
    parser.add_argument(
        "--source-root", type=Path, default=scoreboard_lib.DEFAULT_SOURCE_ROOT
    )
    parser.add_argument(
        "--artifact-root",
        type=Path,
        default=scoreboard_lib.workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
    )
    parser.add_argument("-P", "--profile", default=DEFAULT_PSX_PROFILE)
    parser.add_argument(
        "--view",
        choices=["full", "summary"],
        default="full",
        help="full includes per-binary slot/function detail; summary keeps only ranked overview tables",
    )
    parser.add_argument(
        "--table-limit",
        type=int,
        default=15,
        help="row limit for each ranked overview table",
    )
    parser.add_argument("--output-json", type=Path)
    parser.add_argument("--output-tsv", type=Path)
    parser.add_argument("--output-md", type=Path)
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_enhanced_report")
    output_json, output_tsv, output_md = default_output_paths(
        args.match_root, args.profile
    )
    if args.view == "summary":
        output_md = default_summary_md_path(args.match_root, args.profile)
    if args.output_json is not None:
        output_json = args.output_json
    if args.output_tsv is not None:
        output_tsv = args.output_tsv
    if args.output_md is not None:
        output_md = args.output_md
    scoreboard_payload = scoreboard_lib.build_scoreboard_payload(
        inventory_db=args.inventory_db,
        match_root=args.match_root,
        source_root=args.source_root,
        artifact_root=args.artifact_root,
    )
    payload = build_binary_report_payload(scoreboard_payload, profile=args.profile)
    write_json_output(output_json, payload)
    write_text_output(output_tsv, render_tsv(list(payload.get("binaries") or [])))
    write_markdown_output(
        output_md,
        render_markdown(payload, view=args.view, table_limit=args.table_limit),
    )
    logger.summary(
        " ".join(
            [
                f"view={args.view}",
                f"binaries={len(payload.get('binaries') or [])}",
                f"json={relative_to_root(output_json)}",
                f"tsv={relative_to_root(output_tsv)}",
                f"md={relative_to_root(output_md)}",
            ]
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
