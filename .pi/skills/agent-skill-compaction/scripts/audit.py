#!/usr/bin/env python3
"""Audit project agent/skill Markdown structure and compaction deltas."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from urllib.parse import ParseResult, unquote, urlparse

import yaml

ROOT = Path(__file__).resolve().parents[4]
DEFAULT_ROOTS = (ROOT / ".pi/agents", ROOT / ".pi/skills")
INLINE_LINK_RE = re.compile(r"(?<!!)\[[^]]*\]\(([^)]+)\)")
REFERENCE_USE_RE = re.compile(r"(?<!!)\[[^]]+\]\[([^]]+)\]")
REFERENCE_DEF_RE = re.compile(r"^\s*\[([^]]+)\]:\s*(\S+)", re.MULTILINE)
FENCE_OPEN_RE = re.compile(r"^ {0,3}(`{3,}|~{3,})(.*)$")
FRONT_MATTER_RE = re.compile(r"\A---\n(.*?)\n---(?:\n|\Z)", re.DOTALL)
SKILL_NAME_RE = re.compile(r"[a-z0-9]+(?:-[a-z0-9]+)*\Z")
TRIGGER_CLAUSE_RE = re.compile(
    r"\b(?:use|invoke|select)(?:\s+this\s+skill)?\s+(?:when|for|after|before|only\s+when)\b|\bwhen\b",
    re.IGNORECASE,
)
CAPABILITY_RE = re.compile(
    r"\b(?:audit|check|clean|compact|convert|create|dispatch|document|enforce|extract|"
    r"fix|inspect|lift|load|match|migrate|normalize|organize|prepare|promote|query|"
    r"repair|review|route|select|trace|validate|verify|reverse-engineer|"
    r"reverse-engineering)\w*\b",
    re.IGNORECASE,
)
AGENT_SKILLS_FIELDS = {
    "name",
    "description",
    "license",
    "compatibility",
    "metadata",
    "allowed-tools",
}
PI_FIELDS = {"disable-model-invocation"}
LOCAL_ABSOLUTE_RE = re.compile(r"^(?:/|[A-Za-z]:[\\/])")


def markdown_prose(text: str) -> tuple[str, list[str]]:
    """Return non-fenced Markdown and fence syntax errors."""
    prose: list[str] = []
    opening: tuple[str, int] | None = None
    invalid_closing = False
    for line in text.splitlines(keepends=True):
        match = FENCE_OPEN_RE.match(line.rstrip("\r\n"))
        if opening is None:
            if match:
                marker = match.group(1)
                opening = marker[0], len(marker)
            else:
                prose.append(line)
            continue
        if match:
            marker, suffix = match.groups()
            if marker[0] == opening[0] and len(marker) >= opening[1]:
                if suffix.strip():
                    invalid_closing = True
                else:
                    opening = None
    errors = (
        ["unbalanced or invalid closing code fence"]
        if opening or invalid_closing
        else []
    )
    return "".join(prose), errors


def has_discovery_description(description: str) -> bool:
    """Require separate, substantive capability and explicit trigger clauses."""
    clauses = [
        part.strip() for part in re.split(r"[.;!?]+", description) if part.strip()
    ]
    return any(TRIGGER_CLAUSE_RE.search(clause) for clause in clauses) and any(
        not TRIGGER_CLAUSE_RE.search(clause) and CAPABILITY_RE.search(clause)
        for clause in clauses
    )


def link_parts(target: str) -> tuple[str, ParseResult]:
    raw = target.split(maxsplit=1)[0].strip("<>")
    return raw, urlparse(raw)


def forbidden_local_link(target: str) -> bool:
    raw, parsed = link_parts(target)
    return parsed.scheme.lower() == "file" or bool(LOCAL_ABSOLUTE_RE.match(raw))


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


def discover_skills(paths: list[str]) -> tuple[list[Path], list[str]]:
    found: set[Path] = set()
    errors: list[str] = []
    for value in paths:
        path = Path(value).resolve()
        candidates = (
            [path]
            if path.is_file()
            else list(path.rglob("SKILL.md"))
            if path.is_dir()
            else []
        )
        candidates = [
            candidate for candidate in candidates if candidate.name == "SKILL.md"
        ]
        if not candidates:
            errors.append(f"strict skill scope contains no SKILL.md: {path}")
        found.update(candidates)
    if not found:
        errors.append("strict skill set contains no skills")
    return sorted(found), errors


def split_front_matter(path: Path, text: str) -> tuple[dict | None, str]:
    if not text.startswith("---\n"):
        return None, text
    match = FRONT_MATTER_RE.match(text)
    if not match:
        raise ValueError(f"{path}: unterminated front matter")
    data = yaml.safe_load(match.group(1))
    if not isinstance(data, dict):
        raise ValueError(f"{path}: front matter is not a mapping")
    return data, text[match.end() :]


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


def local_target(path: Path, target: str) -> tuple[Path, str] | None:
    target, parsed = link_parts(target)
    if not target or parsed.scheme or target.startswith("//"):
        return None
    if target.startswith("#"):
        return path, target[1:]
    file_part, _, anchor = target.partition("#")
    return (path.parent / unquote(file_part)).resolve(), anchor


def validate_target(path: Path, target: str, errors: list[str]) -> None:
    resolved = local_target(path, target)
    if resolved is None:
        return
    dest, anchor = resolved
    if not dest.exists():
        errors.append(f"{path}: broken link {target}")
    elif anchor and anchor.lower() not in anchors(dest.read_text(encoding="utf-8")):
        errors.append(f"{path}: missing anchor {target}")


def link_targets(text: str) -> list[str]:
    prose, _ = markdown_prose(text)
    targets = INLINE_LINK_RE.findall(prose)
    definitions = {key.lower(): value for key, value in REFERENCE_DEF_RE.findall(prose)}
    targets.extend(
        definitions[key.lower()]
        for key in REFERENCE_USE_RE.findall(prose)
        if key.lower() in definitions
    )
    return targets


def inspect(path: Path) -> tuple[dict, list[str]]:
    text = path.read_text(encoding="utf-8")
    errors: list[str] = []
    try:
        fm, body = split_front_matter(path, text)
    except (ValueError, yaml.YAMLError) as exc:
        fm, body = None, text
        errors.append(str(exc))
    if path.parent.name == "agents" and fm is None:
        errors.append(f"{path}: agent front matter missing")
    if path.name == "SKILL.md" and (
        fm is None or not fm.get("name") or not fm.get("description")
    ):
        errors.append(f"{path}: skill name/description missing")

    prose, fence_errors = markdown_prose(body)
    errors.extend(f"{path}: {message}" for message in fence_errors)

    for target in INLINE_LINK_RE.findall(prose):
        validate_target(path, target, errors)
    definitions = {key.lower(): value for key, value in REFERENCE_DEF_RE.findall(prose)}
    for key in REFERENCE_USE_RE.findall(prose):
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


def finding(path: Path, rule: str, source: str, message: str) -> dict[str, str]:
    rel = str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)
    return {"path": rel, "rule": rule, "source": source, "message": message}


def estimated_tokens(text: str) -> int:
    """Use the repository's dependency-free ceil(Unicode code points / 4) estimate."""
    return (len(text) + 3) // 4


def strict_findings(path: Path) -> list[dict[str, str]]:
    text = path.read_text(encoding="utf-8")
    found: list[dict[str, str]] = []

    def add(rule: str, source: str, message: str) -> None:
        found.append(finding(path, rule, source, message))

    try:
        fm, body = split_front_matter(path, text)
    except (ValueError, yaml.YAMLError) as exc:
        add("front-matter", "agent-skills", str(exc))
        return found
    if fm is None:
        add("front-matter", "agent-skills", "SKILL.md requires YAML front matter")
        return found

    name = fm.get("name")
    if not isinstance(name, str) or not SKILL_NAME_RE.fullmatch(name) or len(name) > 64:
        add(
            "name",
            "agent-skills",
            "name must be 1-64 lowercase a-z0-9-hyphen characters without edge or consecutive hyphens",
        )
    elif name != path.parent.name:
        add("directory-name", "agent-skills", "name must match the parent directory")

    description = fm.get("description")
    if not isinstance(description, str) or not 1 <= len(description) <= 1024:
        add(
            "description",
            "agent-skills",
            "description must be a non-empty string of at most 1024 characters",
        )
    elif not has_discovery_description(description):
        add(
            "description-discovery",
            "repository",
            "description must say what the skill does and when to use it",
        )

    for field in sorted(
        set(fm) - AGENT_SKILLS_FIELDS - PI_FIELDS, key=lambda value: str(value)
    ):
        add(
            "unknown-field",
            "agent-skills",
            f"unknown Agent Skills top-level field: {field}",
        )

    for field in ("license", "allowed-tools"):
        if field in fm and not isinstance(fm[field], str):
            add(f"field-{field}", "agent-skills", f"{field} must be a string")
    compatibility = fm.get("compatibility")
    if "compatibility" in fm and (
        not isinstance(compatibility, str) or len(compatibility) > 500
    ):
        add(
            "field-compatibility",
            "agent-skills",
            "compatibility must be a string of at most 500 characters",
        )
    metadata = fm.get("metadata")
    if "metadata" in fm and (
        not isinstance(metadata, dict)
        or any(
            not isinstance(key, str) or not isinstance(value, str)
            for key, value in metadata.items()
        )
    ):
        add(
            "field-metadata",
            "agent-skills",
            "metadata must map string keys to string values",
        )
    if "disable-model-invocation" in fm and not isinstance(
        fm["disable-model-invocation"], bool
    ):
        add(
            "field-disable-model-invocation",
            "pi",
            "disable-model-invocation must be a boolean",
        )

    tokens = estimated_tokens(body)
    if tokens >= 5000:
        add(
            "body-token-budget",
            "repository",
            f"body estimate must be under 5000 tokens (found {tokens})",
        )

    structural_errors: list[str] = []
    _, structural_errors = inspect(path)
    for message in structural_errors:
        add("markdown-structure", "repository", message)

    skill_root = path.parent.resolve()
    direct: set[Path] = set()
    for target in link_targets(body):
        if forbidden_local_link(target):
            add(
                "relative-link",
                "agent-skills",
                f"local file link must be relative: {target}",
            )
            continue
        resolved = local_target(path, target)
        if resolved is None:
            continue
        dest, _ = resolved
        try:
            dest.relative_to(skill_root)
        except ValueError:
            continue
        direct.add(dest)
    references = (
        sorted((skill_root / "references").rglob("*.md"))
        if (skill_root / "references").is_dir()
        else []
    )
    for reference in references:
        if reference.resolve() not in direct:
            add(
                "direct-reference",
                "repository",
                f"SKILL.md must directly link {reference.relative_to(skill_root)}",
            )
    return found


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("paths", nargs="*")
    parser.add_argument("--output")
    parser.add_argument("--baseline")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--strict-skill-set", nargs="+", metavar="PATH")
    args = parser.parse_args()

    discovery_paths = args.strict_skill_set if args.strict_skill_set else args.paths
    scoped, errors = (
        discover_skills(discovery_paths)
        if args.strict_skill_set
        else discover(discovery_paths)
    )
    rows = []
    for path in scoped:
        row, found = inspect(path)
        rows.append(row)
        errors.extend(found)

    skills = sorted(path for path in scoped if path.name == "SKILL.md")
    strict = [item for path in skills for item in strict_findings(path)]

    report: dict = {
        "files": rows,
        "totals": {
            "files": len(rows),
            "words": sum(r["words"] for r in rows),
            "bytes": sum(r["bytes"] for r in rows),
            "lines": sum(r["lines"] for r in rows),
        },
        "errors": errors,
        "strictFindings": strict,
        "strictPolicy": {
            "sources": ["agent-skills", "pi", "repository"],
            "tokenEstimator": "ceil(body Unicode code points / 4)",
            "tokenLimitExclusive": 5000,
        },
        "strictSkillSet": [str(path) for path in skills]
        if args.strict_skill_set
        else None,
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
    failures = list(errors)
    if args.strict_skill_set and strict:
        failures.extend(
            f"{item['path']}: {item['rule']}: {item['message']}" for item in strict
        )
    if args.check and failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
