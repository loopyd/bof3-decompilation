from __future__ import annotations

import re

from ....models.metadata import MetadataTypeNormalization
from .type_constants import (
    ARRAY_PREFIX_RE,
    CALLING_CONVENTION_TOKENS,
    C_TYPE_ALIAS_REWRITES,
    FUNCTION_POINTER_PARAM_RE,
    PSEUDO_TYPES,
)


def normalize_type_spec(type_spec: object, *, kind: str) -> MetadataTypeNormalization:
    original = None if type_spec is None else str(type_spec).strip()
    if not original:
        return MetadataTypeNormalization(
            original=original,
            normalized=None,
            status="missing",
            reason="row has no type_spec",
        )
    lowered = original.lower()
    if lowered in PSEUDO_TYPES:
        return MetadataTypeNormalization(
            original=original,
            normalized=None,
            status="pseudo_type",
            reason="workflow pseudo-type should not be parsed by Ghidra",
            is_pseudo_type=True,
        )
    normalized = original
    reasons: list[str] = []
    array_match = ARRAY_PREFIX_RE.match(normalized)
    if array_match is not None:
        array_spec, base_type = array_match.groups()
        rewritten = f"{base_type.strip()}{array_spec.strip()}"
        if rewritten != normalized:
            normalized = rewritten
            reasons.append("rewrote_prefix_array")
    for token in CALLING_CONVENTION_TOKENS:
        stripped = re.sub(rf"\b{re.escape(token)}\b", " ", normalized)
        if stripped != normalized:
            normalized = stripped
            reasons.append(f"stripped_{token}")
    for pattern, replacement in C_TYPE_ALIAS_REWRITES:
        replaced = pattern.sub(replacement, normalized)
        if replaced != normalized:
            normalized = replaced
            reasons.append(f"rewrote_{replacement.replace(' ', '_')}")
    normalized = re.sub(r"\s+", " ", normalized).strip()
    if normalized == original:
        return MetadataTypeNormalization(
            original=original,
            normalized=normalized,
            status="direct",
        )
    return MetadataTypeNormalization(
        original=original,
        normalized=normalized,
        status="normalized",
        reason=",".join(dict.fromkeys(reasons)) or f"normalized_{kind}",
    )


def rewrite_function_pointer_signature(
    type_spec: object,
) -> tuple[str, tuple[dict[str, str], ...]]:
    text = str(type_spec or "").strip()
    if not text:
        return text, ()
    typedefs: list[dict[str, str]] = []

    def replace(match: re.Match[str]) -> str:
        return_type = re.sub(r"\s+", " ", match.group("return_type").strip())
        param_name = match.group("name").strip()
        params = re.sub(r"\s+", " ", match.group("params").strip())
        typedef_name = param_name[0].upper() + param_name[1:]
        if not typedef_name.endswith("Callback"):
            typedef_name = f"{typedef_name}Callback"
        target_type = f"{return_type} (*)({params})"
        typedefs.append(
            {
                "name": typedef_name,
                "target_type": target_type,
                "parameter_name": param_name,
                "original": match.group("full").strip(),
            }
        )
        return f"{typedef_name} {param_name}"

    rewritten = FUNCTION_POINTER_PARAM_RE.sub(replace, text)
    return rewritten, tuple(typedefs)
