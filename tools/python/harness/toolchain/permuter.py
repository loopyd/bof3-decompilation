"""Own decomp-permuter installation, workspace preparation and invocation."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path

from ..common.cli import add_root_argument, run_main
from ..io import repo_layout
from .base import PythonScriptSubmoduleToolchain


class DecompPermuterToolchain(PythonScriptSubmoduleToolchain):
    label = "decomp-permuter"
    submodule = "third_party/decomp-permuter"
    script = "permuter.py"
    interpreter_flags = ("-u",)
    pip_packages = ("toml",)

    @property
    def preparation_tools(self) -> tuple[Path, ...]:
        binaries = self.layout.psyqo_toolchain_root / "bin"
        return (
            self.root / "bin/cc",
            binaries / "mipsel-none-elf-as",
            binaries / "mipsel-none-elf-objdump",
        )

    def verify(self) -> str:
        label = super().verify()
        for path in self.preparation_tools:
            if not path.is_file():
                raise FileNotFoundError(f"missing permuter preparation tool: {path}")
            if path.is_symlink() or path.stat().st_nlink != 1:
                raise ValueError(
                    f"permuter preparation tool must be an independent file: {path}"
                )
        result = subprocess.run(
            [
                str(self.python),
                "-P",
                "-c",
                "from src import ast_util; "
                "from strip_other_fns import strip_other_fns; "
                "from harness.toolchain.permuter import DecompPermuterToolchain",
            ],
            cwd=self.working_directory,
            env=self.environment,
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        if result.returncode:
            raise RuntimeError(
                f"permuter preparation imports exited {result.returncode}"
            )
        with tempfile.TemporaryDirectory(prefix="bof3-permuter-setup-") as temporary:
            source = Path(temporary) / "probe.c"
            source.write_text("int bof3_permuter_setup_probe;\n", encoding="ascii")
            subprocess.run(
                [str(self.root / "bin/cc"), "-x", "c", "-E", str(source)],
                cwd=self.root,
                env=self.environment,
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        for path in self.preparation_tools[1:]:
            subprocess.run(
                [str(path), "--version"],
                cwd=self.root,
                env=self.environment,
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        return label

    def prepare(self, source: Path, function: str, directory: Path) -> int:
        """Prepare through the managed project interpreter and import environment."""
        result = subprocess.run(
            [
                str(self.python),
                "-P",
                "-m",
                "harness.toolchain.permuter",
                "--root",
                str(self.root),
                str(source),
                function,
                str(directory),
            ],
            cwd=self.working_directory,
            env=self.environment,
            check=False,
        )
        return result.returncode

    def prepare_workspace(self, source: Path, function: str, directory: Path) -> None:
        if not source.is_file():
            raise FileNotFoundError(f"source does not exist: {source}")
        if not directory.is_dir():
            raise FileNotFoundError(f"directory does not exist: {directory}")
        if not (directory / "target.s").is_file():
            raise FileNotFoundError(f"directory is missing target.s: {directory}")
        base = self.keep_function_body(self.preprocess(source), function)
        (directory / "base.c").write_text(base, encoding="ascii")
        self.write_compile_script(directory)
        self.assemble_target(directory)
        self.write_settings(directory, function)
        print(f"prepared {directory} ({len(base.splitlines())} lines)")

    def preprocess(self, source: Path) -> str:
        command = [
            str(self.root / "bin/cc"),
            "-E",
            "-P",
            "-DHARNESS_TARGET_PSX=1",
            "-I",
            str(self.root / "include"),
            "-I",
            str(self.root / "toolchains/psyq/4.7/include"),
            str(source),
        ]
        return subprocess.run(
            command, check=True, capture_output=True, text=True
        ).stdout

    def keep_function_body(self, source: str, function: str) -> str:
        """Prune context to declarations and types used by the selected function."""
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

    def write_compile_script(self, directory: Path) -> None:
        script = f'''#!/usr/bin/env bash
set -euo pipefail
ROOT="{self.root}"
INPUT="${{1:?missing input C file}}"
[[ "${{2:-}}" == "-o" ]] || {{ echo "expected -o" >&2; exit 2; }}
OUTPUT="${{3:?missing output object}}"
"$ROOT/bin/cc" -DHARNESS_TARGET_PSX=1 \\
  -I "$ROOT/include" -I "$ROOT/toolchains/psyq/4.7/include" \\
  -O2 -G0 -funsigned-char -msoft-float -gcoff \\
  -Wa,--aspsx-version=2.56 -Wa,-G0,-EL,-mips1 \\
  -c "$INPUT" -o "$OUTPUT"
'''
        path = directory / "compile.sh"
        path.write_text(script, encoding="ascii")
        path.chmod(path.stat().st_mode | 0o111)

    def assemble_target(self, directory: Path) -> None:
        source = (directory / "target.s").read_text(encoding="ascii")
        prepared_lines: list[str] = []
        for line in source.splitlines():
            stripped = line.strip()
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
        prepared = directory / "target.permuter.s"
        prepared.write_text("\n".join(prepared_lines) + "\n", encoding="ascii")
        subprocess.run(
            [
                str(self.root / "toolchains/psyqo/bin/mipsel-none-elf-as"),
                "-EL",
                "-march=r3000",
                "-mips1",
                str(prepared),
                "-o",
                str(directory / "target.o"),
            ],
            check=True,
        )

    def write_settings(self, directory: Path, function: str) -> None:
        objdump = self.root / "toolchains/psyqo/bin/mipsel-none-elf-objdump"
        settings = f'''func_name = "{function}"
compiler_type = "gcc"
objdump_command = "{objdump} -drz -m mips:3000"
'''
        (directory / "settings.toml").write_text(settings, encoding="ascii")


def run(args: argparse.Namespace) -> int:
    DecompPermuterToolchain(repo_layout(args.root.resolve())).prepare_workspace(
        args.source, args.function, args.directory
    )
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Prepare a managed permuter workspace."
    )
    add_root_argument(parser)
    parser.add_argument("source", type=Path)
    parser.add_argument("function")
    parser.add_argument("directory", type=Path)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
