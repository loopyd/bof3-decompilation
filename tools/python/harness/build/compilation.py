"""Fresh ordinary object production with configured source settings."""

from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

from harness.build.arguments import read_source
from harness.build.compiler import (
    build_compiler_arguments,
    load_compiler_configuration,
    resolve_compiler_settings,
    select_configured_compiler,
)
from harness.common.digests import digest
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.tags import count_function_metadata, require_single_function


_ROUTING_ENVIRONMENT = frozenset(
    {
        "BOF3_PRESERVATION_RECORD",
        "BOF3_PRESERVATION_FINGERPRINT",
        "BOF3_PRESERVATION_ROUTES",
        "BOF3_PRESERVATION_ROUTES_SHA256",
    }
)


def _compiler_environment() -> dict[str, str]:
    """Environment for compiler invocations, without routing-selection variables."""
    return {
        name: value
        for name, value in os.environ.items()
        if name not in _ROUTING_ENVIRONMENT
    }


def _preserved_grouped_command(
    root: Path, source: Path, output: Path
) -> tuple[list[str], dict[str, str]]:
    """Resolve a grouped source's ordered invocation from its preserved record."""
    from contextlib import closing

    from harness.build.routing import select_preservation, verify_selection

    with closing(select_preservation(root, [source], [source])) as selection:
        if selection.record is None:
            raise ValueError(
                "grouped compiler input requires an externally pinned preservation record"
            )
        proof = verify_selection(root, selection)
    preserved = list(proof["profile"]["arguments"])
    return [
        preserved[0],
        *preserved[1:],
        "-c",
        str(source),
        "-o",
        str(output),
    ], _compiler_environment()


def configured_command(
    root: Path, source: Path, output: Path, *, allow_grouped: bool = False
) -> tuple[list[str], dict[str, str]]:
    """Mirror the configured build's ordered arguments and compiler overlay."""
    text = read_source(source)
    if allow_grouped and count_function_metadata(text) > 1:
        return _preserved_grouped_command(root, source, output)
    require_single_function(text)
    flags, compilers = load_compiler_configuration(root)
    settings = resolve_compiler_settings(
        source.relative_to(root).as_posix(), text, flags, compilers
    )
    environment = _compiler_environment()
    if settings.compiler_id is not None:
        environment["PSX_GCC"] = str(
            select_configured_compiler(root, settings.compiler_id)
        )
    arguments = [
        *build_compiler_arguments(root, settings.flags),
        "-c",
        str(source),
        "-o",
        str(output),
    ]
    if not Path(arguments[0]).is_file():
        raise FileNotFoundError(f"missing configured compiler driver: {arguments[0]}")
    return arguments, environment


_OBJECT_CACHE = "out/matching/object-cache"


def _tool_identity(arguments: list[str], environment: dict[str, str]) -> str:
    """Identity of the compiler programs a produced object depends on."""
    entries: list[list[object]] = []
    driver = Path(arguments[0])
    try:
        entries.append(
            ["driver", driver.name, hashlib.sha256(driver.read_bytes()).hexdigest()]
        )
    except OSError:
        entries.append(["driver", driver.name, "missing"])
    configured = environment.get("PSX_GCC")
    if configured:
        target = Path(configured)
        try:
            if target.is_dir():
                for path in sorted(target.iterdir()):
                    if path.is_file():
                        status = path.stat()
                        entries.append([path.name, status.st_size, status.st_mtime_ns])
            elif target.is_file():
                entries.append(
                    [
                        "compiler",
                        target.name,
                        hashlib.sha256(target.read_bytes()).hexdigest(),
                    ]
                )
        except OSError:
            entries.append(["compiler", str(target), "missing"])
    return digest(entries)


def _object_cache_key(
    source: Path, arguments: list[str], environment: dict[str, str]
) -> str:
    """Content key: source bytes, ordered arguments and compiler identity."""
    try:
        content = source.read_bytes()
    except OSError:
        return ""
    return digest(
        {
            "source": source.name,
            "source_sha256": hashlib.sha256(content).hexdigest(),
            "arguments": arguments,
            "tools": _tool_identity(arguments, environment),
            "environment": environment,
        }
    )


def _restore_cached_object(
    root: Path,
    source: Path,
    output: Path,
    arguments: list[str],
    environment: dict[str, str],
) -> subprocess.CompletedProcess[str] | None:
    """Reuse an object produced from identical inputs instead of re-running bin/cc."""
    key = _object_cache_key(source, arguments, environment)
    if not key:
        return None
    directory = root / _OBJECT_CACHE / key
    manifest = directory / "manifest.json"
    try:
        record = json.loads(manifest.read_text())
    except (OSError, ValueError):
        return None
    stored = record.get("files") if isinstance(record, dict) else None
    if not isinstance(stored, dict) or "object" not in stored:
        return None
    for name, expected in stored.items():
        path = directory / name
        try:
            content = path.read_bytes()
        except OSError:
            return None
        if hashlib.sha256(content).hexdigest() != expected:
            return None
    try:
        output.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(directory / "object", output)
        sidecar = Path(str(output) + ".s")
        if "assembly" in stored:
            shutil.copyfile(directory / "assembly", sidecar)
        elif sidecar.is_file():
            sidecar.unlink()
    except OSError:
        return None
    return subprocess.CompletedProcess(arguments, 0, "", "")


def _store_cached_object(
    root: Path,
    source: Path,
    output: Path,
    arguments: list[str],
    environment: dict[str, str],
) -> None:
    """Record a produced object for reuse while every input stays identical."""
    key = _object_cache_key(source, arguments, environment)
    if not key:
        return
    directory = root / _OBJECT_CACHE / key
    files: dict[str, str] = {}
    staging = directory.with_name(key + ".staging")
    try:
        shutil.rmtree(staging, ignore_errors=True)
        staging.mkdir(parents=True, exist_ok=True)
        for name, produced in (
            ("object", output),
            ("assembly", Path(str(output) + ".s")),
        ):
            if not produced.is_file():
                continue
            content = produced.read_bytes()
            (staging / name).write_bytes(content)
            files[name] = hashlib.sha256(content).hexdigest()
        if "object" not in files:
            shutil.rmtree(staging, ignore_errors=True)
            return
        (staging / "manifest.json").write_text(
            json.dumps({"key": key, "files": files}, sort_keys=True)
        )
        shutil.rmtree(directory, ignore_errors=True)
        staging.rename(directory)
    except OSError:
        shutil.rmtree(staging, ignore_errors=True)


def compile_object(
    root: Path, source: Path, output: Path
) -> subprocess.CompletedProcess[str]:
    """Run the guarded native driver even when an old object already exists."""
    arguments, environment = configured_command(root, source, output)
    cached = _restore_cached_object(root, source, output, arguments, environment)
    if cached is not None:
        return cached
    result = run_bounded(
        root, arguments, env=environment, output_limit=64 * 1024 * 1024
    )
    if result["failure"]:
        raise RuntimeError(f"object compiler failed: {result['failure']}")
    completed = subprocess.CompletedProcess(
        arguments, result["exit_code"], result["stdout"], result["stderr"]
    )
    if completed.returncode == 0:
        _store_cached_object(root, source, output, arguments, environment)
    return completed


def compile_sources(
    root: Path, sources: list[Path]
) -> subprocess.CompletedProcess[str]:
    """Produce each ordinary miss and retain per-object failure attribution."""
    failures = []
    for source in sources:
        output = root / "build" / source.relative_to(root).with_suffix(".o")
        try:
            result = compile_object(root, source, output)
            if result.returncode == 0:
                continue
            detail = f"{result.stdout or ''}\n{result.stderr or ''}"
        except ProcessCleanupError:
            raise
        except (OSError, RuntimeError, ValueError) as error:
            detail = str(error)
        failures.append(f"FAILED: {output}\n{detail}")
    return subprocess.CompletedProcess([], int(bool(failures)), "\n".join(failures), "")
