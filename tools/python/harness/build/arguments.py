"""Classify compiler operands using bounded source reads and explicit working paths."""

from __future__ import annotations

import stat
from dataclasses import dataclass
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.files import read_file

_LIMIT = 16 * 1024 * 1024
_FORCED_OPTIONS = ("-include", "-imacros")


@dataclass(frozen=True, slots=True)
class Argument:
    role: str
    value: str
    tokens: tuple[str, ...]


def parse_arguments(arguments: list[str] | tuple[str, ...]) -> tuple[Argument, ...]:
    """Preserve explicit forced/output operand spans without interpreting cpp specs."""
    check_deadline()
    if len(arguments) > 256 or sum(len(arg) for arg in arguments) > 65536:
        raise ValueError("compiler argument bounds exceeded")
    if any("\x00" in arg or arg == "-" or arg.startswith("@") for arg in arguments):
        raise ValueError("response files and stdin compiler inputs are unsupported")
    result = []
    index = 0
    operands = False
    while index < len(arguments):
        check_deadline()
        token = arguments[index]
        index += 1
        if not operands and token == "--":
            operands = True
            result.append(Argument("delimiter", token, (token,)))
            continue
        option = (
            None
            if operands
            else next(
                (name for name in (*_FORCED_OPTIONS, "-o") if token.startswith(name)),
                None,
            )
        )
        if option is not None:
            if token == option:
                if index == len(arguments):
                    raise ValueError(f"missing compiler operand after {option}")
                value = arguments[index]
                index += 1
                tokens = (token, value)
            else:
                value = token[len(option) :]
                tokens = (token,)
            result.append(
                Argument("output" if option == "-o" else option, value, tokens)
            )
        else:
            role = "option" if not operands and token.startswith("-") else "source"
            result.append(Argument(role, token, (token,)))
    check_deadline()
    return tuple(result)


@dataclass(frozen=True, slots=True)
class ForcedInput:
    option: str
    path: Path


@dataclass(frozen=True, slots=True)
class Operands:
    sources: tuple[Path, ...]
    forced: tuple[ForcedInput, ...]
    output: Path | None

    @property
    def inputs(self) -> tuple[Path, ...]:
        return tuple(
            dict.fromkeys((*self.sources, *(item.path for item in self.forced)))
        )

    def describe_forced(self) -> list[dict[str, str]]:
        return [{"option": item.option, "path": str(item.path)} for item in self.forced]


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


def inspect_arguments(arguments: list[str], *, cwd: Path | None = None) -> Operands:
    """Classify explicit operands without launching a compiler."""
    cwd = Path.cwd() if cwd is None else cwd
    if not cwd.is_absolute():
        raise ValueError("compiler working directory must be absolute")
    parsed = parse_arguments(arguments)
    options = {argument.value for argument in parsed if argument.role == "option"}
    candidates = []
    forced = []
    output = None
    for argument in parsed:
        check_deadline()
        if argument.role in _FORCED_OPTIONS:
            path = cwd / argument.value
            read_source(path)
            forced.append(ForcedInput(argument.role, path))
            continue
        if argument.role == "output":
            if output is not None:
                raise ValueError("multiple compiler outputs are unsupported")
            output = cwd / argument.value
            continue
        if argument.role != "source":
            continue
        path = cwd / argument.value
        if path.is_file() or path.suffix.lower() in {".c", ".s"}:
            try:
                read_source(path)
            except UnicodeDecodeError:
                if path.suffix.lower() == ".c":
                    raise ValueError("compiler C source must be UTF-8") from None
                continue
            candidates.append(path)
    if (
        output is None
        and candidates
        and "-c" not in options
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
            for argument in options
        )
    ):
        if len(candidates) != 1:
            raise ValueError("implicit compiler outputs require one named source")
        output = cwd / (
            candidates[0].with_suffix(".s").name if "-S" in options else "a.out"
        )
    return Operands(tuple(dict.fromkeys(candidates)), tuple(forced), output)
