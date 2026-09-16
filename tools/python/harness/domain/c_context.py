"""Conservative lexical C parsing and namespace-aware standalone context."""

from __future__ import annotations

import re
from dataclasses import replace

from harness.common.lexicon import iter_c_lexemes
from harness.domain import declarations as model

_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
_C_KEYWORDS = frozenset(
    "auto break case char const continue default do double else enum extern float for "
    "goto if int long register return short signed sizeof static struct switch "
    "typedef union unsigned void volatile while inline restrict _Bool".split()
)
_AGGREGATE = re.compile(
    r"^(?:(?:typedef|extern)\s+)?(?:(?:const|volatile)\s+)*"
    r"(struct|union|enum)\b\s*([A-Za-z_]\w*)?"
)
_FUNCTION_POINTER = re.compile(
    r"\(\s*\*\s*(?:const\s+|volatile\s+)*([A-Za-z_]\w*)\s*"
    r"(?:\[[^][{};]*\]\s*)*\)"
)
_DIRECTIVE = re.compile(r"(?m)^[ \t]*#(?:[^\n]*\\\n)*[^\n]*")


def _mask_lexemes(text: str, *, literals: bool = True) -> str:
    parts, start = [], 0
    for match in iter_c_lexemes(text):
        parts.append(text[start : match.start()])
        value = match.group()
        parts.append(
            re.sub(r"[^\n]", " ", value)
            if literals or value.startswith(("/*", "//"))
            else value
        )
        start = match.end()
    return "".join(parts) + text[start:]


def _compact_statement(text: str, *, punctuation: bool = False) -> str:
    literals: list[str] = []
    parts, start = [], 0
    for match in iter_c_lexemes(text):
        parts.append(text[start : match.start()])
        if match.group().startswith(("/*", "//")):
            parts.append(" ")
        else:
            literals.append(match.group())
            parts.append(f"\x00{len(literals) - 1}\x00")
        start = match.end()
    compact = " ".join(("".join(parts) + text[start:]).split())
    if punctuation:
        compact = re.sub(r"\s+([(),;])", r"\1", compact)
        compact = re.sub(r"([(),;])\s+", r"\1", compact)
    for ordinal, literal in enumerate(literals):
        compact = compact.replace(f"\x00{ordinal}\x00", literal)
    return compact


def declaration_statements(text: str) -> list[str]:
    """Split top-level semicolons, ignoring comments, literals and directives."""

    clean = _mask_lexemes(text, literals=False)
    masked = _mask_lexemes(clean)
    directives = list(_DIRECTIVE.finditer(masked))
    directive_starts = {match.start() for match in directives}
    for match in reversed(directives):
        padding = re.sub(r"[^\n]", " ", match.group())
        clean = clean[: match.start()] + padding + clean[match.end() :]
        masked = masked[: match.start()] + padding + masked[match.end() :]
    rows: list[str] = []
    start = braces = parens = brackets = 0
    for index, character in enumerate(masked):
        if index in directive_starts and (braces or parens or brackets):
            clean = clean[:index] + "#" + clean[index + 1 :]
        braces += (character == "{") - (character == "}")
        parens += (character == "(") - (character == ")")
        brackets += (character == "[") - (character == "]")
        if character == ";" and braces == parens == brackets == 0:
            rows.append(_compact_statement(clean[start : index + 1]))
            start = index + 1
        elif character == "}" and braces == parens == brackets == 0:
            prefix = masked[start : masked.find("{", start)]
            if (
                "(" in prefix
                and "=" not in prefix
                and not _AGGREGATE.match(prefix.strip())
            ):
                start = index + 1
    tail = _compact_statement(clean[start:])
    if tail:
        rows.append(tail)
    return [
        row
        for row in rows
        if re.match(r"^(typedef|extern|struct|union|enum)\b", row)
        or re.match(r"^[A-Za-z_]\w*(?:\s|\*)+.+\(", row)
    ]


def canonical_declaration(statement: str) -> str:
    return _compact_statement(statement, punctuation=True)


def _split_top_level(text: str, delimiter: str) -> list[str]:
    rows, start, braces, parens, brackets = [], 0, 0, 0, 0
    for index, character in enumerate(_mask_lexemes(text)):
        braces += (character == "{") - (character == "}")
        parens += (character == "(") - (character == ")")
        brackets += (character == "[") - (character == "]")
        if character == delimiter and braces == parens == brackets == 0:
            rows.append(text[start:index])
            start = index + 1
    rows.append(text[start:])
    return rows


def _parse_declarators(
    tail: str, kind: str, *, base: str = "", tag: str | None = None
) -> tuple[tuple[model.CDeclarator, ...], str | None]:
    declarators = []
    diagnostic = None
    for item in _split_top_level(tail, ","):
        item = item.strip()
        if not item:
            continue
        pointer = _FUNCTION_POINTER.search(item)
        if pointer and "(" in item[: pointer.start()]:
            pointer = None
        function = re.fullmatch(r"(.*?)\b([A-Za-z_]\w*)\s*(\([^{};]*\))", item)
        ordinary = re.fullmatch(r"(.*?)([A-Za-z_]\w*)\s*((?:\[[^]]*\]\s*)*)", item)
        match = pointer or function or ordinary
        if match is None:
            diagnostic = "unsupported declarator"
            continue
        name_group = 1 if pointer else 2
        name = match.group(name_group)
        if name in _C_KEYWORDS:
            diagnostic = "unsupported declarator"
            continue
        prefix = item[: match.start(name_group)]
        suffix = item[match.end(name_group) :]
        if not base:
            split = re.match(r"^(.*?)(\s*[*\[(].*)?$", prefix)
            base = split.group(1).strip() if split else ""
            prefix = prefix[len(base) :]
        expression = canonical_declaration(f"{base} {prefix}{suffix}")
        relationship = (
            "function"
            if function or (pointer and "(" in suffix)
            else "array"
            if "[" in suffix
            else "pointer"
            if "*" in prefix
            else "identity"
        )
        local_diagnostic = None
        if not base or not re.fullmatch(r"[A-Za-z_][\w\s]*", base):
            local_diagnostic = "unsupported type specifier"
        pointer_prefix = r"\s*\(\s*\*\s*(?:(?:const|volatile|restrict)\s+)*"
        plain_prefix = r"\s*(?:\*\s*(?:(?:const|volatile|restrict)\s+)*)*"
        if not re.fullmatch(pointer_prefix if pointer else plain_prefix, prefix):
            local_diagnostic = "unsupported declarator prefix"
        if pointer and not re.fullmatch(
            r"(?:\[[^][{};]*\]\s*)*\)\s*(?:\([^{};]*\)|(?:\[[^]]*\]\s*)+)", suffix
        ):
            local_diagnostic = "unsupported pointer declarator suffix"
        if re.search(r"\b(?:struct|union|enum)\b", suffix):
            local_diagnostic = "unsupported prototype tag scope"
        if any(token in expression for token in ("__attribute__", "__declspec", "{")):
            local_diagnostic = "unsupported declaration extension or nested scope"
        identifiers = [
            word for word in _IDENTIFIER.findall(base) if word not in _C_KEYWORDS
        ]
        if len(identifiers) > 1:
            local_diagnostic = "unsupported type specifier"
        alias_namespace = (
            "tag"
            if re.search(r"\b(struct|union|enum)\b", base)
            else "ordinary"
            if len(identifiers) == 1
            else None
        )
        declarators.append(
            model.CDeclarator(
                name,
                expression,
                "prototype" if function and not pointer and kind == "extern" else kind,
                relationship,
                alias_namespace,
                tag
                if alias_namespace == "tag"
                else identifiers[0]
                if alias_namespace
                else None,
                local_diagnostic,
            )
        )
        diagnostic = diagnostic or local_diagnostic
    return tuple(declarators), diagnostic


def _field_records(body: str, kind: str) -> tuple[tuple[model.CField, ...], str | None]:
    fields: list[model.CField] = []
    diagnostic = None
    for item in _split_top_level(body, "," if kind == "enum" else ";"):
        item = item.strip()
        if not item:
            continue
        if kind == "enum":
            match = re.fullmatch(r"([A-Za-z_]\w*)\s*(?:=\s*(.+))?", item)
            if match:
                fields.append(model.CField(match.group(1), "enumerator"))
            else:
                diagnostic = "unsupported enumerator"
            continue
        if "{" in _mask_lexemes(item):
            nested = re.search(r"}\s*([A-Za-z_]\w*)\s*$", item)
            if nested:
                fields.append(model.CField(nested.group(1), "unknown"))
            diagnostic = "unsupported nested aggregate scope"
            continue
        declarators, problem = _parse_declarators(item, "field")
        diagnostic = diagnostic or problem
        for declarator in declarators:
            expression = declarator.type_expression
            array = re.search(r"\[\s*([^]]*)\s*\]$", expression)
            prefix = expression[: array.start()] if array else expression
            qualifiers = " ".join(
                word
                for word in ("const", "volatile")
                if re.search(rf"\b{word}\b", prefix)
            )
            fields.append(
                model.CField(
                    declarator.name,
                    " ".join(re.sub(r"\b(?:const|volatile)\b", "", prefix).split()),
                    array.group(1).strip() if array else None,
                    qualifiers,
                )
            )
    return tuple(fields), diagnostic


def _parse_statement(statement: str) -> model.CDeclaration:
    body = statement.rstrip("; ")
    aggregate = _AGGREGATE.match(body)
    tag = aggregate.group(2) if aggregate else None
    storage = (
        "typedef"
        if re.match(r"typedef\b", body)
        else "extern"
        if re.match(r"extern\b", body)
        else "prototype"
    )
    kind = aggregate.group(1) if aggregate else storage
    fields: tuple[model.CField, ...] = ()
    complete, definition, diagnostic = False, None, None
    if aggregate:
        tail = body[aggregate.end() :].strip()
        base = re.sub(r"^(typedef|extern)\s+", "", body[: aggregate.end()]).strip()
        if tail.startswith("{"):
            depth, closing = 0, -1
            for index, character in enumerate(_mask_lexemes(tail)):
                depth += (character == "{") - (character == "}")
                if depth == 0:
                    closing = index
                    break
            if closing >= 0:
                complete = True
                definition = canonical_declaration(f"{kind} {tail[: closing + 1]}")
                fields, diagnostic = _field_records(tail[1:closing], kind)
                tail = tail[closing + 1 :].strip()
            else:
                diagnostic = "unterminated aggregate definition"
        declarators, problem = _parse_declarators(
            tail, "typedef" if storage == "typedef" else "extern", base=base, tag=tag
        )
    else:
        tail = re.sub(r"^(typedef|extern)\s+", "", body)
        declarators, problem = _parse_declarators(tail, storage)
    diagnostic = diagnostic or problem
    if re.search(r"\b(?:__attribute__|__declspec|_Alignas|__packed)\b", statement):
        diagnostic = "unsupported declaration attribute or alignment"
    if "#" in _mask_lexemes(statement):
        diagnostic = "unsupported preprocessor directive inside declaration"
    if not statement.endswith(";"):
        diagnostic = "unterminated declaration"
    if aggregate and not tag and not complete:
        diagnostic = "anonymous aggregate without definition"
    if storage == "typedef" and not declarators:
        diagnostic = diagnostic or "typedef without declarator"
    names = tuple(
        dict.fromkeys(([tag] if tag else []) + [item.name for item in declarators])
    )
    return model.CDeclaration(
        names,
        kind,
        canonical_declaration(statement),
        tag,
        fields,
        diagnostic,
        complete,
        definition,
        declarators,
        statement,
    )


def declaration_records(text: str) -> tuple[model.CDeclaration, ...]:
    """Parse selected header text, without evaluating conditional visibility."""

    records = tuple(
        _parse_statement(statement) for statement in declaration_statements(text)
    )
    if re.search(r"(?m)^\s*#\s*pragma\s+pack\b", _mask_lexemes(text)):
        records = tuple(
            replace(
                record, diagnostic=record.diagnostic or "unsupported packing directive"
            )
            if record.kind in {"struct", "union"}
            else record
            for record in records
        )
    return records


def declaration_names(statement: str) -> tuple[str, ...]:
    """Return exact tag and declarator names without conflating their entities."""

    record = _parse_statement(_compact_statement(statement))
    return tuple(
        dict.fromkeys(
            record.names
            + tuple(field.name for field in record.fields if record.kind == "enum")
        )
    )


def scalar_declaration_context(text: str) -> str:
    """Return scalar typedef declarations from the tracked base type header."""

    names = {"bool", "s8", "s16", "s32", "s64", "u8", "u16", "u32", "u64", "f32", "f64"}
    return (
        "\n".join(
            statement
            for statement in declaration_statements(text)
            if set(declaration_names(statement)) & names
        )
        + "\n"
    )


def collect_declaration_references(
    text: str, scope: str = ""
) -> set[model.DeclarationKey]:
    masked = _mask_lexemes(text)
    references = set()
    for match in re.finditer(r"\b(?:struct|union|enum)\s+([A-Za-z_]\w*)", masked):
        references.add(model.DeclarationKey(scope, "tag", match.group(1)))
    ordinary = re.sub(r"\b(?:struct|union|enum)\s+[A-Za-z_]\w*", "", masked)
    references.update(
        model.DeclarationKey(scope, "ordinary", name)
        for name in _IDENTIFIER.findall(ordinary)
        if name not in _C_KEYWORDS
    )
    return references


def _dependencies(
    occurrence: model.CDeclarationOccurrence,
) -> dict[model.DeclarationKey, bool]:
    references: dict[model.DeclarationKey, bool] = {}
    if occurrence.key.namespace == "tag":
        expressions = [
            field.type_name + (f"[{field.array_extent}]" if field.array_extent else "")
            for field in occurrence.fields
        ]
        if occurrence.kind == "enum" and occurrence.definition:
            expressions = [
                item.partition("=")[2]
                for item in _split_top_level(
                    occurrence.definition.partition("{")[2].rsplit("}", 1)[0], ","
                )
            ]
    else:
        expressions = [occurrence.type_expression]
    for expression in expressions:
        for key in collect_declaration_references(expression, occurrence.key.scope):
            required = "*" not in expression
            references[key] = references.get(key, False) or required
    if occurrence.alias_target:
        references[occurrence.alias_target] = (
            occurrence.relationship in {"array", "enumerator"}
            or occurrence.alias_target.namespace == "ordinary"
        )
    return references


def _requirements(occurrence, entities):
    requirements = _dependencies(occurrence)
    if (
        occurrence.kind not in {"struct", "union"}
        and occurrence.relationship != "array"
    ):
        return requirements
    for key, complete in tuple(requirements.items()):
        visited = set()
        while complete and key in entities and key not in visited:
            visited.add(key)
            entity = entities[key]
            if entity.alias_kind != "identity" or entity.alias_target is None:
                break
            key = entity.alias_target
            requirements[key] = True
    return requirements


def _select_units(entities, units, selected):
    chosen = {unit for unit in units if unit[0] == "base"}
    for key, entity in entities.items():
        if key in selected and key.namespace == "ordinary":
            occurrence = entity.representative
            chosen.add((occurrence.origin, occurrence.ordinal))
    for key, entity in entities.items():
        if key not in selected or key.namespace != "tag":
            continue
        supplied = any(
            member.key == key and member.complete
            for unit in chosen
            for member in units[unit]
        )
        if entity.selected_definition and not supplied:
            occurrence = entity.selected_definition
            chosen.add((occurrence.origin, occurrence.ordinal))
        for occurrence in entity.occurrences:
            unit = (occurrence.origin, occurrence.ordinal)
            if not occurrence.complete:
                chosen.add(unit)
    return chosen


def public_declaration_context(preprocessed: str, source: str, *, base: str) -> str:
    """Close entity dependencies, preserving base forwards and supplied definitions.

    This lexical closure assumes one caller-selected visibility scope. It rejects
    unsupported selected forms and ordering requiring synthesized declarations.
    """

    occurrences = model.build_declaration_occurrences(
        declaration_records(base), origin="base"
    ) + model.build_declaration_occurrences(
        declaration_records(preprocessed), origin="supplemental"
    )
    return render_declaration_context(occurrences, source, base=base)


def render_declaration_context(
    occurrences: tuple[model.CDeclarationOccurrence, ...],
    source: str,
    *,
    base: str = "",
) -> str:
    """Close dependencies without discarding occurrence provenance or diagnostics."""
    entities = {
        entity.key: entity for entity in model.reconcile_declarations(occurrences)
    }
    units: dict[tuple[str, int], list[model.CDeclarationOccurrence]] = {}
    for occurrence in occurrences:
        units.setdefault((occurrence.origin, occurrence.ordinal), []).append(occurrence)
    selected: set[model.DeclarationKey] = set()
    pending = list(collect_declaration_references(source))
    examined = set()
    while True:
        while pending:
            key = pending.pop()
            if key in examined:
                continue
            examined.add(key)
            entity = entities.get(key)
            unsupported = (
                entity
                if entity and entity.diagnostic
                else next(
                    (
                        item
                        for item in entities.values()
                        if entity is None
                        and not item.key.name
                        and item.diagnostic
                        and key.name
                        and key.name in model.tokenize_declaration(item.canonical)
                    ),
                    None,
                )
            )
            if unsupported:
                raise ValueError(
                    f"unsupported declaration for {key.name}: {unsupported.diagnostic}"
                )
            if entity:
                selected.add(key)
                pending.extend(_requirements(entity.representative, entities))
        chosen = _select_units(entities, units, selected)
        for unit in chosen:
            for member in units[unit]:
                if member.diagnostic:
                    raise ValueError(
                        f"unsupported declaration: {member.diagnostic}: {member.statement}"
                    )
                pending.extend(
                    key
                    for key in _requirements(member, entities)
                    if key not in examined
                )
        if not pending:
            break
    emitted = {
        model.tokenize_declaration(units[unit][0].canonical)
        for unit in chosen
        if unit[0] == "base"
    }
    available = {
        member.key for unit in chosen if unit[0] == "base" for member in units[unit]
    }
    completed = {
        member.key
        for unit in chosen
        if unit[0] == "base"
        for member in units[unit]
        if member.complete
    }
    remaining = [unit for unit in units if unit in chosen and unit[0] != "base"]
    output = []
    while remaining:
        progressed = False
        for unit in remaining[:]:
            members = units[unit]
            signature = model.tokenize_declaration(members[0].canonical)
            if signature in emitted:
                remaining.remove(unit)
                progressed = True
                continue
            provided = {member.key for member in members}
            definitions = {member.key for member in members if member.complete}
            ready = True
            for member in members:
                for dependency, needs_complete in _requirements(
                    member, entities
                ).items():
                    if dependency not in entities:
                        continue
                    if dependency in provided:
                        if dependency.namespace == "tag" and (
                            not needs_complete
                            or (member.key != dependency and dependency in definitions)
                        ):
                            continue
                        if member.kind in {"enum", "enumerator"}:
                            continue
                    if dependency not in (completed if needs_complete else available):
                        ready = False
            if not ready:
                continue
            duplicates = [
                member
                for member in members
                if member.key.namespace == "tag"
                and member.complete
                and member.key in completed
            ]
            if duplicates:
                raise ValueError(
                    f"unsupported repeated aggregate definition in context: {members[0].statement}"
                )
            output.append(members[0].statement)
            emitted.add(signature)
            available.update(provided)
            completed.update(member.key for member in members if member.complete)
            remaining.remove(unit)
            progressed = True
        if not progressed:
            raise ValueError(
                "unsupported declaration dependency order: "
                + " | ".join(units[unit][0].statement for unit in remaining)
            )
    return (
        base
        + ("\n" if base and not base.endswith("\n") else "")
        + "\n".join(output)
        + "\n"
    )
