"""Classify compiler operands using bounded source reads and explicit working paths."""

from __future__ import annotations

import stat
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.files import read_file

_LIMIT = 16 * 1024 * 1024
_FORCED_OPTIONS = ("-include", "-imacros")
_VALUE_OPTIONS = ("-D", "-U", "-A", "-I")
_WORD_OPTIONS = (
    "-isystem",
    "-idirafter",
    "-iprefix",
    "-iwithprefix",
    "-iwithprefixbefore",
)


@dataclass(frozen=True, slots=True)
class Argument:
    role: str
    value: str
    tokens: tuple[str, ...]


def parse_arguments(
    arguments: list[str] | tuple[str, ...], *, generated_output: bool = False
) -> tuple[Argument, ...]:
    """Preserve supported preprocessing/output spans without interpreting cpp specs."""
    check_deadline()
    if not isinstance(generated_output, bool):
        raise ValueError("generated output admission must be boolean")
    allowance = 2 if generated_output else 0
    if (
        len(arguments) > 256 + allowance
        or sum(len(arg) for arg in arguments) > 65536 + allowance
    ):
        raise ValueError("compiler argument bounds exceeded")
    return tuple(iter_arguments(arguments))


def iter_arguments(
    arguments: list[str] | tuple[str, ...], *, partial: bool = False
) -> Iterator[Argument]:
    """Read spans; stored flag fragments may end before an operand is supplied."""
    check_deadline()
    if any("\x00" in arg or arg.startswith("@") for arg in arguments):
        raise ValueError("response files and stdin compiler inputs are unsupported")
    index = 0
    operands = False
    while index < len(arguments):
        check_deadline()
        token = arguments[index]
        index += 1
        if token == "-" and not partial:
            raise ValueError("stdin compiler inputs are unsupported")
        if not operands and token == "--":
            operands = True
            yield Argument("delimiter", token, (token,))
            continue
        option = None
        if not operands and token != "-I-":
            option = (
                token
                if token in _WORD_OPTIONS
                else next(
                    (
                        name
                        for name in (*_FORCED_OPTIONS, *_VALUE_OPTIONS, "-o")
                        if token.startswith(name)
                    ),
                    None,
                )
            )
        if option is not None:
            if token == option:
                if index == len(arguments):
                    if not partial:
                        raise ValueError(f"missing compiler operand after {option}")
                    value, tokens = "", (token,)
                else:
                    value = arguments[index]
                    index += 1
                    tokens = (token, value)
            else:
                value = token[len(option) :]
                tokens = (token,)
            if not partial and option in (*_FORCED_OPTIONS, "-o") and value == "-":
                raise ValueError("stdin/stdout compiler operands are unsupported")
            yield Argument("output" if option == "-o" else option, value, tokens)
        else:
            role = "option" if not operands and token.startswith("-") else "source"
            yield Argument(role, token, (token,))
    check_deadline()


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
