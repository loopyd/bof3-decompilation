"""Lexical argument and local-variable facts from attached function records."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Any

from harness.common.lexicon import mask_c_text
from harness.domain.functions import mask_function_attributes, parse_function_records
from harness.domain.manifests import load_target_manifests

KINDS = ("argument", "local")

_IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_C_KEYWORDS = frozenset(
    "auto break case char const continue default do double else enum extern float "
    "for goto if inline int long register restrict return short signed sizeof "
    "static struct switch typedef union unsigned void volatile while _Bool".split()
)
_SCALAR_TYPES = frozenset(
    "bool s8 s16 s32 s64 u8 u16 u32 u64 f32 f64 "
    "char short int long signed unsigned float double void _Bool".split()
)
_QUALIFIERS = frozenset("const volatile static register auto".split())
_TYPE_HEADS = frozenset("struct union enum".split())


def _tags(masked: str) -> set[str]:
    return set(re.findall(r"\b(?:struct|union|enum)\s+([A-Za-z_]\w*)", masked))


def _without_initializers(statement: str) -> str:
    """Keep declarators, excluding top-level initializer expressions."""

    masked = mask_c_text(statement)
    parts: list[str] = []
    parens = brackets = braces = 0
    start = 0
    skipping = False
    for index, character in enumerate(masked):
        if character == "=" and parens == brackets == braces == 0 and not skipping:
            parts.append(statement[start:index])
            skipping = True
        elif character in ",;" and parens == brackets == braces == 0:
            if skipping:
                start = index
                skipping = False
        parens += (character == "(") - (character == ")")
        brackets += (character == "[") - (character == "]")
        braces += (character == "{") - (character == "}")
    if not skipping:
        parts.append(statement[start:])
    return "".join(parts)


def _declaration_names(statement: str) -> tuple[str, ...]:
    """Return declared ordinary names, dropping initializers, tags and type heads."""

    from harness.domain.c_context import declaration_names

    statement = _without_initializers(statement)
    masked = mask_c_text(statement)
    try:
        names = declaration_names(statement)
    except (ValueError, RecursionError):
        return ()
    tags = _tags(masked)
    return tuple(name for name in names if name not in tags and name not in _C_KEYWORDS)


def _declarator_names(remainder: str) -> tuple[str, ...]:
    """Parse a declarator list whose type prefix has already been consumed."""

    if not re.match(r"\s*[*([\s]*[A-Za-z_]", remainder):
        return ()
    names: list[str] = []
    for item in re.split(r",(?![^()\[\]]*[)\]}])", remainder):
        item = item.strip()
        if not item or "->" in item or "." in item.split("=", 1)[0]:
            continue
        head = item.split("=", 1)[0]
        names.extend(_declaration_names(f"{head.strip()};"))
    return tuple(dict.fromkeys(names))


def _is_declaration(statement: str) -> bool:
    """Classify one statement as a scalar declaration without guessing types."""

    masked = mask_c_text(statement).strip()
    if not masked.endswith(";"):
        return False
    masked = masked[:-1].strip()
    tokens = _IDENTIFIER.findall(masked)
    if (
        not tokens
        or tokens[0] in _C_KEYWORDS - _SCALAR_TYPES - _TYPE_HEADS - _QUALIFIERS
    ):
        return False
    # A target-owned typedef is still a valid type head; require a declarator.
    remainder = masked
    while True:
        match = re.match(r"\s*([A-Za-z_]\w*)", remainder)
        if match is None:
            return False
        token = match.group(1)
        if token in _QUALIFIERS:
            remainder = remainder[match.end() :]
            continue
        if token in _TYPE_HEADS:
            remainder = remainder[match.end() :]
            tag = re.match(r"\s*[A-Za-z_]\w*", remainder)
            if tag is None:
                return False
            remainder = remainder[tag.end() :]
            break
        remainder = remainder[match.end() :]
        if token in _SCALAR_TYPES:
            following = re.match(r"\s*([A-Za-z_]\w*)", remainder)
            if following and following.group(1) in _SCALAR_TYPES:
                continue
        break
    if remainder.lstrip().startswith("="):
        return False
    # A parenthesis after a type head must begin a pointer declarator, not
    # the argument list of a call whose callee was mistaken for a typedef.
    if remainder.lstrip().startswith("(") and not re.match(
        r"\s*\(\s*\*\s*(?:(?:const|volatile)\s+)*[A-Za-z_]\w*"
        r"\s*(?:\[[^]]*\]\s*)*\)\s*(?:\(|\[)",
        remainder,
    ):
        return False
    return bool(_declarator_names(remainder))


def _signature_and_body(implementation: str) -> tuple[str, str]:
    """Split one implementation into its signature and body text."""

    masked = mask_c_text(implementation)
    parens = 0
    body_start = None
    for index, character in enumerate(masked):
        if character == "(":
            parens += 1
        elif character == ")":
            parens -= 1
        elif character == "{" and parens == 0:
            body_start = index
            break
    if body_start is None:
        raise ValueError("source identifier scan requires a function body")
    body_end = masked.rfind("}")
    if body_end < body_start:
        raise ValueError("source identifier scan requires a closed function body")
    return implementation[:body_start], implementation[body_start + 1 : body_end]


def _parameter_span(signature: str) -> tuple[int, int]:
    """Locate the parameter-list span inside one function signature."""

    masked = mask_function_attributes(signature)
    depth = 0
    opening = None
    for index, character in enumerate(masked):
        if character == "(":
            if depth == 0:
                opening = index
            depth += 1
        elif character == ")":
            depth -= 1
            if depth == 0 and opening is not None:
                return opening, index
    raise ValueError("source identifier scan requires a closed parameter list")


def _parameter_declarations(signature: str) -> list[str]:
    opening, closing = _parameter_span(signature)
    span = signature[opening + 1 : closing]
    masked = mask_c_text(span)
    if not masked.strip() or masked.strip() == "void":
        return []
    declarations = [
        item.strip() for item in re.split(r",(?![^(]*\))", span) if item.strip()
    ]
    return declarations


def _statement_declarations(body: str) -> list[str]:
    """Enumerate body statements whose leading type head is a declaration."""

    masked = mask_c_text(body)
    declarations: list[str] = []
    start = 0
    parens = brackets = 0
    initializer_braces = 0
    aggregate_braces = 0
    aggregate_statement = False
    for index, character in enumerate(masked):
        if aggregate_statement:
            aggregate_braces += (character == "{") - (character == "}")
            if character == ";" and aggregate_braces == 0:
                aggregate_statement = False
                start = index + 1
            continue
        if character == "(":
            parens += 1
        elif character == ")":
            parens -= 1
        elif character == "[":
            brackets += 1
        elif character == "]":
            brackets -= 1
        elif character == "{" and parens == 0 and brackets == 0:
            prefix = masked[start:index]
            if (
                not initializer_braces
                and "=" not in prefix
                and re.match(
                    r"\s*(?:(?:typedef|const|volatile|static|auto|register|extern)\s+)*"
                    r"(?:struct|union|enum)\b",
                    prefix,
                )
            ):
                aggregate_statement = True
                aggregate_braces = 1
            elif initializer_braces or (
                "=" in prefix and _is_declaration(body[start:index] + ";")
            ):
                initializer_braces += 1
            else:
                start = index + 1
        elif character == "}" and parens == 0 and brackets == 0:
            if initializer_braces:
                initializer_braces -= 1
            else:
                start = index + 1
        elif (
            character == ":"
            and parens == 0
            and brackets == 0
            and not initializer_braces
        ):
            prefix = masked[start : index + 1]
            if re.fullmatch(r"\s*(?:case\b[^{};]*|default|[A-Za-z_]\w*)\s*:", prefix):
                start = index + 1
        elif (
            character == ";"
            and parens == 0
            and brackets == 0
            and not initializer_braces
        ):
            statement = body[start : index + 1]
            if _is_declaration(statement):
                declarations.append(statement)
            start = index + 1
    for match in re.finditer(r"\bfor\s*\(([^;{}]*);", masked):
        clause = body[match.start(1) : match.end(1)]
        if _is_declaration(clause + ";"):
            declarations.append(clause + ";")
    return declarations


def collect_source_identifiers(root: Path, target: str) -> list[dict[str, Any]]:
    """Report lexical names; unsupported source forms remain explicit failures."""
    from harness.domain.ids import normalize_target_id

    target = normalize_target_id(target).value
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    manifest = manifests[target]
    rows = []
    for relative in sorted(set(manifest.sources + manifest.support_sources)):
        if not relative.endswith(".c"):
            continue
        text = (root / relative).read_text(encoding="utf-8")
        try:
            records = parse_function_records(text, validate_progress=False)
        except ValueError as error:
            raise ValueError(
                f"cannot inspect identifiers in {relative}: {error}"
            ) from error
        expanded = None
        for record in records:
            implementation = text[
                record.implementation_start : record.implementation_end
            ]
            function = record.spelling
            provenance = {}
            if record.kind == "template":
                from harness.build.expansion import expand_function_members

                if expanded is None:
                    expanded = expand_function_members(root, root / relative)
                member = expanded[record.address]
                implementation = member.implementation
                function = member.symbol
                provenance = {
                    "implementation_kind": "template",
                    "macro": record.spelling,
                    "invocation": member.invocation,
                }
            signature, body = _signature_and_body(implementation)
            tokens = _IDENTIFIER.findall(mask_c_text(implementation))
            selector = f"{target}@{record.address:08X}"
            for kind, statements in (
                ("argument", _parameter_declarations(signature)),
                ("local", _statement_declarations(body)),
            ):
                seen = set()
                for statement in statements:
                    for name in _declaration_names(statement):
                        if name in seen or name in _C_KEYWORDS:
                            continue
                        seen.add(name)
                        rows.append(
                            {
                                "id": f"{selector}@{kind}:{name}",
                                "target": target,
                                "selector": selector,
                                "function": function,
                                "kind": kind,
                                "name": name,
                                "source": relative,
                                "line": record.line,
                                "uses": tokens.count(name),
                                "declaration": " ".join(statement.split()),
                                **provenance,
                            }
                        )
    return sorted(
        rows, key=lambda row: (row["selector"], row["kind"] != "argument", row["name"])
    )
