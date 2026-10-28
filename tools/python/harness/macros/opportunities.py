"""Current lexical repetition locations; semantic selection belongs to the skill."""

from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
import hashlib
from pathlib import Path
import re
import sqlite3

from harness.common.lexicon import mask_c_text
from .facts import _GENERATED


@dataclass(frozen=True)
class SourceInput:
    target: str
    source_path: str
    text: str
    sha256: str


def _source_inputs(
    connection: sqlite3.Connection, root: Path, target: str | None
) -> list[SourceInput]:
    clauses = "WHERE m.owner_target = m.target_id"
    params: list[object] = []
    if target:
        clauses += " AND m.target_id = ?"
        params.append(target)
    rows = connection.execute(
        "SELECT m.target_id, m.source_path, m.sha256, m.input_kind "
        f"FROM macro_input_fingerprints m {clauses} "
        "ORDER BY m.target_id, m.source_path",
        params,
    ).fetchall()
    result: list[SourceInput] = []
    root_resolved = root.resolve()
    for target_id, source_path, digest, input_kind in rows:
        path = (root / source_path).resolve()
        if not path.is_relative_to(root_resolved) or not path.is_file():
            raise ValueError(
                f"stale macro opportunity source: {target_id}:{source_path}"
            )
        content = path.read_bytes()
        if hashlib.sha256(content).hexdigest() != digest:
            raise ValueError(
                f"stale macro opportunity source: {target_id}:{source_path}"
            )
        text = (
            content.decode("utf-8", errors="replace")
            .replace("\r\n", "\n")
            .replace("\r", "\n")
        )
        if not _GENERATED.search(text):
            result.append(SourceInput(target_id, source_path, text, digest))
    return result


def repetition_rows(
    connection: sqlite3.Connection, root: Path, target: str
) -> list[dict]:
    groups = defaultdict(list)
    for source in _source_inputs(connection, root, target):
        text = mask_c_text(source.text)
        # Preprocessor bodies are already available as definition facts.
        text = re.sub(
            r"(?m)^\s*#[^\n]*(?:\\\n[^\n]*)*",
            lambda m: re.sub(r"[^\n]", " ", m.group()),
            text,
        )
        patterns = {
            "constant": r"\b(?:0[xX][0-9A-Fa-f]+|[0-9]+)[uUlL]*\b",
            "member_access": r"\b[A-Za-z_]\w*\s*(?:->|\.)\s*[A-Za-z_]\w*",
            "statement": r"[^;{}]+;",
        }
        for kind, pattern in patterns.items():
            for match in re.finditer(pattern, text):
                value = " ".join(match.group().split())
                if not value or value == ";":
                    continue
                leading = len(match.group()) - len(match.group().lstrip())
                groups[(kind, value)].append(
                    {
                        "target": source.target,
                        "source_path": source.source_path,
                        "source_line": text.count("\n", 0, match.start() + leading) + 1,
                        "source_sha256": source.sha256,
                    }
                )
    rows = []
    for (kind, spelling), locations in sorted(groups.items()):
        if len(locations) >= 2:
            rows.append(
                {
                    "kind": kind,
                    "spelling": spelling,
                    "count": len(locations),
                    "locations": locations,
                }
            )
    return rows
