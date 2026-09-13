"""Bounded traversal of repository-local literal include dependencies."""

from __future__ import annotations

import re
from pathlib import Path

_LOCAL_INCLUDE_RE = re.compile(
    r'^\s*#\s*include\s*(?:"([^"]+)"|<([^>]+)>)', re.MULTILINE
)
_COMMENTS_AND_LITERALS = re.compile(
    r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*[\s\S]*?\*/'
)


def _include_text(text: str) -> str:
    text = re.sub(r"\\\r?\n", "", text)
    return _COMMENTS_AND_LITERALS.sub(
        lambda match: (
            " " + "\n" * match.group().count("\n")
            if match.group().startswith(("//", "/*"))
            else match.group()
        ),
        text,
    )


def local_include_files(
    root: Path,
    seeds: list[Path],
    *,
    include_roots: tuple[Path, ...] | None = None,
    include_angle: bool = False,
    replacements: dict[Path, str] | None = None,
    strict: bool = False,
    require_literal: bool = False,
) -> list[Path]:
    """Follow literal includes once per file, retaining repository containment.

    Defaults preserve quoted local/root/include lookup. Explicit include roots
    and angle lookup support one traversal across compiler search directories;
    angle includes do not search the including file's directory.
    """
    roots = (root, root / "include") if include_roots is None else include_roots
    replacements = {path.resolve(): text for path, text in (replacements or {}).items()}
    found: list[Path] = []
    pending = list(seeds)
    seen = set(pending)
    while pending:
        path = pending.pop()
        if path not in replacements and not path.is_file():
            continue
        try:
            text = _include_text(
                replacements[path]
                if path in replacements
                else path.read_text(encoding="utf-8")
            )
            if require_literal:
                for directive in re.finditer(
                    r"^\s*#\s*include\b[^\n]*", text, re.MULTILINE
                ):
                    if _LOCAL_INCLUDE_RE.fullmatch(directive.group().rstrip()) is None:
                        raise ValueError(f"unresolved nonliteral include in {path}")
            names = _LOCAL_INCLUDE_RE.findall(text)
        except (OSError, UnicodeDecodeError):
            if strict:
                raise
            continue
        for quoted, angle in names:
            if not quoted and not include_angle:
                continue
            name = quoted or angle
            candidates = ([path.parent / name] if quoted else []) + [
                base / name for base in roots
            ]
            resolved = next(
                (
                    candidate
                    for candidate in (path.resolve() for path in candidates)
                    if candidate in replacements or candidate.is_file()
                ),
                None,
            )
            if (
                resolved is not None
                and root in resolved.parents
                and resolved not in seen
            ):
                seen.add(resolved)
                found.append(resolved)
                pending.append(resolved)
    return found
