"""Lift/symbol metadata tag parsers: the single authority for @source/@behavior.

Comment-syntax agnostic (`/* */` or `//`); hex is case-insensitive and the
`0x` prefix optional (legacy forms accepted, tree writes `0x` uppercase).
"""

from __future__ import annotations

import re

SOURCE_TAG_RE = re.compile(r"@source\s+(?:0x)?([0-9A-Fa-f]{8})\b")
BEHAVIOR_TAG_RE = re.compile(r"@behavior (?:UNKNOWN: .+|[^\n]+)")

# Raw address-encoding symbol names; conflicts resolve by a different name or
# a suffix, never an overlay-name prefix (`SCENA16_D_*` ban).
RAW_SYMBOL_NAME_RE = re.compile(r"^(?:func|D|T)_[0-9A-Fa-f]{8}$")
PREFIXED_RAW_NAME_RE = re.compile(r"(?:^|_)(?:func|D)_[0-9A-Fa-f]{8}\b")


def parse_source_tag(text: str) -> int | None:
    """Return the lift file's function address from its @source tag, or None.

    A tag in the same comment block as a `@behavior` identifies the file's
    function and wins immediately. Tags attached to data declarations
    (trailing on, or directly above, an extern/#define) record a symbol's
    origin address without identifying the function and are skipped.
    """

    fallback = None
    for match in SOURCE_TAG_RE.finditer(text):
        start = text.rfind("/*", 0, match.start())
        prev_end = text.rfind("*/", 0, match.start())
        end = text.find("*/", match.end())
        block = (
            text[start : end + 2]
            if start != -1 and end != -1 and prev_end < start
            else ""
        )
        if "@behavior" in block:
            return int(match.group(1), 16)
        line_start = text.rfind("\n", 0, match.start()) + 1
        before = text[line_start : match.start()]
        if "extern" in before or before.lstrip().startswith("#define"):
            continue  # trailing tag on a data declaration
        rest = text[end + 2 :].lstrip() if end != -1 else ""
        if rest.startswith(("extern", "#define", "typedef")):
            continue  # leading tag on a data declaration
        if fallback is None:
            fallback = int(match.group(1), 16)
    return fallback


def parse_behavior_tag(text: str) -> str | None:
    """Return the @behavior tag text, or None when absent."""

    match = BEHAVIOR_TAG_RE.search(text)
    return match.group(0) if match is not None else None


def parse_declaration_source_tag(text: str, name: str) -> int | None:
    """Return the @source address attached to `name`'s declaration, or None.

    Considers declaration lines (`extern ... name ...;`, `#define name`, or
    a function declaration/definition `type name(...)`). The tag is accepted
    trailing on the declaration line or in the comment block directly above
    it (blank/comment lines only between tag and declaration; the nearest
    tag wins).
    """

    for match in re.finditer(rf"\b{re.escape(name)}\b", text):
        line_start = text.rfind("\n", 0, match.start()) + 1
        line_end = text.find("\n", match.end())
        line = text[line_start : line_end if line_end != -1 else len(text)]
        stripped = line.lstrip()
        after_name = line[match.end() - line_start :].lstrip()
        is_decl = (
            "extern" in line
            or stripped.startswith("#define")
            or after_name.startswith("(")
        )
        if not is_decl:
            continue
        trailing = SOURCE_TAG_RE.search(line, match.end() - line_start)
        if trailing is not None:
            return int(trailing.group(1), 16)
        tags: list[str] = []
        pos = line_start
        while pos > 0:
            prev_end = pos - 1
            prev_start = text.rfind("\n", 0, prev_end) + 1
            prev = text[prev_start:prev_end].strip()
            comment = (
                prev == ""
                or prev.startswith(("/*", "*", "//"))
                or prev.endswith("*/")
            )
            if not comment:
                break
            tags = SOURCE_TAG_RE.findall(prev) + tags
            pos = prev_start
        if tags:
            return int(tags[-1], 16)
    return None


__all__ = [
    "BEHAVIOR_TAG_RE",
    "PREFIXED_RAW_NAME_RE",
    "RAW_SYMBOL_NAME_RE",
    "SOURCE_TAG_RE",
    "parse_behavior_tag",
    "parse_declaration_source_tag",
    "parse_source_tag",
]
