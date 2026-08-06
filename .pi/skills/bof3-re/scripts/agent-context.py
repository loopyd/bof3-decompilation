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
    "SOUL.md",
    "AGENTS.md",
    "docs/agents/CODING_STANDARDS.md",
    ".pi/skills/bof3-re/SKILL.md",
    "docs/agents/memory-api.md",
    "docs/agents/matching.md",
    "docs/agents/matching-playbook.md",
    "docs/agents/project-context.md",
    "docs/agents/plan-authoring.md",
    "docs/agents/lessons.md",
)
ROLE = {
    "agents": (),
    "reverse": (".pi/skills/bof3-re/references/REVERSE/MISSION_PROTOCOL.md",),
    "review": (
        ".pi/skills/bof3-re/references/REVIEW/REVIEW_CHECKLIST.md",
        ".pi/skills/bof3-re/references/REVIEW/SHARING_NONMATCHES.md",
    ),
    "cleanup": (
        ".pi/skills/bof3-re/references/CLEANUP/RULES.md",
        ".pi/skills/bof3-re/references/CLEANUP/REFACTOR_PLAYBOOK.md",
    ),
}
# Workflow agents get bounded, role-targeted context instead of FULL: Qwen
# models have a very limited context window, so their roles stay tiny.
WORKFLOW = {
    "classifier": (),  # wording-only classification; must not inspect files
    "context-builder": ("AGENTS.md", "docs/agents/project-context.md"),
    "oracle": ("AGENTS.md", "docs/agents/plan-authoring.md"),
    "planner": (
        "AGENTS.md",
        "docs/agents/project-context.md",
        "docs/agents/plan-authoring.md",
    ),
    "researcher": ("docs/agents/project-context.md",),
    "reviewer": ("AGENTS.md", "docs/agents/plan-authoring.md"),
    "scout": ("docs/agents/project-context.md",),
    "worker": (
        "AGENTS.md",
        "docs/agents/CODING_STANDARDS.md",
        "docs/agents/project-context.md",
    ),
}
IDENTIFIER = re.compile(r"\b(?:D|func)_[0-9A-Fa-f]{8}\b")


def knowledge_paths(root: Path) -> tuple[str, ...]:
    """Return the stable common context; specs stay targeted reads."""
    del root
    return FULL


def roster(root: Path) -> str:
    """Summarize subagent definitions and skills for the core agents mode."""
    lines = ["\n===== subagent roster (.pi/agents) =====\n"]
    for path in sorted((root / ".pi" / "agents").glob("*.md")):
        front = path.read_text(encoding="utf-8").split("---")[1]
        fields = dict(
            line.split(": ", 1) for line in front.splitlines() if ": " in line
        )
        lines.append(f"{fields.get('name', path.stem)}: {fields.get('description', '')}\n")
    skills = sorted(p.parent.name for p in (root / ".pi" / "skills").glob("*/SKILL.md"))
    lines.append("\n===== skills (.pi/skills) =====\n" + "\n".join(skills) + "\n")
    return "".join(lines)


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


def unlabeled_refs(root: Path, target: str, address: int, manifest) -> str:
    """This function's unlabeled data references plus the target's hot gaps."""

    index = root / "out" / "index" / "reverse.sqlite"
    binary = root / manifest.binary
    if not index.is_file() or not binary.is_file():
        return ""
    import sqlite3

    load = manifest.load_address
    end = load + binary.stat().st_size
    try:
        connection = sqlite3.connect(index)
        try:
            own = [
                f"0x{row[0]:08X}"
                for row in connection.execute(
                    "SELECT address FROM data_references "
                    "WHERE function_id = ? AND symbol IS NULL ORDER BY address",
                    (f"{target}@{address:08x}",),
                )
                if load <= row[0] < end
            ]
            hot = [
                (row[0], row[1])
                for row in connection.execute(
                    "SELECT address, COUNT(*) FROM data_references "
                    "WHERE target_id = ? AND symbol IS NULL "
                    "GROUP BY address ORDER BY 2 DESC LIMIT 8",
                    (target,),
                )
                if load <= row[0] < end
            ]
        finally:
            connection.close()
    except sqlite3.DatabaseError:
        return ""
    lines = ["unlabeled data references (label in the target map when proven):"]
    if own:
        lines.append("this function: " + " ".join(own))
    if hot:
        lines.append(
            "target hot gaps: "
            + " ".join(f"0x{address:08X}({refs})" for address, refs in hot)
        )
    return "\n".join(lines)


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
    sections = [section(path, root, text) for path, text in paths]
    refs = unlabeled_refs(root, target, address, manifest)
    if refs:
        sections.append(f"===== data-scan: {target} =====\n{refs}\n")
    return sections


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "role", nargs="?", default="agents", choices=sorted((*ROLE, *WORKFLOW))
    )
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
    if args.role in WORKFLOW:
        paths = WORKFLOW[args.role]
        missing = [path for path in paths if not (root / path).is_file()]
        if missing:
            print(
                f"missing required context: {', '.join(missing)}", file=sys.stderr
            )
            return 2
        if not paths:
            sys.stdout.write(f"role {args.role}: no repository context required\n")
            return 0
        sys.stdout.write(
            "".join(section(root / path, root) for path in paths).lstrip()
        )
        return 0
    common = knowledge_paths(root)
    paths = (*common, *ROLE[args.role])
    missing = [path for path in paths if not (root / path).is_file()]
    if missing:
        print(f"missing required context: {', '.join(missing)}", file=sys.stderr)
        return 2
    try:
        output = [section(root / path, root) for path in common]
        output.extend(section(root / path, root) for path in ROLE[args.role])
        if args.role == "agents":
            output.append(roster(root))
        if args.function:
            output.extend(target_context(root, *args.function))
    except ValueError as error:
        print(error, file=sys.stderr)
        return 2
    sys.stdout.write("".join(output).lstrip())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
