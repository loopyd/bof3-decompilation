from __future__ import annotations

from collections.abc import Mapping, Sequence
from dataclasses import dataclass
import os
from pathlib import Path
import shlex
import subprocess
import sys
from threading import Thread
from typing import TextIO

Command = Sequence[str | os.PathLike[str]]


@dataclass(frozen=True)
class ProcessResult:
    command: tuple[str, ...]
    returncode: int
    cwd: Path | None
    stdout: str
    stderr: str


class ProcessError(RuntimeError):
    def __init__(self, result: ProcessResult) -> None:
        self.result = result
        super().__init__(_format_process_error(result))


def run_process(
    command: Command,
    *,
    cwd: Path | str | None = None,
    env: Mapping[str, str] | None = None,
    capture: bool = True,
    stream: bool = False,
    check: bool = True,
) -> ProcessResult:
    command_text = tuple(os.fspath(part) for part in command)
    if not command_text:
        raise ValueError("command must not be empty")

    resolved_cwd = Path(cwd) if cwd is not None else None
    full_env = os.environ.copy()
    if env is not None:
        full_env.update(env)

    if stream:
        result = _run_streaming(
            command_text,
            cwd=resolved_cwd,
            env=full_env,
            capture=capture,
        )
    else:
        completed = subprocess.run(
            command_text,
            cwd=resolved_cwd,
            env=full_env,
            text=True,
            stdout=subprocess.PIPE if capture else None,
            stderr=subprocess.PIPE if capture else None,
            check=False,
        )
        result = ProcessResult(
            command=command_text,
            returncode=completed.returncode,
            cwd=resolved_cwd,
            stdout=completed.stdout or "",
            stderr=completed.stderr or "",
        )

    if check and result.returncode != 0:
        raise ProcessError(result)
    return result


def _run_streaming(
    command: tuple[str, ...],
    *,
    cwd: Path | None,
    env: Mapping[str, str],
    capture: bool,
) -> ProcessResult:
    process = subprocess.Popen(
        command,
        cwd=cwd,
        env=dict(env),
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout_parts: list[str] = []
    stderr_parts: list[str] = []

    stdout_thread = Thread(
        target=_copy_stream,
        args=(process.stdout, sys.stdout, stdout_parts, capture),
    )
    stderr_thread = Thread(
        target=_copy_stream,
        args=(process.stderr, sys.stderr, stderr_parts, capture),
    )
    stdout_thread.start()
    stderr_thread.start()
    returncode = process.wait()
    stdout_thread.join()
    stderr_thread.join()

    return ProcessResult(
        command=command,
        returncode=returncode,
        cwd=cwd,
        stdout="".join(stdout_parts),
        stderr="".join(stderr_parts),
    )


def _copy_stream(
    source: TextIO | None,
    dest: TextIO,
    captured: list[str],
    capture: bool,
) -> None:
    if source is None:
        return
    for line in source:
        dest.write(line)
        dest.flush()
        if capture:
            captured.append(line)


def _format_process_error(result: ProcessResult) -> str:
    lines = [
        f"command failed with exit code {result.returncode}: {_format_command(result.command)}",
    ]
    if result.cwd is not None:
        lines.append(f"cwd: {result.cwd}")
    if result.stdout:
        lines.append("stdout:")
        lines.append(result.stdout.rstrip())
    if result.stderr:
        lines.append("stderr:")
        lines.append(result.stderr.rstrip())
    return "\n".join(lines)


def _format_command(command: tuple[str, ...]) -> str:
    return shlex.join(command)
