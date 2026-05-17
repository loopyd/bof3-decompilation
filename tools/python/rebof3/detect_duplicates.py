"""
Detect duplicate functions across ALL binary files (SLUS, LOGO, and *.bin).

Reads the Ghidra function index, groups game functions by body byte content,
filters to cross-program duplicates only, and writes a JSON report.

Usage: bin/detect-duplicates [--ghidra]
Output: out/harness/duplicate_groups.json

With --ghidra: uses Ghidra headless for exact function byte comparison.
Without --ghidra: uses the emi_catalog SHA256 + function body range fallback.
"""

from __future__ import annotations

import hashlib
import re
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any

from .harness.config import load_harness_config
from .jsonio import read_json, write_json


EMI_SOURCE_RE = re.compile(
    r"^output/extracted/(.+)/(.+)\.EMI(#(?P<slot>[0-9]+))?$"
)


def _load_function_index(path: Path) -> list[dict[str, Any]]:
    if not path.is_file():
        raise SystemExit(f"function index not found: {path}")
    payload = read_json(path)
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        raise SystemExit(f"unexpected function index format: {path}")
    return rows


def _is_game_func(row: dict[str, Any]) -> bool:
    if str(row.get("name_source") or "").upper() == "IMPORTED":
        return False
    try:
        addr = int(str(row.get("entry_hex") or "0"), 0)
    except (ValueError, TypeError):
        return False
    return addr >= 0x80000000


def _resolve_binary(source_hint: str, config_root: Path) -> Path | None:
    """Resolve a source_hint to an actual binary file on disk.

    source_hint examples:
      "output/extracted/SLUS_004.22"
      "output/extracted/LOGO/LOGO.EXE"
      "output/extracted/BATTLE/BATTLE.EMI#15"
    """
    if not source_hint:
        return None

    # Core executables
    if source_hint == "output/extracted/SLUS_004.22":
        p = config_root / "output/extracted/SLUS_004.22"
        return p if p.is_file() else None
    if "LOGO.EXE" in source_hint or source_hint == "output/extracted/LOGO/LOGO.EXE":
        p = config_root / "output/extracted/LOGO/LOGO.EXE"
        return p if p.is_file() else None

    # EMI entries: output/extracted/<FAMILY>/<ARCHIVE>.EMI#<SLOT>
    m = EMI_SOURCE_RE.match(source_hint)
    if m:
        family = m.group(1)
        archive = m.group(2)
        slot = m.group("slot") or "0"
        # EMI raw path: output/extracted/<FAMILY>/<ARCHIVE>/<SLOT>.bin
        p = config_root / "output" / "extracted" / family / archive / f"{slot}.bin"
        if p.is_file():
            return p
        # Try with BIN prefix
        p = config_root / "output" / "extracted" / "BIN" / family / archive / f"{slot}.bin"
        if p.is_file():
            return p

    return None


def _normalize_program(row: dict[str, Any]) -> str:
    return str(row.get("source_hint") or row.get("program_path") or "unknown")


def _sha256_bytes(bin_path: Path, offset: int, length: int) -> str:
    with open(bin_path, "rb") as fh:
        fh.seek(offset)
        return hashlib.sha256(fh.read(length)).hexdigest()


def run(report_path: Path | None = None) -> dict[str, Any]:
    config = load_harness_config()
    rows = _load_function_index(config.function_index)
    game_rows = [r for r in rows if _is_game_func(r)]

    # Group by SHA256 of actual binary bytes
    byte_groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
    skipped = 0

    for r in game_rows:
        body_min = str(r.get("body_min") or "")
        body_max = str(r.get("body_max") or "")
        if not body_min or not body_max:
            continue
        try:
            addr = int(str(r.get("entry_hex") or "0"), 0)
        except (ValueError, TypeError):
            continue
        size = int(body_max, 16) - int(body_min, 16) + 1
        if size <= 0 or size > 65536:
            continue

        hint = str(r.get("source_hint") or "")
        bin_path = _resolve_binary(hint, config.root)
        if bin_path is None:
            skipped += 1
            continue

        # File offset: RAM address - 0x80000000 (PSX RAM base)
        offset = addr - 0x80000000
        file_size = bin_path.stat().st_size
        if offset < 0 or offset + size > file_size:
            skipped += 1
            continue

        sha256 = _sha256_bytes(bin_path, offset, size)
        byte_groups[sha256].append(r)

    # Filter to cross-program groups
    groups: list[dict[str, Any]] = []
    for sha256, entries in sorted(byte_groups.items()):
        program_set = sorted(set(_normalize_program(e) for e in entries))
        if len(program_set) <= 1:
            continue

        first = entries[0]
        body_min = str(first.get("body_min") or "")
        body_max = str(first.get("body_max") or "")
        try:
            size = int(body_max, 16) - int(body_min, 16) + 1
        except (ValueError, TypeError):
            size = 0

        names = sorted(
            set(
                str(e.get("name") or "")
                for e in entries
                if str(e.get("name") or "").startswith("func_")
            )
        )
        if not names:
            names = sorted(set(str(e.get("name") or "") for e in entries))

        groups.append(
            {
                "sha256": sha256,
                "size": size,
                "body_min": body_min,
                "body_max": body_max,
                "occurrence_count": len(program_set),
                "programs": program_set,
                "names": names,
                "entries": [
                    {
                        "program_path": str(e.get("program_path") or ""),
                        "source_hint": str(e.get("source_hint") or ""),
                        "entry_hex": str(e.get("entry_hex") or ""),
                        "name": str(e.get("name") or ""),
                    }
                    for e in entries
                ],
            }
        )

    output = {
        "schema": "rebof3-simple.detect-duplicates/v1",
        "total_game_functions": len(game_rows),
        "skipped_not_resolved": skipped,
        "duplicate_group_count": len(groups),
        "functions_in_duplicate_groups": sum(
            g["occurrence_count"] for g in groups
        ),
        "groups": groups,
    }

    if report_path is None:
        report_path = config.out_dir / "duplicate_groups.json"
    report_path.parent.mkdir(parents=True, exist_ok=True)
    write_json(report_path, output)
    return output


def main() -> None:
    use_ghidra = "--ghidra" in sys.argv
    if use_ghidra:
        print("Ghidra mode not yet implemented. Use bin/ghidra-export-symbols first.")
        sys.exit(1)
    result = run()
    print(
        f"Duplicate detection complete: "
        f"{result['duplicate_group_count']} groups, "
        f"{result['functions_in_duplicate_groups']} total occurrences, "
        f"{result['skipped_not_resolved']} skipped."
    )


if __name__ == "__main__":
    main()
