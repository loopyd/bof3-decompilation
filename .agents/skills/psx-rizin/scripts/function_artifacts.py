#!/usr/bin/env python3
"""Export a focused Rizin/rz-ghidra artifact bundle for one PS1 function."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

from rizin_common import RizinError, parse_json_output, run_rizin


def parse_int(value: str) -> int:
    return int(value, 0)


def json_export(
    binary: Path, base: int, command: str, rizin: str, analyze: bool
) -> tuple[Any | None, str, str]:
    stdout, stderr = run_rizin(binary, base, command, rizin=rizin, analyze=analyze)
    try:
        value = parse_json_output(stdout)
    except RizinError:
        value = None
    return value, stdout, stderr


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("address", type=parse_int)
    parser.add_argument("--base", type=parse_int, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--rizin", default="rizin")
    parser.add_argument("--no-analysis", action="store_true")
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    analyze = not args.no_analysis
    addr = f"0x{args.address:x}"

    try:
        function_info, raw_info, info_err = json_export(
            args.binary, args.base, f"af @ {addr};afij @ {addr}", args.rizin, analyze
        )
        (args.out / "function-info.raw.txt").write_text(raw_info, encoding="utf-8")
        if function_info is not None:
            (args.out / "function-info.json").write_text(
                json.dumps(function_info, indent=2, sort_keys=True) + "\n", encoding="utf-8"
            )

        size = 0x100
        if isinstance(function_info, list) and function_info and isinstance(function_info[0], dict):
            candidate = function_info[0].get("size") or function_info[0].get("realsz")
            if isinstance(candidate, int) and candidate > 0:
                size = candidate

        text_commands = {
            "disassembly.txt": f"pdf @ {addr}",
            "disassembly-bytes.txt": f"pD {size} @ {addr}",
            "decompile.txt": f"pdg @ {addr}",
            "decompile-offsets.txt": f"pdgo @ {addr}",
            "variables.txt": f"afvl @ {addr}",
            "variable-accesses.txt": f"afv= @ {addr}",
        }
        json_commands = {
            "decompile.json": f"pdgj @ {addr}",
            "xrefs-to.json": f"axtj @ {addr}",
            "xrefs-from.json": f"axfj @ {addr}",
        }

        stderr_parts = [info_err] if info_err else []
        for filename, command in text_commands.items():
            stdout, stderr = run_rizin(
                args.binary, args.base, command, rizin=args.rizin, analyze=analyze
            )
            (args.out / filename).write_text(stdout, encoding="utf-8")
            if stderr:
                stderr_parts.append(f"## {command}\n{stderr}")

        for filename, command in json_commands.items():
            value, stdout, stderr = json_export(
                args.binary, args.base, command, args.rizin, analyze
            )
            if value is None:
                (args.out / filename.replace(".json", ".raw.txt")).write_text(
                    stdout, encoding="utf-8"
                )
            else:
                (args.out / filename).write_text(
                    json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8"
                )
            if stderr:
                stderr_parts.append(f"## {command}\n{stderr}")

        if stderr_parts:
            (args.out / "rizin-stderr.txt").write_text("\n".join(stderr_parts), encoding="utf-8")

        metadata = {
            "binary": str(args.binary),
            "base": f"0x{args.base:08x}",
            "address": f"0x{args.address:08x}",
            "size_used_for_pD": size,
            "analysis": analyze,
            "note": "Decompiler artifacts may be unavailable when a compatible rz-ghidra is not installed.",
        }
        (args.out / "metadata.json").write_text(
            json.dumps(metadata, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
        notes = args.out / "notes.md"
        if not notes.exists():
            notes.write_text(
                "# Function notes\n\n"
                "## Identity and confidence\n\n"
                "## Callers and callees\n\n"
                "## Arguments and return values\n\n"
                "## Structure/global offsets\n\n"
                "## Replay observations\n\n"
                "## Contradictions and next experiment\n",
                encoding="utf-8",
            )
    except (OSError, RizinError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    print(args.out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
