"""Generate the ignored compilation database used by focused tooling."""

from __future__ import annotations

import argparse
import json

from ..build.compiler import (
    load_object_compilers,
    load_object_flags,
    sanitize_identifier,
)
from ..io import repo_layout
from ..toolchain.gcc_variants import ensure_variant, lookup_variant
from ._common import add_root_argument, run_main

OPTIMIZATION_RE = __import__("re").compile(r"^-O(?:[0-3s]|fast)$")


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    output = root / "compile_commands.json"
    cc_driver = root / "bin" / "cc"
    common = [
        str(cc_driver),
        "-DHARNESS_TARGET_PSX=1",
        f"-I{root / 'src'}",
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
    c_flags = [flag for flag in common if not flag.startswith("-Wa")]
    wa_flags = [flag for flag in common if flag.startswith("-Wa")]
    c_flags_base = [flag for flag in c_flags if not OPTIMIZATION_RE.match(flag)]
    object_flags = load_object_flags(root)
    object_compilers = load_object_compilers(root)
    src_root = root / "src"
    entries = []
    for source in sorted(src_root.rglob("*.c")):
        object_path = root / "build" / source.relative_to(root).with_suffix(".o")
        relative = source.relative_to(src_root).as_posix()
        key = sanitize_identifier(relative)
        override = object_flags.get(key)
        compiler_id = object_compilers.get(key)
        # Build argument vector
        if compiler_id is None:
            variant_prefix: list[str] = []
        else:
            # Resolve the specific requested compiler ID; a missing install is
            # installed on demand (only this catalog ID is ever downloaded).
            layout = repo_layout(root)
            variant = lookup_variant(layout, compiler_id)
            gcc_path = ensure_variant(layout, variant)
            variant_prefix = [
                "cmake",
                "-E",
                "env",
                f"PSX_GCC={gcc_path}",
            ]
        if override is None:
            base_args = [*common, "-c", str(source), "-o", str(object_path)]
        else:
            base_args = [
                *c_flags_base,
                *override,
                *wa_flags,
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
