"""Lexical records for C preprocessor macro definitions."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

_DIRECTIVE = re.compile(r"^\s*#\s*([A-Za-z_]\w*)")
_NAME = re.compile(r"[A-Za-z_]\w*")
_GENERATED = re.compile(
    r"(?:generated\s+by|auto[- ]?generated|do\s+not\s+edit|@generated)", re.IGNORECASE
)


@dataclass(frozen=True)
class MacroDefinition:
    """One source-ordered macro definition and its lexical context."""

    name: str
    parameters: tuple[str, ...] | None
    body: str
    conditions: tuple[str, ...]
    source_line: int
    classification: str
    restrictions: tuple[str, ...]
    generated: bool
    diagnostic: str | None = None


@dataclass(frozen=True)
class MacroUse:
    """One lexical use of a known macro spelling."""

    name: str
    arguments: str | None
    conditions: tuple[str, ...]
    source_line: int
    source_column: int
    context: str
    generated: bool


def _mask_line(line: str, in_comment: bool) -> tuple[str, bool]:
    """Blank comments and literals while retaining character positions."""

    masked = list(line)
    index = 0
    while index < len(line):
        if in_comment:
            end = line.find("*/", index)
            if end < 0:
                masked[index:] = " " * (len(line) - index)
                return "".join(masked), True
            masked[index : end + 2] = " " * (end + 2 - index)
            index = end + 2
            in_comment = False
            continue
        if line.startswith("//", index):
            masked[index:] = " " * (len(line) - index)
            break
        if line.startswith("/*", index):
            masked[index : index + 2] = "  "
            index += 2
            in_comment = True
            continue
        if line[index] in {'"', "'"}:
            quote = line[index]
            masked[index] = " "
            index += 1
            while index < len(line):
                masked[index] = " "
                if line[index] == "\\" and index + 1 < len(line):
                    masked[index + 1] = " "
                    index += 2
                    continue
                character = line[index]
                index += 1
                if character == quote:
                    break
            continue
        index += 1
    return "".join(masked), in_comment


def _masked_lines(lines: list[str]) -> list[str]:
    rows: list[str] = []
    in_comment = False
    for line in lines:
        masked, in_comment = _mask_line(line, in_comment)
        rows.append(masked)
    return rows


def _continued(line: str) -> tuple[str, bool]:
    stripped = line.rstrip()
    if not stripped.endswith("\\"):
        return line, False
    return line[: len(stripped) - 1].rstrip(), True


def _logical_directive(
    lines: list[str], masks: list[str], start: int
) -> tuple[str, str, int, bool]:
    raw_rows: list[str] = []
    mask_rows: list[str] = []
    index = start
    malformed = False
    while index < len(lines):
        raw, raw_continues = _continued(lines[index])
        masked, mask_continues = _continued(masks[index])
        continues = raw_continues and mask_continues
        raw_rows.append(raw if continues else lines[index])
        mask_rows.append(masked if continues else masks[index])
        if not continues:
            break
        index += 1
        if index == len(lines):
            malformed = True
            break
    return "\n".join(raw_rows), "\n".join(mask_rows), index, malformed


def _condition(keyword: str, value: str) -> str:
    value = " ".join(value.split())
    if keyword == "ifdef":
        return f"defined({value})"
    if keyword == "ifndef":
        return f"!defined({value})"
    return value


def _closing_parenthesis(text: str, start: int) -> int | None:
    depth = 0
    for index in range(start, len(text)):
        if text[index] == "(":
            depth += 1
        elif text[index] == ")":
            depth -= 1
            if depth == 0:
                return index
    return None


def _restrictions(name: str, generated: bool) -> tuple[str, ...]:
    restrictions: list[str] = []
    if name == "REGISTER_PIN":
        restrictions.append("allocator_constraint")
    if name == "barrier" or name.startswith("CLOBBER_"):
        restrictions.append("register_scheduling")
    if name.startswith("INCLUDE_ASM") or name.startswith("INCLUDE_RODATA"):
        restrictions.append("inline_assembly")
    if name == "WEAK_SYMBOL_AT":
        restrictions.append("absolute_symbol_binding")
    if generated:
        restrictions.append("generator_owned")
    return tuple(restrictions)


def _classification(
    name: str, parameters: tuple[str, ...] | None, body_mask: str, source: str
) -> str:
    if name == "WEAK_SYMBOL_AT":
        return "generated_binding"
    if (
        name == "REGISTER_PIN"
        or name == "barrier"
        or name.startswith("CLOBBER_")
        or name.startswith("INCLUDE_ASM")
        or name.startswith("INCLUDE_RODATA")
    ):
        return "matching_helper"
    if Path(source).suffix == ".inc" and parameters is not None and "{" in body_mask:
        return "body_emitting_template"
    return "object_like" if parameters is None else "function_like"


def _parse_definition(
    raw: str,
    masked: str,
    *,
    conditions: tuple[str, ...],
    source_line: int,
    source: str,
    generated: bool,
    malformed: bool,
) -> MacroDefinition | None:
    directive = _DIRECTIVE.match(masked)
    if directive is None or directive.group(1) != "define":
        return None
    name_match = _NAME.search(masked, directive.end())
    if name_match is None:
        return None
    name = name_match.group()
    body_start = name_match.end()
    parameters: tuple[str, ...] | None = None
    diagnostic = "unterminated macro continuation" if malformed else None
    if body_start < len(masked) and masked[body_start] == "(":
        close = _closing_parenthesis(masked, body_start)
        if close is None:
            parameters = ()
            body_start += 1
            diagnostic = diagnostic or "unterminated macro parameter list"
        else:
            parameter_text = masked[body_start + 1 : close].strip()
            parameters = (
                tuple(item.strip() for item in parameter_text.split(","))
                if parameter_text
                else ()
            )
            body_start = close + 1
    body = raw[body_start:].strip()
    body_mask = masked[body_start:].strip()
    return MacroDefinition(
        name=name,
        parameters=parameters,
        body=body,
        conditions=conditions,
        source_line=source_line,
        classification=_classification(name, parameters, body_mask, source),
        restrictions=_restrictions(name, generated),
        generated=generated,
        diagnostic=diagnostic,
    )


def parse_macro_definitions(
    text: str, source: str | Path = "<memory>"
) -> tuple[MacroDefinition, ...]:
    """Parse source-ordered ``#define`` records without expanding them."""

    lines = text.splitlines()
    masks = _masked_lines(lines)
    conditions: list[str] = []
    definitions: list[MacroDefinition] = []
    generated = _GENERATED.search(text) is not None
    source_name = str(source)
    index = 0
    while index < len(lines):
        directive = _DIRECTIVE.match(masks[index])
        if directive is None:
            index += 1
            continue
        keyword = directive.group(1)
        value = masks[index][directive.end() :].strip()
        if keyword in {"if", "ifdef", "ifndef"}:
            conditions.append(_condition(keyword, value))
        elif keyword == "elif" and conditions:
            conditions[-1] = _condition(keyword, value)
        elif keyword == "else" and conditions:
            conditions[-1] = "else"
        elif keyword == "endif" and conditions:
            conditions.pop()
        elif keyword == "define":
            raw, masked, end, malformed = _logical_directive(lines, masks, index)
            definition = _parse_definition(
                raw,
                masked,
                conditions=tuple(conditions),
                source_line=index + 1,
                source=source_name,
                generated=generated,
                malformed=malformed,
            )
            if definition is not None:
                definitions.append(definition)
            index = end
        index += 1
    return tuple(definitions)


def _line_conditions(masks: list[str]) -> list[tuple[str, ...]]:
    conditions: list[str] = []
    result: list[tuple[str, ...]] = []
    for line in masks:
        directive = _DIRECTIVE.match(line)
        if directive is not None:
            keyword = directive.group(1)
            value = line[directive.end() :].strip()
            if keyword in {"if", "ifdef", "ifndef"}:
                conditions.append(_condition(keyword, value))
            elif keyword == "elif" and conditions:
                conditions[-1] = _condition(keyword, value)
            elif keyword == "else" and conditions:
                conditions[-1] = "else"
            elif keyword == "endif" and conditions:
                conditions.pop()
        result.append(tuple(conditions))
    return result


def parse_macro_uses(
    text: str, names: set[str] | frozenset[str]
) -> tuple[MacroUse, ...]:
    """Find lexical uses of known macro names without preprocessing C."""

    if not names:
        return ()
    lines = text.splitlines()
    masks = _masked_lines(lines)
    definition_lines: set[int] = set()
    for start, masked in enumerate(masks):
        directive = _DIRECTIVE.match(masked)
        if directive is None or directive.group(1) != "define":
            continue
        name = _NAME.search(masked, directive.end())
        if name is not None:
            masks[start] = (
                masked[: name.start()] + " " * len(name.group()) + masked[name.end() :]
            )
        index = start
        while index < len(lines):
            definition_lines.add(index)
            _row, continued = _continued(lines[index])
            if not continued:
                break
            index += 1
    conditions = _line_conditions(masks)
    generated = _GENERATED.search(text) is not None
    pattern = re.compile(
        r"\b(?:" + "|".join(re.escape(name) for name in sorted(names)) + r")\b"
    )
    masked_text = "\n".join(masks)
    raw_text = "\n".join(lines)
    offsets: list[int] = []
    offset = 0
    for line in lines:
        offsets.append(offset)
        offset += len(line) + 1
    uses: list[MacroUse] = []
    for match in pattern.finditer(masked_text):
        line_index = masked_text.count("\n", 0, match.start())
        column = match.start() - offsets[line_index]
        cursor = match.end()
        while cursor < len(masked_text) and masked_text[cursor].isspace():
            cursor += 1
        arguments = None
        if cursor < len(masked_text) and masked_text[cursor] == "(":
            close = _closing_parenthesis(masked_text, cursor)
            if close is not None:
                arguments = raw_text[cursor + 1 : close].strip()
        uses.append(
            MacroUse(
                name=match.group(),
                arguments=arguments,
                conditions=conditions[line_index],
                source_line=line_index + 1,
                source_column=column + 1,
                context="definition_body"
                if line_index in definition_lines
                else "expansion",
                generated=generated,
            )
        )
    return tuple(uses)


def parse_macro_file(path: str | Path) -> tuple[MacroDefinition, ...]:
    """Parse macro records from one UTF-8 source file."""

    source = Path(path)
    return parse_macro_definitions(source.read_text(encoding="utf-8"), source=source)


__all__ = [
    "MacroDefinition",
    "MacroUse",
    "parse_macro_definitions",
    "parse_macro_file",
    "parse_macro_uses",
]
