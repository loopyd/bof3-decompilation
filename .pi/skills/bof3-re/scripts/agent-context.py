#!/usr/bin/env python3
"""Emit bounded, role-specific BOF3 context in one read-only command."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys


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


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("role", choices=sorted(ROLE))
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
    except ValueError as error:
        print(error, file=sys.stderr)
        return 2
    sys.stdout.write("".join(output).lstrip())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
