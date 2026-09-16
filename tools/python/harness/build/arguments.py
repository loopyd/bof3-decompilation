"""Classify compiler operands using bounded source reads and explicit working paths."""

from __future__ import annotations

import stat
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.files import read_file

_LIMIT = 16 * 1024 * 1024


def observe_input(path: Path) -> list:
    current = path
    while not current.exists():
        current = current.parent
    status = current.stat(follow_symlinks=False)
    identity = [
        str(current),
        status.st_dev,
        status.st_ino,
        status.st_mode,
    ]
    if stat.S_ISDIR(status.st_mode):
        return identity
    return [
        *identity,
        status.st_nlink,
        status.st_size,
        status.st_mtime_ns,
        status.st_ctime_ns,
    ]


def read_source(path: Path) -> str:
    before = observe_input(path)
    content = read_file(path.parent, path.name, max_bytes=_LIMIT)
    if before != observe_input(path):
        raise ValueError("compiler source changed during inspection")
    return content.decode("utf-8")


def inspect_arguments(
    arguments: list[str], *, cwd: Path | None = None
) -> tuple[list[Path], Path | None]:
    """Classify explicit operands without launching a compiler."""
    cwd = Path.cwd() if cwd is None else cwd
    if not cwd.is_absolute():
        raise ValueError("compiler working directory must be absolute")
    if len(arguments) > 256 or sum(len(arg) for arg in arguments) > 65536:
        raise ValueError("compiler argument bounds exceeded")
    if any("\x00" in arg or arg == "-" or arg.startswith("@") for arg in arguments):
        raise ValueError("response files and stdin compiler inputs are unsupported")
    candidates = []
    output = None
    output_pending = False
    operands = False
    for argument in arguments:
        check_deadline()
        if output_pending:
            output = cwd / argument
            output_pending = False
            continue
        if argument == "-o":
            if output is not None:
                raise ValueError("multiple compiler outputs are unsupported")
            output_pending = True
            continue
        if argument.startswith("-o") and len(argument) > 2:
            if output is not None:
                raise ValueError("multiple compiler outputs are unsupported")
            output = cwd / argument[2:]
            continue
        if argument == "--":
            operands = True
            continue
        candidate = argument
        if not operands and argument.startswith("-"):
            for option in ("-include", "-imacros"):
                if argument.startswith(option) and len(argument) > len(option):
                    candidate = argument[len(option) :]
                    break
            else:
                continue
        path = cwd / candidate
        if path.is_file() or path.suffix.lower() in {".c", ".s"}:
            try:
                read_source(path)
            except UnicodeDecodeError:
                if path.suffix.lower() == ".c":
                    raise ValueError("compiler C source must be UTF-8") from None
                continue
            candidates.append(path)
    if output_pending:
        raise ValueError("missing compiler output operand")
    if (
        output is None
        and candidates
        and "-c" not in arguments
        and not any(
            argument
            in {
                "-E",
                "-M",
                "-MM",
                "-fsyntax-only",
                "--version",
                "-dumpversion",
                "-dumpmachine",
            }
            or argument.startswith("-print-")
            for argument in arguments
        )
    ):
        if len(candidates) != 1:
            raise ValueError("implicit compiler outputs require one named source")
        output = cwd / (
            candidates[0].with_suffix(".s").name if "-S" in arguments else "a.out"
        )
    return list(dict.fromkeys(candidates)), output
