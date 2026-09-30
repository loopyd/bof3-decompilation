"""Bounded traversal of repository-local literal include dependencies."""

from __future__ import annotations

import re
from collections.abc import Callable
from pathlib import Path

from harness.common.deadlines import check_deadline

_LOCAL_INCLUDE_RE = re.compile(
    r'^\s*#\s*include\s*(?:"([^"]+)"|<([^>]+)>)', re.MULTILINE
)
_COMMENTS_AND_LITERALS = re.compile(
    r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*[\s\S]*?\*/'
)
_RESOLVE_CACHE: dict[str, Path] = {}


def _resolve(path: Path) -> Path:
    """Memoize symlink resolution; the filesystem layout is stable per command."""
    key = str(path)
    resolved = _RESOLVE_CACHE.get(key)
    if resolved is None:
        resolved = path.resolve()
        _RESOLVE_CACHE[key] = resolved
    return resolved


def _strip_comments(text: str) -> str:
    return _COMMENTS_AND_LITERALS.sub(
        lambda match: (
            " " + "\n" * match.group().count("\n")
            if match.group().startswith(("//", "/*"))
            else match.group()
        ),
        text,
    )


def _include_text(text: str) -> str:
    return _strip_comments(re.sub(r"\\\r?\n", "", text))


def parse_literal_includes(text: str, *, source: str) -> list[tuple[str, bool]]:
    """Parse conservative literal dependencies, rejecting unsupported directives."""
    check_deadline()
    if (
        "\x00" in text
        or "??" in text
        or "%:" in text
        or "\r" in text.replace("\r\n", "")
    ):
        raise ValueError(f"unsupported preprocessing spelling in {source}")
    text = re.sub(r"\\\r?\n", "", text)
    for token in _COMMENTS_AND_LITERALS.finditer(text):
        check_deadline()
        prefix = text[text.rfind("\n", 0, token.start()) + 1 : token.start()]
        if token.group().startswith(("/*", "//")) and re.fullmatch(
            r"\s*#\s*include\s*<[^>]*", _strip_comments(prefix)
        ):
            raise ValueError(f"unsupported angle include spelling in {source}")
        if token.group().startswith("/*") and "\n" in token.group():
            if "#" in prefix:
                raise ValueError(f"unsupported multiline directive comment in {source}")
    result = []
    for directive in re.finditer(r"^\s*#[^\n]*", _strip_comments(text), re.MULTILINE):
        check_deadline()
        value = directive.group().strip()
        keyword = re.match(r"#\s*([A-Za-z_][A-Za-z_0-9]*)", value)
        if keyword is None:
            if value != "#":
                raise ValueError(f"unsupported preprocessing directive in {source}")
            continue
        name = keyword.group(1)
        if name in {
            "define",
            "undef",
            "if",
            "ifdef",
            "ifndef",
            "elif",
            "else",
            "endif",
            "error",
            "line",
        }:
            continue
        match = _LOCAL_INCLUDE_RE.fullmatch(value)
        if name != "include" or match is None:
            raise ValueError(f"unsupported or nonliteral include directive in {source}")
        quoted, angle = match.groups()
        filename = quoted or angle
        if (
            not filename
            or any(character in filename for character in "\\\r\n")
            or Path(filename).as_posix() != filename
            or ".." in Path(filename).parts
            or (angle is not None and ("/*" in filename or "//" in filename))
        ):
            raise ValueError(f"unsupported include filename in {source}")
        result.append((filename, quoted is not None))
        if len(result) > 16384:
            raise ValueError(f"include directive count exceeds its bound: {source}")
    check_deadline()
    return result


def local_include_files(
    root: Path,
    seeds: list[Path],
    *,
    include_roots: tuple[Path, ...] | None = None,
    include_angle: bool = False,
    replacements: dict[Path, str] | None = None,
    strict: bool = False,
    require_literal: bool = False,
    read_source: Callable[[Path], str] | None = None,
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
                else read_source(path)
                if read_source is not None
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
                    for candidate in (_resolve(path) for path in candidates)
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
