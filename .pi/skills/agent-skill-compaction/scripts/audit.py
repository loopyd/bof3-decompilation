#!/usr/bin/env python3
"""Audit project agent/skill Markdown structure and compaction deltas."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from urllib.parse import unquote

import yaml

ROOT = Path(__file__).resolve().parents[4]
DEFAULT_ROOTS = (ROOT / ".pi/agents", ROOT / ".pi/skills")
INLINE_LINK_RE = re.compile(r"(?<!!)\[[^]]*\]\(([^)]+)\)")
REFERENCE_USE_RE = re.compile(r"(?<!!)\[[^]]+\]\[([^]]+)\]")
REFERENCE_DEF_RE = re.compile(r"^\s*\[([^]]+)\]:\s*(\S+)", re.MULTILINE)
FENCE_RE = re.compile(r"^\s*(`{3,}|~{3,})", re.MULTILINE)


def discover(paths: list[str]) -> tuple[list[Path], list[str]]:
    roots = [Path(p).resolve() for p in paths] if paths else list(DEFAULT_ROOTS)
    found: set[Path] = set()
    errors: list[str] = []
    for root in roots:
        if not root.exists():
            errors.append(f"scope does not exist: {root}")
        elif root.is_file():
            if root.suffix != ".md":
                errors.append(f"scope is not Markdown: {root}")
            else:
                found.add(root)
        elif root.is_dir():
            found.update(root.rglob("*.md"))
        else:
            errors.append(f"unsupported scope: {root}")
    if not found:
        errors.append("scope contains no Markdown files")
    return sorted(found), errors


def front_matter(path: Path, text: str) -> dict | None:
    if not text.startswith("---\n"):
        return None
    match = re.match(r"\A---\n(.*?)\n---(?:\n|\Z)", text, re.DOTALL)
    if not match:
        raise ValueError(f"{path}: unterminated front matter")
    data = yaml.safe_load(match.group(1))
    if not isinstance(data, dict):
        raise ValueError(f"{path}: front matter is not a mapping")
    return data


def anchors(text: str) -> set[str]:
    result = set()
    for line in text.splitlines():
        if not line.startswith("#"):
            continue
        name = line.lstrip("#").strip().lower()
        name = re.sub(r"[^a-z0-9 _-]", "", name).replace(" ", "-")
        name = re.sub(r"-+", "-", name).strip("-")
        if name:
            result.add(name)
    return result


def validate_target(path: Path, target: str, errors: list[str]) -> None:
    target = target.split(maxsplit=1)[0].strip("<>")
    if not target or target.startswith(("http://", "https://", "mailto:")):
        return
    if target.startswith("#"):
        file_part, anchor = "", target[1:]
    else:
        file_part, _, anchor = target.partition("#")
    dest = path if not file_part else (path.parent / unquote(file_part)).resolve()
    if not dest.exists():
        errors.append(f"{path}: broken link {target}")
    elif anchor and anchor.lower() not in anchors(dest.read_text(encoding="utf-8")):
        errors.append(f"{path}: missing anchor {target}")


def inspect(path: Path) -> tuple[dict, list[str]]:
    text = path.read_text(encoding="utf-8")
    errors: list[str] = []
    try:
        fm = front_matter(path, text)
    except (ValueError, yaml.YAMLError) as exc:
        fm = None
        errors.append(str(exc))
    if path.parent.name == "agents" and fm is None:
        errors.append(f"{path}: agent front matter missing")
    if path.name == "SKILL.md" and (fm is None or not fm.get("name") or not fm.get("description")):
        errors.append(f"{path}: skill name/description missing")

    stack: list[str] = []
    for fence in FENCE_RE.findall(text):
        marker = fence[0]
        if not stack:
            stack.append(fence)
        elif stack[-1][0] == marker and len(fence) >= len(stack[-1]):
            stack.pop()
    if stack:
        errors.append(f"{path}: unbalanced code fences")

    for target in INLINE_LINK_RE.findall(text):
        validate_target(path, target, errors)
    definitions = {key.lower(): value for key, value in REFERENCE_DEF_RE.findall(text)}
    for key in REFERENCE_USE_RE.findall(text):
        target = definitions.get(key.lower())
        if target is None:
            errors.append(f"{path}: missing reference link [{key}]")
        else:
            validate_target(path, target, errors)

    rel = str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)
    return {
        "path": rel,
        "words": len(text.split()),
        "bytes": len(text.encode()),
        "lines": len(text.splitlines()),
        "frontMatter": fm,
    }, errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("paths", nargs="*")
    parser.add_argument("--output")
    parser.add_argument("--baseline")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    scoped, errors = discover(args.paths)
    rows = []
    for path in scoped:
        row, found = inspect(path)
        rows.append(row)
        errors.extend(found)

    report: dict = {
        "files": rows,
        "totals": {
            "files": len(rows),
            "words": sum(r["words"] for r in rows),
            "bytes": sum(r["bytes"] for r in rows),
            "lines": sum(r["lines"] for r in rows),
        },
        "errors": errors,
    }

    if args.baseline:
        before = json.loads(Path(args.baseline).read_text(encoding="utf-8"))
        old = {r["path"]: r for r in before["files"]}
        current = {r["path"]: r for r in rows}
        for path in sorted(set(old) - set(current)):
            errors.append(f"missing scoped file: {path}")
        for path in sorted(set(current) - set(old)):
            errors.append(f"file absent from baseline: {path}")
        report["delta"] = {}
        for path in sorted(set(old) & set(current)):
            prior, row = old[path], current[path]
            report["delta"][path] = {
                "words": row["words"] - prior["words"],
                "bytes": row["bytes"] - prior["bytes"],
                "lines": row["lines"] - prior["lines"],
            }
            if prior["frontMatter"] != row["frontMatter"]:
                errors.append(f"{path}: front matter changed")

    output = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        Path(args.output).write_text(output, encoding="utf-8")
    else:
        print(output, end="")
    if args.check and errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
