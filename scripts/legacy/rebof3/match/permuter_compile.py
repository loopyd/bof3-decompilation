from __future__ import annotations

import argparse
import json
import shlex
import subprocess
from pathlib import Path
from typing import Any


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Compile one decomp-permuter candidate with repo flags."
    )
    parser.add_argument("input_c", type=Path)
    parser.add_argument("-o", "--output", required=True, type=Path)
    parser.add_argument("--compile-commands", required=True, type=Path)
    parser.add_argument("--source-file", required=True, type=Path)
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def load_compile_entry(
    compile_commands_path: Path, *, source_file: Path
) -> dict[str, object]:
    payload = json.loads(compile_commands_path.read_text(encoding="utf-8"))
    resolved_source = source_file.resolve()
    for entry in payload:
        candidate = Path(str(entry.get("file") or "")).resolve()
        if candidate == resolved_source:
            return dict(entry)
    raise LookupError(f"compile command not found for {resolved_source}")


def compile_entry_args(entry: dict[str, object]) -> list[str]:
    arguments = entry.get("arguments")
    if isinstance(arguments, list) and arguments:
        return [str(value) for value in arguments]
    command_text = str(entry.get("command") or "")
    if not command_text:
        raise LookupError("compile command entry is missing command text")
    return shlex.split(command_text)


def compile_entry_directory(entry: dict[str, object], *, fallback: Path) -> Path:
    directory = Path(str(entry.get("directory") or ""))
    base = directory if str(directory) else fallback
    return base.resolve()


def resolve_output_path(entry: dict[str, object], *, fallback: Path) -> Path:
    output = str(entry.get("output") or "")
    if output:
        path = Path(output)
        return path.resolve() if path.is_absolute() else (fallback / path).resolve()

    args = compile_entry_args(entry)
    for index, value in enumerate(args[:-1]):
        if value == "-o":
            output_path = Path(args[index + 1])
            if output_path.is_absolute():
                return output_path.resolve()
            return (fallback / output_path).resolve()
    raise LookupError("-o output slot not found in compile command")


def rewrite_compile_args(
    args: list[str],
    *,
    source_file: Path,
    input_c: Path,
    output: Path,
    directory: Path | None = None,
) -> list[str]:
    rewritten = list(args)
    resolved_source = source_file.resolve()
    source_replaced = False
    output_replaced = False
    for index, value in enumerate(rewritten):
        candidate = Path(value)
        resolved_candidate = None
        if candidate.is_absolute():
            resolved_candidate = candidate.resolve()
        elif directory is not None:
            resolved_candidate = (directory / candidate).resolve()
        if resolved_candidate == resolved_source:
            rewritten[index] = str(input_c.resolve())
            source_replaced = True
            continue
        if value == "-o" and index + 1 < len(rewritten):
            rewritten[index + 1] = str(output.resolve())
            output_replaced = True
    if not source_replaced:
        raise LookupError(
            f"source file not found in compile command: {resolved_source}"
        )
    if not output_replaced:
        raise LookupError("-o output slot not found in compile command")
    return rewritten


def rewrite_compile_command(
    command_text: str, *, source_file: Path, input_c: Path, output: Path
) -> list[str]:
    return rewrite_compile_args(
        shlex.split(command_text),
        source_file=source_file,
        input_c=input_c,
        output=output,
    )


def rewrite_compile_entry(
    entry: dict[str, Any], *, source_file: Path, input_c: Path, output: Path
) -> list[str]:
    directory = compile_entry_directory(entry, fallback=Path.cwd())
    return rewrite_compile_args(
        compile_entry_args(entry),
        source_file=source_file,
        input_c=input_c,
        output=output,
        directory=directory,
    )


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    entry = load_compile_entry(args.compile_commands, source_file=args.source_file)
    command = rewrite_compile_entry(
        entry,
        source_file=args.source_file,
        input_c=args.input_c,
        output=args.output,
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run(
        command,
        cwd=compile_entry_directory(entry, fallback=args.compile_commands.parent),
        check=False,
    )
    return int(result.returncode)


if __name__ == "__main__":
    raise SystemExit(main())
