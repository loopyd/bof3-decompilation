#!/usr/bin/env python3
"""Emit one target-qualified BOF3 lift brief without editing the worktree."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tomllib
from typing import Any


ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / "tools" / "python"))

from harness.domain import (  # noqa: E402
    FUNCTION_ID_FORMAT,
    FUNCTION_ID_HELP,
    parse_function_id,
)

SYMBOL = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*) = 0x([0-9A-F]{8});$")


def run(root: Path, *args: str) -> dict[str, Any]:
    completed = subprocess.run(args, cwd=root, text=True, capture_output=True)
    result: dict[str, Any] = {"command": list(args), "exit_code": completed.returncode}
    stdout = completed.stdout.strip()
    if stdout:
        try:
            result["json"] = json.loads(stdout)
        except json.JSONDecodeError:
            result["stdout"] = stdout[-2000:]
    if completed.stderr.strip():
        result["stderr"] = completed.stderr.strip()[-2000:]
    return result


def selector(value: str) -> tuple[str, int]:
    try:
        function = parse_function_id(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError(f"expected {FUNCTION_ID_HELP}") from error
    return function.target.value, function.address


def data_table_probe(binary: Path, load_address: int, address: int) -> dict[str, Any] | None:
    """Heuristic: an alleged function whose window is mostly aligned code
    pointers (and lacks a stack prolog) is probably a mis-analyzed data table
    (sce10eff/00@0x801D2708, scena16/00@0x801F8538 precedents)."""
    if not binary.is_file() or address < load_address:
        return None
    payload = binary.read_bytes()
    offset = address - load_address
    window = payload[offset : offset + 64]
    if len(window) < 16:
        return None
    words = [
        int.from_bytes(window[i : i + 4], "little") for i in range(0, len(window), 4)
    ]
    nonzero = [w for w in words if w]
    pointers = [w for w in nonzero if w % 4 == 0 and load_address <= w < load_address + len(payload)]
    prolog = bool(words) and (words[0] >> 16) == 0x27BD  # addiu $sp,$sp,-N
    likely = len(nonzero) >= 3 and len(pointers) * 4 >= len(nonzero) * 3 and not prolog
    return {
        "window_bytes": len(window),
        "nonzero_words": len(nonzero),
        "code_pointer_words": len(pointers),
        "stack_prolog": prolog,
        "likely_data_table": likely,
        "warning": (
            "bytes look like a function-pointer table, not code: verify with raw "
            "disassembly before lifting; promote splat asm->rodata (T_<ADDR>) if data"
            if likely
            else None
        ),
    }


def map_names(path: Path, address: int) -> list[str]:
    if not path.is_file():
        return []
    return [
        match.group(1)
        for line in path.read_text().splitlines()
        if (match := SYMBOL.fullmatch(line)) and int(match.group(2), 16) == address
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "function", type=selector, metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP
    )
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument(
        "--prepare",
        action="store_true",
        help="regenerate disposable splat/m2ctx/m2c evidence under out/skill-evidence",
    )
    args = parser.parse_args()
    root = args.root.resolve()
    target, address = args.function
    manifest_path = root / "config" / "targets" / target / "target.toml"
    if not manifest_path.is_file():
        parser.error(f"unknown target: {target}")
    manifest = tomllib.loads(manifest_path.read_text())
    source = root / manifest["source_dir"] / f"func_{address:08X}.c"
    header = source.parent / "internal.h"
    map_path = root / "config" / "targets" / target / "symbols.txt"
    binary = root / manifest["binary"]
    payload_offset = address - int(manifest["load_address"])

    report: dict[str, Any] = {
        "schema": "bof3.skill-function-brief/v1",
        "function": f"{target}@0x{address:08X}",
        "target": target,
        "address": f"0x{address:08X}",
        "manifest": manifest_path.relative_to(root).as_posix(),
        "load_address": f"0x{int(manifest['load_address']):08X}",
        "payload_offset": None if payload_offset < 0 else f"0x{payload_offset:X}",
        "binary": {
            "path": binary.relative_to(root).as_posix(),
            "exists": binary.is_file(),
            **(
                {
                    "size": binary.stat().st_size,
                    "sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
                }
                if binary.is_file()
                else {}
            ),
        },
        "source": {
            "path": source.relative_to(root).as_posix(),
            "exists": source.is_file(),
        },
        "local_declarations": {
            "map_names_at_address": map_names(map_path, address),
            "header_mentions_address": bool(
                header.is_file() and f"{address:08X}" in header.read_text()
            ),
        },
        "data_table_probe": data_table_probe(
            binary, int(manifest["load_address"]), address
        ),
        "rizin": run(root, "bin/rz-project", "status", target, "--json"),
        "mission": run(
            root, "bin/rev-query", "mission", f"{target}@0x{address:08X}", "--json"
        ),
    }
    if source.is_file():
        comparison = run(root, "bin/asm-diff", f"{target}@0x{address:08X}", "--json")
        report["asm_diff"] = comparison
        payload = comparison.get("json")
        if isinstance(payload, dict):
            report["lift_status"] = {
                "status": "exact" if payload.get("byte_match") else "partial",
                "function": payload.get("function"),
                "address": payload.get("address"),
                "instruction_count": payload.get("instruction_count"),
                "match_percent": payload.get("instruction_count", {}).get(
                    "match_percent"
                )
                if isinstance(payload.get("instruction_count"), dict)
                else None,
                "original_size": payload.get("original_size"),
                "current_size": payload.get("current_size"),
                "size_delta": payload.get("size_delta"),
            }
            report["byte_match"] = {
                "command": comparison["command"],
                "exit_code": comparison["exit_code"],
                "json": {
                    key: payload[key]
                    for key in (
                        "function",
                        "address",
                        "original_size",
                        "current_size",
                        "byte_match",
                    )
                    if key in payload
                },
            }
    if args.prepare:
        evidence = root / "out" / "skill-evidence" / target / f"func_{address:08X}"
        evidence.mkdir(parents=True, exist_ok=True)
        report["prepare"] = [
            run(root, "bin/splat", target),
            run(
                root,
                "bin/m2ctx",
                f"{target}@0x{address:08X}",
                "-o",
                str(evidence / "context.c"),
            ),
            run(
                root,
                "bin/m2c",
                f"{target}@0x{address:08X}",
                "-o",
                str(evidence / "candidate.c"),
            ),
        ]
        report["prepare_output"] = evidence.relative_to(root).as_posix()
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
