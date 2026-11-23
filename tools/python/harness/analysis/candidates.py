"""Screened lift candidates for one target or a cross-target batch.

Ranking and screening are analysis facts, not command policy, so the commands
that expose them stay thin adapters. A candidate survives when its address is
unclaimed, its original bytes carry no ``div``/``break`` trap, its head is not an
in-payload pointer run, and the SDK map does not already own the address.
"""

from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

from harness.common.commands import command_at
from harness.domain.claims import collect_manifest_source_addresses
from harness.domain.registry import lookup_target_manifest
from harness.domain.sdk_map import load_sdk_map, sdk_map_path

_DIV_FUNCTS = {0x1A, 0x1B}
_BREAK_FUNCT = 0x0D
_MAX_SCREEN_POOL = 512
_HEAD_WORDS = 7
_HEAD_POINTERS = 3


def load_address(root: Path, target: str) -> int | None:
    """Read the target's configured load address without a full manifest parse."""
    manifest = root / "config" / "targets" / target / "target.toml"
    if not manifest.is_file():
        return None
    match = re.search(
        r"^\s*load_address\s*=\s*(0x[0-9A-Fa-f]+|\d+)",
        manifest.read_text(encoding="utf-8"),
        re.MULTILINE,
    )
    return None if match is None else int(match.group(1), 0)


def has_trap(payload: bytes) -> bool:
    for offset in range(0, len(payload) - 3, 4):
        word = int.from_bytes(payload[offset : offset + 4], "little")
        opcode = word >> 26
        funct = word & 0x3F
        if opcode == 0 and (funct in _DIV_FUNCTS or funct == _BREAK_FUNCT):
            return True
    return False


def candidate_address(row: dict) -> int:
    value = row["address"]
    return value if isinstance(value, int) else int(value, 0)


def claimed_addresses(root: Path, target: str) -> set[int]:
    """Read current function claims owned by this target, not other overlays."""
    manifest = lookup_target_manifest(root, target)
    if manifest is None:
        raise ValueError(f"unknown target: {target}")
    return {address for _, address in collect_manifest_source_addresses(root, manifest)}


def drop_claimed(rows: list[dict], claimed: set[int]) -> list[dict]:
    if not claimed:
        return rows
    return [row for row in rows if candidate_address(row) not in claimed]


def trap_screen(root: Path, rows: list[dict]) -> list[dict]:
    survivors: list[dict] = []
    load_cache: dict[str, int | None] = {}
    for row in rows:
        target = row["target"]
        if target not in load_cache:
            load_cache[target] = load_address(root, target)
        load = load_cache[target]
        binary = root / "out" / "binaries" / f"{target}.bin"
        if load is None or not binary.is_file():
            survivors.append({**row, "trap_screened": False})
            continue
        offset = candidate_address(row) - load
        data = binary.read_bytes()
        if offset < 0 or offset + row["size"] > len(data):
            survivors.append({**row, "trap_screened": False})
            continue
        if has_trap(data[offset : offset + row["size"]]):
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


def surviving_rows(root: Path, target: str, limit: int) -> tuple[list[dict], int]:
    """Return screened rows, widening the ranked pool while nothing survives.

    The ranking limit is applied before the claimed/trap screen, so a fixed pool
    reports no candidate whenever the top rows all fail it. Widening doubles the
    pool until a row survives or the ranking is exhausted.
    """
    requested = max(1, limit)
    claimed = claimed_addresses(root, target)
    while True:
        rows = _ranked_rows(root, target, requested)
        survivors = trap_screen(root, drop_claimed(rows, claimed))
        if survivors or len(rows) < requested or requested >= _MAX_SCREEN_POOL:
            return survivors, len(rows)
        requested = min(requested * 2, _MAX_SCREEN_POOL)


def targets(root: Path, limit: int) -> list[str]:
    """Return distinct unlifted targets in quick-wins rank order."""
    result = subprocess.run(
        command_at(
            root,
            "rev-query",
            "quick-wins",
            "--unlifted",
            "--detail",
            "minimal",
            "--limit",
            str(limit),
            "--json",
        ),
        cwd=root,
        text=True,
        capture_output=True,
    )
    if result.returncode != 0:
        sys.stderr.write(result.stdout + result.stderr)
        raise SystemExit(result.returncode)
    ordered: list[str] = []
    for row in json.loads(result.stdout):
        target = row["id"].split("@")[0]
        if target not in ordered:
            ordered.append(target)
    return ordered


def sdk_addresses(root: Path) -> set[int]:
    """Addresses the combined SDK map assigns inside any target image."""
    return {symbol.address for symbol in load_sdk_map(sdk_map_path(root))}


def is_data_head(root: Path, target: str, address: int) -> bool:
    """True when the candidate opens with a run of in-payload pointers."""
    load = load_address(root, target)
    binary = root / "out" / "binaries" / f"{target}.bin"
    if load is None or not binary.is_file():
        return False
    data = binary.read_bytes()
    offset = address - load
    if offset < 0 or offset + 4 * _HEAD_WORDS > len(data):
        return False
    words = [
        int.from_bytes(data[offset + 4 * index : offset + 4 * index + 4], "little")
        for index in range(_HEAD_WORDS)
    ]
    return sum(1 for word in words if load <= word < load + len(data)) >= _HEAD_POINTERS


def select_lanes(
    root: Path, *, limit: int, pool: int, exclude: set[str]
) -> tuple[list[dict], list[dict]]:
    """Return disjoint lanes plus the candidates a screen rejected."""
    excluded = {value.lower() for value in exclude}
    sdk = sdk_addresses(root)
    lanes: list[dict] = []
    skipped: list[dict] = []
    for target in targets(root, pool):
        if len(lanes) >= limit:
            break
        survivors, _screened = surviving_rows(root, target, 12)
        if not survivors:
            continue
        address = candidate_address(survivors[0])
        selector = f"{target}@{address:#010x}"
        if selector.lower() in excluded:
            skipped.append({"selector": selector, "reason": "excluded"})
            continue
        if address in sdk:
            skipped.append({"selector": selector, "reason": "sdk_address"})
            continue
        if is_data_head(root, target, address):
            skipped.append({"selector": selector, "reason": "data_head"})
            continue
        lanes.append(
            {
                "key": target.replace("/", "_"),
                "target": target,
                "refs": [selector],
            }
        )
    return lanes, skipped
