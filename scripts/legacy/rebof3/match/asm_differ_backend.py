from __future__ import annotations

import json
import re
import shutil
import sqlite3
import subprocess
import sys
from pathlib import Path
from typing import Any

from ..common import ROOT as COMMON_ROOT
from ..common import run_command
from ..config import ASM_DIFFER_SCRIPT, PSN00B_TOOLCHAIN_BIN
from ..inventory.db.connection import inventory_db
from . import object_slices


ROOT = COMMON_ROOT
OBJCOPY = PSN00B_TOOLCHAIN_BIN / "mipsel-none-elf-objcopy"
LOCAL_LINE_LABEL_RE = re.compile(r"\b(LM\d+)\s*$")


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


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def write_text(path: Path, text: str) -> None:
    ensure_parent(path)
    path.write_text(text, encoding="utf-8")


def maybe_load_json(text: str) -> dict[str, Any] | None:
    payload = text.strip()
    if not payload:
        return None
    try:
        loaded = json.loads(payload)
    except json.JSONDecodeError:
        return None
    if not isinstance(loaded, dict):
        return None
    return loaded


def choose_object_path(workspace_payload: dict[str, Any]) -> Path | None:
    source_mapping = workspace_payload.get("source_mapping") or {}
    candidates = source_mapping.get("object_candidates") or []
    for candidate in candidates:
        path = Path(candidate)
        if not path.is_absolute():
            path = ROOT / path
        if path.exists():
            return path.resolve()
    return None


def workspace_backend_dir(workspace_dir: Path) -> Path:
    return workspace_dir / "asm_differ"


def backend_layout(workspace_dir: Path) -> dict[str, Path]:
    backend_dir = workspace_backend_dir(workspace_dir)
    return {
        "backend_dir": backend_dir,
        "current_asm": backend_dir / "current" / "current.s",
        "current_object": backend_dir / "objects" / "current.o",
        "expected_asm": backend_dir / "expected" / "expected.s",
        "expected_object": backend_dir / "expected" / "objects" / "current.o",
        "diff_settings": backend_dir / "diff_settings.py",
        "stdout": backend_dir / "diff.stdout.json",
        "stderr": backend_dir / "diff.stderr.log",
        "report": backend_dir / "backend.json",
    }


def render_diff_settings(objdump_path: str) -> str:
    return (
        "def apply(config, args):\n"
        '    config["arch"] = "mips"\n'
        '    config["diff_obj"] = True\n'
        '    config["diff_function_symbols"] = False\n'
        '    config["show_rodata_refs"] = False\n'
        '    config["expected_dir"] = "expected"\n'
        f'    config["source_directories"] = [{str(ROOT / "bof3")!r}]\n'
        '    config["show_line_numbers_default"] = True\n'
        f'    config["objdump_executable"] = {objdump_path!r}\n'
    )


def resolve_objdump_path() -> str:
    local_objdump = PSN00B_TOOLCHAIN_BIN / "mipsel-none-elf-objdump"
    if local_objdump.exists():
        return str(local_objdump)
    for candidate in ("mipsel-none-elf-objdump", "mipsel-linux-gnu-objdump"):
        resolved = shutil.which(candidate)
        if resolved:
            return resolved
    return str(local_objdump)


def strip_local_line_labels(object_path: Path) -> None:
    objdump = resolve_objdump_path()
    result = run_command([objdump, "-t", str(object_path)])
    if result.returncode != 0:
        return
    labels: list[str] = []
    for line in result.stdout.splitlines():
        match = LOCAL_LINE_LABEL_RE.search(line)
        if match is None:
            continue
        labels.append(match.group(1))
    if not labels:
        return
    objcopy = str(OBJCOPY)
    if not OBJCOPY.exists():
        objcopy = shutil.which("mipsel-none-elf-objcopy") or objcopy
    command = [objcopy]
    for label in sorted(set(labels)):
        command.extend(["--strip-symbol", label])
    command.append(str(object_path))
    result = run_command(command)
    if result.returncode != 0:
        return


def load_program_symbol_resolver(
    workspace_payload: dict[str, Any],
) -> object_slices.AddressSymbolResolver | None:
    inventory_path_text = workspace_payload.get("inventory_db")
    program_path = str(workspace_payload.get("program_path") or "").strip()
    if not inventory_path_text or not program_path:
        return None
    inventory_path = normalize_repo_path(str(inventory_path_text))
    if inventory_path is None or not inventory_path.exists():
        return None
    function_symbols: dict[int, str] = {}
    data_symbols: dict[int, str] = {}
    with inventory_db(inventory_path) as connection:
        for row in connection.execute(
            "SELECT entry_address, name FROM functions f JOIN programs p ON p.id = f.program_id WHERE p.program_path = ?",
            (program_path,),
        ).fetchall():
            address = row[0]
            name = str(row[1] or "").strip()
            if address is None:
                continue
            normalized_name = object_slices.normalize_function_symbol_name(
                name or f"func_{int(address):08x}",
                int(address),
            )
            function_symbols[int(address)] = normalized_name
        for row in connection.execute(
            "SELECT address, kind, name FROM metadata_rows WHERE program_path = ? AND kind IN ('data', 'label', 'symbol') AND address IS NOT NULL",
            (program_path,),
        ).fetchall():
            address = row[0]
            kind = str(row[1] or "")
            name = str(row[2] or "").strip()
            if address is None:
                continue
            address_int = int(address)
            candidate_name = name or (
                f"DAT_{address_int:08x}"
                if kind != "label"
                else f"LAB_{address_int:08x}"
            )
            existing = data_symbols.get(address_int)
            if existing is None or kind == "label":
                data_symbols[address_int] = candidate_name
    return object_slices.AddressSymbolResolver(
        function_symbols=function_symbols,
        data_symbols=data_symbols,
    )


def prepare_backend(
    workspace_dir: Path, workspace_payload: dict[str, Any]
) -> dict[str, Any]:
    object_path = choose_object_path(workspace_payload)
    if object_path is None:
        raise FileNotFoundError(
            "no built object matched the workspace object candidates"
        )

    source_mapping = workspace_payload.get("source_mapping") or {}
    symbol_name = source_mapping.get("source_function") or workspace_payload.get("name")
    if not symbol_name:
        raise ValueError("workspace is missing a source function name for asm-differ")

    baseline_info = workspace_payload.get("expected_baseline") or {}
    baseline_asm_path = normalize_repo_path(baseline_info.get("asm_source"))
    baseline_symbol_name = baseline_info.get("symbol_name")
    if baseline_asm_path is None or not baseline_asm_path.exists():
        raise FileNotFoundError("workspace is missing an expected baseline asm source")
    if not baseline_symbol_name:
        raise ValueError("workspace baseline is missing a symbol name")

    layout = backend_layout(workspace_dir)
    layout["backend_dir"].mkdir(parents=True, exist_ok=True)

    current_slice = object_slices.slice_from_object(object_path, str(symbol_name))
    resolver = load_program_symbol_resolver(workspace_payload)
    object_slices.write_current_slice_asm(current_slice, layout["current_asm"])
    ensure_parent(layout["current_object"])
    shutil.copyfile(object_path, layout["current_object"])
    strip_local_line_labels(layout["current_object"])

    object_slices.write_expected_slice_asm(
        baseline_asm_path,
        original_symbol_name=str(baseline_symbol_name),
        target_symbol_name=str(symbol_name),
        output_path=layout["expected_asm"],
        resolver=resolver,
    )
    object_slices.assemble_text(layout["expected_asm"], layout["expected_object"])

    write_text(
        layout["diff_settings"],
        render_diff_settings(resolve_objdump_path()),
    )

    return {
        "backend": "asm-differ",
        "backend_dir": relative_to_root(layout["backend_dir"]),
        "current_asm": relative_to_root(layout["current_asm"]),
        "current_object": relative_to_root(layout["current_object"]),
        "current_object_source": relative_to_root(object_path),
        "expected_asm": relative_to_root(layout["expected_asm"]),
        "expected_object": relative_to_root(layout["expected_object"]),
        "expected_asm_source": relative_to_root(baseline_asm_path),
        "baseline_kind": baseline_info.get("kind"),
        "baseline_symbol_name": baseline_symbol_name,
        "diff_settings": relative_to_root(layout["diff_settings"]),
        "stdout_path": relative_to_root(layout["stdout"]),
        "stderr_path": relative_to_root(layout["stderr"]),
        "report_path": relative_to_root(layout["report"]),
        "symbol_name": str(symbol_name),
        "current_slice": {
            "start_offset": current_slice.start_offset,
            "size": current_slice.size,
        },
        "workspace_dir": workspace_payload.get("workspace_dir"),
    }


def asm_differ_command(prepared: dict[str, Any], *, json_output: bool) -> list[str]:
    command = [sys.executable, str(ASM_DIFFER_SCRIPT), "-o"]
    if json_output:
        command.extend(
            [
                "--format",
                "json",
                "--no-pager",
            ]
        )
    command.extend(
        [
            "--file",
            "objects/current.o",
            str(prepared["symbol_name"]),
        ]
    )
    return command


def viewer_command(prepared: dict[str, Any]) -> list[str]:
    return asm_differ_command(prepared, json_output=False)


def backend_command(prepared: dict[str, Any]) -> list[str]:
    return asm_differ_command(prepared, json_output=True)


def run_viewer(prepared: dict[str, Any]) -> subprocess.CompletedProcess[str]:
    backend_dir = ROOT / str(prepared["backend_dir"])
    return subprocess.run(
        viewer_command(prepared),
        cwd=backend_dir,
        check=False,
    )


def run_backend(prepared: dict[str, Any]) -> subprocess.CompletedProcess[str]:
    backend_dir = ROOT / str(prepared["backend_dir"])
    return subprocess.run(
        backend_command(prepared),
        cwd=backend_dir,
        capture_output=True,
        text=True,
        check=False,
    )


def write_backend_outputs(
    prepared: dict[str, Any], result: subprocess.CompletedProcess[str]
) -> dict[str, Any]:
    stdout_path = ROOT / str(prepared["stdout_path"])
    stderr_path = ROOT / str(prepared["stderr_path"])
    report_path = ROOT / str(prepared["report_path"])
    write_text(stdout_path, result.stdout)
    write_text(stderr_path, result.stderr)
    stdout_json = maybe_load_json(result.stdout)
    rows = stdout_json.get("rows") if stdout_json else None
    row_count = len(rows) if isinstance(rows, list) else None
    report = {
        **prepared,
        "command": backend_command(prepared),
        "returncode": int(result.returncode),
        "succeeded": result.returncode == 0,
        "diff_summary": None
        if stdout_json is None
        else {
            "arch_str": stdout_json.get("arch_str"),
            "current_score": stdout_json.get("current_score"),
            "max_score": stdout_json.get("max_score"),
            "row_count": row_count,
            "has_rows": bool(row_count),
        },
    }
    write_text(report_path, json.dumps(report, indent=2, sort_keys=True) + "\n")
    return report
