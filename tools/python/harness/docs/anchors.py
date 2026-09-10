"""Collect Markdown heading slugs and explicit HTML anchor identifiers."""

from __future__ import annotations

import re
import unicodedata

from harness.docs.markdown import (
    HTML_TAG,
    decode_text,
    mask_blocks,
    mask_spans,
    parse_attributes,
)


def collect_anchors(text: str) -> set[str]:
    visible = mask_blocks(text)
    anchors = set()
    for tag in HTML_TAG.finditer(mask_spans(visible)):
        for name, value, offset in parse_attributes(tag[0]):
            if name == "id" or (name == "name" and re.match(r"<a\b", tag[0], re.I)):
                anchors.add(decode_text(value))
    used = set()
    lines = visible.splitlines()
    for index, line in enumerate(lines):
        line = re.sub(r"^(?: {0,3}> ?)+", "", line)
        heading = re.match(r"^ {0,3}#{1,6}(?:[ \t]+(.*?)|[ \t]*)$", line)
        if heading:
            title = re.sub(r"[ \t]+#+[ \t]*$", "", heading[1] or "")
        elif (
            line.strip()
            and index + 1 < len(lines)
            and re.fullmatch(r" {0,3}(?:=+|-+)[ \t]*", lines[index + 1])
        ):
            title = line.strip()
        else:
            continue
        title = re.sub(r"!?\[([^\]]*)\]\([^)]*\)", r"\1", title)
        title = re.sub(r"\[([^\]]*)\]\[[^\]]*\]", r"\1", title)
        title = re.sub(r"<[^>]+>", "", title)
        title = decode_text(title).lower().replace(" ", "-")
        base = "".join(
            character
            for character in title
            if character in "-_"
            or not unicodedata.category(character).startswith(("P", "S", "C"))
        )
        slug = base
        suffix = 0
        while slug in used:
            suffix += 1
            slug = f"{base}-{suffix}"
        used.add(slug)
        anchors.add(slug)
    return anchors
