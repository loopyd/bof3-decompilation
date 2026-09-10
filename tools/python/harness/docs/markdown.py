"""Extract positioned Markdown links without reading targets or rendering documents."""

from __future__ import annotations

import bisect
import html
import re
import string


HTML_TAG = re.compile(r"<[A-Za-z][\w-]*(?=[\s/>])(?:[^\"'<>]|\"[^\"]*\"|'[^']*')*>")


def parse_attributes(tag: str) -> list[tuple[str, str, int]]:
    attributes = []
    for match in re.finditer(
        r"(?<=\s)([^\s/=<>\"']+)(?:\s*=\s*(?:\"([^\"]*)\"|'([^']*)'|([^\s>]+)))?",
        tag,
    ):
        value = next((value for value in match.groups()[1:] if value is not None), None)
        if value is not None:
            attributes.append((match[1].lower(), value, match.start()))
    return attributes


def mask_text(text: str) -> str:
    return re.sub(r"[^\r\n]", " ", text)


def mask_blocks(text: str) -> str:
    text = re.sub(r"<!--[\s\S]*?(?:-->|\Z)", lambda match: mask_text(match[0]), text)
    result = []
    fence = None
    indented = False
    previous_blank = True
    list_indent = 0
    for line in text.splitlines(keepends=True):
        content = re.sub(r"^(?: {0,3}> ?)+", "", line)
        if list_indent and content.startswith(" " * list_indent):
            content = content[list_indent:]
        elif content.strip():
            list_indent = 0
        item = re.match(r"^ {0,3}(?:[-+*]|\d{1,9}[.)])[ \t]+", content)
        if item and fence is None:
            list_indent += item.end()
            content = content[item.end() :]
        marker = re.match(r"^ {0,3}(`{3,}|~{3,})(.*)$", content.rstrip("\r\n"))
        if fence is not None:
            result.append(mask_text(line))
            if (
                marker
                and marker[1][0] == fence[0]
                and len(marker[1]) >= len(fence)
                and not marker[2].strip()
            ):
                fence = None
        elif marker and not (marker[1][0] == "`" and "`" in marker[2]):
            fence = marker[1]
            result.append(mask_text(line))
        elif (content.startswith("    ") or content.startswith("\t")) and (
            previous_blank or indented
        ):
            indented = True
            result.append(mask_text(line))
        else:
            result.append(line)
            indented = indented and not content.strip()
        previous_blank = not content.strip()
    return "".join(result)


def mask_spans(text: str) -> str:
    result = list(text)
    position = 0
    while position < len(text):
        if text[position] == "\\":
            position += 2
            continue
        if text[position] != "`":
            position += 1
            continue
        marker = re.match(r"`+", text[position:])[0]
        start = position + len(marker)
        closing = re.search(r"(?<!`)" + re.escape(marker) + r"(?!`)", text[start:])
        if closing:
            end = start + closing.end()
            result[position:end] = mask_text(text[position:end])
            position = end
        else:
            position = start
    return "".join(result)


def decode_text(text: str) -> str:
    return html.unescape(
        re.sub(r"\\([" + re.escape(string.punctuation) + r"])", r"\1", text)
    )


def normalize_label(text: str) -> str:
    return " ".join(decode_text(text).split()).casefold()


def find_closing(text: str, start: int, opening: str, closing: str) -> int | None:
    depth = 1
    position = start + 1
    while position < len(text):
        character = text[position]
        if character == "\\":
            position += 2
            continue
        if character == opening:
            depth += 1
        elif character == closing:
            depth -= 1
            if depth == 0:
                return position
        position += 1
    return None


def parse_destination(text: str, start: int) -> tuple[str, int] | None:
    position = start
    while position < len(text) and text[position].isspace():
        position += 1
    if re.search(r"\n[ \t\r]*\n", text[start:position]):
        return None
    if position < len(text) and text[position] == "<":
        begin = position + 1
        position = begin
        while position < len(text):
            if text[position] == "\\":
                position += 2
                continue
            if text[position] == ">":
                return text[begin:position], position + 1
            if text[position] in "\r\n<":
                return None
            position += 1
        return None
    begin = position
    depth = 0
    while position < len(text):
        character = text[position]
        if character == "\\":
            position += 2
            continue
        if character.isspace() or (character == ")" and depth == 0):
            break
        if character == "(":
            depth += 1
        elif character == ")":
            depth -= 1
        position += 1
    if depth:
        return None
    return text[begin:position], position


def parse_inline(text: str, start: int) -> tuple[str, int] | None:
    parsed = parse_destination(text, start + 1)
    if parsed is None:
        return None
    destination, end = parsed
    position = end
    while position < len(text) and text[position].isspace():
        position += 1
    if position > end and position < len(text) and text[position] in "\"'(":
        quote = text[position]
        if quote == "(":
            closing = find_closing(text, position, "(", ")")
        else:
            match = re.match(r"[\s\S]*?(?<!\\)" + quote, text[position + 1 :])
            closing = position + match.end() if match else None
        if closing is None:
            return None
        position = closing + 1
        while position < len(text) and text[position].isspace():
            position += 1
    if position < len(text) and text[position] == ")":
        return destination, position + 1
    return None


def collect_links(text: str) -> list[dict]:
    visible = mask_spans(mask_blocks(text))
    lines = [0, *(match.end() for match in re.finditer("\n", text))]
    links = []
    definitions = {}
    body = list(visible)

    def append_link(
        position: int, kind: str, destination: str | None, **extra: object
    ) -> None:
        line = bisect.bisect_right(lines, position)
        links.append(
            {
                "line": line,
                "column": position - lines[line - 1] + 1,
                "kind": kind,
                "destination": destination,
                **extra,
            }
        )

    for match in re.finditer(
        r"(?m)^ {0,3}\[((?:\\.|[^\]\\\n])+)\]:[ \t]*(?:\n[ \t]*)?", visible
    ):
        if match[1].startswith("^") or text[match.end() : match.end() + 1] in {
            "",
            "\r",
            "\n",
        }:
            continue
        parsed = parse_destination(text, match.end())
        if parsed is None or not parsed[0]:
            continue
        destination, end = parsed
        label = normalize_label(match[1])
        definitions.setdefault(
            label, (destination, bisect.bisect_right(lines, match.start()))
        )
        append_link(match.start(), "definition", destination, label=label)
        line_end = text.find("\n", end)
        line_end = len(text) if line_end < 0 else line_end
        body[match.start() : line_end] = mask_text(visible[match.start() : line_end])
    visible = "".join(body)
    position = 0
    while position < len(visible):
        character = visible[position]
        if character == "\\":
            position += 2
            continue
        if character == "<":
            tag = HTML_TAG.match(visible, position)
            if tag:
                for name, value, offset in parse_attributes(text[position : tag.end()]):
                    if name in {"href", "src"}:
                        append_link(position + offset, "html", value)
                position = tag.end()
                continue
            auto = re.match(
                r"<([A-Za-z][A-Za-z0-9+.-]*:[^\s<>]*|[^\s<>@]+@[^\s<>@]+)>",
                visible[position:],
            )
            if auto:
                destination = auto[1] if ":" in auto[1] else "mailto:" + auto[1]
                append_link(position, "autolink", destination)
                position += auto.end()
                continue
        if character != "[":
            position += 1
            continue
        closing = find_closing(visible, position, "[", "]")
        if closing is None:
            position += 1
            continue
        label = text[position + 1 : closing]
        end = closing + 1
        kind = "image" if position and visible[position - 1] == "!" else "link"
        parsed = parse_inline(text, end) if visible[end : end + 1] == "(" else None
        if parsed is not None:
            append_link(position, kind, parsed[0])
            visible = (
                visible[:closing]
                + mask_text(visible[closing : parsed[1]])
                + visible[parsed[1] :]
            )
            position += 1
            continue
        explicit = visible[end : end + 1] == "["
        if explicit:
            reference_end = find_closing(visible, end, "[", "]")
            if reference_end is None:
                position = end
                continue
            reference = text[end + 1 : reference_end] or label
            end = reference_end + 1
        else:
            reference = label
        normalized = normalize_label(reference)
        if normalized in definitions or explicit:
            destination, definition_line = definitions.get(normalized, (None, None))
            append_link(
                position,
                kind + "-reference",
                destination,
                label=normalized,
                definition_line=definition_line,
            )
        visible = visible[:closing] + mask_text(visible[closing:end]) + visible[end:]
        position += 1
    return sorted(links, key=lambda item: (item["line"], item["column"]))
