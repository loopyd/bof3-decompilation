"""Exact-name catalog of declarations in the official Psy-Q header baseline.

This is deliberately a lexical catalog, not a C parser.  It records only
complete, unambiguous declarations and never invents aliases for private SDK
object labels.  The scanner can therefore attach header evidence without
turning a missing declaration into negative provenance evidence.
"""

from __future__ import annotations

import json
from pathlib import Path
import re
from typing import Any


HEADER_SCHEMA = "bof3.psyq-headers/v2"
_IDENTIFIER = r"[A-Za-z_]\w*"
_DEFINE_RE = re.compile(rf"^\s*#\s*define\s+({_IDENTIFIER})(?=\s|\(|$)")
_TYPEDEF_NAME_RE = re.compile(rf"({_IDENTIFIER})\s*$")
_STRUCT_TAG_RE = re.compile(rf"^\s*(?:struct|union|enum)\s+({_IDENTIFIER})\s*(?:\{{|;)")
_FUNCTION_RE = re.compile(rf"({_IDENTIFIER})\s*\(")
_VARIABLE_RE = re.compile(rf"\b({_IDENTIFIER})\s*(?:\[[^]]*\])?\s*;")


def _without_comments(text: str) -> str:
    """Remove comments while retaining newlines for stable source locations."""

    text = re.sub(
        r"/\*.*?\*/", lambda match: "\n" * match.group(0).count("\n"), text, flags=re.S
    )
    return re.sub(r"//[^\n]*", "", text)


def _record(
    *, name: str, kind: str, source: str, line: int, declaration: str
) -> dict[str, str | int]:
    return {
        "name": name,
        "kind": kind,
        "header": source,
        "line": line,
        "declaration": " ".join(declaration.split()),
    }


def _statements(text: str) -> list[tuple[int, str]]:
    """Return semicolon-terminated declarations outside preprocessor lines."""

    rows: list[tuple[int, str]] = []
    pending = ""
    start_line: int | None = None
    brace_depth = 0
    preprocessor_continues = False
    for line_number, line in enumerate(_without_comments(text).splitlines(), start=1):
        if preprocessor_continues:
            preprocessor_continues = line.rstrip().endswith("\\")
            continue
        if not pending and line.lstrip().startswith("#"):
            preprocessor_continues = line.rstrip().endswith("\\")
            continue
        # C++ linkage guards wrap declarations but are not declarations.
        if not pending and line.strip() in {'extern "C" {', "}"}:
            continue
        if not pending and not line.strip():
            continue
        if not pending:
            start_line = line_number
        for character in line:
            pending += character
            if character == "{":
                brace_depth += 1
            elif character == "}" and brace_depth:
                brace_depth -= 1
            elif character == ";" and brace_depth == 0:
                rows.append((start_line or line_number, pending))
                pending = ""
                start_line = None
        if pending:
            pending += "\n"
    return rows


def _declarations(text: str, source: str) -> list[dict[str, str | int]]:
    records: list[dict[str, str | int]] = []
    for line_number, line in enumerate(_without_comments(text).splitlines(), start=1):
        match = _DEFINE_RE.match(line)
        if match:
            records.append(
                _record(
                    name=match.group(1),
                    kind="macro",
                    source=source,
                    line=line_number,
                    declaration=line,
                )
            )

        tag = _STRUCT_TAG_RE.match(line)
        if tag:
            records.append(
                _record(
                    name=tag.group(1),
                    kind="type",
                    source=source,
                    line=line_number,
                    declaration=line,
                )
            )

    for line_number, statement in _statements(text):
        compact = " ".join(statement.split())
        if compact.startswith("typedef "):
            # Function-pointer typedefs are types too; the final identifier is
            # their public name.  Anonymous structs/unions also end in alias.
            tail = compact[:-1]
            candidates = re.findall(_IDENTIFIER, tail)
            if candidates:
                records.append(
                    _record(
                        name=candidates[-1],
                        kind="type",
                        source=source,
                        line=line_number,
                        declaration=statement,
                    )
                )
            continue
        if "(" in compact:
            # A normal prototype has its exported name immediately before its
            # first parameter list.  Function-pointer variables are excluded
            # because their name is preceded by `(*`, not a declaration name.
            match = _FUNCTION_RE.search(compact)
            if match and not re.search(
                rf"\(\s*\*\s*{re.escape(match.group(1))}\s*\)", compact
            ):
                records.append(
                    _record(
                        name=match.group(1),
                        kind="function",
                        source=source,
                        line=line_number,
                        declaration=statement,
                    )
                )
            continue
        if compact.startswith("extern "):
            candidates = _VARIABLE_RE.findall(compact)
            if candidates:
                records.append(
                    _record(
                        name=candidates[-1],
                        kind="variable",
                        source=source,
                        line=line_number,
                        declaration=statement,
                    )
                )
    return records


def parse_headers(include_root: Path) -> dict[str, Any]:
    """Build a stable, exact-name declaration catalog from official headers."""

    records: list[dict[str, str | int]] = []
    for path in sorted(include_root.rglob("*.h")):
        if not path.is_file():
            continue
        records.extend(
            _declarations(
                path.read_text(encoding="utf-8", errors="replace"),
                path.relative_to(include_root).as_posix(),
            )
        )
    records.sort(
        key=lambda row: (
            str(row["name"]),
            str(row["kind"]),
            str(row["header"]),
            int(row["line"]),
            str(row["declaration"]),
        )
    )
    # Same declaration can be reachable through compatibility headers.  Keep
    # one deterministic record, but preserve genuinely distinct declarations.
    unique: list[dict[str, str | int]] = []
    for record in records:
        if not unique or record != unique[-1]:
            unique.append(record)
    return {"schema": HEADER_SCHEMA, "declarations": unique}


def declarations_by_name(
    catalog: dict[str, Any],
) -> dict[str, tuple[dict[str, str | int], ...]]:
    """Index a validated catalog for exact, case-sensitive lookup."""

    if catalog.get("schema") != HEADER_SCHEMA or not isinstance(
        catalog.get("declarations"), list
    ):
        raise ValueError("invalid Psy-Q header catalog")
    indexed: dict[str, list[dict[str, str | int]]] = {}
    for record in catalog["declarations"]:
        if not isinstance(record, dict) or not isinstance(record.get("name"), str):
            raise ValueError("invalid Psy-Q header declaration")
        indexed.setdefault(record["name"], []).append(record)
    return {name: tuple(records) for name, records in indexed.items()}


def declaration_for(catalog: dict[str, Any], name: str) -> dict[str, str | int] | None:
    """Return one declaration only when an exact name has one clear kind."""

    return declaration_from_index(declarations_by_name(catalog), name)


def declaration_from_index(
    indexed: dict[str, tuple[dict[str, str | int], ...]], name: str
) -> dict[str, str | int] | None:
    """Look up an exact declaration in a previously built catalog index."""

    records = indexed.get(name, ())
    # Different compatibility headers may declare the same function.  Their
    # kind is still clear, so choose the first stable catalog entry.
    kinds = {str(record["kind"]) for record in records}
    return dict(records[0]) if len(kinds) == 1 and records else None


def index_headers(root: Path, version: str) -> Path:
    """Write the generated catalog; callers never edit this disposable output."""

    include_root = root / "toolchains" / "psyq" / version / "include"
    payload = (
        parse_headers(include_root)
        if include_root.is_dir()
        else {"schema": HEADER_SCHEMA, "declarations": []}
    )
    output = root / "out" / "psyq" / version / "headers.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return output
