#!/usr/bin/env python3
"""Create a minimal decomp-permuter workspace for one BOF3 function."""

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PERMUTER = ROOT / "third_party/decomp-permuter"


def preprocess(source: Path) -> str:
    command = [
        str(ROOT / "bin/cc"),
        "-E",
        "-P",
        "-DHARNESS_TARGET_PSX=1",
        "-I",
        str(ROOT / "include"),
        "-I",
        str(ROOT / "toolchains/psyq/4.7/include"),
        str(source),
    ]
    return subprocess.run(command, check=True, capture_output=True, text=True).stdout


def keep_function_body(source: str, function: str) -> str:
    """Prune context to declarations and types used by the selected function."""
    sys.path.insert(0, str(PERMUTER))
    from src import ast_util

    try:
        ast = ast_util.parse_c(source, from_import=True)
        selected, _ = ast_util.extract_fn(ast, function)
        ast_util.prune_ast(selected, ast)
        return ast_util.to_c_raw(ast)
    except Exception as exc:
        from strip_other_fns import strip_other_fns

        print(
            f"warning: AST context pruning failed for {function}: {exc}; "
            "keeping all declarations",
            file=sys.stderr,
        )
        return strip_other_fns(source, function)


def write_compile_script(directory: Path) -> None:
    script = f"""#!/usr/bin/env bash
set -euo pipefail
ROOT="{ROOT}"
INPUT="${{1:?missing input C file}}"
[[ "${{2:-}}" == "-o" ]] || {{ echo "expected -o" >&2; exit 2; }}
OUTPUT="${{3:?missing output object}}"
"$ROOT/bin/cc" -DHARNESS_TARGET_PSX=1 \
  -I "$ROOT/include" -I "$ROOT/toolchains/psyq/4.7/include" \
  -O2 -G0 -funsigned-char -msoft-float -gcoff \
  -Wa,--aspsx-version=2.56 -Wa,-G0,-EL,-mips1 \
  -c "$INPUT" -o "$OUTPUT"
"""
    path = directory / "compile.sh"
    path.write_text(script, encoding="ascii")
    path.chmod(0o755)


def assemble_target(directory: Path) -> None:
    source = (directory / "target.s").read_text(encoding="ascii")
    prepared_lines: list[str] = []
    for line in source.splitlines():
        stripped = line.strip()
        # Splat's macro include only supplies metadata directives. The target
        # object needs ordinary assembler syntax for the function label.
        if stripped in {'.include "macro.inc"', ".set gp=64"}:
            continue
        if stripped.startswith("nonmatching ") or stripped.startswith("endlabel "):
            continue
        match = re.fullmatch(r"glabel\s+(\S+)", stripped)
        if match is not None:
            name = match.group(1)
            prepared_lines.extend((f".globl {name}", f"{name}:"))
            continue
        prepared_lines.append(line)
    source = "\n".join(prepared_lines)
    prepared = directory / "target.permuter.s"
    prepared.write_text(source + "\n", encoding="ascii")
    subprocess.run(
        [
            str(ROOT / "toolchains/psn00b_toolchain/bin/mipsel-none-elf-as"),
            "-EL",
            "-march=r3000",
            "-mips1",
            str(prepared),
            "-o",
            str(directory / "target.o"),
        ],
        check=True,
    )


def write_settings(directory: Path, function: str) -> None:
    objdump = ROOT / "toolchains/psn00b_toolchain/bin/mipsel-none-elf-objdump"
    settings = f'''func_name = "{function}"
compiler_type = "gcc"
objdump_command = "{objdump} -drz -m mips:3000"
'''
    (directory / "settings.toml").write_text(settings, encoding="ascii")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("function")
    parser.add_argument("directory", type=Path)
    args = parser.parse_args()

    if not args.source.is_file():
        parser.error(f"source does not exist: {args.source}")
    if not args.directory.is_dir():
        parser.error(f"directory does not exist: {args.directory}")
    if not (args.directory / "target.s").is_file():
        parser.error(f"directory is missing target.s: {args.directory}")

    base = keep_function_body(preprocess(args.source), args.function)
    (args.directory / "base.c").write_text(base, encoding="ascii")
    write_compile_script(args.directory)
    assemble_target(args.directory)
    write_settings(args.directory, args.function)
    print(f"prepared {args.directory} ({len(base.splitlines())} lines)")


if __name__ == "__main__":
    main()
