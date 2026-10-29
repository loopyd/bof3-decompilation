"""Leading metadata and lexical implementation ranges for authored C functions."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from harness.common.lexicon import iter_c_lexemes

from .tags import (
    SOURCE_TAG_RE,
    canonical_exact_progress,
    count_function_metadata,
    lift_lifecycle,
    parse_behavior_tag,
    parse_progress_tags,
    parse_source_tag,
    require_single_function,
)

_IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_DIRECTIVE = re.compile(r"^[ \t]*#((?:[^\n]*\\\r?\n)*[^\n]*)(?:\n|$)", re.MULTILINE)
_SOURCE = re.compile(r"@source\b")
_BEHAVIOR = re.compile(r"@behavior\b")


@dataclass(frozen=True)
class FunctionRecord:
    """One lexical implementation; offsets are half-open Unicode text positions."""

    address: int
    metadata: str
    metadata_start: int
    metadata_end: int
    implementation_start: int
    implementation_end: int
    line: int
    kind: str
    spelling: str

    @property
    def status(self) -> str:
        return lift_lifecycle(self.metadata)


def _mask_lexemes(text: str) -> tuple[str, list[re.Match[str]]]:
    masked = list(text)
    comments = []
    for match in iter_c_lexemes(text):
        is_comment = match.group().startswith(("/*", "//"))
        if is_comment:
            comments.append(match)
        masked[match.start() : match.end()] = [
            "\n" if character == "\n" else " " for character in match.group()
        ]
        if not is_comment:
            masked[match.start()] = "!"
    code = "".join(masked)
    if "/*" in code or any(character in code for character in "\"'"):
        raise ValueError("unterminated C comment or literal")
    return code, comments


def _mask_directives(masked: str) -> str:
    """Hide logical directives without changing source offsets or line numbers."""

    return _DIRECTIVE.sub(lambda match: re.sub(r"[^\n]", " ", match.group()), masked)


def _find_closing(masked: str, start: int, opening: str, closing: str) -> int:
    depth = 0
    for position in range(start, len(masked)):
        character = masked[position]
        depth += (character == opening) - (character == closing)
        if depth == 0:
            return position + 1
    raise ValueError(f"unterminated implementation delimiter {opening!r}")


def mask_function_attributes(text: str) -> str:
    """Mask balanced GNU prefix attributes while retaining signature offsets."""

    masked, _ = _mask_lexemes(text)
    position = 0
    while (opening := masked.find("(", position)) >= 0:
        names = list(_IDENTIFIER.finditer(masked, position, opening))
        if not names or names[-1].group() != "__attribute__":
            break
        attribute = names[-1]
        if masked[attribute.end() : opening].strip():
            raise ValueError("unsupported GNU attribute prefix")
        end = _find_closing(masked, opening, "(", ")")
        inner = opening + 1
        inner += len(masked[inner:]) - len(masked[inner:].lstrip())
        if inner >= end - 1 or masked[inner] != "(":
            raise ValueError("GNU attributes require nested parentheses")
        inner_end = _find_closing(masked, inner, "(", ")")
        if masked[inner_end : end - 1].strip():
            raise ValueError("unsupported GNU attribute suffix")
        masked = (
            masked[: attribute.start()]
            + re.sub(r"[^\n]", " ", masked[attribute.start() : end])
            + masked[end:]
        )
        position = end
    return masked


def _parse_implementation(masked: str, start: int) -> tuple[int, int, str, str]:
    start += len(masked[start:]) - len(masked[start:].lstrip())
    implementation = mask_function_attributes(masked[start:])
    relative_opening = implementation.find("(")
    if relative_opening < 0:
        raise ValueError("function metadata has no following implementation")
    opening = start + relative_opening
    prefix = implementation[:relative_opening].strip()
    if not prefix or re.sub(r"[A-Za-z_][A-Za-z0-9_]*|[\s*]+", "", prefix):
        raise ValueError("function metadata is not immediately above an implementation")
    names = _IDENTIFIER.findall(prefix)
    if not names or names[0] in {
        "extern",
        "typedef",
        "return",
        "if",
        "while",
        "for",
        "switch",
    }:
        raise ValueError(
            "function metadata cannot attach to a declaration or statement"
        )
    end = _find_closing(masked, opening, "(", ")")
    following = end + len(masked[end:]) - len(masked[end:].lstrip())
    if len(names) > 1:
        if following == len(masked) or masked[following] != "{":
            raise ValueError("function metadata requires a body, not a prototype")
        end = _find_closing(masked, following, "{", "}")
        return start, end, "function", names[-1]
    if following < len(masked) and masked[following] == "{":
        raise ValueError("implicit-int definitions require an explicit return type")
    if not _IDENTIFIER.fullmatch(prefix):
        raise ValueError("unsupported implementation declarator")
    line_end = masked.find("\n", end)
    line_end = len(masked) if line_end < 0 else line_end
    trailing = masked[end:line_end].strip()
    if trailing not in {"", ";"}:
        raise ValueError("template invocation must occupy its own logical line")
    if trailing:
        end = masked.index(";", end) + 1
    return start, end, "template", prefix


def parse_function_records(
    text: str, *, validate_progress: bool = True
) -> tuple[FunctionRecord, ...]:
    """Inspect attached metadata without expanding macros or claiming native identity.

    C89 definitions, GNU prefix attributes and standalone templates are supported.
    Continuations require macro definitions. Unconditional define/undef directives
    may occupy the outer body of an ordinary function. Conditional source and
    unsupported declarators reject. Data tags without behavior are ignored.
    """

    masked, comments = _mask_lexemes(text)
    candidates = []
    for comment in comments:
        if _BEHAVIOR.search(comment.group()):
            candidates.append(comment)
        elif _SOURCE.search(comment.group()):
            following = masked[comment.end() :].lstrip()
            declaration = re.match(r"(?:extern|typedef)\b|#\s*define\b", following)
            definition = re.match(r"extern\b[^;{}]*\([^;{}]*\)\s*\{", following)
            if definition or not declaration:
                line_start = text.rfind("\n", 0, comment.start()) + 1
                if not masked[line_start : comment.start()].strip():
                    raise ValueError("function metadata requires @behavior")
    if not candidates:
        return ()
    directives = list(_DIRECTIVE.finditer(masked))
    structural = _mask_directives(masked)
    for continuation in re.finditer(r"\\\r?\n", text):
        definitions = [
            item
            for item in directives
            if item.start() <= continuation.start() < item.end()
            and re.match(r"\s*define\b", item.group(1))
        ]
        if len(definitions) != 1:
            raise ValueError("continued source requires a macro definition")
    if any(
        re.match(r"\s*(?:if|ifdef|ifndef|elif|else|endif)\b", item.group(1))
        for item in directives
    ):
        raise ValueError("conditional source requires explicit preprocessing review")
    records = []
    addresses: set[int] = set()
    previous_end = 0
    for comment in candidates:
        metadata = comment.group()
        sources = SOURCE_TAG_RE.findall(metadata)
        if len(sources) != 1 or len(_SOURCE.findall(metadata)) != 1:
            raise ValueError(
                "each function metadata block requires exactly one @source"
            )
        if len(_BEHAVIOR.findall(metadata)) != 1:
            raise ValueError(
                "each function metadata block requires exactly one @behavior"
            )
        behavior = parse_behavior_tag(metadata)
        if behavior is None or not behavior.removeprefix("@behavior").strip(
            " */\r\n\t"
        ):
            raise ValueError("function metadata requires nonempty @behavior")
        if validate_progress:
            parse_progress_tags(metadata)
        address = int(sources[0], 16)
        if address in addresses:
            raise ValueError(f"duplicate function metadata address 0x{address:08X}")
        if any(item.start() <= comment.start() < item.end() for item in directives):
            raise ValueError("function metadata cannot occur inside a directive")
        gap = structural[previous_end : comment.start()].strip()
        if gap and gap[-1] != ";":
            raise ValueError("function metadata interrupts an unfinished declaration")
        preceding = structural[: comment.start()]
        if (
            any(
                preceding.count(opening) != preceding.count(closing)
                for opening, closing in (("{", "}"), ("(", ")"), ("[", "]"))
            )
            or comment.start() < previous_end
        ):
            raise ValueError(
                "function metadata must precede a top-level implementation"
            )
        start, end, kind, spelling = _parse_implementation(structural, comment.end())
        if any(comment.end() <= item.start() < start for item in directives):
            raise ValueError("directive interrupts attached function metadata")
        records.append(
            FunctionRecord(
                address,
                metadata,
                comment.start(),
                comment.end(),
                start,
                end,
                text.count("\n", 0, start) + 1,
                kind,
                spelling,
            )
        )
        addresses.add(address)
        previous_end = end
    for directive in directives:
        owner = next(
            (
                record
                for record in records
                if record.implementation_start <= directive.start()
                and directive.end() <= record.implementation_end
            ),
            None,
        )
        if owner is not None:
            body = -1
            if owner.kind == "function":
                implementation = mask_function_attributes(
                    structural[owner.implementation_start : owner.implementation_end]
                )
                opening = implementation.index("(")
                parameter_end = _find_closing(implementation, opening, "(", ")")
                following = (
                    parameter_end
                    + len(implementation[parameter_end:])
                    - len(implementation[parameter_end:].lstrip())
                )
                if implementation[following] != "{":
                    raise ValueError("function requires a body after its parameters")
                body = owner.implementation_start + following
            prefix = structural[body : directive.start()] if body >= 0 else ""
            if (
                owner.kind != "function"
                or body < 0
                or directive.start() <= body
                or prefix.count("{") - prefix.count("}") != 1
                or any(
                    prefix.count(opening) != prefix.count(closing)
                    for opening, closing in (("(", ")"), ("[", "]"))
                )
                or not re.match(r"\s*(?:define|undef)\b", directive.group(1))
            ):
                raise ValueError("directive requires an outer ordinary function body")
        else:
            prefix = structural[: directive.start()]
            if any(
                prefix.count(opening) != prefix.count(closing)
                for opening, closing in (("{", "}"), ("(", ")"), ("[", "]"))
            ):
                raise ValueError(
                    "directive is not top-level or in an owned function body"
                )
    return tuple(records)


def has_function_templates(text: str) -> bool:
    """Find top-level standalone invocations without tightening ordinary metadata.

    A generated unit must pass complete attached-record parsing during expansion,
    even if an unrelated definition supplies its expected assembler symbol.
    """

    masked, _ = _mask_lexemes(text)
    masked = _mask_directives(masked)
    for candidate in re.finditer(r"(?m)^[ \t]*[A-Za-z_][A-Za-z0-9_]*\s*\(", masked):
        preceding = masked[: candidate.start()]
        if any(
            preceding.count(opening) != preceding.count(closing)
            for opening, closing in (("{", "}"), ("(", ")"), ("[", "]"))
        ):
            continue
        opening = masked.index("(", candidate.start())
        end = _find_closing(masked, opening, "(", ")")
        if masked[end:].lstrip().startswith("{"):
            continue
        _, _, kind, _ = _parse_implementation(masked, candidate.start())
        if kind == "template":
            return True
    return False


def requires_function_expansion(text: str) -> bool:
    """Route templates and directives within C delimiters to configured expansion."""

    if has_function_templates(text):
        return True
    masked, _ = _mask_lexemes(text)
    structural = _mask_directives(masked)
    return any(
        any(
            structural[: directive.start()].count(opening)
            != structural[: directive.start()].count(closing)
            for opening, closing in (("{", "}"), ("(", ")"), ("[", "]"))
        )
        for directive in _DIRECTIVE.finditer(masked)
    )


def select_function_record(text: str, address: int) -> FunctionRecord:
    """Resolve exactly one attached function record by address, never by filename."""

    for record in parse_function_records(text):
        if record.address == address:
            return record
    raise ValueError(f"no function metadata for 0x{address:08X}")


def collect_lift_metadata(text: str) -> dict[int, str]:
    """Enumerate attached records or one legacy file-level lift without guessing."""

    if count_function_metadata(text) > 1:
        return {
            record.address: record.metadata
            for record in parse_function_records(text, validate_progress=False)
        }
    address = parse_source_tag(text)
    return {} if address is None else {address: text}


def select_lift_metadata(text: str, address: int) -> str:
    """Select metadata by original address, retaining legacy single-file syntax."""

    records = collect_lift_metadata(text)
    if address not in records:
        raise ValueError(f"no function metadata for 0x{address:08X}")
    return records[address]


def canonical_exact_lift_progress(text: str, address: int) -> tuple[str, bool]:
    """Canonicalize only the selected lift's existing progress metadata."""

    if count_function_metadata(text) <= 1:
        select_lift_metadata(text, address)
        return canonical_exact_progress(text)
    records = parse_function_records(text, validate_progress=False)
    record = next((item for item in records if item.address == address), None)
    if record is None:
        raise ValueError(f"no function metadata for 0x{address:08X}")
    metadata, changed = canonical_exact_progress(record.metadata)
    if not changed:
        return text, False
    return (
        text[: record.metadata_start] + metadata + text[record.metadata_end :],
        True,
    )


def require_single_source(source: Path) -> None:
    """Keep unmigrated native/mutation owners from accepting a combined source."""

    require_single_function(source.read_text(encoding="utf-8"))


def validate_single_source_changes(changes: object) -> None:
    """Reject proposed grouped C images before installation by legacy owners."""

    if isinstance(changes, dict):
        for name, content in changes.items():
            if (
                isinstance(name, str)
                and name.endswith(".c")
                and isinstance(content, str)
            ):
                require_single_function(content)
