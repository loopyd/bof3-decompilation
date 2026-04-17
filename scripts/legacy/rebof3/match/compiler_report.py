from __future__ import annotations

import argparse
import json
import os
import shlex
import shutil
import statistics
import subprocess
import tempfile
import re
from dataclasses import dataclass
from datetime import date
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import relative_to_root, write_json_output, write_text_output
from ..config import (
    GCC272_PSX_GCC,
    OLD_GCC_TOOLCHAINS_ROOT,
    PSN00B_TOOLCHAIN_BIN,
    ROOT,
)
from ..inventory.layout import INVENTORY_SQLITE
from ..toolchain.old_gcc_catalog import (
    DEFAULT_OLD_GCC_COMPILER_SET,
    OLD_GCC_TESTED_MATRIX_COMPILER_IDS,
    expand_compiler_ids,
    release_for_compiler,
)
from . import asm_differ_backend, baseline, objdiff_backend, source_map
from . import sweep as sweep_lib
from . import workspace as workspace_lib


DEFAULT_BUILD_ROOT = ROOT / "build" / "bof3-psyq40"
DEFAULT_REPORT_ROOT = ROOT / "tmp" / "compiler_reports"
DEFAULT_SOURCE_PREFIX = "bof3/src/modules"
DEFAULT_COMPILER_ID = "gcc-2.7.2-psx"


@dataclass(frozen=True, slots=True)
class CompilerSpec:
    compiler_id: str
    compiler_label: str
    gcc_path: Path
    kind: str = "legacy"


def requested_compiler_ids(
    requested_ids: list[str] | None,
    compiler_sets: list[str] | None,
) -> tuple[str, ...]:
    return expand_compiler_ids(
        requested_ids,
        compiler_sets,
        default_ids=(DEFAULT_COMPILER_ID,),
        set_prefix_ids={DEFAULT_OLD_GCC_COMPILER_SET: (DEFAULT_COMPILER_ID,)},
    )


def known_compiler_specs(*, old_gcc_root: Path) -> tuple[CompilerSpec, ...]:
    specs = [
        CompilerSpec(
            compiler_id=DEFAULT_COMPILER_ID,
            compiler_label=f"{DEFAULT_COMPILER_ID} + maspsx",
            gcc_path=GCC272_PSX_GCC,
        )
    ]
    for compiler_id in OLD_GCC_TESTED_MATRIX_COMPILER_IDS:
        release = release_for_compiler(compiler_id)
        specs.append(
            CompilerSpec(
                compiler_id=compiler_id,
                compiler_label=f"{compiler_id} + maspsx",
                gcc_path=release.install_path(old_gcc_root) / "gcc",
            )
        )
    return tuple(specs)


def resolve_compilers(
    compiler_ids: tuple[str, ...], *, old_gcc_root: Path
) -> tuple[CompilerSpec, ...]:
    known = {
        spec.compiler_id: spec
        for spec in known_compiler_specs(old_gcc_root=old_gcc_root)
    }
    resolved: list[CompilerSpec] = []
    for compiler_id in compiler_ids:
        if compiler_id not in known:
            raise LookupError(f"unknown compiler id: {compiler_id}")
        spec = known[compiler_id]
        if not spec.gcc_path.exists():
            if compiler_id == DEFAULT_COMPILER_ID:
                raise FileNotFoundError(
                    f"missing canonical compiler: {relative_to_root(spec.gcc_path)}; run `make setup_toolchain`"
                )
            raise FileNotFoundError(
                f"missing compiler: {relative_to_root(spec.gcc_path)}; run `python3 -m scripts.rebof3 re setup-old-gcc --compiler {compiler_id}`"
            )
        resolved.append(spec)
    return tuple(resolved)


def list_compilers(*, old_gcc_root: Path) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for spec in known_compiler_specs(old_gcc_root=old_gcc_root):
        rows.append(
            {
                "compiler_id": spec.compiler_id,
                "gcc_path": relative_to_root(spec.gcc_path),
                "installed": "yes" if spec.gcc_path.exists() else "no",
            }
        )
    return rows


def ensure_build_configured(*, build_root: Path, report_root: Path) -> None:
    log_path = report_root / "configure.log"
    result = subprocess.run(
        ["cmake", "--preset", build_root.name],
        cwd=ROOT,
        capture_output=True,
        text=True,
        errors="replace",
        check=False,
    )
    write_text_output(
        log_path, result.stdout + ("" if not result.stderr else "\n" + result.stderr)
    )
    if result.returncode != 0:
        raise RuntimeError(f"cmake configure failed; see {relative_to_root(log_path)}")


def load_compile_commands(*, build_root: Path) -> dict[str, dict[str, Any]]:
    compile_commands_path = build_root / "compile_commands.json"
    if not compile_commands_path.exists():
        raise FileNotFoundError(
            f"compile_commands.json not found: {compile_commands_path}"
        )
    payload = json.loads(compile_commands_path.read_text(encoding="utf-8"))
    commands: dict[str, dict[str, Any]] = {}
    for entry in payload:
        file_path = Path(str(entry["file"])).resolve()
        try:
            relative = file_path.relative_to(ROOT).as_posix()
        except ValueError:
            continue
        commands[relative] = dict(entry)
    return commands


def load_targets(
    *,
    inventory_db: Path,
    build_root: Path,
    allow_synthetic: bool,
    skip_empty_stubs: bool,
    exact_source_files: tuple[str, ...] = (),
    exact_source_functions: tuple[str, ...] = (),
) -> dict[str, list[dict[str, Any]]]:
    if not inventory_db.exists():
        raise FileNotFoundError(f"inventory db not found: {inventory_db}")

    rows = workspace_lib.load_function_rows(inventory_db)
    program_rows = workspace_lib.load_program_rows(inventory_db)
    targets: list[dict[str, Any]] = []
    unresolved: list[dict[str, Any]] = []
    seen: set[tuple[str, str]] = set()

    for mapping in sorted(
        collect_requested_mappings(
            source_root=ROOT / "bof3",
            exact_source_files=exact_source_files,
            exact_source_functions=exact_source_functions,
        ),
        key=lambda item: (
            str(item.get("source_file") or ""),
            str(item.get("entry_hex") or ""),
            str(item.get("source_function") or ""),
        ),
    ):
        target, unresolved_row = resolve_target(
            mapping,
            rows=rows,
            program_rows=program_rows,
            build_root=build_root,
            allow_synthetic=allow_synthetic,
            skip_empty_stubs=skip_empty_stubs,
        )
        if unresolved_row is not None:
            unresolved.append(unresolved_row)
            continue
        if target is None:
            continue
        key = (
            str(target.get("program_path") or ""),
            str(target.get("entry_hex") or ""),
        )
        if key in seen:
            continue
        seen.add(key)
        targets.append(target)

    targets.sort(key=lambda item: str(item["source_file"]))
    return {"targets": targets, "unresolved": unresolved}


def filter_targets(
    targets: list[dict[str, Any]],
    *,
    source_prefixes: tuple[str, ...],
    source_files: tuple[str, ...],
    source_functions: tuple[str, ...],
) -> list[dict[str, Any]]:
    if not source_prefixes and not source_files and not source_functions:
        return list(targets)
    exact_files = set(source_files)
    exact_functions = set(source_functions)
    filtered: list[dict[str, Any]] = []
    for target in targets:
        source_file = str(target.get("source_file") or "")
        source_function = str(target.get("source_function") or "")
        path_matches = not source_prefixes and not source_files
        if not path_matches:
            path_matches = source_file in exact_files or any(
                source_file.startswith(prefix) for prefix in source_prefixes
            )
        function_matches = not exact_functions or source_function in exact_functions
        if path_matches and function_matches:
            filtered.append(target)
    return filtered


def is_empty_stub_source(source_path: Path) -> bool:
    text = source_path.read_text(encoding="utf-8", errors="replace")
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//.*$", "", text, flags=re.M)
    text = re.sub(r"^\s*#include[^\n]*$", "", text, flags=re.M)
    squashed = "".join(text.split())
    return re.fullmatch(r"voidfunc_[0-9A-Fa-f]+\(void\)\{\}", squashed) is not None


def unresolved_target(mapping: dict[str, Any], *, reason: str) -> dict[str, Any]:
    return {
        "source_file": mapping.get("source_file"),
        "source_function": mapping.get("source_function"),
        "entry_hex": mapping.get("entry_hex"),
        "reason": reason,
    }


def collect_requested_mappings(
    *,
    source_root: Path,
    exact_source_files: tuple[str, ...],
    exact_source_functions: tuple[str, ...],
) -> list[dict[str, Any]]:
    if len(exact_source_files) != 1:
        return source_map.collect_source_mappings(source_root)

    source_file = exact_source_files[0]
    source_path = ROOT / source_file
    if not source_path.exists():
        return []
    try:
        text = source_path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return []
    if source_map.DISABLED_STUB_MARKER in text:
        return []

    mappings = source_map.extract_tagged_functions_from_text(
        text, file_path=source_file
    )
    if exact_source_functions:
        allowed = set(exact_source_functions)
        mappings = [
            mapping
            for mapping in mappings
            if str(mapping.get("source_function") or "") in allowed
        ]
    return mappings


def resolve_target(
    mapping: dict[str, Any],
    *,
    rows: list[dict[str, Any]],
    program_rows: list[dict[str, Any]],
    build_root: Path,
    allow_synthetic: bool,
    skip_empty_stubs: bool,
) -> tuple[dict[str, Any] | None, dict[str, Any] | None]:
    source_file = str(mapping.get("source_file") or "")
    source_path = ROOT / source_file if source_file else None
    if skip_empty_stubs and source_path is not None and source_path.exists():
        if is_empty_stub_source(source_path):
            return None, unresolved_target(mapping, reason="empty_stub_source")

    row = sweep_lib.resolve_row_for_mapping(rows, mapping)
    if row is None and allow_synthetic:
        program_row = sweep_lib.select_seed_program_row(
            mapping, program_rows=program_rows
        )
        if program_row is not None:
            row = workspace_lib.build_synthetic_function_row(
                program_row,
                entry=str(mapping.get("entry_hex") or ""),
                source_function=str(mapping.get("source_function") or "") or None,
                source_signature=str(mapping.get("source_signature") or "") or None,
            )
    if row is None:
        return None, unresolved_target(mapping, reason="missing_inventory_row")

    artifacts_dir = workspace_lib.suggested_artifacts_dir(
        row,
        workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
        source_override=None,
    )
    bundle_json = None if artifacts_dir is None else artifacts_dir / "func.json"
    if bundle_json is None or not bundle_json.exists():
        return None, unresolved_target(mapping, reason="missing_bundle_json")

    baseline_info = baseline.baseline_from_bundle_json(bundle_json)
    if baseline_info is None:
        return None, unresolved_target(mapping, reason="missing_baseline")

    return {
        "source_file": str(mapping["source_file"]),
        "source_function": str(mapping["source_function"]),
        "entry_hex": str(row["entry_hex"]),
        "bundle_json": relative_to_root(bundle_json),
        "source_hint": row.get("source_hint"),
        "program_path": row.get("program_path"),
        "object_candidates": source_map.predict_object_candidates(
            str(mapping["source_file"]),
            build_root=build_root.parent,
        ),
        "baseline_symbol_name": baseline_info.get("symbol_name"),
    }, None


def resolve_source_filters(
    args: argparse.Namespace,
) -> tuple[tuple[str, ...], tuple[str, ...], tuple[str, ...]]:
    source_prefixes = tuple(args.source_prefix or ())
    source_files = tuple(args.source_file or ())
    source_functions = tuple(args.source_function or ())
    if not source_prefixes and not source_files and not source_functions:
        source_prefixes = (DEFAULT_SOURCE_PREFIX,)
    return source_prefixes, source_files, source_functions


def build_compile_units(
    targets: list[dict[str, Any]],
    compile_commands: dict[str, dict[str, Any]],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    grouped: dict[str, dict[str, Any]] = {}
    unresolved: list[dict[str, Any]] = []
    for target in targets:
        source_file = str(target["source_file"])
        command = compile_commands.get(source_file)
        if command is None:
            unresolved.append(
                {
                    "source_file": source_file,
                    "source_function": target.get("source_function"),
                    "entry_hex": target.get("entry_hex"),
                    "reason": "missing_compile_command",
                }
            )
            continue
        unit = grouped.setdefault(
            source_file,
            {
                "source_file": source_file,
                "directory": str(command["directory"]),
                "command": str(command["command"]),
                "output": str(command["output"]),
                "targets": [],
            },
        )
        unit["targets"].append(target)
    return [grouped[key] for key in sorted(grouped)], unresolved


def build_env(compiler: CompilerSpec) -> dict[str, str]:
    env = dict(os.environ)
    path_entries = [str(PSN00B_TOOLCHAIN_BIN)]
    existing_path = env.get("PATH")
    if existing_path:
        path_entries.append(existing_path)
    env["PATH"] = os.pathsep.join(path_entries)
    env["BOF3_PROFILE"] = "capcom97-bof3"
    env["BOF3_PSX_GCC"] = str(compiler.gcc_path)
    env["BOF3_PSX_GCC_ROOT"] = str(compiler.gcc_path.parent)
    return env


def run_compile_unit(
    compiler: CompilerSpec,
    unit: dict[str, Any],
    *,
    report_root: Path,
) -> dict[str, Any]:
    source_file = str(unit["source_file"])
    source_dir = report_root / compiler.compiler_id / Path(source_file).with_suffix("")
    log_path = source_dir / "compile.log"
    source_dir.mkdir(parents=True, exist_ok=True)

    output_path = Path(str(unit["output"]))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.unlink(missing_ok=True)

    result = subprocess.run(
        shlex.split(str(unit["command"])),
        cwd=Path(str(unit["directory"])),
        env=build_env(compiler),
        capture_output=True,
        text=True,
        errors="replace",
        check=False,
    )
    write_text_output(
        log_path, result.stdout + ("" if not result.stderr else "\n" + result.stderr)
    )
    return {
        "source_file": source_file,
        "output": relative_to_root(output_path),
        "compile_log": relative_to_root(log_path),
        "returncode": int(result.returncode),
        "succeeded": result.returncode == 0 and output_path.exists(),
    }


def collect_objdiff_for_target(
    compiler: CompilerSpec,
    target: dict[str, Any],
    *,
    report_root: Path,
) -> dict[str, Any]:
    workspace_dir = (
        report_root
        / compiler.compiler_id
        / Path(str(target["source_file"])).with_suffix("")
    )
    workspace_payload = {
        "workspace_dir": relative_to_root(workspace_dir),
        "name": target["source_function"],
        "source_mapping": {
            "source_file": target["source_file"],
            "source_function": target["source_function"],
            "object_candidates": list(target.get("object_candidates") or []),
        },
        "expected_baseline": baseline.baseline_from_bundle_json(
            ROOT / str(target["bundle_json"])
        ),
    }

    try:
        asm_prepared = asm_differ_backend.prepare_backend(
            workspace_dir, workspace_payload
        )
        obj_prepared = objdiff_backend.prepare_backend(
            workspace_dir,
            workspace_payload,
            asm_backend_report=asm_prepared,
        )
        obj_result = objdiff_backend.run_backend(obj_prepared)
        obj_report = objdiff_backend.write_backend_outputs(obj_prepared, obj_result)
        diff_summary = obj_report.get("diff_summary") or {}
        return {
            **target,
            "compiler_id": compiler.compiler_id,
            "compiler_label": compiler.compiler_label,
            "status": "ok" if obj_report.get("succeeded") else "objdiff_failed",
            "object_path": asm_prepared.get("current_object_source"),
            "objdiff_backend_report": obj_report.get("report_path"),
            "match_metrics": {
                "objdiff_backend_report": obj_report.get("report_path"),
                "objdiff_instruction_count": diff_summary.get("instruction_count"),
                "objdiff_match_percent": diff_summary.get("text_match_percent"),
                "objdiff_mismatch_count": diff_summary.get("mismatch_count"),
            },
        }
    except Exception as exc:  # noqa: BLE001
        return {
            **target,
            "compiler_id": compiler.compiler_id,
            "compiler_label": compiler.compiler_label,
            "status": "error",
            "error": f"{type(exc).__name__}: {exc}",
        }


def summarize_rows(
    compiler: CompilerSpec,
    rows: list[dict[str, Any]],
    *,
    total_targets: int,
) -> dict[str, Any]:
    ok_rows = [
        row
        for row in rows
        if row.get("compiler_id") == compiler.compiler_id
        and row.get("status") == "ok"
        and (row.get("match_metrics") or {}).get("objdiff_match_percent") is not None
    ]
    values = sorted(
        float((row.get("match_metrics") or {}).get("objdiff_match_percent") or 0.0)
        for row in ok_rows
    )
    average = None if not values else sum(values) / len(values)
    return {
        "compiler_id": compiler.compiler_id,
        "compiler_label": compiler.compiler_label,
        "kind": compiler.kind,
        "gcc_path": str(compiler.gcc_path),
        "total_functions": total_targets,
        "successful_functions": len(ok_rows),
        "failed_functions": total_targets - len(ok_rows),
        "highest_objdiff_match_percent": None if not values else max(values),
        "lowest_objdiff_match_percent": None if not values else min(values),
        "average_objdiff_match_percent": average,
        "median_objdiff_match_percent": None
        if not values
        else statistics.median(values),
    }


def sort_summary_rows(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    return sorted(
        rows,
        key=lambda row: (
            float("inf")
            if row.get("highest_objdiff_match_percent") is None
            else -float(row["highest_objdiff_match_percent"]),
            float("inf")
            if row.get("average_objdiff_match_percent") is None
            else -float(row["average_objdiff_match_percent"]),
            -int(row.get("successful_functions") or 0),
            str(row.get("compiler_id") or ""),
        ),
    )


def sort_function_rows(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    return sorted(
        rows,
        key=lambda row: (
            float("inf")
            if (row.get("match_metrics") or {}).get("objdiff_match_percent") is None
            else -float((row.get("match_metrics") or {}).get("objdiff_match_percent")),
            float("inf")
            if (row.get("match_metrics") or {}).get("objdiff_mismatch_count") is None
            else float((row.get("match_metrics") or {}).get("objdiff_mismatch_count")),
            str(row.get("compiler_id") or ""),
            str(row.get("source_file") or ""),
            str(row.get("source_function") or ""),
            str(row.get("entry_hex") or ""),
        ),
    )


def render_summary_tsv(summary_rows: list[dict[str, Any]]) -> str:
    header = [
        "compiler_id",
        "compiler_label",
        "kind",
        "successful_functions",
        "failed_functions",
        "highest_objdiff_match_percent",
        "lowest_objdiff_match_percent",
        "average_objdiff_match_percent",
        "median_objdiff_match_percent",
    ]
    lines = ["\t".join(header)]
    for row in summary_rows:
        lines.append(
            "\t".join(
                [
                    str(row.get("compiler_id") or ""),
                    str(row.get("compiler_label") or ""),
                    str(row.get("kind") or ""),
                    str(row.get("successful_functions") or 0),
                    str(row.get("failed_functions") or 0),
                    ""
                    if row.get("highest_objdiff_match_percent") is None
                    else f"{float(row['highest_objdiff_match_percent']):.6f}",
                    ""
                    if row.get("lowest_objdiff_match_percent") is None
                    else f"{float(row['lowest_objdiff_match_percent']):.6f}",
                    ""
                    if row.get("average_objdiff_match_percent") is None
                    else f"{float(row['average_objdiff_match_percent']):.6f}",
                    ""
                    if row.get("median_objdiff_match_percent") is None
                    else f"{float(row['median_objdiff_match_percent']):.6f}",
                ]
            )
        )
    return "\n".join(lines) + "\n"


def _stringify_table_value(value: Any) -> str:
    if value is None:
        return ""
    return str(value)


def render_table(headers: list[str], rows: list[list[Any]]) -> str:
    table_rows = [[_stringify_table_value(cell) for cell in row] for row in rows]
    widths = [
        max(len(header), *(len(row[index]) for row in table_rows))
        for index, header in enumerate(headers)
    ]
    header_line = "  ".join(
        header.ljust(widths[index]) for index, header in enumerate(headers)
    )
    separator_line = "  ".join("-" * widths[index] for index, _ in enumerate(headers))
    body_lines = [
        "  ".join(cell.ljust(widths[index]) for index, cell in enumerate(row))
        for row in table_rows
    ]
    return "\n".join([header_line, separator_line, *body_lines]) + "\n"


def render_summary_table(summary_rows: list[dict[str, Any]]) -> str:
    headers = [
        "compiler_id",
        "ok",
        "fail",
        "best",
        "worst",
        "avg",
        "median",
    ]
    rows = [
        [
            str(row.get("compiler_id") or ""),
            str(row.get("successful_functions") or 0),
            str(row.get("failed_functions") or 0),
            ""
            if row.get("highest_objdiff_match_percent") is None
            else f"{float(row['highest_objdiff_match_percent']):.6f}",
            ""
            if row.get("lowest_objdiff_match_percent") is None
            else f"{float(row['lowest_objdiff_match_percent']):.6f}",
            ""
            if row.get("average_objdiff_match_percent") is None
            else f"{float(row['average_objdiff_match_percent']):.6f}",
            ""
            if row.get("median_objdiff_match_percent") is None
            else f"{float(row['median_objdiff_match_percent']):.6f}",
        ]
        for row in summary_rows
    ]
    return render_table(headers, rows)


def _format_metric(value: Any) -> str:
    if value is None:
        return "n/a"
    return f"{float(value):.3f}"


def render_summary_brief(summary_rows: list[dict[str, Any]]) -> str:
    lines: list[str] = []
    for row in summary_rows:
        lines.append(
            f"{row.get('compiler_id') or 'unknown'}: "
            f"ok {row.get('successful_functions') or 0}, "
            f"fail {row.get('failed_functions') or 0}, "
            f"best {_format_metric(row.get('highest_objdiff_match_percent'))}, "
            f"avg {_format_metric(row.get('average_objdiff_match_percent'))}, "
            f"median {_format_metric(row.get('median_objdiff_match_percent'))}, "
            f"worst {_format_metric(row.get('lowest_objdiff_match_percent'))}"
        )
    return "\n".join(lines) + ("\n" if lines else "")


def render_function_rows_tsv(rows: list[dict[str, Any]]) -> str:
    header = [
        "compiler_id",
        "source_file",
        "source_function",
        "status",
        "objdiff_match_percent",
        "objdiff_mismatch_count",
        "entry_hex",
        "program_path",
        "objdiff_backend_report",
    ]
    lines = ["\t".join(header)]
    for row in rows:
        metrics = row.get("match_metrics") or {}
        match_percent = metrics.get("objdiff_match_percent")
        mismatch_count = metrics.get("objdiff_mismatch_count")
        lines.append(
            "\t".join(
                [
                    str(row.get("compiler_id") or ""),
                    str(row.get("source_file") or ""),
                    str(row.get("source_function") or ""),
                    str(row.get("status") or ""),
                    "" if match_percent is None else str(match_percent),
                    "" if mismatch_count is None else str(mismatch_count),
                    str(row.get("entry_hex") or ""),
                    str(row.get("program_path") or ""),
                    str(row.get("objdiff_backend_report") or ""),
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_function_rows_table(rows: list[dict[str, Any]]) -> str:
    headers = [
        "compiler_id",
        "source_file",
        "source_function",
        "status",
        "match",
        "mismatches",
    ]
    table_rows = []
    for row in rows:
        metrics = row.get("match_metrics") or {}
        match_percent = metrics.get("objdiff_match_percent")
        mismatch_count = metrics.get("objdiff_mismatch_count")
        table_rows.append(
            [
                str(row.get("compiler_id") or ""),
                str(row.get("source_file") or ""),
                str(row.get("source_function") or ""),
                str(row.get("status") or ""),
                "" if match_percent is None else str(match_percent),
                "" if mismatch_count is None else str(mismatch_count),
            ]
        )
    return render_table(headers, table_rows)


def render_function_rows_brief(rows: list[dict[str, Any]]) -> str:
    lines: list[str] = []
    for row in rows:
        metrics = row.get("match_metrics") or {}
        parts = [
            f"{row.get('source_function') or '<unknown>'}: {row.get('status') or 'unknown'}",
            f"match {_format_metric(metrics.get('objdiff_match_percent'))}",
            "mismatches "
            + (
                "n/a"
                if metrics.get("objdiff_mismatch_count") is None
                else str(metrics.get("objdiff_mismatch_count"))
            ),
        ]
        entry_hex = row.get("entry_hex")
        if entry_hex:
            parts.append(f"entry {entry_hex}")
        source_file = row.get("source_file")
        if source_file:
            parts.append(str(source_file))
        error = row.get("error")
        if error:
            parts.append(f"error {error}")
        lines.append(", ".join(parts))
    return "\n".join(lines) + ("\n" if lines else "")


def render_stdout_payload(
    *,
    summary_rows: list[dict[str, Any]],
    function_rows: list[dict[str, Any]],
    view: str,
    output_format: str,
) -> str:
    summary_text = ""
    functions_text = ""

    if view in ("summary", "both"):
        if output_format == "brief":
            summary_text = render_summary_brief(summary_rows)
        elif output_format == "table":
            summary_text = render_summary_table(summary_rows)
        elif output_format == "tsv":
            summary_text = render_summary_tsv(summary_rows)
        else:
            summary_text = json.dumps(summary_rows, indent=2, sort_keys=True) + "\n"

    if view in ("functions", "both"):
        if output_format == "brief":
            functions_text = render_function_rows_brief(function_rows)
        elif output_format == "table":
            functions_text = render_function_rows_table(function_rows)
        elif output_format == "tsv":
            functions_text = render_function_rows_tsv(function_rows)
        else:
            functions_text = json.dumps(function_rows, indent=2, sort_keys=True) + "\n"

    if view == "both":
        if output_format == "brief":
            summary_block = "Summary\n" + summary_text.rstrip("\n")
            functions_block = "Functions\n" + functions_text.rstrip("\n")
            return summary_block + "\n\n" + functions_block + "\n"
        return summary_text.rstrip("\n") + "\n\n" + functions_text
    if view == "summary":
        return summary_text
    return functions_text


def write_report_outputs(
    *,
    report_root: Path,
    targets: list[dict[str, Any]],
    unresolved: list[dict[str, Any]],
    compiler_ids: tuple[str, ...],
    source_prefixes: tuple[str, ...],
    source_files: tuple[str, ...],
    source_functions: tuple[str, ...],
    summary_rows: list[dict[str, Any]],
    rows: list[dict[str, Any]],
) -> None:
    write_json_output(
        report_root / "targets.json",
        {
            "targets": targets,
            "unresolved": unresolved,
            "compiler_ids": list(compiler_ids),
            "source_prefixes": list(source_prefixes),
            "source_files": list(source_files),
            "source_functions": list(source_functions),
        },
    )
    write_json_output(
        report_root / "summary.json",
        {
            "report_root": relative_to_root(report_root),
            "compiler_ids": list(compiler_ids),
            "source_prefixes": list(source_prefixes),
            "source_files": list(source_files),
            "source_functions": list(source_functions),
            "summary_rows": summary_rows,
            "unresolved_count": len(unresolved),
        },
    )
    write_text_output(report_root / "results.tsv", render_summary_tsv(summary_rows))
    write_text_output(report_root / "function_rows.tsv", render_function_rows_tsv(rows))
    write_text_output(
        report_root / "rows.jsonl",
        "".join(json.dumps(row, sort_keys=True) + "\n" for row in rows),
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "compiler-report"),
        description="Build mapped source files under one or more compiler wrappers and emit objdiff-backed report rows.",
    )
    add_logging_args(parser)
    parser.add_argument(
        "--compiler",
        action="append",
        choices=(DEFAULT_COMPILER_ID, *OLD_GCC_TESTED_MATRIX_COMPILER_IDS),
        help="Compiler id to include. May be passed multiple times.",
    )
    parser.add_argument(
        "--compiler-set",
        action="append",
        choices=(DEFAULT_OLD_GCC_COMPILER_SET,),
        help=(
            "Named compiler set. "
            f"`{DEFAULT_OLD_GCC_COMPILER_SET}` expands to the canonical compiler "
            "plus the optional old-gcc matrix."
        ),
    )
    parser.add_argument(
        "--source-prefix",
        action="append",
        default=None,
        help=f"Repo-relative source prefix filter. Defaults to {DEFAULT_SOURCE_PREFIX}.",
    )
    parser.add_argument(
        "--source-file",
        action="append",
        default=None,
        help="Exact repo-relative source file to include. May be passed multiple times.",
    )
    parser.add_argument(
        "--source-function",
        action="append",
        default=None,
        help="Exact function symbol to include. May be passed multiple times.",
    )
    parser.add_argument("--limit", type=int, default=None)
    parser.add_argument("--inventory-db", type=Path, default=INVENTORY_SQLITE)
    parser.add_argument("--build-root", type=Path, default=DEFAULT_BUILD_ROOT)
    parser.add_argument("--report-root", type=Path, default=DEFAULT_REPORT_ROOT)
    parser.add_argument("--old-gcc-root", type=Path, default=OLD_GCC_TOOLCHAINS_ROOT)
    parser.add_argument(
        "--run-name",
        default=f"compiler_report_{date.today().isoformat()}",
    )
    parser.add_argument(
        "--list-compilers",
        action="store_true",
        help="List known compiler ids and whether their gcc executable is installed.",
    )
    parser.add_argument(
        "--inventory-only",
        action="store_true",
        help="Only include source files backed by real inventory rows; skip synthetic placeholder/stub mappings.",
    )
    parser.add_argument(
        "--skip-empty-stubs",
        action="store_true",
        help="Skip trivial empty placeholder stub source files like `void func_xxx(void) {}`.",
    )
    parser.add_argument(
        "--output-mode",
        choices=("files", "stdout", "both"),
        default="files",
        help="Emit summary tables/files to disk, stdout, or both. Internal logs still live under the report root.",
    )
    parser.add_argument(
        "--stdout-view",
        choices=("summary", "functions", "both"),
        default="summary",
        help="Which report section to print when --output-mode includes stdout.",
    )
    parser.add_argument(
        "--stdout-format",
        choices=("brief", "table", "tsv", "json"),
        default="brief",
        help="Stdout format to use when --output-mode includes stdout.",
    )
    parser.add_argument(
        "--ephemeral",
        action="store_true",
        help="Delete the report root after the run completes. Useful with --output-mode stdout for fast prototype checks.",
    )
    parser.add_argument(
        "--quick",
        action="store_true",
        help="Shortcut for a fast prototype loop: --output-mode stdout --stdout-view functions --stdout-format brief --ephemeral.",
    )
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    if args.quick:
        args.output_mode = "stdout"
        args.stdout_view = "functions"
        args.stdout_format = "brief"
        args.ephemeral = True
    logger = logger_from_args(args, "match_compiler_report")

    if args.list_compilers:
        payload = list_compilers(old_gcc_root=args.old_gcc_root)
        for row in payload:
            logger.summary(
                f"compiler={row['compiler_id']} installed={row['installed']} gcc={row['gcc_path']}"
            )
        return 0

    compiler_ids = requested_compiler_ids(args.compiler, args.compiler_set)
    try:
        compilers = resolve_compilers(compiler_ids, old_gcc_root=args.old_gcc_root)
    except (LookupError, FileNotFoundError) as exc:
        logger.error(str(exc))
        return 1

    cleanup_report_root = bool(args.ephemeral)
    if cleanup_report_root:
        args.report_root.mkdir(parents=True, exist_ok=True)
        report_root = Path(
            tempfile.mkdtemp(prefix=f"{args.run_name}_", dir=args.report_root)
        )
    else:
        report_root = args.report_root / str(args.run_name)
        report_root.mkdir(parents=True, exist_ok=True)

    try:
        ensure_build_configured(build_root=args.build_root, report_root=report_root)
        compile_commands = load_compile_commands(build_root=args.build_root)
        target_info = load_targets(
            inventory_db=args.inventory_db,
            build_root=args.build_root,
            allow_synthetic=not args.inventory_only,
            skip_empty_stubs=args.skip_empty_stubs,
            exact_source_files=tuple(args.source_file or ()),
            exact_source_functions=tuple(args.source_function or ()),
        )
    except (FileNotFoundError, RuntimeError) as exc:
        logger.error(str(exc))
        if cleanup_report_root:
            shutil.rmtree(report_root, ignore_errors=True)
        return 1

    source_prefixes, source_files, source_functions = resolve_source_filters(args)
    targets = filter_targets(
        target_info["targets"],
        source_prefixes=source_prefixes,
        source_files=source_files,
        source_functions=source_functions,
    )
    unresolved = [
        item
        for item in target_info["unresolved"]
        if item
        in filter_targets(
            [item],
            source_prefixes=source_prefixes,
            source_files=source_files,
            source_functions=source_functions,
        )
    ]
    compile_units, compile_unresolved = build_compile_units(targets, compile_commands)
    unresolved.extend(compile_unresolved)

    if args.limit is not None:
        limit = max(args.limit, 0)
        targets = targets[:limit]
        allowed = {str(target["source_file"]) for target in targets}
        compile_units = [
            {
                **unit,
                "targets": [
                    target
                    for target in unit["targets"]
                    if str(target["source_file"]) in allowed
                ],
            }
            for unit in compile_units
            if str(unit["source_file"]) in allowed
        ]

    if not targets:
        logger.error("no targets matched the requested source filters")
        if cleanup_report_root:
            shutil.rmtree(report_root, ignore_errors=True)
        return 1

    rows: list[dict[str, Any]] = []
    for compiler in compilers:
        logger.summary(
            f"compiler={compiler.compiler_id} compile_units={len(compile_units)}"
        )
        ok_count = 0
        fail_count = 0
        for unit in compile_units:
            compile_result = run_compile_unit(
                compiler,
                unit,
                report_root=report_root,
            )
            if not compile_result["succeeded"]:
                fail_count += len(unit["targets"])
                for target in unit["targets"]:
                    rows.append(
                        {
                            **target,
                            "compiler_id": compiler.compiler_id,
                            "compiler_label": compiler.compiler_label,
                            "status": "compile_failed",
                            "error": f"compile failed; see {compile_result['compile_log']}",
                            "compile_log": compile_result["compile_log"],
                        }
                    )
                continue

            ok_count += len(unit["targets"])
            for target in unit["targets"]:
                rows.append(
                    collect_objdiff_for_target(
                        compiler,
                        target,
                        report_root=report_root,
                    )
                )
        logger.summary(
            f"compiler={compiler.compiler_id} ok={ok_count} fail={fail_count}"
        )

    summary_rows = [
        summarize_rows(compiler, rows, total_targets=len(targets))
        for compiler in compilers
    ]
    summary_rows = sort_summary_rows(summary_rows)
    rows = sort_function_rows(rows)
    if args.output_mode in ("files", "both"):
        write_report_outputs(
            report_root=report_root,
            targets=targets,
            unresolved=unresolved,
            compiler_ids=compiler_ids,
            source_prefixes=source_prefixes,
            source_files=source_files,
            source_functions=source_functions,
            summary_rows=summary_rows,
            rows=rows,
        )
    if args.output_mode in ("stdout", "both"):
        print(
            render_stdout_payload(
                summary_rows=summary_rows,
                function_rows=rows,
                view=args.stdout_view,
                output_format=args.stdout_format,
            ),
            end="",
        )
    if args.output_mode in ("files", "both") and not cleanup_report_root:
        logger.summary(
            f"report_root={relative_to_root(report_root)} summary={relative_to_root(report_root / 'results.tsv')} functions={relative_to_root(report_root / 'function_rows.tsv')}"
        )
    else:
        logger.summary(
            f"report_root={relative_to_root(report_root)} stdout_view={args.stdout_view} stdout_format={args.stdout_format} ephemeral={'yes' if cleanup_report_root else 'no'}"
        )
    if cleanup_report_root:
        shutil.rmtree(report_root, ignore_errors=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
