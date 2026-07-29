#!/usr/bin/env python3
"""Emit bounded, role-specific BOF3 context in one read-only command."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys
import tomllib


ROOT = Path(__file__).resolve().parents[4]
FULL = (
    "AGENTS.md",
    "docs/index.md",
    ".pi/skills/bof3-re/SKILL.md",
    "docs/memory-api.md",
    "LESSONS.md",
)
SECTIONS = {
    "docs/usage.md": ("## Output budget", "## Ordered workflow", "## Command ownership"),
    "docs/matching.md": (
        "## Loop",
        "## Reuse exact duplicate groups",
        "## Validate a candidate",
        "## Local matching aids",
        "## Data materialization",
        "## Header barrel convention (`internal.h`)",
        "## Naming convention (PSX-era Capcom style)",
    ),
    "docs/matching-playbook.md": (
        "## Symptom-to-lever table",
        "## 1. Compiler profile verification",
        "## 2. Pointer vs array declaration",
        "## 3. Control flow",
        "## 4. `MATCHING_AID` comment convention",
        "## 5. Temporaries and register allocation",
        "## 7. Signedness",
        "## 10. Padding and alignment",
        "## 13. Jump tables",
        "## 16. Register allocation ladder (no pinning)",
        "## 17. `INCLUDE_ASM` fallback",
        "## 18. Permuter gotchas",
    ),
}
ROLE = {
    "reverse": (".pi/skills/bof3-lift-loop/references/MISSION_PROTOCOL.md",),
    "review": (".pi/skills/bof3-lift-loop/references/REVIEW_CHECKLIST.md",),
}
SELECTOR = re.compile(r"^(?P<target>[^@]+)@(?P<address>0x[0-9a-fA-F]+|[0-9]+)$")


def section(path: Path, root: Path, text: str | None = None) -> str:
    name = path.relative_to(root).as_posix()
    return f"\n===== {name} =====\n{text if text is not None else path.read_text(encoding='utf-8')}"


def selected_sections(path: Path, headings: tuple[str, ...]) -> str:
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    wanted = set(headings)
    output: list[str] = []
    index = 0
    while index < len(lines):
        line = lines[index]
        if line.rstrip() not in wanted:
            index += 1
            continue
        level = len(line) - len(line.lstrip("#"))
        end = index + 1
        while end < len(lines):
            candidate = lines[end]
            candidate_level = len(candidate) - len(candidate.lstrip("#"))
            if candidate.startswith("#") and candidate_level <= level:
                break
            end += 1
        output.extend(lines[index:end])
        index = end
    missing = wanted - {line.rstrip() for line in output if line.startswith("#")}
    if missing:
        raise ValueError(f"missing headings in {path}: {', '.join(sorted(missing))}")
    return "".join(output)


def selector(value: str) -> tuple[str, int]:
    match = SELECTOR.fullmatch(value)
    if not match:
        raise argparse.ArgumentTypeError("expected TARGET[#INDEX]@0xADDRESS")
    try:
        address = int(match.group("address"), 0)
    except ValueError as error:
        raise argparse.ArgumentTypeError("expected TARGET[#INDEX]@0xADDRESS") from error
    if address < 0 or address > 0xFFFFFFFF:
        raise argparse.ArgumentTypeError("expected TARGET[#INDEX]@0xADDRESS")
    return match.group("target"), address


def manifest_aliases(manifest_path: Path, manifest: dict[str, object]) -> set[str]:
    target = str(manifest["id"])
    aliases = {target, str(manifest["disc_id"])}
    parts = target.rsplit("/", 1)
    if len(parts) == 2 and parts[1].isdigit():
        aliases.update((f"{parts[0]}#{parts[1]}", f"{Path(parts[0]).name}#{parts[1]}"))
    disc_id = str(manifest["disc_id"])
    aliases.add(Path(disc_id).name)
    if disc_id.startswith("BIN/"):
        aliases.add(disc_id[4:])
    return {alias.casefold() for alias in aliases}


def resolve_target(root: Path, requested: str) -> tuple[Path, dict[str, object]]:
    direct = root / "config" / "targets" / requested / "target.toml"
    if direct.is_file():
        return direct, tomllib.loads(direct.read_text(encoding="utf-8"))
    wanted = requested.casefold()
    matches: list[tuple[Path, dict[str, object]]] = []
    for manifest_path in (root / "config" / "targets").glob("**/target.toml"):
        manifest = tomllib.loads(manifest_path.read_text(encoding="utf-8"))
        if wanted in manifest_aliases(manifest_path, manifest):
            matches.append((manifest_path, manifest))
    if len(matches) != 1:
        raise ValueError(f"unknown or ambiguous target selector: {requested}")
    return matches[0]


def asm_path(root: Path, target: str, splat: Path, address: int) -> Path:
    match = re.search(r"^\s*asm_path:\s*(\S+)\s*$", splat.read_text(encoding="utf-8"), re.MULTILINE)
    directory = root / match.group(1) if match else root / "out" / "splat" / target / "asm"
    return directory / f"func_{address:08X}.s"


def target_paths(root: Path, requested: str, address: int) -> tuple[Path, ...]:
    manifest_path, manifest = resolve_target(root, requested)
    target = str(manifest["id"])
    source_dir = root / str(manifest["source_dir"])
    splat = root / str(manifest["splat"])
    paths = (
        manifest_path,
        root / "config" / "targets" / target / "symbols.txt",
        splat,
        source_dir / "internal.h",
        source_dir / "symbols.c",
        source_dir / f"func_{address:08X}.c",
        asm_path(root, target, splat, address),
    )
    return tuple(path for path in paths if path.is_file())


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("role", choices=sorted(ROLE))
    parser.add_argument("function", nargs="?", type=selector, metavar="TARGET[#INDEX]@0xADDRESS")
    parser.add_argument("--root", type=Path, default=ROOT)
    args = parser.parse_args()
    root = args.root.resolve()
    paths = (*FULL, *SECTIONS, *ROLE[args.role])
    missing = [path for path in paths if not (root / path).is_file()]
    if missing:
        print(f"missing required context: {', '.join(missing)}", file=sys.stderr)
        return 2
    try:
        output = [section(root / path, root) for path in FULL]
        output.extend(
            section(root / path, root, selected_sections(root / path, headings))
            for path, headings in SECTIONS.items()
        )
        output.extend(section(root / path, root) for path in ROLE[args.role])
        if args.function:
            requested, address = args.function
            output.extend(section(path, root) for path in target_paths(root, requested, address))
    except ValueError as error:
        print(error, file=sys.stderr)
        return 2
    sys.stdout.write("".join(output).lstrip())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
