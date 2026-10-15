"""Fresh ordinary object production with configured source settings."""

from __future__ import annotations

import os
from pathlib import Path
import subprocess

from harness.build.arguments import read_source
from harness.build.compiler import (
    build_compiler_arguments,
    load_compiler_configuration,
    resolve_compiler_settings,
    select_configured_compiler,
)
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.tags import require_single_function


def configured_command(
    root: Path, source: Path, output: Path
) -> tuple[list[str], dict[str, str]]:
    """Mirror the configured build's ordered arguments and compiler overlay."""
    text = read_source(source)
    require_single_function(text)
    flags, compilers = load_compiler_configuration(root)
    settings = resolve_compiler_settings(
        source.relative_to(root).as_posix(), text, flags, compilers
    )
    environment = dict(os.environ)
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


def compile_object(
    root: Path, source: Path, output: Path
) -> subprocess.CompletedProcess[str]:
    """Run the guarded native driver even when an old object already exists."""
    arguments, environment = configured_command(root, source, output)
    result = run_bounded(
        root, arguments, env=environment, output_limit=64 * 1024 * 1024
    )
    if result["failure"]:
        raise RuntimeError(f"object compiler failed: {result['failure']}")
    return subprocess.CompletedProcess(
        arguments, result["exit_code"], result["stdout"], result["stderr"]
    )


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
