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
        target, raw_address = value.rsplit("@", 1)
        address = int(raw_address, 0)
    except ValueError as error:
        raise argparse.ArgumentTypeError("expected TARGET@0xADDRESS") from error
    if not target or address < 0 or address > 0xFFFFFFFF:
        raise argparse.ArgumentTypeError("expected TARGET@0xADDRESS")
    return target, address


def map_names(path: Path, address: int) -> list[str]:
    if not path.is_file():
        return []
    return [match.group(1) for line in path.read_text().splitlines() if (match := SYMBOL.fullmatch(line)) and int(match.group(2), 16) == address]


def lift_record(payload: Any, target: str, address: int) -> dict[str, Any] | None:
    if not isinstance(payload, dict):
        return None
    for item in payload.get("targets", []):
        if item.get("target") != target:
            continue
        for function in item.get("functions", []):
            if function.get("address") == f"0x{address:08X}":
                return function
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("function", type=selector, metavar="TARGET@0xADDRESS")
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--prepare", action="store_true", help="regenerate disposable splat/m2ctx/m2c evidence under out/skill-evidence")
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
            **({"size": binary.stat().st_size, "sha256": hashlib.sha256(binary.read_bytes()).hexdigest()} if binary.is_file() else {}),
        },
        "source": {"path": source.relative_to(root).as_posix(), "exists": source.is_file()},
        "local_declarations": {
            "map_names_at_address": map_names(map_path, address),
            "header_mentions_address": bool(header.is_file() and f"{address:08X}" in header.read_text()),
        },
        "rizin": run(root, "bin/rz-project", "status", target, "--json"),
        "mission": run(root, "bin/rev-query", "mission", f"{target}@0x{address:08X}", "--json"),
    }
    if source.is_file():
        status = run(root, "bin/decomp-status", target, "--json")
        report["lift_status"] = lift_record(status.get("json"), target, address)
        report["asm_diff"] = run(root, "bin/asm-diff", f"{target}@0x{address:08X}", "--json")
        report["byte_match"] = run(root, "bin/byte-match", f"{target}@0x{address:08X}", "--json")
    if args.prepare:
        evidence = root / "out" / "skill-evidence" / target / f"func_{address:08X}"
        evidence.mkdir(parents=True, exist_ok=True)
        report["prepare"] = [
            run(root, "bin/splat", target),
            run(root, "bin/m2ctx", f"{target}@0x{address:08X}", "-o", str(evidence / "context.c")),
            run(root, "bin/m2c", f"{target}@0x{address:08X}", "-o", str(evidence / "candidate.c")),
        ]
        report["prepare_output"] = evidence.relative_to(root).as_posix()
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
