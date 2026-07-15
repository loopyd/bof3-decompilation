"""Analyzer adapter.

Owns engine lifecycle, capability probing, JSON normalization, and
subprocess boundaries.  Rizin is the primary engine for projects,
snapshots, and cross-target analysis.  Radare2 is available as a
fallback for commands where its JSON output is more mature or
reliable.

The adapter requires ``rzpipe`` for Rizin and subprocess for radare2.
Engine selection is explicit per operation, never implicit fallback.
"""

from __future__ import annotations

import json
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..domain.registry import ResolvedTarget
from .replay import ReplayPlan


@dataclass(frozen=True)
class EngineIdentity:
    """Verified analyzer installation."""

    name: str
    executable: Path
    version: str
    capabilities: dict[str, bool]


@dataclass(frozen=True)
class RawFunction:
    """A function record as returned by the analyzer."""

    addr: int
    size: int
    name: str


@dataclass(frozen=True)
class RawXref:
    """An xref record as returned by the analyzer."""

    from_addr: int
    to_addr: int
    xref_type: str


@dataclass(frozen=True)
class RawString:
    """A string record as returned by the analyzer."""

    vaddr: int
    string: str


@dataclass(frozen=True)
class AnalyzerDump:
    """Raw analyzer output for one target."""

    functions: tuple[RawFunction, ...]
    xrefs: tuple[RawXref, ...]
    strings: tuple[RawString, ...]


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


def _get_version(executable: Path, name: str) -> str:
    flag = "-V" if name == "rizin" else "-v"
    result = subprocess.run(
        [str(executable), flag], capture_output=True, text=True, timeout=10
    )
    output = (result.stdout or result.stderr).strip()
    if not output:
        raise RuntimeError(f"{name} {flag} returned empty output")
    return output.splitlines()[0]


def _probe_capabilities(executable: Path, name: str) -> dict[str, bool]:
    """Probe the analyzer for required MIPS analysis capabilities."""

    capabilities: dict[str, bool] = {}

    arch_result = _run_probe(executable, ["e asm.arch", "e asm.bits", "e cfg.bigendian"])
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


def _run_probe(executable: Path, commands: list[str]) -> str:
    argv = [str(executable), "-q0", "-N"]
    for command in commands:
        argv.extend(["-c", command])
    argv.append("/dev/null")
    result = subprocess.run(argv, capture_output=True, text=True, timeout=10)
    return result.stdout or ""


def run_plan(
    engine: EngineIdentity,
    resolved: ResolvedTarget,
    plan: ReplayPlan,
    *,
    timeout: int = 120,
) -> AnalyzerDump:
    """Execute a replay plan against a target and return normalized dumps.

    Launches one analyzer process, applies the plan, queries functions and
    xrefs, then closes the process.
    """

    if engine.name == "rizin":
        return _run_rizin(engine, resolved, plan, timeout=timeout)
    return _run_r2(engine, resolved, plan, timeout=timeout)


def _run_rizin(
    engine: EngineIdentity,
    resolved: ResolvedTarget,
    plan: ReplayPlan,
    *,
    timeout: int = 120,
) -> AnalyzerDump:
    import rzpipe

    flags = [
        "-q0",
        "-a", "mips",
        "-b", "32",
        "-e", "cfg.bigendian=false",
        "-m", f"0x{resolved.load_address:08x}",
    ]

    rz = rzpipe.open(str(resolved.binary_path), flags=flags)
    try:
        for command in plan.commands:
            rz.cmd(command)

        functions_raw = rz.cmdj("aflj") or []
        functions = tuple(
            RawFunction(
                addr=int(row.get("offset", 0)),
                size=int(row.get("size", 0)),
                name=str(row.get("name", "")),
            )
            for row in functions_raw
            if isinstance(row, dict) and row.get("offset") is not None
        )

        xrefs_raw = rz.cmdj("axlj") or []
        xrefs = tuple(
            RawXref(
                from_addr=int(row.get("from", 0)),
                to_addr=int(row.get("to", 0)),
                xref_type=str(row.get("type", "")).upper(),
            )
            for row in xrefs_raw
            if isinstance(row, dict) and row.get("from") is not None
        )

        strings_raw = rz.cmdj("izzj") or []
        strings = tuple(
            RawString(
                vaddr=int(row.get("vaddr", 0)),
                string=str(row.get("string", "")),
            )
            for row in strings_raw
            if isinstance(row, dict)
        )

        return AnalyzerDump(functions=functions, xrefs=xrefs, strings=strings)
    finally:
        rz.quit()


def _run_r2(
    engine: EngineIdentity,
    resolved: ResolvedTarget,
    plan: ReplayPlan,
    *,
    timeout: int = 120,
) -> AnalyzerDump:
    argv = [
        str(engine.executable),
        "-q0",
        "-N",
        "-n",
        "-a", "mips",
        "-b", "32",
        "-e", "cfg.bigendian=false",
        "-m", f"0x{resolved.load_address:08x}",
    ]
    for command in plan.commands:
        argv.extend(["-c", command])
    argv.extend(["-c", "aflj", "-c", "axlj", "-c", "izzj"])
    argv.append(str(resolved.binary_path))

    result = subprocess.run(argv, capture_output=True, text=True, timeout=timeout)
    lines = [line for line in result.stdout.splitlines() if line.strip()]

    # Parse the last three JSON arrays: functions, xrefs, strings.
    functions: list[RawFunction] = []
    xrefs: list[RawXref] = []
    strings: list[RawString] = []

    if len(lines) >= 3:
        try:
            functions_raw = json.loads(lines[-3])
            xrefs_raw = json.loads(lines[-2])
            strings_raw = json.loads(lines[-1])
        except json.JSONDecodeError:
            pass
        else:
            functions = [
                RawFunction(
                    addr=int(row.get("addr", 0)),
                    size=int(row.get("size", 0)),
                    name=str(row.get("name", "")),
                )
                for row in functions_raw
                if isinstance(row, dict)
            ]
            xrefs = [
                RawXref(
                    from_addr=int(row.get("from", 0)),
                    to_addr=int(row.get("addr", 0)),
                    xref_type=str(row.get("type", "")).upper(),
                )
                for row in xrefs_raw
                if isinstance(row, dict)
            ]
            strings = [
                RawString(
                    vaddr=int(row.get("vaddr", 0)),
                    string=str(row.get("string", "")),
                )
                for row in strings_raw
                if isinstance(row, dict)
            ]

    return AnalyzerDump(
        functions=tuple(functions),
        xrefs=tuple(xrefs),
        strings=tuple(strings),
    )


def run_query(
    engine: EngineIdentity,
    resolved: ResolvedTarget,
    plan: ReplayPlan,
    query: str,
    *,
    timeout: int = 120,
) -> Any:
    """Execute a named JSON query against a target."""

    QUERY_COMMANDS = {
        "functions": "aflj",
        "strings": "izzj",
        "xrefs": "axlj",
        "types": "tj",
    }
    command = QUERY_COMMANDS.get(query)
    if command is None:
        raise ValueError(
            f"unsupported query {query!r}; allowed: {sorted(QUERY_COMMANDS)}"
        )

    if engine.name == "rizin":
        import rzpipe

        flags = [
            "-q0",
            "-a", "mips",
            "-b", "32",
            "-e", "cfg.bigendian=false",
            "-m", f"0x{resolved.load_address:08x}",
        ]
        rz = rzpipe.open(str(resolved.binary_path), flags=flags)
        try:
            for cmd in plan.commands:
                rz.cmd(cmd)
            return rz.cmdj(command)
        finally:
            rz.quit()
    else:
        argv = [
            str(engine.executable),
            "-q0", "-N", "-n",
            "-a", "mips",
            "-b", "32",
            "-e", "cfg.bigendian=false",
            "-m", f"0x{resolved.load_address:08x}",
        ]
        for cmd in plan.commands:
            argv.extend(["-c", cmd])
        argv.extend(["-c", command])
        argv.append(str(resolved.binary_path))
        result = subprocess.run(argv, capture_output=True, text=True, timeout=timeout)
        lines = [line for line in result.stdout.splitlines() if line.strip()]
        if lines:
            try:
                return json.loads(lines[-1])
            except json.JSONDecodeError:
                pass
        return []


def run_project_init(
    engine: EngineIdentity,
    resolved: ResolvedTarget,
    plan: ReplayPlan,
    project_path: Path,
    sentinel: str,
    *,
    timeout: int = 120,
) -> bool:
    """Initialize an analyzer project and verify it reopens correctly.

    Only Rizin supports projects natively.  For radare2, this is a
    best-effort operation using ``Ps``/``P-``.
    """

    project_path.parent.mkdir(parents=True, exist_ok=True)

    if engine.name == "rizin":
        return _init_rizin(engine, resolved, plan, project_path, sentinel, timeout=timeout)
    return _init_r2(engine, resolved, plan, project_path, sentinel, timeout=timeout)


def _init_rizin(
    engine: EngineIdentity,
    resolved: ResolvedTarget,
    plan: ReplayPlan,
    project_path: Path,
    sentinel: str,
    *,
    timeout: int = 120,
) -> bool:
    import rzpipe

    flags = [
        "-q0",
        "-a", "mips",
        "-b", "32",
        "-e", "cfg.bigendian=false",
        "-e", f"dir.projects={project_path.parent}",
        "-e", "prj.vc=false",
        "-m", f"0x{resolved.load_address:08x}",
    ]

    rz = rzpipe.open(str(resolved.binary_path), flags=flags)
    try:
        for command in plan.commands:
            rz.cmd(command)
        rz.cmd(f"f+ {sentinel} 1")
        rz.cmd(f"Ps {project_path}")

        rz2 = rzpipe.open(str(resolved.binary_path), flags=flags)
        try:
            rz2.cmd(f"-p {project_path}")
            flags_raw = rz2.cmdj("fs *;flj") or []
            flag_names = {row.get("name") for row in flags_raw if isinstance(row, dict)}
            return sentinel in flag_names
        finally:
            rz2.quit()
    finally:
        rz.quit()


def _init_r2(
    engine: EngineIdentity,
    resolved: ResolvedTarget,
    plan: ReplayPlan,
    project_path: Path,
    sentinel: str,
    *,
    timeout: int = 120,
) -> bool:
    project_name = project_path.stem
    argv = [
        str(engine.executable),
        "-q0", "-N", "-n",
        "-a", "mips", "-b", "32",
        "-e", "cfg.bigendian=false",
        "-e", f"dir.projects={project_path.parent}",
        "-e", "prj.vc=false",
        "-m", f"0x{resolved.load_address:08x}",
    ]
    for command in plan.commands:
        argv.extend(["-c", command])
    argv.extend([
        "-c", f"f {sentinel}=1",
        "-c", f"P- {project_name}",
        "-c", f"Ps {project_name}",
    ])
    argv.append(str(resolved.binary_path))
    subprocess.run(argv, capture_output=True, text=True, timeout=timeout, check=True)

    # Verify reopen.
    verify_argv = [
        str(engine.executable),
        "-q0", "-N", "-n",
        "-a", "mips", "-b", "32",
        "-e", "cfg.bigendian=false",
        "-e", f"dir.projects={project_path.parent}",
        "-m", f"0x{resolved.load_address:08x}",
        "-p", project_name,
        "-c", "fs *;fj",
    ]
    verify_argv.append(str(resolved.binary_path))
    result = subprocess.run(verify_argv, capture_output=True, text=True, timeout=timeout)
    try:
        flags = json.loads(result.stdout.strip() or "[]")
        return sentinel in {row.get("name") for row in flags if isinstance(row, dict)}
    except json.JSONDecodeError:
        return False


__all__ = [
    "AnalyzerDump",
    "EngineIdentity",
    "RawFunction",
    "RawString",
    "RawXref",
    "find_best_engine",
    "find_engine",
    "run_plan",
    "run_project_init",
    "run_query",
]
