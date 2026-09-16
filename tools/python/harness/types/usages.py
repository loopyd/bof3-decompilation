"""Fingerprint-bound lexical type spellings with conservative function ownership."""

from __future__ import annotations

from bisect import bisect_right
import hashlib
import json
from pathlib import Path
import re
import sqlite3

from harness.common.files import read_file
from harness.common.lexicon import iter_c_lexemes
from harness.domain.claims import manifest_source_paths
from harness.domain.functions import parse_function_records

_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
_LIMIT = 4 * 1024 * 1024


def _resolve_functions(connection, target, source, path, text):
    try:
        records = parse_function_records(text, validate_progress=False)
    except ValueError as error:
        return (), {}, str(error)
    functions = connection.execute(
        "SELECT id, address FROM functions WHERE target_id = ? AND source IN (?, ?)",
        (target, source, path.as_posix()),
    ).fetchall()
    identities = {}
    for identity, address in functions:
        if type(address) is not int or not 0 <= address <= 0xFFFFFFFF:
            raise ValueError(f"invalid indexed type-use address: {source}")
        if identity != f"{target}@{address:08x}" or address in identities:
            raise ValueError(f"ambiguous indexed type-use identity: {source}")
        identities[address] = identity
    return records, identities, None


def _collect_occurrences(text, names, records, identities):
    masked = list(text)
    for match in iter_c_lexemes(text):
        masked[match.start() : match.end()] = [
            "\n" if character == "\n" else " " for character in match.group()
        ]
    offsets = [0] + [match.end() for match in re.finditer("\n", text)]
    starts = [record.implementation_start for record in records]
    occurrences = {}
    for match in _IDENTIFIER.finditer("".join(masked)):
        name = match.group()
        if name not in names:
            continue
        position = match.start()
        member = bisect_right(starts, position) - 1
        identity = None
        if member >= 0 and position < records[member].implementation_end:
            identity = identities.get(records[member].address)
        line = bisect_right(offsets, position)
        occurrences.setdefault((identity, name), []).append(
            {"line": line, "column": position - offsets[line - 1] + 1}
        )
    return occurrences


def insert_source_usages(
    connection: sqlite3.Connection, root: Path, target: str, manifest, *, files=None
) -> None:
    """Index candidate spellings, never semantic bindings or native type evidence."""

    if not manifest.has_explicit_sources:
        return
    names = {
        row[0]
        for row in connection.execute(
            "SELECT name FROM type_declarations WHERE target_id IN (?, '__shared__') "
            "AND kind IN ('typedef', 'struct', 'union', 'enum') "
            "AND review_status = 'reviewed'",
            (target,),
        )
        if re.fullmatch(r"[A-Za-z_]\w*", row[0])
    }
    for path in manifest_source_paths(root, manifest):
        if path.suffix != ".c":
            continue
        source = path.relative_to(root).as_posix()
        if files is None:
            raw = read_file(root, source, max_bytes=_LIMIT)
        else:
            if files.canonical(path) != path:
                raise ValueError(f"transaction path is unsafe: {source}")
            raw = files.read(path)
            if len(raw) > _LIMIT:
                raise ValueError(f"transaction file exceeds capture limit: {source}")
        fingerprint = hashlib.sha256(raw).hexdigest()
        expected = connection.execute(
            "SELECT sha256, input_kind FROM type_input_fingerprints "
            "WHERE target_id = ? AND source_path = ?",
            (target, source),
        ).fetchone()
        if expected is None or tuple(expected) != (fingerprint, "source"):
            raise ValueError(f"type-use source differs from indexed inputs: {source}")
        text = raw.decode("utf-8").replace("\r\n", "\n").replace("\r", "\n")
        records, identities, diagnostic = _resolve_functions(
            connection, target, source, path, text
        )
        occurrences = _collect_occurrences(text, names, records, identities)
        for (identity, name), positions in occurrences.items():
            evidence = json.dumps(
                {
                    "class": "lexical",
                    "binding": "unresolved",
                    "scope": "function" if identity else "source-context",
                    "positions": positions,
                    "source_sha256": fingerprint,
                    "metadata_diagnostic": diagnostic,
                },
                sort_keys=True,
            )
            connection.execute(
                "INSERT OR IGNORE INTO type_usages VALUES "
                "(?, ?, ?, ?, ?, 'lexical', NULL, 'source_claim', ?)",
                (target, source, name, identity, name, evidence),
            )
