"""Cross-target function hotspot analysis.

Identifies high-impact functions across all binary targets by computing:
  callers (in-degree) — how many unique functions call this one
  callees (out-degree) — how many unique functions this one calls
  cross_target — how many callers come from different targets
  exact_duplicate — same sha256 at same size, anywhere across all targets
  leaf / root — no outgoing / no incoming calls
"""

from __future__ import annotations

import hashlib
import json
import shutil
import subprocess
from collections import defaultdict
from pathlib import Path
from typing import Any

from ..domain import TargetManifest, load_target_manifests, normalize_target_id


def _engine() -> tuple[str, Path]:
    for name in ("rizin", "r2"):
        if exe := shutil.which(name):
            return name, Path(exe)
    raise FileNotFoundError("analysis engine not found")


def _function_hashes(
    binary: bytes, functions: list[dict[str, Any]], load_address: int
) -> dict[int, str]:
    """Return {address: sha256} for each function's binary bytes."""
    result: dict[int, str] = {}
    for f in functions:
        addr = int(f.get("offset", 0))
        size = int(f.get("size", 0))
        offset = addr - load_address
        if offset < 0 or offset + size > len(binary) or size <= 0:
            continue
        data = binary[offset : offset + size]
        result[addr] = hashlib.sha256(data).hexdigest()
    return result


def _relocation_fingerprint(data: bytes) -> str:
    """Hash with JAL/J targets zeroed out to detect same function relocated."""
    normalized = bytearray(data)
    for off in range(0, len(normalized) - 3, 4):
        word = int.from_bytes(normalized[off : off + 4], "little")
        if (word >> 26) in (2, 3):
            normalized[off : off + 4] = (word & 0xFC000000).to_bytes(4, "little")
    return hashlib.sha256(normalized).hexdigest()


def _reloc_hashes(
    binary: bytes, functions: list[dict[str, Any]], load_address: int
) -> dict[int, str]:
    """Return {address: reloc_sha256} for each function."""
    result: dict[int, str] = {}
    for f in functions:
        addr = int(f.get("offset", 0))
        size = int(f.get("size", 0))
        offset = addr - load_address
        if offset < 0 or offset + size > len(binary) or size <= 0:
            continue
        result[addr] = _relocation_fingerprint(binary[offset : offset + size])
    return result


def _known_addresses(
    root: Path, target_id: str
) -> set[int]:
    """Return all known function addresses from Splat YAML."""
    from .operations import _reviewed_function_addresses, _function_bindings

    manifests = load_target_manifests(root)
    m = manifests.get(target_id)
    if m is None:
        return set()
    addrs = set(_reviewed_function_addresses(root, m))
    for _, a in _function_bindings(root / m.source_dir):
        if m.load_address <= a < m.load_address + (root / m.binary).stat().st_size:
            addrs.add(a)
    return addrs


def hotspot_analysis(root: Path, target: str | None = None) -> dict[str, Any]:
    """Run cross-target hotspot analysis."""
    engine_name, executable = _engine()
    manifests = load_target_manifests(root)

    selected: dict[str, TargetManifest] = {}
    if target:
        tid = normalize_target_id(target).value
        m = manifests.get(tid)
        if m is None:
            raise ValueError(f"unknown target: {target}")
        binary = root / m.binary
        if not binary.is_file():
            raise FileNotFoundError(f"binary missing: {m.binary}")
        selected[tid] = m
    else:
        for tid in sorted(manifests):
            m = manifests[tid]
            if (root / m.binary).is_file():
                selected[tid] = m

    # Per-target data
    func_map: dict[int, dict[str, Any]] = {}  # address -> {target, size, name, sha256, reloc_sha256}
    callers: dict[int, set[int]] = defaultdict(set)  # callee_addr -> {caller_addrs}
    callees: dict[int, set[int]] = defaultdict(set)  # caller_addr -> {callee_addrs}
    known: dict[str, set[int]] = {}  # target -> {known addresses}

    for tid, m in sorted(selected.items()):
        known[tid] = _known_addresses(root, tid)
        addrs = sorted(known[tid] | set(_reviewed_function_addresses_from_manifest(root, m)))
        af_cmds = [f"af @ 0x{a:08x}" for a in addrs]

        cmd = [
            str(executable), "-q0", "-a", "mips", "-b", "32",
            "-e", "cfg.bigendian=false",
            "-m", f"0x{m.load_address:08x}",
        ]
        for c in af_cmds + ["aaa", "aflj", "axlj"]:
            cmd.extend(["-c", c])
        cmd.append(str(root / m.binary))

        result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        lines = [line for line in result.stdout.strip().split("\n") if line.strip()]
        if len(lines) < 2:
            continue

        funcs_raw = json.loads(lines[0])
        xrefs_raw = json.loads(lines[1])

        binary = (root / m.binary).read_bytes()
        exact_hashes = _function_hashes(binary, funcs_raw, m.load_address)
        relocs = _reloc_hashes(binary, funcs_raw, m.load_address)

        for f in funcs_raw:
            addr = int(f.get("offset", 0))
            size = int(f.get("size", 0))
            func_map[addr] = {
                "target": tid,
                "size": size,
                "name": f.get("name", ""),
                "sha256": exact_hashes.get(addr, ""),
                "reloc_sha256": relocs.get(addr, ""),
                "is_known": addr in known[tid],
            }

        for x in xrefs_raw:
            from_addr = int(x.get("from", 0))
            to_addr = int(x.get("to", x.get("addr", 0)))
            x_type = x.get("type", "")
            if x_type in ("CALL", "JMP"):
                callers[to_addr].add(from_addr)
                callees[from_addr].add(to_addr)

    # ---- Rankings ----

    # Hottest by callers (in-degree)
    hot: list[dict[str, Any]] = []
    for addr, info in func_map.items():
        c = callers.get(addr, set())
        if not c:
            continue
        cross = sum(1 for ca in c if ca in func_map and func_map[ca]["target"] != info["target"])
        hot.append({
            "address": addr,
            "target": info["target"],
            "size": info["size"],
            "name": info["name"],
            "callers": len(c),
            "cross_target_callers": cross,
            "is_known": info["is_known"],
        })
    hot.sort(key=lambda x: (-x["callers"], -x["cross_target_callers"]))

    # Leaf functions — no outgoing calls (terminal / bottom-up candidates)
    leaves: list[dict[str, Any]] = []
    roots: list[dict[str, Any]] = []
    shallow: list[dict[str, Any]] = []
    for addr, info in func_map.items():
        out_deg = len(callees.get(addr, set()))
        in_deg = len(callers.get(addr, set()))
        entry = {
            "address": addr,
            "target": info["target"],
            "size": info["size"],
            "name": info["name"],
            "in_degree": in_deg,
            "out_degree": out_deg,
            "is_known": info["is_known"],
        }
        if out_deg == 0:
            leaves.append(entry)
        if in_deg == 0:
            roots.append(entry)
        if info["is_known"] and 0 < out_deg <= 3 and in_deg <= 5:
            shallow.append(entry)
    leaves.sort(key=lambda x: (-x["in_degree"], x["size"]))
    roots.sort(key=lambda x: x["address"])
    shallow.sort(key=lambda x: (x["out_degree"], -x["in_degree"]))

    # Unknown call targets (called but not in func_map)
    unknown: list[dict[str, Any]] = []
    for to_addr, from_set in callers.items():
        if to_addr not in func_map:
            unknown.append({
                "address": to_addr,
                "callers": len(from_set),
                "sample_callers": sorted(from_set)[:5],
            })
    unknown.sort(key=lambda x: -x["callers"])

    # Exact duplicates
    sha_groups: dict[str, list[int]] = defaultdict(list)
    for addr, info in func_map.items():
        if info["sha256"]:
            key = f"{info['size']}:{info['sha256']}"
            sha_groups[key].append(addr)
    exact_dupes = [
        {"size": int(k.split(":")[0]), "functions": v}
        for k, v in sha_groups.items()
        if len(v) > 1
    ]
    exact_dupes.sort(key=lambda x: -len(x["functions"]))

    # Relocation duplicates (same function at different addresses)
    reloc_groups: dict[str, list[int]] = defaultdict(list)
    for addr, info in func_map.items():
        if info["reloc_sha256"]:
            key = f"{info['size']}:{info['reloc_sha256']}"
            reloc_groups[key].append(addr)
    reloc_dupes = [
        {"size": int(k.split(":")[0]), "functions": v}
        for k, v in reloc_groups.items()
        if len(v) > 1
    ]
    reloc_dupes.sort(key=lambda x: -len(x["functions"]))

    # Unknown callees per known function (discovery leverage)
    discovery: list[dict[str, Any]] = []
    for addr, info in func_map.items():
        if not info["is_known"]:
            continue
        out = callees.get(addr, set())
        unknown_out = sum(1 for ca in out if ca not in func_map)
        known_out = sum(1 for ca in out if ca in func_map)
        if unknown_out > 0:
            discovery.append({
                "address": addr,
                "target": info["target"],
                "name": info["name"],
                "total_callees": len(out),
                "unknown_callees": unknown_out,
                "known_callees": known_out,
                "callers": len(callers.get(addr, set())),
            })
    discovery.sort(key=lambda x: (-x["unknown_callees"], -x["callers"]))

    return {
        "schema": "bof3.hotspots/v1",
        "engine": engine_name,
        "targets_analyzed": len(selected),
        "total_functions": len(func_map),
        "total_call_edges": sum(len(v) for v in callers.values()),
        "hot": hot[:50],
        "leaves": leaves[:40],
        "roots": roots[:30],
        "shallow": shallow[:30],
        "unknown_targets": unknown[:30],
        "exact_duplicates": exact_dupes[:20],
        "relocation_duplicates": reloc_dupes[:20],
        "discovery": discovery[:30],
    }


def _reviewed_function_addresses_from_manifest(
    root: Path, manifest: TargetManifest
) -> list[int]:
    """Same as _reviewed_function_addresses but takes manifest directly."""
    import re

    splat = root / manifest.splat
    if not splat.is_file():
        return []
    return sorted(
        {
            int(m.group(1), 16)
            for m in re.finditer(r"\bfunc_([0-9a-fA-F]{8})\b", splat.read_text())
        }
    )
