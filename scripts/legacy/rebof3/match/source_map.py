from __future__ import annotations

import re
from pathlib import Path
from typing import Any

from ..common import ROOT, format_hex, parse_hexish, relative_to_root


DEFAULT_SOURCE_ROOT = ROOT / "bof3"
DEFAULT_BUILD_ROOT = ROOT / "build"
SOURCE_TAG_RE = re.compile(
    r"@source:\s*0x(?P<addr>[0-9a-fA-F]{8})\b(?:\s+(?P<label>\S+))?"
)
ADDRESS_NAMED_FUNCTION_RE = re.compile(
    r"^[ \t]*(?!if\b|while\b|for\b|switch\b|return\b|case\b)(?P<signature>(?:[A-Za-z_][A-Za-z0-9_\s\*\(\),\[\]]*?\b)?(?P<name>(?:func|FUN)_(?P<addr>[0-9a-fA-F]{8}))\s*\([^;{}]*\))\s*\{",
    re.MULTILINE,
)
COMMENT_BLOCK_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
FUNCTION_NAME_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(")
PROFILE_PATH_PRIORITIES = (("bof3-psyq40", 0),)
OBJECT_TARGET_DIR_NAMES = ("bof3.dir", "psxbof3.dir")
FUNCTION_SOURCE_LABEL_RE = re.compile(
    r"^(?:(?:FUN|func)_[0-9a-fA-F]{8}|thunk_(?:FUN|func)_[0-9a-fA-F]{8})$"
)
DISABLED_STUB_MARKER = "@stub-disabled"


def profile_priority(path: Path | str) -> tuple[int, str]:
    lowered = str(path).lower()
    for marker, priority in PROFILE_PATH_PRIORITIES:
        if marker in lowered:
            return priority, lowered
    return len(PROFILE_PATH_PRIORITIES), lowered


def object_roots(build_root: Path) -> list[Path]:
    if build_root.name in OBJECT_TARGET_DIR_NAMES:
        return [build_root]

    direct_root = build_root / "bof3" / "CMakeFiles" / OBJECT_TARGET_DIR_NAMES[0]
    discovered = sorted(
        (
            path
            for target_dir in OBJECT_TARGET_DIR_NAMES
            for path in build_root.rglob(target_dir)
            if path.is_dir() and "cmakefiles" in path.as_posix().lower()
        ),
        key=profile_priority,
    )
    if discovered:
        return discovered
    return [direct_root]


def predict_object_candidates(
    source_file: str, *, build_root: Path = DEFAULT_BUILD_ROOT
) -> list[str]:
    rel = Path(source_file)
    discovered: list[str] = []
    if build_root.exists():
        candidates: list[tuple[tuple[int, int, str, str], str]] = []
        source_parts = [part.lower() for part in rel.parts]
        for suffix in (".obj", ".o"):
            for path in build_root.rglob(rel.name + suffix):
                lowered = path.as_posix().lower()
                score = sum(1 for part in source_parts if part in lowered)
                priority, priority_key = profile_priority(path)
                candidates.append(
                    (
                        (
                            -score,
                            priority,
                            priority_key,
                            relative_to_root(path),
                        ),
                        relative_to_root(path),
                    )
                )
        if candidates:
            discovered = [path for _, path in sorted(candidates)]

    object_rel = Path(rel.name)
    if rel.parts[:2] == ("bof3", "src"):
        object_rel = rel.relative_to("bof3")

    candidates = list(discovered)
    for object_root in object_roots(build_root):
        for suffix in (".obj", ".o"):
            candidate = relative_to_root(object_root / object_rel) + suffix
            if candidate not in candidates:
                candidates.append(candidate)
    return candidates


def find_function_signature_span(text: str, start: int) -> tuple[int, int] | None:
    index = start
    while index < len(text) and text[index].isspace():
        index += 1

    semicolon_index = text.find(";", index)
    brace_index = text.find("{", index)
    if brace_index == -1:
        return None
    if semicolon_index != -1 and semicolon_index < brace_index:
        return None

    signature_text = text[index:brace_index]
    if "=" in signature_text:
        return None
    if "(" not in signature_text or ")" not in signature_text:
        return None
    return index, brace_index


def find_matching_brace(text: str, open_index: int) -> int | None:
    depth = 0
    for index in range(open_index, len(text)):
        char = text[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index
    return None


def parse_function_name(signature_text: str) -> str | None:
    matches = FUNCTION_NAME_RE.findall(signature_text)
    if not matches:
        return None
    return matches[-1]


def dedupe_mappings(mappings: list[dict[str, Any]]) -> list[dict[str, Any]]:
    deduped: list[dict[str, Any]] = []
    seen: set[tuple[str, str, str, int]] = set()
    for mapping in mappings:
        key = (
            str(mapping.get("entry_hex") or ""),
            str(mapping.get("source_file") or ""),
            str(mapping.get("source_function") or ""),
            int(mapping.get("source_line") or 0),
        )
        if key in seen:
            continue
        seen.add(key)
        deduped.append(mapping)
    return deduped


def is_function_source_label(label: str | None) -> bool:
    if label is None:
        return True
    stripped = label.strip()
    if not stripped:
        return True
    return FUNCTION_SOURCE_LABEL_RE.match(stripped) is not None


def extract_comment_tagged_functions_from_text(
    text: str, *, file_path: str
) -> list[dict[str, Any]]:
    mappings: list[dict[str, Any]] = []
    for match in COMMENT_BLOCK_RE.finditer(text):
        comment_text = match.group(0)
        source_tags = list(SOURCE_TAG_RE.finditer(comment_text))
        if not source_tags:
            continue
        signature_span = find_function_signature_span(text, match.end())
        if signature_span is None:
            continue
        signature_start, brace_index = signature_span
        end_brace_index = find_matching_brace(text, brace_index)
        if end_brace_index is None:
            continue

        signature_text = text[signature_start:brace_index].strip()
        function_name = parse_function_name(signature_text)
        if function_name is None:
            continue

        function_text = text[signature_start : end_brace_index + 1]
        line = text.count("\n", 0, signature_start) + 1
        for source_tag in source_tags:
            if not is_function_source_label(source_tag.group("label")):
                continue
            entry_hex = "0x" + source_tag.group("addr").lower()
            mappings.append(
                {
                    "entry_hex": entry_hex,
                    "source_file": file_path,
                    "source_line": line,
                    "source_function": function_name,
                    "source_signature": signature_text,
                    "source_text": function_text.strip() + "\n",
                    "source_label": source_tag.group("label"),
                }
            )
    return mappings


def extract_address_named_functions_from_text(
    text: str, *, file_path: str
) -> list[dict[str, Any]]:
    mappings: list[dict[str, Any]] = []
    for match in ADDRESS_NAMED_FUNCTION_RE.finditer(text):
        brace_index = match.end() - 1
        end_brace_index = find_matching_brace(text, brace_index)
        if end_brace_index is None:
            continue
        signature_start = match.start("signature")
        signature_text = str(match.group("signature") or "").strip()
        function_name = str(match.group("name") or "")
        function_text = text[signature_start : end_brace_index + 1]
        line = text.count("\n", 0, signature_start) + 1
        mappings.append(
            {
                "entry_hex": "0x" + str(match.group("addr") or "").lower(),
                "source_file": file_path,
                "source_line": line,
                "source_function": function_name,
                "source_signature": signature_text,
                "source_text": function_text.strip() + "\n",
                "source_label": function_name,
            }
        )
    return mappings


def extract_tagged_functions_from_text(
    text: str, *, file_path: str
) -> list[dict[str, Any]]:
    return dedupe_mappings(
        extract_address_named_functions_from_text(text, file_path=file_path)
        + extract_comment_tagged_functions_from_text(text, file_path=file_path)
    )


def collect_source_mappings(
    source_root: Path = DEFAULT_SOURCE_ROOT,
) -> list[dict[str, Any]]:
    mappings: list[dict[str, Any]] = []
    for path in sorted(source_root.rglob("*.c")):
        rel_path = relative_to_root(path)
        if rel_path.startswith("bof3/stubs/"):
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        if DISABLED_STUB_MARKER in text:
            continue
        mappings.extend(
            extract_tagged_functions_from_text(
                text,
                file_path=rel_path,
            )
        )
    return mappings


def mapping_score(
    mapping: dict[str, Any],
    *,
    program_path: str | None = None,
    program_name: str | None = None,
    source_hint: str | None = None,
) -> tuple[int, int, str]:
    source_file = str(mapping.get("source_file") or "")
    file_name = Path(source_file).name.lower()
    stem = Path(file_name).stem.lower()
    score = 0
    if source_hint:
        hint_stem = Path(str(source_hint)).stem.lower()
        if hint_stem and stem == hint_stem:
            score += 100
        if hint_stem and hint_stem in source_file.lower():
            score += 40
    if program_name:
        normalized_program = Path(str(program_name)).stem.lower()
        if normalized_program and stem == normalized_program:
            score += 80
        if normalized_program and normalized_program in source_file.lower():
            score += 30
    if (
        program_path
        and "logo" in str(program_path).lower()
        and "logo" in source_file.lower()
    ):
        score += 20
    if "/modules/" in source_file.lower():
        score += 5
    return score, -int(mapping.get("source_line") or 0), source_file


def find_source_mappings(
    entry: str, source_root: Path = DEFAULT_SOURCE_ROOT
) -> list[dict[str, Any]]:
    entry_hex = format_hex(parse_hexish(entry))
    matches: list[dict[str, Any]] = []
    for mapping in collect_source_mappings(source_root):
        if mapping["entry_hex"] != entry_hex:
            continue
        enriched = dict(mapping)
        enriched["object_candidates"] = predict_object_candidates(
            mapping["source_file"]
        )
        matches.append(enriched)
    return matches


def find_source_mapping(
    entry: str,
    source_root: Path = DEFAULT_SOURCE_ROOT,
    *,
    program_path: str | None = None,
    program_name: str | None = None,
    source_hint: str | None = None,
) -> dict[str, Any] | None:
    matches = find_source_mappings(entry, source_root)
    if not matches:
        return None
    return max(
        matches,
        key=lambda mapping: mapping_score(
            mapping,
            program_path=program_path,
            program_name=program_name,
            source_hint=source_hint,
        ),
    )
