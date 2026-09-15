"""Generate the ignored compilation database used by focused tooling."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import add_root_argument, run_main

from ..build.compiler import (
    build_compiler_arguments,
    load_object_compilers,
    load_object_flags,
    sanitize_identifier,
)
from ..io import repo_layout
from ..toolchain.gcc_variants import lookup_variant, resolve_variant


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    output = root / "compile_commands.json"
    object_flags = load_object_flags(root)
    object_compilers = load_object_compilers(root)
    src_root = root / "src"
    entries = []
    for source in sorted(src_root.rglob("*.c")):
        object_path = root / "build" / source.relative_to(root).with_suffix(".o")
        relative = source.relative_to(src_root).as_posix()
        key = sanitize_identifier(relative)
        compiler_id = object_compilers.get(key)
        # Build argument vector
        if compiler_id is None:
            variant_prefix: list[str] = []
        else:
            layout = repo_layout(root)
            variant = lookup_variant(layout, compiler_id)
            gcc_path = resolve_variant(layout, variant)
            variant_prefix = [
                "cmake",
                "-E",
                "env",
                f"PSX_GCC={gcc_path}",
            ]
        base_args = [
            *build_compiler_arguments(root, source, object_flags),
            "-c",
            str(source),
            "-o",
            str(object_path),
        ]
        arguments = [*variant_prefix, *base_args]
        entries.append(
            {
                "directory": str(root),
                "file": str(source),
                "arguments": arguments,
            }
        )
    output.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    print(output.relative_to(root))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="compile-commands")
    add_root_argument(parser)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
