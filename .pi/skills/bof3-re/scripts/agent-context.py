#!/usr/bin/env python3
"""Emit bounded, role-specific BOF3 context in one read-only command."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / "tools" / "python"))

from harness.domain import (  # noqa: E402
    FUNCTION_ID_FORMAT,
    FUNCTION_ID_HELP,
    lookup_target_manifest,
    parse_function_id,
)

FULL = (
    "AGENTS.md",
    ".pi/skills/bof3-re/SKILL.md",
    "docs/memory-api.md",
)
ROLE = {
    "reverse": (".pi/skills/bof3-lift-loop/references/MISSION_PROTOCOL.md",),
    "review": (".pi/skills/bof3-lift-loop/references/REVIEW_CHECKLIST.md",),
}
IDENTIFIER = re.compile(r"\b(?:D|func)_[0-9A-Fa-f]{8}\b")


def section(path: Path, root: Path, text: str | None = None) -> str:
    name = path.relative_to(root).as_posix()
    return f"\n===== {name} =====\n{text if text is not None else path.read_text(encoding='utf-8')}"


def selector(value: str) -> tuple[str, int]:
    try:
        function = parse_function_id(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError(f"expected {FUNCTION_ID_HELP}") from error
    return function.target.value, function.address


def asm_path(root: Path, target: str, splat: Path, address: int) -> Path:
    match = re.search(
        r"^\s*asm_path:\s*(\S+)\s*$", splat.read_text(encoding="utf-8"), re.MULTILINE
    )
    directory = (
        root / match.group(1) if match else root / "out" / "splat" / target / "asm"
    )
    return directory / f"func_{address:08X}.s"


def around(lines: list[str], needle: str, radius: int = 5) -> str:
    for index, line in enumerate(lines):
        if needle in line:
            start = max(0, index - radius)
            end = min(len(lines), index + radius + 1)
            return "".join(lines[start:end])
    return ""


def map_excerpt(path: Path, address: int) -> str:
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    index = next(
        (
            i
            for i, line in enumerate(lines)
            if (match := re.search(r"=\s*(0x[0-9A-Fa-f]+);", line))
            and int(match.group(1), 0) >= address
        ),
        len(lines),
    )
    return "".join(lines[max(0, index - 3) : index + 3])


def header_excerpt(path: Path, names: set[str]) -> str:
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    boundary = next(
        (i for i, line in enumerate(lines) if "Absolute-address globals" in line),
        min(len(lines), 120),
    )
    output = lines[:boundary]
    output.extend(
        line
        for line in lines[boundary:]
        if any(re.search(rf"\b{re.escape(name)}\b", line) for name in names)
    )
    return "".join(output)


def target_context(root: Path, target: str, address: int) -> list[str]:
    manifest = lookup_target_manifest(root, target)
    if manifest is None:
        raise ValueError(f"unknown target: {target}")
    target = manifest.id.value
    manifest_path = root / "config" / "targets" / target / "target.toml"
    source_dir = root / manifest.source_dir
    map_path = root / "config" / "targets" / target / "symbols.txt"
    splat = root / manifest.splat
    source = source_dir / f"func_{address:08X}.c"
    asm = asm_path(root, target, splat, address)
    asm_text = asm.read_text(encoding="utf-8") if asm.is_file() else ""
    names = set(IDENTIFIER.findall(asm_text)) | {f"func_{address:08X}"}
    paths: list[tuple[Path, str | None]] = [(manifest_path, None)]
    if map_path.is_file():
        paths.append((map_path, map_excerpt(map_path, address)))
    if splat.is_file():
        lines = splat.read_text(encoding="utf-8").splitlines(keepends=True)
        paths.append(
            (splat, "".join(lines[:16]) + around(lines, f"func_{address:08X}"))
        )
    header = source_dir / "internal.h"
    if header.is_file():
        paths.append((header, header_excerpt(header, names)))
    bindings = source_dir / "symbols.c"
    if bindings.is_file():
        paths.append((bindings, None))
    if source.is_file():
        paths.append((source, None))
    if asm.is_file():
        paths.append((asm, asm_text))
    return [section(path, root, text) for path, text in paths]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("role", choices=sorted(ROLE))
    parser.add_argument(
        "function",
        nargs="?",
        type=selector,
        metavar=FUNCTION_ID_FORMAT,
        help=FUNCTION_ID_HELP,
    )
    parser.add_argument("--root", type=Path, default=ROOT)
    args = parser.parse_args()
    root = args.root.resolve()
    paths = (*FULL, *ROLE[args.role])
    missing = [path for path in paths if not (root / path).is_file()]
    if missing:
        print(f"missing required context: {', '.join(missing)}", file=sys.stderr)
        return 2
    try:
        output = [section(root / path, root) for path in FULL]
        output.extend(section(root / path, root) for path in ROLE[args.role])
        if args.function:
            output.extend(target_context(root, *args.function))
    except ValueError as error:
        print(error, file=sys.stderr)
        return 2
    sys.stdout.write("".join(output).lstrip())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
