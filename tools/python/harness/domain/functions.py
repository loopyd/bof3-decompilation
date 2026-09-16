"""Leading metadata and lexical implementation ranges for authored C functions."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from harness.common.lexicon import iter_c_lexemes

from .tags import (
    SOURCE_TAG_RE,
    count_function_metadata,
    lift_lifecycle,
    parse_behavior_tag,
    parse_progress_tags,
    parse_source_tag,
    require_single_function,
)

_IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_DIRECTIVE = re.compile(r"^[ \t]*#([^\n]*)(?:\n|$)", re.MULTILINE)
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


def _find_closing(masked: str, start: int, opening: str, closing: str) -> int:
    depth = 0
    for position in range(start, len(masked)):
        character = masked[position]
        depth += (character == opening) - (character == closing)
        if depth == 0:
            return position + 1
    raise ValueError(f"unterminated implementation delimiter {opening!r}")


def _parse_implementation(masked: str, start: int) -> tuple[int, int, str, str]:
    start += len(masked[start:]) - len(masked[start:].lstrip())
    opening = masked.find("(", start)
    if opening < 0:
        raise ValueError("function metadata has no following implementation")
    prefix = masked[start:opening].strip()
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

    Ordinary ANSI C89 definitions and standalone template invocations are supported.
    Conditional source, continued lines and unsupported declarators reject rather
    than assign ambiguous ranges. Data declaration tags without behavior are ignored.
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
    if re.search(r"\\\r?\n", text):
        raise ValueError("continued source requires explicit preprocessing review")
    directives = list(_DIRECTIVE.finditer(masked))
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
        gap = _DIRECTIVE.sub("", masked[previous_end : comment.start()]).strip()
        if gap and gap[-1] != ";":
            raise ValueError("function metadata interrupts an unfinished declaration")
        preceding = _DIRECTIVE.sub("", masked[: comment.start()])
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
        start, end, kind, spelling = _parse_implementation(masked, comment.end())
        if any(start <= item.start() < end for item in directives):
            raise ValueError("implementation directives require preprocessing review")
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
    return tuple(records)


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
