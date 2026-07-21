"""Generate the ignored compilation database used by focused tooling."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

from ..io import repo_layout
from ._common import run_main


OPTIMIZATION_RE = re.compile(r"^-O(?:[0-3s]|fast)$")
OBJECT_FLAGS_RE = re.compile(r"^\s*set\(\s*BOF3_OBJFLAGS_(\S+)\s+(.*?)\)\s*$")


def _sanitize_identifier(relative: str) -> str:
    return re.sub(r"[^A-Za-z0-9]", "_", relative)


def _load_object_flags(root: Path) -> dict[str, list[str]]:
    """Parse config/compiler/object-flags.cmake into {sanitized_key: flags}.

    Mirrors the include() in CMakeLists.txt so the compile database matches the
    actual per-object build flags.
    """
    path = root / "config" / "compiler" / "object-flags.cmake"
    overrides: dict[str, list[str]] = {}
    if not path.is_file():
        return overrides
    for line in path.read_text(encoding="utf-8").splitlines():
        match = OBJECT_FLAGS_RE.match(line)
        if match is None:
            continue
        overrides[match.group(1)] = match.group(2).split()
    return overrides


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    output = root / "compile_commands.json"
    compiler = root / "bin" / "cc"
    common = [
        str(compiler),
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
    object_flags = _load_object_flags(root)
    src_root = root / "src"
    entries = []
    for source in sorted(src_root.rglob("*.c")):
        object_path = root / "build" / source.relative_to(root).with_suffix(".o")
        relative = source.relative_to(src_root).as_posix()
        override = object_flags.get(_sanitize_identifier(relative))
        if override is None:
            arguments = [*common, "-c", str(source), "-o", str(object_path)]
        else:
            arguments = [
                *c_flags_base,
                *override,
                *wa_flags,
                "-c",
                str(source),
                "-o",
                str(object_path),
            ]
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
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
