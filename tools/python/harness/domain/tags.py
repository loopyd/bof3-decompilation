"""Lift/symbol metadata tag parsers: the single authority for @source/@behavior.

Comment-syntax agnostic (`/* */` or `//`); hex is case-insensitive and the
`0x` prefix optional (legacy forms accepted, tree writes `0x` uppercase).
"""

from __future__ import annotations

import re

SOURCE_TAG_RE = re.compile(r"@source\s+(?:0x)?([0-9A-Fa-f]{8})\b")
BEHAVIOR_TAG_RE = re.compile(r"@behavior (?:UNKNOWN: .+|[^\n]+)")
STATUS_TAG_RE = re.compile(r"@status\s+(exact|partial|invalid)\b")
MATCH_TAG_RE = re.compile(r"@match\s+([0-9]+(?:\.[0-9]+)?|unavailable)\b")
RESIDUAL_TAG_RE = re.compile(r"@residual\s+([^\n]+)")
KIND_TAG_RE = re.compile(r"@kind\s*:?\s*([A-Za-z_][A-Za-z0-9_]*)\b")
_TAG_OCCURRENCES = {
    "status": re.compile(r"@status\b"),
    "match": re.compile(r"@match\b"),
    "residual": re.compile(r"@residual\b"),
}
_COMMENT_RE = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)
_STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
DECLARATION_KINDS = frozenset({"bss", "data", "rodata", "string", "table", "unknown"})

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


def set_tag_line(text: str, tag: str, value: str) -> tuple[str, bool]:
    pattern = re.compile(rf"^([ \\t]*\\S*? ?@){tag}\\b[^\\r\\n]*", re.MULTILINE)
    match = pattern.search(text)
    if match is None:
        return text, False
    return pattern.sub(rf"{match.group(1)}{tag} {value}", text, count=1), True


def canonical_exact_progress(text: str) -> tuple[str, bool]:
    """Canonicalize an existing progress block without touching C syntax."""
    if not STATUS_TAG_RE.search(text):
        return text, False
    fixed, _ = set_tag_line(text, "status", "exact")
    fixed, _ = set_tag_line(fixed, "match", "100.00")
    fixed, residual = set_tag_line(fixed, "residual", "none")
    if not residual:
        match = MATCH_TAG_RE.search(fixed)
        if match:
            line_end = fixed.find("\n", match.start())
            line_end = len(fixed) if line_end < 0 else line_end
            line_start = fixed.rfind("\n", 0, match.start()) + 1
            prefix = fixed[line_start : match.start()]
            fixed = fixed[:line_end] + f"\n{prefix}@residual none" + fixed[line_end:]
    return fixed, fixed != text


def parse_progress_tags(text: str) -> tuple[str, float | None, str] | None:
    """Return validated lift status, live match percentage, and residual."""

    occurrences = {
        name: len(pattern.findall(text)) for name, pattern in _TAG_OCCURRENCES.items()
    }
    if not any(occurrences.values()):
        return None
    if any(count == 0 for count in occurrences.values()):
        raise ValueError(
            "partial progress metadata requires @status, @match, and @residual"
        )
    if any(count != 1 for count in occurrences.values()):
        raise ValueError("progress metadata tags must occur exactly once")
    statuses = STATUS_TAG_RE.findall(text)
    matches = MATCH_TAG_RE.findall(text)
    residuals = [
        value.strip().removesuffix("*/").strip()
        for value in RESIDUAL_TAG_RE.findall(text)
    ]
    if not statuses or not matches or not residuals:
        raise ValueError(
            "partial progress metadata requires @status, @match, and @residual"
        )
    if len(statuses) != 1 or len(matches) != 1 or len(residuals) != 1:
        raise ValueError("progress metadata tags must occur exactly once")
    status = statuses[0]
    value = matches[0]
    match = None if value == "unavailable" else float(value)
    residual = residuals[0]
    if match is not None and not 0.0 <= match <= 100.0:
        raise ValueError("@match must be between 0 and 100")
    if status == "exact" and (match != 100.0 or residual != "none"):
        raise ValueError("exact progress requires @match 100 and @residual none")
    if status == "partial" and (match == 100.0 or residual == "none"):
        raise ValueError("partial progress requires a non-100 match and residual")
    return status, match, residual


def lift_lifecycle(text: str | None) -> str:
    """Derive the indexed lift status from one lift's progress tags.

    Consumes :func:`parse_progress_tags` so the exact/partial/invalid
    lifecycle never diverges from the domain tag authority:

    - ``None`` text (no claimed source) is ``unlifted``;
    - incomplete progress metadata (the parser requires @status, @match,
      and @residual together) is ``invalid``;
    - a claimed lift with no progress tags at all is unproven, so it is
      ``partial``, never ``exact``;
    - ``@status exact`` counts as ``exact`` only with a 100% live match;
      an exact claim without it is ``invalid``.
    """

    if text is None:
        return "unlifted"
    try:
        progress = parse_progress_tags(text)
    except ValueError:
        return "invalid"
    if progress is None:
        return "partial"
    status, match, _residual = progress
    if status == "partial":
        return "partial"
    if status == "invalid":
        return "invalid"
    if status == "exact" and match is not None and match == 100.0:
        return "exact"
    return "invalid"


def _declaration_line(line: str, name: str) -> bool:
    match = re.search(rf"\b{re.escape(name)}\b", line)
    if match is None:
        return False
    stripped = line.lstrip()
    after_name = line[match.end() :].lstrip()
    return (
        "extern" in line
        or stripped.startswith(("#define", "typedef"))
        or after_name.startswith("(")
        or (_any_declaration_line(line) and ";" in line)
    )


def _any_declaration_line(line: str) -> bool:
    code = line.split("/*", 1)[0].split("//", 1)[0].strip()
    if not code or code.startswith("*"):
        return False
    if code.startswith(("extern ", "#define ", "typedef ")):
        return True
    if re.search(r"\b[A-Za-z_]\w*\s*\([^;{}]*\)\s*[;{]", code):
        return True
    return bool(
        re.match(
            r"(?:static\s+)?(?:const\s+|volatile\s+)*"
            r"(?:struct\s+\w+|union\s+\w+|enum\s+\w+|[A-Za-z_]\w*)"
            r"(?:\s+|\s*\*+\s*)[A-Za-z_]\w*(?:\s*\[[^]]*\])?\s*(?:=[^;]*)?;\s*$",
            code,
        )
    )


def _attached_comment_start(lines: list[str], declaration_index: int) -> int:
    start = declaration_index
    in_block = False
    for index in range(declaration_index - 1, -1, -1):
        stripped = lines[index].strip()
        if not stripped:
            break
        if in_block:
            start = index
            if "/*" in stripped:
                in_block = False
            continue
        if stripped.startswith("//"):
            start = index
            continue
        if stripped.startswith(("/*", "*")) and stripped.endswith("*/"):
            start = index
            in_block = "/*" not in stripped
            continue
        break
    return start


def _kind_tags_by_line(text: str) -> dict[int, list[str]]:
    tags: dict[int, list[str]] = {}
    for comment in _COMMENT_RE.finditer(text):
        body = comment.group(0)
        occurrences = len(re.findall(r"@kind\b", body))
        matches = KIND_TAG_RE.findall(body)
        if occurrences != len(matches):
            raise ValueError("malformed @kind tag")
        line = text.count("\n", 0, comment.start())
        for value in matches:
            value = value.lower()
            if value not in DECLARATION_KINDS:
                raise ValueError(f"unknown @kind value: {value}")
            tags.setdefault(line, []).append(value)
    code_without_comments = _STRING_RE.sub("", _COMMENT_RE.sub("", text))
    if re.search(r"@kind\b", code_without_comments):
        raise ValueError("@kind tag outside a comment")
    return tags


def parse_declaration_kind_tag(text: str, name: str) -> str | None:
    """Return the validated ``@kind`` attached to ``name``'s declaration.

    A header can contain hundreds of unrelated declarations.  Validate every
    comment tag once, but associate only the nearest uninterrupted comment
    block (plus a trailing declaration-line comment) with the requested name.
    """

    lines = text.splitlines()
    declaration_indexes = [
        index for index, line in enumerate(lines) if _declaration_line(line, name)
    ]
    tags = _kind_tags_by_line(text)
    values: list[str] = []
    for index, line in enumerate(lines):
        if not _any_declaration_line(line):
            continue
        start = _attached_comment_start(lines, index)
        candidate_lines = [
            line_index for line_index in range(start, index + 1) if tags.get(line_index)
        ]
        source_lines = [
            line_index
            for line_index in candidate_lines
            if "@source" in lines[line_index]
        ]
        if len(source_lines) > 1:
            candidate_lines = [max(source_lines), index]
        declaration_values = [
            value
            for line_index in candidate_lines
            for value in tags.get(line_index, [])
        ]
        if len(set(declaration_values)) > 1:
            raise ValueError(f"conflicting @kind tags for {name}")
        if index in declaration_indexes:
            values.extend(declaration_values)
    if not values and len(tags) == 1 and declaration_indexes:
        tag_line = next(iter(tags))
        if tag_line < declaration_indexes[0] and any(
            not line.strip() for line in lines[tag_line + 1 : declaration_indexes[0]]
        ):
            raise ValueError(f"free-floating @kind tag for {name}")
    if len(set(values)) > 1:
        raise ValueError(f"conflicting @kind tags for {name}")
    return values[0] if values else None


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
                prev == "" or prev.startswith(("/*", "*", "//")) or prev.endswith("*/")
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
    "DECLARATION_KINDS",
    "KIND_TAG_RE",
    "MATCH_TAG_RE",
    "PREFIXED_RAW_NAME_RE",
    "RAW_SYMBOL_NAME_RE",
    "RESIDUAL_TAG_RE",
    "SOURCE_TAG_RE",
    "STATUS_TAG_RE",
    "parse_behavior_tag",
    "parse_declaration_kind_tag",
    "parse_declaration_source_tag",
    "lift_lifecycle",
    "parse_progress_tags",
    "parse_source_tag",
]
