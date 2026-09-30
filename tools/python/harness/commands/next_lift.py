"""Return the first liftable candidate for a target in one call.

Consolidates the per-lane sequence measured in the fan-out runs: run
``rev-query quick-wins``, then hand-screen every row's original bytes for
``div``/``break`` trap opcodes, then take the first survivor. Lanes repeated
that loop for up to 8 rows on every mission.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

from harness.common.commands import command_at

_ROOT = Path(__file__).resolve().parents[4]
_DIV_FUNCTS = {0x1A, 0x1B}
_BREAK_FUNCT = 0x0D


def _load_address(root: Path, target: str) -> int | None:
    manifest = root / "config" / "targets" / target / "target.toml"
    if not manifest.is_file():
        return None
    match = re.search(
        r"^\s*load_address\s*=\s*(0x[0-9A-Fa-f]+|\d+)",
        manifest.read_text(encoding="utf-8"),
        re.MULTILINE,
    )
    if match is None:
        return None
    return int(match.group(1), 0)


def _has_trap(payload: bytes) -> bool:
    for offset in range(0, len(payload) - 3, 4):
        word = int.from_bytes(payload[offset : offset + 4], "little")
        opcode = word >> 26
        funct = word & 0x3F
        if opcode == 0 and (funct in _DIV_FUNCTS or funct == _BREAK_FUNCT):
            return True
    return False


def _address(row: dict) -> int:
    value = row["address"]
    return value if isinstance(value, int) else int(value, 0)


def _claimed_addresses(root: Path) -> set[int]:
    """Addresses already claimed in the working tree via ``@source 0x...`` tags.

    The reverse-index snapshot can be days old, so ranked rows still list lifts
    that sibling lanes already completed. Reading the working tree keeps
    ``next-lift`` from handing a lane an already-lifted address.
    """
    claimed: set[int] = set()
    pattern = re.compile(r"@source\s+0x([0-9A-Fa-f]+)")
    for source in (root / "src").rglob("*.c"):
        try:
            text = source.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        for match in pattern.finditer(text):
            claimed.add(int(match.group(1), 16))
    return claimed


def _drop_claimed(rows: list[dict], claimed: set[int]) -> list[dict]:
    if not claimed:
        return rows
    return [row for row in rows if _address(row) not in claimed]


def _trap_screen(root: Path, rows: list[dict]) -> list[dict]:
    survivors: list[dict] = []
    load_cache: dict[str, int | None] = {}
    for row in rows:
        target = row["target"]
        if target not in load_cache:
            load_cache[target] = _load_address(root, target)
        load = load_cache[target]
        binary = root / "out" / "binaries" / f"{target}.bin"
        if load is None or not binary.is_file():
            # Cannot screen: keep the row but mark it unverified.
            survivors.append({**row, "trap_screened": False})
            continue
        offset = _address(row) - load
        data = binary.read_bytes()
        if offset < 0 or offset + row["size"] > len(data):
            survivors.append({**row, "trap_screened": False})
            continue
        if _has_trap(data[offset : offset + row["size"]]):
            continue
        survivors.append({**row, "trap_screened": True})
    return survivors


def _ranked_rows(root: Path, target: str, limit: int) -> list[dict]:
    result = subprocess.run(
        command_at(
            root,
            "rev-query",
            "--allow-stale",
            "--json",
            "quick-wins",
            "--target",
            target,
            "--unlifted",
            "--limit",
            str(limit),
        ),
        cwd=root,
        text=True,
        capture_output=True,
    )
    if result.returncode != 0:
        sys.stderr.write(result.stdout + result.stderr)
        raise SystemExit(result.returncode)
    return json.loads(result.stdout)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="bin/harness lift next", description=__doc__)
    parser.add_argument("target", help="target id, e.g. emi/battle/battle/15")
    parser.add_argument("--limit", type=int, default=12, help="rows to screen")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)

    root = _ROOT
    rows = _drop_claimed(
        _ranked_rows(root, args.target, args.limit), _claimed_addresses(root)
    )
    survivors = _trap_screen(root, rows)
    if not survivors:
        if args.json:
            print(json.dumps({"target": args.target, "candidate": None}, indent=2))
        else:
            print(
                f"{args.target}: no candidate survived the claimed + div/break screen"
            )
        return 1
    best = survivors[0]
    address = _address(best)
    if args.json:
        print(json.dumps(best, indent=2, sort_keys=True))
        return 0
    print(
        f"{best['target']}@{address:#010x}"
        f"  insns={best['instruction_count']} size={best['size']}"
        f"  trap_screened={best.get('trap_screened')}"
        f"  unresolved_calls={best['unresolved_calls']}"
    )
    print(f"bin/harness lift gate {best['target']}@{address:#010x} {best['target']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
