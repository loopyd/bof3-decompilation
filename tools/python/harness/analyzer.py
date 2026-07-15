"""Subprocess-based stateless analyzer adapters.

Replaces persistent analyzer projects with subprocess-based stateless
snapshots.  Supports Rizin and radare2 via subprocess, with capability-based
engine selection.
"""

from __future__ import annotations

import json
import hashlib
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from .snapshot import (
    SNAPSHOT_SCHEMA,
    SnapshotCall,
    SnapshotFunction,
    SnapshotUnresolvedCall,
    TargetSnapshot,
    snapshot_path,
    write_snapshot,
)


@dataclass(frozen=True)
class EngineIdentity:
    """Verified analyzer installation."""

    name: str
    executable: Path
    version: str
    capabilities: dict[str, bool]


_REQUIRED_CAPABILITIES = {
    "mips32_little_endian": True,
    "json": True,
}


def _get_version_flag(name: str) -> str:
    return "-V" if name == "rizin" else "-v"


def _get_version(executable: Path, name: str) -> str:
    flag = _get_version_flag(name)
    result = subprocess.run(
        [str(executable), flag],
        capture_output=True,
        text=True,
        timeout=10,
    )
    output = (result.stdout or result.stderr).strip()
    if not output:
        raise RuntimeError(f"{name} {flag} returned empty output")
    return output.splitlines()[0]


def _run_probe(executable: Path, commands: list[str]) -> str:
    argv = [
        str(executable),
        "-q0",
        "-N",
        "-a",
        "mips",
        "-b",
        "32",
        "-e",
        "cfg.bigendian=false",
    ]
    for command in commands:
        argv.extend(["-c", command])
    argv.append("/dev/null")
    result = subprocess.run(argv, capture_output=True, text=True, timeout=10)
    return result.stdout or ""


def _probe_capabilities(executable: Path, name: str) -> dict[str, bool]:
    """Probe the analyzer for required MIPS analysis capabilities."""

    capabilities: dict[str, bool] = {}

    arch_result = _run_probe(
        executable, ["e asm.arch", "e asm.bits", "e cfg.bigendian"]
    )
    arch_lines = [line.strip() for line in arch_result.splitlines() if line.strip()]
    capabilities["mips32_little_endian"] = (
        len(arch_lines) >= 3
        and arch_lines[-3] == "mips"
        and arch_lines[-2] == "32"
        and arch_lines[-1] == "false"
    )

    json_result = _run_probe(executable, ["aflj", "axlj"])
    json_lines = [line.strip() for line in json_result.splitlines() if line.strip()]
    json_ok = len(json_lines) >= 2
    if json_ok:
        try:
            json.loads(json_lines[-2])
            json.loads(json_lines[-1])
        except json.JSONDecodeError:
            json_ok = False
    capabilities["json"] = json_ok

    proj_result = _run_probe(executable, ["P?"])
    capabilities["projects"] = "Project management" in proj_result

    decomp_result = _run_probe(executable, ["pdg?"])
    capabilities["decompiler"] = "decompiler" in decomp_result.lower()

    return capabilities


def find_engine(name: str = "rizin") -> EngineIdentity:
    """Locate and verify the installed analyzer engine.

    Raises ``FileNotFoundError`` if the engine is not installed, or
    ``RuntimeError`` if it lacks required capabilities.
    """

    path = shutil.which(name)
    if path is None:
        raise FileNotFoundError(
            f"{name} not found; install it and ensure it is on PATH"
        )
    executable = Path(path)
    version = _get_version(executable, name)
    capabilities = _probe_capabilities(executable, name)
    missing = [
        capability
        for capability, expected in _REQUIRED_CAPABILITIES.items()
        if capabilities.get(capability) is not expected
    ]
    if missing:
        raise RuntimeError(
            f"{name} lacks required capabilities: {', '.join(sorted(missing))}"
        )
    return EngineIdentity(
        name=name,
        executable=executable,
        version=version,
        capabilities=capabilities,
    )


def find_best_engine() -> EngineIdentity:
    """Find the best available engine for general analysis.

    Prefers Rizin for its project support and modern API.
    Falls back to radare2 if Rizin is not available.
    """

    for name in ("rizin", "r2"):
        try:
            return find_engine(name)
        except (FileNotFoundError, RuntimeError):
            continue
    raise FileNotFoundError("neither rizin nor r2 found on PATH")


def _run_subprocess_query(
    engine: EngineIdentity,
    binary_path: Path,
    load_address: int,
    query_command: str,
    *,
    timeout: int = 120,
) -> Any:
    argv = [
        str(engine.executable),
        "-q0",
        "-N",
        "-n",
        "-a", "mips",
        "-b", "32",
        "-e", "cfg.bigendian=false",
        "-m", f"0x{load_address:08x}",
        "-c", "aa",
        "-c", query_command,
        str(binary_path),
    ]
    result = subprocess.run(argv, capture_output=True, text=True, timeout=timeout)
    lines = [line for line in result.stdout.splitlines() if line.strip()]
    if lines:
        try:
            return json.loads(lines[-1])
        except json.JSONDecodeError:
            pass
    return []


_QUERY_COMMANDS = {
    "functions": "aflj",
    "strings": "izzj",
    "xrefs": "axlj",
    "types": "tj",
}


def query_project(
    engine: EngineIdentity,
    binary_path: Path,
    load_address: int,
    query: str,
    *,
    timeout: int = 120,
) -> Any:
    """Run a named read-only query against a target."""

    if query not in _QUERY_COMMANDS:
        raise ValueError(
            f"unsupported query {query!r}; allowed: {sorted(_QUERY_COMMANDS)}"
        )
    command = _QUERY_COMMANDS[query]
    return _run_subprocess_query(
        engine, binary_path, load_address, command, timeout=timeout
    )


def build_snapshot(
    engine: EngineIdentity,
    binary_path: Path,
    load_address: int,
    target_id: str,
    *,
    reviewed_addresses: set[int] | None = None,
    source_dir: Path | None = None,
    timeout: int = 120,
) -> TargetSnapshot:
    """Build a portable snapshot from one stateless analyzer invocation set."""

    binary = binary_path.read_bytes()
    raw_functions = query_project(
        engine, binary_path, load_address, "functions", timeout=timeout
    )
    functions: list[SnapshotFunction] = []
    function_ids: dict[int, str] = {}
    ranges: list[tuple[int, int, str]] = []
    reviewed = reviewed_addresses or set()

    for raw in raw_functions if isinstance(raw_functions, list) else []:
        address = int(raw.get("offset", raw.get("addr", 0)))
        size = int(raw.get("size", 0))
        offset = address - load_address
        if size <= 0 or offset < 0 or offset + size > len(binary):
            continue
        function_id = f"{target_id}@{address:08x}"
        source = None
        if source_dir is not None:
            candidate = source_dir / f"func_{address:08x}.c"
            if candidate.is_file():
                source = str(candidate)
        functions.append(
            SnapshotFunction(
                id=function_id,
                address=address,
                analyzer_size=size,
                analyzer_name=str(raw.get("name", function_id)),
                exact_sha256=hashlib.sha256(binary[offset : offset + size]).hexdigest(),
                is_reviewed=address in reviewed,
                is_lifted=source is not None,
                source=source,
            )
        )
        function_ids[address] = function_id
        ranges.append((address, address + size, function_id))

    functions.sort(key=lambda function: function.address)
    calls: list[SnapshotCall] = []
    unresolved: list[SnapshotUnresolvedCall] = []
    raw_xrefs = query_project(engine, binary_path, load_address, "xrefs", timeout=timeout)
    for raw in raw_xrefs if isinstance(raw_xrefs, list) else []:
        if str(raw.get("type", "")).upper() not in {"CALL", "C"}:
            continue
        callsite = int(raw.get("from", raw.get("fcn_addr", 0)))
        target = int(raw.get("to", raw.get("addr", 0)))
        caller = next(
            (function_id for start, end, function_id in ranges if start <= callsite < end),
            None,
        )
        if caller is None:
            continue
        callee = function_ids.get(target)
        if callee is not None:
            calls.append(SnapshotCall(caller, callee, callsite))
        else:
            unresolved.append(
                SnapshotUnresolvedCall(
                    caller=caller,
                    target_address=target,
                    callsite=callsite,
                    kind="unknown",
                )
            )

    return TargetSnapshot(
        schema=SNAPSHOT_SCHEMA,
        target=target_id,
        engine={"name": engine.name, "version": engine.version},
        inputs={"binary_sha256": hashlib.sha256(binary).hexdigest()},
        functions=tuple(functions),
        calls=tuple(sorted(set(calls), key=lambda call: (call.caller, call.callsite))),
        unresolved_calls=tuple(unresolved),
    )


def write_target_snapshot(root: Path, target_id: str, *, timeout: int = 120) -> Path:
    """Analyze a manifest-backed target and atomically write its snapshot."""

    from .domain import load_target_manifests, normalize_target_id

    normalized = normalize_target_id(target_id).value
    manifest = load_target_manifests(root).get(normalized)
    if manifest is None:
        raise ValueError(f"unknown target: {normalized}")
    binary_path = root / manifest.binary
    if not binary_path.is_file():
        raise FileNotFoundError(f"target binary not found: {manifest.binary}")
    engine = find_best_engine()
    reviewed: set[int] = set()
    splat_path = root / manifest.splat
    if splat_path.is_file():
        from .binaries import SPLAT_FUNCTION_SUBSEGMENT_RE

        for line in splat_path.read_text(encoding="utf-8").splitlines():
            match = SPLAT_FUNCTION_SUBSEGMENT_RE.match(line)
            if match is not None:
                reviewed.add(manifest.load_address + int(match.group("offset"), 0))
    snapshot = build_snapshot(
        engine,
        binary_path,
        manifest.load_address,
        normalized,
        reviewed_addresses=reviewed,
        source_dir=root / manifest.source_dir,
        timeout=timeout,
    )
    output = snapshot_path(root, normalized)
    write_snapshot(snapshot, output)
    return output


def doctor() -> dict[str, Any]:
    """Check analyzer availability and capabilities."""

    try:
        identity = find_engine("rizin")
        return {
            "engine": "rizin",
            "available": True,
            "path": str(identity.executable),
            "version": identity.version,
            "capabilities": identity.capabilities,
        }
    except (FileNotFoundError, RuntimeError):
        pass
    try:
        identity = find_engine("r2")
        return {
            "engine": "r2",
            "available": True,
            "path": str(identity.executable),
            "version": identity.version,
            "capabilities": identity.capabilities,
        }
    except (FileNotFoundError, RuntimeError) as error:
        return {
            "engine": None,
            "available": False,
            "error": str(error),
        }


__all__ = [
    "EngineIdentity",
    "build_snapshot",
    "doctor",
    "find_best_engine",
    "find_engine",
    "query_project",
    "write_target_snapshot",
]
