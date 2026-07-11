from __future__ import annotations

import os
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[4]
DEFAULT_GCC = ROOT / "toolchains" / "gcc-2.7.2-psx" / "gcc"
DEFAULT_MASPSX = ROOT / "third_party" / "maspsx" / "maspsx.py"
DEFAULT_AS = ROOT / "bin" / "as"


def _tool(env_name: str, default: Path) -> str:
    return os.environ.get(env_name, str(default))


def _source_args(args: list[str]) -> list[str]:
    return [arg for arg in args if Path(arg).suffix in {".c", ".s", ".S"}]


def _output_arg(args: list[str]) -> str | None:
    for index, arg in enumerate(args):
        if arg == "-o" and index + 1 < len(args):
            return args[index + 1]
        if arg.startswith("-o") and len(arg) > 2:
            return arg[2:]
    return None


def _without_aspsx_option(args: list[str]) -> tuple[list[str], str]:
    version = os.environ.get("ASPSX_VERSION", "2.56")
    forwarded: list[str] = []
    for arg in args:
        prefix = "-Wa,--aspsx-version="
        if arg.startswith(prefix):
            version = arg.removeprefix(prefix)
        else:
            forwarded.append(arg)
    return forwarded, version


def _run(command: list[str], *, env: dict[str, str] | None = None) -> None:
    result = subprocess.run(command, env=env)
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def _gcc_environment(gcc: str) -> dict[str, str]:
    env = os.environ.copy()
    gcc_dir = str(Path(gcc).resolve().parent)
    toolchain_bin = str(ROOT / "toolchains" / "psn00b_toolchain" / "bin")
    env["PATH"] = os.pathsep.join((gcc_dir, toolchain_bin, env.get("PATH", "")))
    env["GCC_EXEC_PREFIX"] = f"{gcc_dir}/"
    env["COMPILER_PATH"] = gcc_dir
    return env


def _assemble(source: str, output: str, args: list[str]) -> None:
    assembler_args: list[str] = []
    for arg in args:
        if arg.startswith("-Wa,"):
            assembler_args.extend(part for part in arg[4:].split(",") if part)
    _run([_tool("PSX_AS", DEFAULT_AS), *assembler_args, "-o", output, source])


def main(argv: list[str] | None = None) -> int:
    args = list(sys.argv[1:] if argv is None else argv)
    forwarded, aspsx_version = _without_aspsx_option(args)
    gcc = _tool("PSX_GCC", DEFAULT_GCC)

    if any(flag in forwarded for flag in ("-E", "-M", "-MM", "-S")):
        _run([gcc, *forwarded], env=_gcc_environment(gcc))
        return 0

    if "-c" not in forwarded:
        print(
            "cc: link mode is unsupported; compile with -c and link with bin/ld",
            file=sys.stderr,
        )
        return 2

    sources = _source_args(forwarded)
    output = _output_arg(forwarded)
    if len(sources) != 1 or output is None:
        print(
            "cc: -c requires exactly one C/assembly source and one -o output",
            file=sys.stderr,
        )
        return 2

    source = sources[0]
    if Path(source).suffix in {".s", ".S"}:
        _assemble(source, output, forwarded)
        return 0

    output_path = Path(output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    compiler_asm = Path(f"{output}.s")
    with tempfile.TemporaryDirectory(prefix="cc-", dir=output_path.parent) as tmp:
        translated_asm = Path(tmp) / "translated.s"
        gcc_args: list[str] = []
        skip_output = False
        for arg in forwarded:
            if skip_output:
                skip_output = False
                continue
            if arg == "-c":
                continue
            if arg == "-o":
                skip_output = True
                continue
            if arg.startswith("-o") and len(arg) > 2:
                continue
            gcc_args.append(arg)
        _run(
            [gcc, *gcc_args, "-S", "-o", str(compiler_asm)],
            env=_gcc_environment(gcc),
        )
        with translated_asm.open("wb") as stream:
            result = subprocess.run(
                [
                    os.environ.get("MASPSX_PYTHON", sys.executable),
                    _tool("MASPSX", DEFAULT_MASPSX),
                    f"--aspsx-version={aspsx_version}",
                    str(compiler_asm),
                ],
                stdout=stream,
            )
        if result.returncode != 0:
            output_path.unlink(missing_ok=True)
            return result.returncode
        try:
            _assemble(str(translated_asm), output, forwarded)
        except SystemExit:
            output_path.unlink(missing_ok=True)
            raise
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
