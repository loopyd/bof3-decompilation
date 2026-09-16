"""Describe and render compiler stages without executing or allocating artifacts."""

from __future__ import annotations

import os
import shutil
from dataclasses import dataclass, field
from pathlib import Path

from harness.build.runtime import ASSEMBLER_POLICY, MASPSX_POLICY, Runtime
from harness.common.digests import digest
from harness.common.process import resolve_supervisor


@dataclass(frozen=True, slots=True)
class Artifact:
    name: str

    def __post_init__(self) -> None:
        if self.name not in {
            "root",
            "direct-output",
            "compiler.s",
            "partitioned.s",
            "translation.o",
        }:
            raise ValueError("unknown compiler staging artifact")

    def render(self, temporary: Path) -> str:
        if not temporary.is_absolute():
            raise ValueError("compiler staging directory must be absolute")
        return str(temporary if self.name == "root" else temporary / self.name)


def _describe_value(value: str | Artifact) -> dict:
    return (
        {"artifact": value.name} if isinstance(value, Artifact) else {"literal": value}
    )


@dataclass(frozen=True, slots=True)
class Stage:
    name: str
    arguments: tuple[str | Artifact, ...]
    environment: tuple[tuple[str, str | Artifact], ...] = field(repr=False)
    cwd: Path
    stdin: Artifact | None = None
    output: Artifact | None = None
    files: tuple[int, ...] = ()
    supervisor: tuple[str, ...] = field(default_factory=resolve_supervisor)
    runtime: Runtime | None = None

    def describe(self) -> dict:
        return {
            "name": self.name,
            "arguments": [_describe_value(value) for value in self.arguments],
            "environment_fingerprint": digest(
                {name: _describe_value(value) for name, value in self.environment}
            ),
            "temporary_environment": {
                name: value.name
                for name, value in self.environment
                if isinstance(value, Artifact)
            },
            "cwd": str(self.cwd),
            "stdin": self.stdin.name if self.stdin is not None else None,
            "output": self.output.name if self.output is not None else None,
            "files": list(self.files),
            "supervisor": list(self.supervisor),
            "runtime": self.runtime.describe() if self.runtime is not None else None,
        }

    def render(self, temporary: Path) -> tuple[list[str], dict[str, str]]:
        if not temporary.is_absolute():
            raise ValueError("compiler staging directory must be absolute")
        return (
            [
                value.render(temporary) if isinstance(value, Artifact) else value
                for value in self.arguments
            ],
            {
                name: value.render(temporary) if isinstance(value, Artifact) else value
                for name, value in self.environment
            },
        )


@dataclass(frozen=True, slots=True)
class Invocation:
    mode: str
    source: Path | None
    output: Path | None
    stages: tuple[Stage, ...]

    def describe(self) -> dict:
        return {
            "schema": "bof3.compiler-invocation/v3",
            "mode": self.mode,
            "source": str(self.source) if self.source is not None else None,
            "output": str(self.output) if self.output is not None else None,
            "stages": [stage.describe() for stage in self.stages],
        }


def plan_dispatch(state: dict, environment: dict[str, str]) -> Invocation:
    """Build the same stage recipe for capture, execution and fresh verification."""
    return plan_invocation(
        Path(state["root"]),
        state["arguments"],
        sources=[Path(name) for name in state["sources"]],
        output=Path(state["output"]) if state["output"] is not None else None,
        compiler=Path(state["compiler"]),
        executable=state["executable"],
        cwd=Path(state["working_directory"]["path"]),
        environment=environment,
    )


def _compiler_environment(
    root: Path, compiler: Path, environment: dict, cwd: Path
) -> dict:
    executable = str(compiler)
    search = os.pathsep.join(
        str(cwd / entry)
        for entry in environment.get("PATH", os.defpath).split(os.pathsep)
    )
    resolved = (
        shutil.which(executable, path=search) if "/" not in executable else executable
    )
    if resolved is None:
        raise ValueError(f"missing compiler: {compiler}")
    directory = str((cwd / resolved).parent)
    return {
        **environment,
        "GCC_EXEC_PREFIX": directory + "/",
        "COMPILER_PATH": directory,
        "PATH": os.pathsep.join(
            [
                directory,
                str(root / "toolchains/psn00b_toolchain/bin"),
                environment.get("PATH", ""),
            ]
        ),
    }


def _resolve_version(arguments: tuple[str, ...], environment: dict[str, str]) -> str:
    version = environment.get("ASPSX_VERSION", "2.56")
    for argument in arguments:
        if argument.startswith("-Wa,"):
            for flag in argument[4:].split(","):
                if flag.startswith("--aspsx-version="):
                    version = flag.split("=", 1)[1]
    return version


def _replace_output(arguments: tuple[str, ...], output: Artifact) -> list:
    result = []
    skip = False
    for argument in arguments:
        if skip:
            skip = False
        elif argument == "-o":
            skip = True
        elif not argument.startswith("-o"):
            result.append(argument)
    return [*result, "-o", output]


def _assembler_flags(arguments: tuple[str, ...]) -> list[str]:
    return [
        flag
        for argument in arguments
        if argument.startswith("-Wa,")
        for flag in argument[4:].split(",")
        if flag and flag != "--expand-div" and not flag.startswith("--aspsx-version=")
    ]


def _compiler_arguments(arguments: tuple[str, ...], assembly: Artifact) -> list:
    result = []
    skip_output = False
    for argument in arguments:
        if skip_output:
            skip_output = False
        elif argument == "-o":
            skip_output = True
        elif argument == "-c" or argument.startswith("-o"):
            continue
        elif argument.startswith("-Wa,"):
            retained = _assembler_flags((argument,))
            if retained:
                result.append("-Wa," + ",".join(retained))
        else:
            result.append(argument)
    return [*result, "-S", "-o", assembly]


def select_mode(
    arguments: list[str] | tuple[str, ...],
    *,
    sources: list[Path],
    output: Path | None,
    cwd: Path,
    environment: dict[str, str],
) -> tuple[str, Path | None]:
    """Validate the supported branch before allocating compiler staging."""
    compile_mode = "-c" in arguments and not any(
        flag in arguments for flag in ("-E", "-S", "-M", "-MM")
    )
    named = [path for path in sources if path.suffix == ".c"]
    named.extend(
        cwd / argument
        for argument in arguments
        if not argument.startswith("-")
        and argument.endswith((".s", ".S"))
        and cwd / argument != output
    )
    driver = environment.get("PSX_CC_DRIVER")
    if compile_mode and not driver and (len(named) != 1 or output is None):
        raise ValueError(
            "bin/cc expects exactly one .c/.s/.S source and a -o output for -c"
        )
    if (
        not compile_mode
        and not driver
        and not any(argument in {"-E", "-M", "-MM", "-S"} for argument in arguments)
    ):
        raise ValueError(
            "link mode is unsupported; compile with -c and link with bin/ld"
        )
    if driver or not compile_mode:
        return "direct", named[0] if len(named) == 1 else None
    return ("c" if named[0].suffix == ".c" else "assembly"), named[0]


def plan_invocation(
    root: Path,
    arguments: list[str] | tuple[str, ...],
    *,
    sources: list[Path],
    output: Path | None,
    compiler: Path,
    executable: str,
    cwd: Path,
    environment: dict[str, str],
) -> Invocation:
    """Freeze ordered stage recipes from explicit inputs, preserving branch behavior."""
    if not root.is_absolute() or not cwd.is_absolute():
        raise ValueError("compiler recipe root and working directory must be absolute")
    arguments = tuple(arguments)
    environment = dict(environment)
    mode, source = select_mode(
        arguments, sources=sources, output=output, cwd=cwd, environment=environment
    )
    driver = environment.get("PSX_CC_DRIVER")
    base = {**environment, "PYTHONDONTWRITEBYTECODE": "1", "TMPDIR": Artifact("root")}
    compiler_environment = _compiler_environment(root, compiler, base, cwd)

    def stage(name, argv, values, *, stdin=None, produced=None, files=(), runtime=None):
        return Stage(
            name,
            tuple(argv),
            tuple(sorted(values.items())),
            cwd,
            stdin,
            produced,
            files,
            runtime=runtime,
        )

    if mode == "direct":
        produced = Artifact("direct-output") if output is not None else None
        argv = _replace_output(arguments, produced) if produced else list(arguments)
        return Invocation(
            "direct",
            source,
            output,
            (
                stage(
                    "direct",
                    [executable, *argv],
                    base if driver else compiler_environment,
                    produced=produced,
                ),
            ),
        )
    assembler = environment.get("PSX_AS") or str(root / "bin/as")
    produced = Artifact("translation.o")
    assembler_arguments = [assembler, *_assembler_flags(arguments), "-o", produced]
    if mode == "assembly":
        operand = next(
            argument
            for argument in arguments
            if not argument.startswith("-") and cwd / argument == source
        )
        return Invocation(
            "assembly",
            source,
            output,
            (
                stage(
                    "assembler",
                    [*assembler_arguments, operand],
                    base,
                    produced=produced,
                    runtime=Runtime(ASSEMBLER_POLICY, root),
                ),
            ),
        )
    assembly = Artifact("compiler.s")
    maspsx = environment.get("PSX_MASPSX") or str(root / "third_party/maspsx/maspsx.py")
    interpreter = environment.get("MASPSX_PYTHON") or "python3"
    python_environment = {
        **base,
        "PYTHONPATH": os.pathsep.join(
            [str(root / "third_party/maspsx"), str(root / "tools/python")]
        ),
        "PYTHONSAFEPATH": "1",
    }
    expand = any(
        "--expand-div" in argument[4:].split(",")
        for argument in arguments
        if argument.startswith("-Wa,")
    )
    return Invocation(
        "c",
        source,
        output,
        (
            stage(
                "compiler",
                [executable, *_compiler_arguments(arguments, assembly)],
                compiler_environment,
                produced=assembly,
            ),
            stage(
                "maspsx",
                [
                    interpreter,
                    "-P",
                    maspsx,
                    "--aspsx-version=" + _resolve_version(arguments, environment),
                    *(["--expand-div"] if expand else []),
                ],
                python_environment,
                stdin=assembly,
                files=(2,),
                runtime=Runtime(MASPSX_POLICY, root),
            ),
            stage(
                "assembler",
                [*assembler_arguments, Artifact("partitioned.s")],
                base,
                produced=produced,
                runtime=Runtime(ASSEMBLER_POLICY, root),
            ),
        ),
    )
