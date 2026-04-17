from __future__ import annotations

import re

from .type_constants import IDENTIFIER_RE, IGNORED_TYPE_TOKENS, TRAILING_ARRAY_SUFFIX_RE


def contains_unsupported_signature_shape(type_spec: str) -> bool:
    text = str(type_spec or "").strip()
    if not text or "(" not in text or ")" not in text:
        return False
    return "(*" in text or ")(" in text


def token_type_names(text: str) -> tuple[str, ...]:
    candidates = IDENTIFIER_RE.findall(text)
    names: list[str] = []
    for candidate in candidates:
        if candidate in IGNORED_TYPE_TOKENS:
            continue
        if candidate.startswith("FUN_"):
            continue
        if candidate not in names:
            names.append(candidate)
    return tuple(names)


def split_trailing_array_suffix(text: str) -> tuple[str, str]:
    match = TRAILING_ARRAY_SUFFIX_RE.search(text)
    if match is None:
        return text, ""
    return text[: match.start()].rstrip(), match.group(1).strip()


def strip_declarator_name(text: str) -> str:
    normalized = str(text or "").strip()
    if not normalized:
        return normalized
    base, suffix = split_trailing_array_suffix(normalized)
    matches = list(IDENTIFIER_RE.finditer(base))
    if not matches:
        return normalized
    candidate_match = matches[-1]
    prefix = base[: candidate_match.start()].rstrip()
    if not prefix:
        return normalized
    significant_prefix = [
        token
        for token in IDENTIFIER_RE.findall(prefix)
        if token not in IGNORED_TYPE_TOKENS
    ]
    if not significant_prefix and "*" not in prefix:
        return normalized
    cleaned = prefix.strip()
    if suffix:
        cleaned = f"{cleaned}{suffix}"
    return cleaned.strip()


def strip_function_name(head: str) -> str:
    text = str(head or "").strip()
    matches = list(IDENTIFIER_RE.finditer(text))
    if not matches:
        return text
    candidate_match = matches[-1]
    prefix = text[: candidate_match.start()].rstrip()
    if not prefix:
        return text
    return prefix.strip()


def split_parameter_specs(params_text: str) -> tuple[str, ...]:
    parts: list[str] = []
    buffer: list[str] = []
    depth = 0
    for char in str(params_text or ""):
        if char == "(":
            depth += 1
        elif char == ")" and depth > 0:
            depth -= 1
        elif char == "," and depth == 0:
            part = "".join(buffer).strip()
            if part:
                parts.append(part)
            buffer = []
            continue
        buffer.append(char)
    trailing = "".join(buffer).strip()
    if trailing:
        parts.append(trailing)
    return tuple(parts)


def strip_parameter_name(text: str) -> str:
    normalized = str(text or "").strip()
    if not normalized:
        return normalized
    if "(*" in normalized:
        return re.sub(r"\(\s*\*\s*[A-Za-z_][A-Za-z0-9_:]*\s*\)", "(*)", normalized)
    stripped = strip_declarator_name(normalized)
    if stripped != normalized:
        return stripped
    tokens = list(IDENTIFIER_RE.finditer(normalized))
    if len(tokens) < 2:
        return normalized
    last = tokens[-1]
    prefix = normalized[: last.start()].rstrip()
    if not prefix:
        return normalized
    return prefix


def referenced_function_type_names(text: str) -> tuple[str, ...]:
    open_idx = text.find("(")
    close_idx = text.rfind(")")
    if open_idx <= 0 or close_idx <= open_idx:
        return token_type_names(strip_declarator_name(text))
    head = text[:open_idx].strip()
    params_text = text[open_idx + 1 : close_idx].strip()
    names: list[str] = []
    for part in (strip_function_name(head),):
        for candidate in token_type_names(part):
            if candidate not in names:
                names.append(candidate)
    for param in split_parameter_specs(params_text):
        if param in {"void", "..."}:
            continue
        for candidate in token_type_names(strip_parameter_name(param)):
            if candidate not in names:
                names.append(candidate)
    return tuple(names)


def referenced_type_names(
    type_spec: object, *, kind: str | None = None
) -> tuple[str, ...]:
    text = str(type_spec or "").strip()
    if not text:
        return ()
    if kind == "data":
        return token_type_names(strip_declarator_name(text))
    if kind == "function":
        return referenced_function_type_names(text)
    return token_type_names(text)
