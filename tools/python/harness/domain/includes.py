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
) -> list[Path]:
    """Follow literal includes once per file, retaining repository containment.

    Defaults preserve quoted local/root/include lookup. Explicit include roots
    and angle lookup support one traversal across compiler search directories;
    angle includes do not search the including file's directory.
    """
    roots = (root, root / "include") if include_roots is None else include_roots
    found: list[Path] = []
    pending = list(seeds)
    seen = set(pending)
    while pending:
        path = pending.pop()
        if not path.is_file():
            continue
        try:
            names = _LOCAL_INCLUDE_RE.findall(
                _include_text(path.read_text(encoding="utf-8"))
            )
        except (OSError, UnicodeDecodeError):
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
                    candidate.resolve()
                    for candidate in candidates
                    if candidate.is_file()
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
