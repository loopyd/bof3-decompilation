"""Read scoped Markdown snapshots and derive bounded context or literal searches."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

from harness.common.files import read_file
from harness.docs.paths import validate_document_paths


def read_documents(root: Path, paths: list[str]) -> list[dict]:
    documents = []
    for name in validate_document_paths(root, paths):
        original = read_file(root, name)
        text = original.decode("utf-8")
        documents.append(
            {
                "path": name,
                "sha256": hashlib.sha256(original).hexdigest(),
                "bytes": len(original),
                "total_lines": len(text.splitlines()),
                "text": text,
            }
        )
    return documents


def build_context(documents: list[dict], *, start_line: int, lines: int) -> list[dict]:
    if start_line < 1 or lines < 1:
        raise ValueError("context start-line and lines must be positive")
    result = []
    for document in documents:
        content = document["text"].splitlines(keepends=True)
        if start_line > max(1, len(content)):
            raise ValueError(f"context start-line exceeds {document['path']}")
        selected = content[start_line - 1 : start_line - 1 + lines]
        result.append(
            {
                **document,
                "start_line": start_line,
                "end_line": start_line + len(selected) - 1,
                "partial": start_line > 1 or len(selected) < len(content),
                "text": "".join(selected),
            }
        )
    return result


def search_documents(
    documents: list[dict], pattern: str, *, ignore_case: bool, limit: int
) -> dict:
    if not pattern or "\n" in pattern or "\r" in pattern or limit < 1:
        raise ValueError(
            "search needs a nonempty single-line literal and positive limit"
        )
    needle = pattern.casefold() if ignore_case else pattern
    matches = []
    total = 0
    for document in documents:
        for line_number, line in enumerate(document["text"].splitlines(), 1):
            if needle not in (line.casefold() if ignore_case else line):
                continue
            total += 1
            if len(matches) < limit:
                matches.append(
                    {"path": document["path"], "line": line_number, "text": line}
                )
    return {
        "documents": [
            {key: value for key, value in item.items() if key != "text"}
            for item in documents
        ],
        "pattern": pattern,
        "ignore_case": ignore_case,
        "matching_lines": total,
        "truncated": total > len(matches),
        "matches": matches,
    }


def prepare_edit(documents: list[dict], expected_sha256: str | None) -> list[dict]:
    if expected_sha256 is not None:
        if (
            len(documents) != 1
            or re.fullmatch(r"[0-9a-f]{64}", expected_sha256) is None
        ):
            raise ValueError("expected-sha256 requires one document and a SHA-256 pin")
        if documents[0]["sha256"] != expected_sha256:
            raise ValueError("documentation input fingerprint drifted")
    return documents
