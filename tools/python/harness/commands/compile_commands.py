"""Generate the ignored compilation database used by focused tooling."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from ..io import repo_layout
from ._common import run_main


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    output = root / "compile_commands.json"
    compiler = root / "bin" / "cc"
    common = [
        str(compiler),
        "-DHARNESS_TARGET_PSX=1",
        f"-I{root / 'include'}",
        f"-I{root / 'toolchains' / 'psyq' / '4.7' / 'include'}",
        "-O2",
        "-G0",
        "-funsigned-char",
        "-msoft-float",
        "-gcoff",
        "-Wa,--aspsx-version=2.56",
        "-Wa,-G0,-EL,-mips1",
    ]
    entries = []
    for source in sorted((root / "src").rglob("*.c")):
        object_path = root / "build" / source.relative_to(root).with_suffix(".o")
        entries.append(
            {
                "directory": str(root),
                "file": str(source),
                "arguments": [*common, "-c", str(source), "-o", str(object_path)],
            }
        )
    output.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    print(output.relative_to(root))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="compile-commands")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
