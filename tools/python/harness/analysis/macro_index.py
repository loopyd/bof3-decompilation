"""Deterministic target-qualified indexing for existing C macros and templates."""

from __future__ import annotations

import hashlib
import json
import sqlite3
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..discovery import file_sha256
from ..domain.claims import manifest_header_paths, manifest_source_paths
from ..domain.macro_facts import parse_macro_definitions, parse_macro_uses

SHARED_MACRO_HEADERS = (
    Path("include/base/barrier.h"),
    Path("include/bof3/asm.h"),
    Path("include/bof3/symbols.h"),
    Path("include/include_asm.h"),
)


@dataclass(frozen=True)
class MacroInput:
    """One owned macro registry input and its provenance."""

    path: Path
    owner: str
    provenance: str
    generated_psyq: bool = False


def _relative(root: Path, path: Path) -> str:
    return path.resolve().relative_to(root.resolve()).as_posix()


def shared_template_paths(root: Path) -> list[Path]:
    """Return only explicitly shared source templates."""

    directory = root / "src/shared"
    return sorted(directory.rglob("*.inc")) if directory.is_dir() else []


def macro_inputs(root: Path, target: str, manifest: Any) -> list[MacroInput]:
    """Return target claims plus the bounded shared macro owner set."""

    rows = [
        MacroInput(path, target, "header_claim")
        for path in manifest_header_paths(root, manifest)
    ]
    if manifest.has_explicit_sources:
        for path in manifest_source_paths(root, manifest):
            rows.append(
                MacroInput(
                    path,
                    target,
                    "source_claim",
                    bool(manifest.psyq_source)
                    and _relative(root, path) == manifest.psyq_source,
                )
            )
    rows.extend(
        MacroInput(root / relative, "__shared__", "sanctioned_helper")
        for relative in SHARED_MACRO_HEADERS
        if (root / relative).is_file()
    )
    rows.extend(
        MacroInput(path, "__shared__", "shared_template")
        for path in shared_template_paths(root)
    )
    unique = {
        (row.owner, row.path.resolve()): row
        for row in rows
        if row.path.suffix in {".c", ".h", ".inc"}
    }
    missing = sorted(
        _relative(root, row.path) for row in unique.values() if not row.path.is_file()
    )
    if missing:
        raise ValueError(f"missing claimed macro inputs for {target}: {missing}")
    return sorted(
        unique.values(), key=lambda row: (row.owner, _relative(root, row.path))
    )


def macro_input_rows(
    root: Path, target: str, manifest: Any
) -> list[tuple[str, str, str, str]]:
    """Return deterministic input fingerprints for one target."""

    return sorted(
        (
            _relative(root, row.path),
            file_sha256(row.path),
            row.provenance,
            row.owner,
        )
        for row in macro_inputs(root, target, manifest)
    )


def macro_input_digest(inputs: list[tuple[str, str, str, str]]) -> str:
    payload = "\n".join("\0".join(row) for row in inputs).encode()
    return hashlib.sha256(payload).hexdigest()


def _definition_id(owner: str, source_path: str, line: int, name: str) -> str:
    return f"{owner}:{source_path}:{line}:{name}"


def _insert_definition(
    connection: sqlite3.Connection,
    root: Path,
    row: MacroInput,
    definition: Any,
) -> str:
    source_path = _relative(root, row.path)
    definition_id = _definition_id(
        row.owner, source_path, definition.source_line, definition.name
    )
    generated = row.generated_psyq or definition.generated
    restrictions = list(definition.restrictions)
    if row.generated_psyq and "generator_owned" not in restrictions:
        restrictions.append("generator_owned")
    candidate_status = "noncandidate" if generated else "existing"
    connection.execute(
        "INSERT INTO macro_definitions VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        (
            definition_id,
            row.owner,
            definition.name,
            source_path,
            definition.source_line,
            json.dumps(definition.parameters),
            definition.body,
            json.dumps(definition.conditions),
            definition.classification,
            row.provenance,
            json.dumps(restrictions),
            int(generated),
            candidate_status,
            file_sha256(row.path),
            definition.diagnostic,
        ),
    )
    return definition_id


def _insert_template(
    connection: sqlite3.Connection,
    definition_id: str,
    owner: str,
    source_path: str,
    name: str,
    source_sha256: str,
) -> None:
    connection.execute(
        "INSERT OR IGNORE INTO macro_templates VALUES (?, ?, ?, ?, ?, ?, ?)",
        (
            definition_id,
            owner,
            source_path,
            name,
            "body_emitting_inc",
            "target_local_wrapper_required",
            source_sha256,
        ),
    )


def insert_macro_registry(
    connection: sqlite3.Connection, root: Path, target: str, manifest: Any
) -> None:
    """Populate definitions, templates, uses, and fingerprints for one target."""

    inputs = macro_inputs(root, target, manifest)
    definitions: dict[str, list[str]] = {}
    definition_names: set[str] = set()
    for definition_id, name in connection.execute(
        "SELECT id, name FROM macro_definitions WHERE owner_target = '__shared__'"
    ):
        definitions.setdefault(name, []).append(definition_id)
        definition_names.add(name)
    for row in inputs:
        source_path = _relative(root, row.path)
        connection.execute(
            "INSERT OR IGNORE INTO macro_input_fingerprints VALUES (?, ?, ?, ?, ?)",
            (target, source_path, file_sha256(row.path), row.provenance, row.owner),
        )
        text = row.path.read_text(encoding="utf-8", errors="replace")
        for definition in parse_macro_definitions(text, source=row.path):
            definition_id = _definition_id(
                row.owner,
                _relative(root, row.path),
                definition.source_line,
                definition.name,
            )
            if (
                connection.execute(
                    "SELECT 1 FROM macro_definitions WHERE id = ?", (definition_id,)
                ).fetchone()
                is None
            ):
                _insert_definition(connection, root, row, definition)
            if definition.classification == "body_emitting_template":
                _insert_template(
                    connection,
                    definition_id,
                    row.owner,
                    _relative(root, row.path),
                    definition.name,
                    file_sha256(row.path),
                )
            definitions.setdefault(definition.name, []).append(definition_id)
            definitions[definition.name] = sorted(set(definitions[definition.name]))
            definition_names.add(definition.name)
    for row in inputs:
        text = row.path.read_text(encoding="utf-8", errors="replace")
        source_path = _relative(root, row.path)
        function_id = connection.execute(
            "SELECT id FROM functions WHERE target_id = ? AND source = ?",
            (target, source_path),
        ).fetchone()
        for use in parse_macro_uses(text, definition_names):
            for definition_id in definitions[use.name]:
                definition = connection.execute(
                    "SELECT generated, candidate_status, restrictions "
                    "FROM macro_definitions WHERE id = ?",
                    (definition_id,),
                ).fetchone()
                generated = row.generated_psyq or use.generated or bool(definition[0])
                restrictions = json.loads(definition[2])
                if generated and "generator_owned" not in restrictions:
                    restrictions.append("generator_owned")
                connection.execute(
                    "INSERT OR IGNORE INTO macro_uses VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                    (
                        target,
                        definition_id,
                        use.name,
                        source_path,
                        use.source_line,
                        use.source_column,
                        use.arguments,
                        json.dumps(use.conditions),
                        use.context,
                        function_id[0] if function_id is not None else None,
                        int(generated),
                        "noncandidate" if generated else definition[1],
                        json.dumps(restrictions),
                    ),
                )


__all__ = [
    "SHARED_MACRO_HEADERS",
    "insert_macro_registry",
    "macro_input_digest",
    "macro_input_rows",
    "macro_inputs",
    "shared_template_paths",
]
