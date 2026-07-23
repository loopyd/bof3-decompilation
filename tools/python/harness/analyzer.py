"""Subprocess-based, target-qualified Rizin snapshots."""

from __future__ import annotations

import json
import hashlib
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from .io import repo_layout

from .snapshot import (
    SNAPSHOT_SCHEMA,
    SnapshotCall,
    SnapshotFunction,
    SnapshotUnresolvedCall,
    TargetSnapshot,
    snapshot_path,
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


def _optional_integer(row: dict[str, Any], key: str, *, minimum: int = 0) -> int | None:
    value = row.get(key)
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        return None
    return value


def _get_version(executable: Path) -> str:
    flag = "-V"
    result = subprocess.run(
        [str(executable), flag],
        capture_output=True,
        text=True,
        timeout=10,
    )
    output = (result.stdout or result.stderr).strip()
    if not output:
        raise RuntimeError(f"rizin {flag} returned empty output")
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
        "-E",
        "little",
    ]
    for command in commands:
        argv.extend(["-c", command])
    argv.append("/dev/null")
    result = subprocess.run(argv, capture_output=True, text=True, timeout=10)
    return result.stdout or ""


def _probe_capabilities(executable: Path) -> dict[str, bool]:
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


def find_engine(name: str = "rizin", *, root: Path | None = None) -> EngineIdentity:
    """Locate and verify the installed analyzer engine.

    Raises ``FileNotFoundError`` if the engine is not installed, or
    ``RuntimeError`` if it lacks required capabilities.
    """

    if name != "rizin":
        raise ValueError("only rizin is supported")
    executable = repo_layout(root).toolchains_dir / "rizin" / "bin" / "rizin"
    if not executable.is_file():
        raise FileNotFoundError(f"missing project Rizin: {executable}; run `just setup`")
    version = _get_version(executable)
    capabilities = _probe_capabilities(executable)
    missing = [
        capability
        for capability, expected in _REQUIRED_CAPABILITIES.items()
        if capabilities.get(capability) is not expected
    ]
    if missing:
        raise RuntimeError(
            f"rizin lacks required capabilities: {', '.join(sorted(missing))}"
        )
    return EngineIdentity(
        name="rizin",
        executable=executable,
        version=version,
        capabilities=capabilities,
    )


def _run_analysis(
    engine: EngineIdentity,
    binary_path: Path,
    load_address: int,
    *,
    replay_commands: list[str],
    timeout: int = 120,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    """Run one staged analysis and return functions and xrefs or fail closed."""

    argv = [
        str(engine.executable),
        "-q0",
        "-N",
        "-n",
        "-a",
        "mips",
        "-b",
        "32",
        "-E",
        "little",
        "-m",
        f"0x{load_address:08x}",
    ]
    for command in replay_commands:
        argv.extend(["-c", command])
    argv.extend(["-c", "aa", "-c", "aflj", "-c", "axlj", str(binary_path)])
    result = subprocess.run(argv, capture_output=True, text=True, timeout=timeout)
    if result.returncode != 0:
        diagnostic = (result.stderr or result.stdout).strip()
        raise RuntimeError(
            f"rizin analysis failed with exit code {result.returncode}: {diagnostic}"
        )
    payloads: list[Any] = []
    for line in result.stdout.splitlines():
        if not line.strip().startswith(("[", "{")):
            continue
        try:
            payloads.append(json.loads(line))
        except json.JSONDecodeError:
            continue
    if len(payloads) < 2 or not all(isinstance(value, list) for value in payloads[-2:]):
        raise RuntimeError(
            "rizin analysis did not return function and xref JSON arrays"
        )
    return payloads[-2], payloads[-1]


def build_snapshot(
    engine: EngineIdentity,
    binary_path: Path,
    load_address: int,
    target_id: str,
    *,
    reviewed_addresses: set[int] | None = None,
    replay_commands: list[str] | None = None,
    replay_sha256: str | None = None,
    source_dir: Path | None = None,
    timeout: int = 120,
) -> TargetSnapshot:
    """Build a portable snapshot from one stateless analyzer invocation set."""

    binary = binary_path.read_bytes()
    commands = replay_commands or [
        f"af @ 0x{address:08x}" for address in sorted(reviewed_addresses or set())
    ]
    raw_functions, raw_xrefs = _run_analysis(
        engine,
        binary_path,
        load_address,
        replay_commands=commands,
        timeout=timeout,
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
            candidate = source_dir / f"func_{address:08X}.c"
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
                basic_blocks=_optional_integer(raw, "nbbs", minimum=1),
                cyclomatic_complexity=_optional_integer(raw, "cc", minimum=1),
                edges=_optional_integer(raw, "edges"),
                loops=_optional_integer(raw, "loops"),
                stack_frame=_optional_integer(raw, "stackframe"),
                local_count=_optional_integer(raw, "nlocals"),
                argument_count=_optional_integer(raw, "nargs"),
            )
        )
        function_ids[address] = function_id
        ranges.append((address, address + size, function_id))

    functions.sort(key=lambda function: function.address)
    calls: list[SnapshotCall] = []
    unresolved: list[SnapshotUnresolvedCall] = []
    for raw in raw_xrefs if isinstance(raw_xrefs, list) else []:
        if str(raw.get("type", "")).upper() not in {"CALL", "C"}:
            continue
        callsite = int(raw.get("from", raw.get("fcn_addr", 0)))
        target = int(raw.get("to", raw.get("addr", 0)))
        caller = next(
            (
                function_id
                for start, end, function_id in ranges
                if start <= callsite < end
            ),
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
        inputs={
            "binary_sha256": hashlib.sha256(binary).hexdigest(),
            "replay_sha256": replay_sha256,
        },
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
    from .rizin_project import analyze_project

    # Keep this legacy entry point on the same composed replay as rz-project.
    analyze_project(root, normalized, timeout=timeout)
    output = snapshot_path(root, normalized)
    return output


def doctor() -> dict[str, Any]:
    """Check analyzer availability and capabilities."""

    try:
        identity = find_engine()
        return {
            "engine": "rizin",
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
    "find_engine",
    "write_target_snapshot",
]
