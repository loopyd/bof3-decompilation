from __future__ import annotations

import json
import hashlib
from pathlib import Path
import re
import shutil
import subprocess
from typing import Any

from ..domain import TargetManifest, load_target_manifests, normalize_target_id
from ..jsonio import write_json
from ..psyq import parse_headers


_QUERY_COMMANDS = {
    "functions": "aflj",
    "strings": "izzj",
    "xrefs": "axlj",
    "types": "tj",
}

_PROJECT_STATE_SCHEMA = "bof3.analysis-project/v2"
_PROJECT_STATE_FILE = "state.json"


def _engine(requested: str | None = None) -> tuple[str, Path]:
    candidates = (requested,) if requested else ("rizin", "r2")
    for name in candidates:
        if name and (executable := shutil.which(name)):
            return name, Path(executable)
    expected = requested or "rizin or r2"
    raise FileNotFoundError(f"analysis engine not found: {expected}")


def _target(root: Path, value: str) -> TargetManifest:
    target_id = normalize_target_id(value).value
    manifest = load_target_manifests(root).get(target_id)
    if manifest is None:
        raise ValueError(f"unknown promoted target: {value}")
    binary = root / manifest.binary
    if not binary.is_file():
        raise FileNotFoundError(f"target binary missing: {manifest.binary}")
    return manifest


def _slug(manifest: TargetManifest) -> str:
    return manifest.id.value.replace("/", "__")


def _paths(root: Path, engine: str, manifest: TargetManifest) -> tuple[Path, Path]:
    project = root / "out" / "analysis" / "projects" / engine / _slug(manifest)
    export = root / "out" / "analysis" / "exports" / _slug(manifest)
    return project, export


def _sha256(path: Path) -> str | None:
    if not path.is_file():
        return None
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _analysis_inputs(root: Path, manifest: TargetManifest) -> dict[str, Any]:
    binary = root / manifest.binary
    replay = root / "config" / "analysis" / f"{manifest.id.value}.r2"
    types = root / "config" / "analysis" / "bof3_objects.h"
    splat = root / manifest.splat
    return {
        "binary": str(binary.relative_to(root)),
        "binary_sha256": _sha256(binary),
        "replay": str(replay.relative_to(root)) if replay.is_file() else None,
        "replay_sha256": _sha256(replay),
        "types": str(types.relative_to(root)) if types.is_file() else None,
        "types_sha256": _sha256(types),
        "splat": str(splat.relative_to(root)) if splat.is_file() else None,
        "splat_sha256": _sha256(splat),
    }


def _reviewed_function_addresses(root: Path, manifest: TargetManifest) -> list[int]:
    """Return function starts already reviewed into the target's Splat layout."""

    splat = root / manifest.splat
    if not splat.is_file():
        return []
    return sorted(
        {
            int(match.group(1), 16)
            for match in re.finditer(r"\bfunc_([0-9a-fA-F]{8})\b", splat.read_text())
        }
    )


def _reviewed_analysis_commands(addresses: list[int], binary_end: int) -> list[str]:
    commands: list[str] = []
    for index, address in enumerate(addresses):
        commands.append(f"af @ 0x{address:08x}")
        next_address = addresses[index + 1] if index + 1 < len(addresses) else None
        next_boundary = binary_end if next_address is None else next_address
        span = min(next_boundary - address, 0x1000)
        if span > 0:
            commands.append(f"aar 0x{span:x} @ 0x{address:08x}")
    return commands


def _sentinel(inputs: dict[str, Any]) -> str:
    payload = json.dumps(inputs, sort_keys=True, separators=(",", ":")).encode()
    return f"harness.sentinel_{hashlib.sha256(payload).hexdigest()[:16]}"


def _project_reference(engine: str, project_dir: Path, manifest: TargetManifest) -> str:
    if engine == "rizin":
        return str(project_dir / f"{_slug(manifest)}.rzdb")
    return _slug(manifest)


def _flag_json_command(engine: str) -> str:
    return "fs *;flj" if engine == "rizin" else "fs *;fj"


def _run(
    executable: Path,
    manifest: TargetManifest,
    root: Path,
    commands: list[str],
    *,
    project_dir: Path,
    project: str | None = None,
    timeout: int = 120,
) -> str:
    arguments = [
        str(executable),
        "-q0",
        "-a",
        "mips",
        "-b",
        "32",
        "-e",
        "cfg.bigendian=false",
        "-e",
        f"dir.projects={project_dir}",
        "-e",
        "prj.vc=false",
        "-m",
        f"0x{manifest.load_address:08x}",
    ]
    if project is not None:
        arguments.extend(("-p", project))
    for command in commands:
        arguments.extend(("-c", command))
    arguments.append(str(root / manifest.binary))
    result = subprocess.run(
        arguments, check=True, capture_output=True, text=True, timeout=timeout
    )
    return result.stdout.rstrip("\x00\n")


def _probe(executable: Path, commands: list[str]) -> subprocess.CompletedProcess[str]:
    arguments = [str(executable), "-q0", "-N"]
    for command in commands:
        arguments.extend(("-c", command))
    arguments.append("/dev/null")
    return subprocess.run(arguments, capture_output=True, text=True, timeout=10)


def _probe_engine(path: str) -> dict[str, Any]:
    executable = Path(path)
    version_result = subprocess.run(
        [path, "-v"], capture_output=True, text=True, timeout=10
    )
    version = (version_result.stdout or version_result.stderr).splitlines()
    mips = _probe(
        executable,
        [
            "e asm.arch=mips",
            "e asm.bits=32",
            "e cfg.bigendian=false",
            "e asm.arch",
            "e asm.bits",
            "e cfg.bigendian",
        ],
    )
    json_probe = _probe(executable, ["aflj", "tj"])
    project_probe = _probe(executable, ["P?"])
    decompiler_probe = _probe(executable, ["pdg?"])
    mips_lines = [line.strip() for line in mips.stdout.splitlines() if line.strip()]
    json_lines = [
        line.strip() for line in json_probe.stdout.splitlines() if line.strip()
    ]
    json_ok = json_probe.returncode == 0 and len(json_lines) >= 2
    if json_ok:
        try:
            json.loads(json_lines[-2])
            json.loads(json_lines[-1])
        except json.JSONDecodeError:
            json_ok = False
    return {
        "available": True,
        "path": path,
        "version": version[0] if version else None,
        "capabilities": {
            "mips32_little_endian": mips.returncode == 0
            and mips_lines[-3:] == ["mips", "32", "false"],
            "json": json_ok,
            "projects": project_probe.returncode == 0
            and "Project management" in project_probe.stdout,
            "decompiler": decompiler_probe.returncode == 0
            and "decompiler" in decompiler_probe.stdout.lower(),
        },
    }


def doctor() -> dict[str, Any]:
    tools: dict[str, dict[str, Any]] = {}
    for name in ("rizin", "r2"):
        path = shutil.which(name)
        tools[name] = (
            _probe_engine(path)
            if path is not None
            else {"available": False, "path": None, "version": None}
        )
    return {
        "preferred": "rizin" if tools["rizin"]["available"] else "r2",
        "tools": tools,
    }


def initialize_project(
    root: Path, target: str, requested: str | None = None
) -> dict[str, Any]:
    engine, executable = _engine(requested)
    manifest = _target(root, target)
    project_dir, _ = _paths(root, engine, manifest)
    project_dir.mkdir(parents=True, exist_ok=True)
    project = _project_reference(engine, project_dir, manifest)
    inputs = _analysis_inputs(root, manifest)
    sentinel = _sentinel(inputs)
    addresses = _reviewed_function_addresses(root, manifest)
    binary_end = manifest.load_address + (root / manifest.binary).stat().st_size
    commands = _reviewed_analysis_commands(addresses, binary_end)
    types = root / "config" / "analysis" / "bof3_objects.h"
    if types.is_file():
        commands.append(f"to {types}")
    replay = root / "config" / "analysis" / f"{manifest.id.value}.r2"
    if replay.is_file():
        commands.append(f". {replay}")
    # Generated projects are disposable snapshots; replace the named snapshot.
    if engine == "rizin":
        commands.extend((f"f+ {sentinel} 1", f"Ps {project}"))
    else:
        commands.extend((f"f {sentinel}=1", f"P- {project}", f"Ps {project}"))
    _run(executable, manifest, root, commands, project_dir=project_dir)
    reopened = _run(
        executable,
        manifest,
        root,
        [_flag_json_command(engine)],
        project_dir=project_dir,
        project=project,
    )
    flags = _json_output(reopened)
    if not isinstance(flags, list) or sentinel not in {
        row.get("name") for row in flags
    }:
        raise RuntimeError(f"analysis project failed reopen verification: {project}")
    state = {
        "schema": _PROJECT_STATE_SCHEMA,
        "engine": engine,
        "engine_path": str(executable),
        "target": manifest.id.value,
        "project": project,
        "sentinel": sentinel,
        "reviewed_function_starts": len(addresses),
        "inputs": inputs,
    }
    write_json(project_dir / _PROJECT_STATE_FILE, state)
    return {
        "engine": engine,
        "target": manifest.id.value,
        "project": str(project_dir.relative_to(root)),
        "replay": inputs["replay"],
        "reviewed_function_starts": len(addresses),
        "verified_reopen": True,
    }


def _json_output(output: str) -> Any:
    value = json.loads(output or "[]")
    if isinstance(value, list):
        return sorted(
            value, key=lambda row: (row.get("offset", 0), row.get("name", ""))
        )
    return value


def _verified_project(
    root: Path,
    engine: str,
    executable: Path,
    manifest: TargetManifest,
) -> tuple[Path, str]:
    project_dir, _ = _paths(root, engine, manifest)
    state_path = project_dir / _PROJECT_STATE_FILE
    if not state_path.is_file():
        raise RuntimeError(
            f"analysis project is not initialized: run analysis init {manifest.id.value}"
        )
    state = json.loads(state_path.read_text(encoding="utf-8"))
    expected_inputs = _analysis_inputs(root, manifest)
    stale = (
        state.get("schema") != _PROJECT_STATE_SCHEMA
        or state.get("engine") != engine
        or state.get("engine_path") != str(executable)
        or state.get("target") != manifest.id.value
        or state.get("inputs") != expected_inputs
    )
    if stale:
        raise RuntimeError(
            f"analysis project is stale: rerun analysis init {manifest.id.value}"
        )
    project = str(state.get("project", ""))
    sentinel = str(state.get("sentinel", ""))
    flags = _json_output(
        _run(
            executable,
            manifest,
            root,
            [_flag_json_command(engine)],
            project_dir=project_dir,
            project=project,
        )
    )
    if not isinstance(flags, list) or sentinel not in {
        row.get("name") for row in flags
    }:
        raise RuntimeError(f"analysis project reopen sentinel missing: {project}")
    return project_dir, project


def query_project(
    root: Path, target: str, query: str, requested: str | None = None
) -> Any:
    engine, executable = _engine(requested)
    manifest = _target(root, target)
    project_dir, project = _verified_project(root, engine, executable, manifest)
    command = _QUERY_COMMANDS.get(query, query)
    return _json_output(
        _run(
            executable,
            manifest,
            root,
            [command],
            project_dir=project_dir,
            project=project,
        )
    )


def export_project(
    root: Path, target: str, requested: str | None = None
) -> dict[str, Any]:
    engine, _ = _engine(requested)
    manifest = _target(root, target)
    _, export_dir = _paths(root, engine, manifest)
    payload = {
        "schema": "bof3.analysis/v1",
        "engine": engine,
        "target": manifest.id.value,
        "binary": manifest.binary,
        "load_address": manifest.load_address,
        "functions": query_project(root, target, "functions", engine),
        "strings": query_project(root, target, "strings", engine),
        "xrefs": query_project(root, target, "xrefs", engine),
    }
    output = export_dir / "analysis.json"
    write_json(output, payload)
    return {"output": str(output.relative_to(root)), **payload}


def _function_ranges(functions: list[dict[str, Any]]) -> list[tuple[int, int, int]]:
    return sorted(
        (
            int(row.get("addr", 0)),
            int(row.get("addr", 0)) + int(row.get("size", 0)),
            int(row.get("addr", 0)),
        )
        for row in functions
        if row.get("addr") is not None and row.get("size")
    )


def _containing(ranges: list[tuple[int, int, int]], address: int) -> int | None:
    for start, end, function_address in ranges:
        if start <= address < end:
            return function_address
    return None


def _relocation_fingerprint(data: bytes) -> str:
    normalized = bytearray(data)
    for offset in range(0, len(normalized) - 3, 4):
        word = int.from_bytes(normalized[offset : offset + 4], "little")
        if (word >> 26) in (2, 3):
            normalized[offset : offset + 4] = (word & 0xFC000000).to_bytes(4, "little")
    return hashlib.sha256(normalized).hexdigest()


def _load_psyq_symbols(root: Path, names: set[str]) -> dict[int, str]:
    """Read reviewed PsyQ names from locally linked ELF symbol tables."""

    nm_candidates = (
        shutil.which("mipsel-none-elf-nm"),
        root / "toolchains/psn00b_toolchain/bin/mipsel-none-elf-nm",
        root / "toolchains/psn00b_toolchain/mipsel-none-elf/bin/nm",
    )
    nm = next(
        (Path(item) for item in nm_candidates if item and Path(item).is_file()), None
    )
    if nm is None:
        return {}
    symbols: dict[int, str] = {}
    for binary in sorted((root / "build").glob("**/*.elf")):
        result = subprocess.run(
            [str(nm), "-n", str(binary)], capture_output=True, text=True
        )
        for line in result.stdout.splitlines():
            match = re.match(r"^([0-9a-fA-F]+)\s+[A-Za-z]\s+(\S+)$", line)
            if match and match.group(2) in names:
                symbols[int(match.group(1), 16)] = match.group(2)
    return symbols


def _analysis_type_bindings(
    root: Path, manifest: TargetManifest
) -> list[dict[str, Any]]:
    replay = root / "config" / "analysis" / f"{manifest.id.value}.r2"
    if not replay.is_file():
        return []
    bindings = []
    for match in re.finditer(
        r"^\s*tl\s+(\w+)\s*=\s*(0x[0-9a-fA-F]+)\s*$",
        replay.read_text(encoding="utf-8"),
        re.M,
    ):
        bindings.append({"type": match.group(1), "address": int(match.group(2), 16)})
    return bindings


def _target_snapshot(
    root: Path,
    manifest: TargetManifest,
    engine: str,
    executable: Path,
    psyq_symbols: dict[int, str],
    type_bindings: list[dict[str, Any]],
) -> dict[str, Any]:
    project_dir, _ = _paths(root, engine, manifest)
    functions = _json_output(
        _run(executable, manifest, root, ["aaa", "aflj"], project_dir=project_dir)
    )
    xrefs = _json_output(
        _run(executable, manifest, root, ["aaa", "axlj"], project_dir=project_dir)
    )
    ranges = _function_ranges(functions if isinstance(functions, list) else [])
    binary = (root / manifest.binary).read_bytes()
    function_rows: list[dict[str, Any]] = []
    exact: dict[str, list[str]] = {}
    reloc: dict[str, list[str]] = {}
    for row in functions if isinstance(functions, list) else []:
        address = int(row.get("addr", 0))
        size = int(row.get("size", 0))
        offset = address - manifest.load_address
        if offset < 0 or offset + size > len(binary) or size <= 0:
            continue
        data = binary[offset : offset + size]
        function_id = f"{manifest.id.value}@{address:08x}"
        exact_hash = hashlib.sha256(data).hexdigest()
        reloc_hash = _relocation_fingerprint(data)
        item = {
            "id": function_id,
            "address": address,
            "size": size,
            "name": row.get("name", ""),
            "exact_sha256": exact_hash,
            "relocation_sha256": reloc_hash,
        }
        source = root / manifest.source_dir / f"func_{address:08x}.c"
        if source.is_file():
            item["source"] = str(source.relative_to(root))
        function_rows.append(item)
        exact.setdefault(f"{size}:{exact_hash}", []).append(function_id)
        reloc.setdefault(f"{size}:{reloc_hash}", []).append(function_id)
    calls: set[tuple[str, str]] = set()
    psyq_calls: list[dict[str, Any]] = []
    for ref in xrefs if isinstance(xrefs, list) else []:
        if str(ref.get("type", "")).upper() != "CALL":
            continue
        caller = _containing(ranges, int(ref.get("from", 0)))
        callee = _containing(ranges, int(ref.get("addr", 0)))
        if caller is not None and callee is not None and caller != callee:
            calls.add(
                (
                    f"{manifest.id.value}@{caller:08x}",
                    f"{manifest.id.value}@{callee:08x}",
                )
            )
        symbol = psyq_symbols.get(int(ref.get("addr", 0)))
        if caller is not None and symbol is not None:
            psyq_calls.append(
                {
                    "caller": f"{manifest.id.value}@{caller:08x}",
                    "from": int(ref.get("from", 0)),
                    "address": int(ref.get("addr", 0)),
                    "function": symbol,
                }
            )
    xref_counts: dict[str, int] = {}
    for ref in xrefs if isinstance(xrefs, list) else []:
        kind = str(ref.get("type", "UNKNOWN")).upper()
        xref_counts[kind] = xref_counts.get(kind, 0) + 1
    type_xrefs: list[dict[str, Any]] = []
    for binding in type_bindings:
        refs = []
        for ref in xrefs if isinstance(xrefs, list) else []:
            if int(ref.get("addr", -1)) != binding["address"]:
                continue
            caller = _containing(ranges, int(ref.get("from", 0)))
            if caller is not None:
                refs.append(
                    {
                        "function": f"{manifest.id.value}@{caller:08x}",
                        "from": int(ref.get("from", 0)),
                        "kind": str(ref.get("type", "UNKNOWN")).upper(),
                    }
                )
        type_xrefs.append(
            {
                **binding,
                "xrefs": sorted(refs, key=lambda row: row["from"]),
            }
        )
    return {
        "target": manifest.id.value,
        "binary": manifest.binary,
        "load_address": manifest.load_address,
        "functions": sorted(function_rows, key=lambda row: row["address"]),
        "calls": sorted(
            [{"caller": caller, "callee": callee} for caller, callee in calls],
            key=lambda row: (row["caller"], row["callee"]),
        ),
        "psyq_calls": sorted(
            psyq_calls, key=lambda row: (row["caller"], row["address"], row["from"])
        ),
        "xref_counts": dict(sorted(xref_counts.items())),
        "type_xrefs": type_xrefs,
        "exact_groups": sorted(
            (group for group in exact.values() if len(group) > 1),
            key=lambda group: group[0],
        ),
        "relocation_groups": sorted(
            (group for group in reloc.values() if len(group) > 1),
            key=lambda group: group[0],
        ),
    }


def graph_analysis(
    root: Path, target: str | None = None, requested: str | None = None
) -> dict[str, Any]:
    engine, executable = _engine(requested)
    manifests = load_target_manifests(root)
    selected = (
        [_target(root, target)]
        if target
        else [manifests[key] for key in sorted(manifests)]
    )
    skipped = [
        manifest.id.value
        for manifest in selected
        if not (root / manifest.binary).is_file()
    ]
    selected = [manifest for manifest in selected if (root / manifest.binary).is_file()]
    psyq_versions: dict[str, dict[str, Any]] = {}
    for manifest in selected:
        version = {
            "native/capcom97": "4.7",
            "original/psyq36": "3.6",
            "original/psyq40": "4.0",
        }.get(manifest.profile, manifest.profile)
        if version not in psyq_versions:
            psyq_versions[version] = parse_headers(
                root / "toolchains" / "psyq" / version / "include"
            )
    ignored_names = {
        "char",
        "const",
        "double",
        "float",
        "int",
        "long",
        "short",
        "signed",
        "unsigned",
        "void",
        "volatile",
    }
    psyq_functions = sorted(
        {
            decl["name"]
            for graph in psyq_versions.values()
            for decl in graph["declarations"]
            if decl["name"] not in ignored_names
            and "__asm__" not in decl.get("return_type", "")
        }
    )
    psyq_symbols = _load_psyq_symbols(root, set(psyq_functions))
    type_bindings = {
        manifest.id.value: _analysis_type_bindings(root, manifest)
        for manifest in selected
    }
    snapshots = [
        _target_snapshot(
            root,
            manifest,
            engine,
            executable,
            psyq_symbols,
            type_bindings[manifest.id.value],
        )
        for manifest in selected
    ]
    exact_groups: dict[tuple[int, str], list[str]] = {}
    reloc_groups: dict[tuple[int, str], list[str]] = {}
    for snapshot in snapshots:
        for function in snapshot["functions"]:
            exact_groups.setdefault(
                (function["size"], function["exact_sha256"]), []
            ).append(function["id"])
            reloc_groups.setdefault(
                (function["size"], function["relocation_sha256"]), []
            ).append(function["id"])
    payload = {
        "schema": "bof3.analysis-graph/v1",
        "engine": engine,
        "targets": snapshots,
        "skipped_targets": skipped,
        "psyq_symbols": [
            {"address": address, "function": name}
            for address, name in sorted(psyq_symbols.items())
        ],
        "duplicate_groups": {
            "exact": sorted(
                (group for group in exact_groups.values() if len(group) > 1),
                key=lambda group: group[0],
            ),
            "relocation_candidates": sorted(
                (group for group in reloc_groups.values() if len(group) > 1),
                key=lambda group: group[0],
            ),
        },
        "psyq_functions": psyq_functions,
        "type_bindings": [
            {"target": target, **binding}
            for target, bindings in sorted(type_bindings.items())
            for binding in bindings
        ],
    }
    output = root / "out" / "analysis" / "graph.json"
    write_json(output, payload)
    return {
        "output": str(output.relative_to(root)),
        "engine": engine,
        "targets": len(snapshots),
        "skipped_targets": skipped,
        "functions": sum(len(snapshot["functions"]) for snapshot in snapshots),
        "calls": sum(len(snapshot["calls"]) for snapshot in snapshots),
        "exact_duplicates": len(payload["duplicate_groups"]["exact"]),
        "relocation_candidates": len(
            payload["duplicate_groups"]["relocation_candidates"]
        ),
        "psyq_functions": len(psyq_functions),
        "psyq_calls": sum(len(snapshot["psyq_calls"]) for snapshot in snapshots),
        "type_xrefs": sum(
            len(ref["xrefs"])
            for snapshot in snapshots
            for ref in snapshot["type_xrefs"]
        ),
    }
