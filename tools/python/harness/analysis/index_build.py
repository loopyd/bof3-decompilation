"""Reverse-index build: per-target records, duplicate lifecycle, and grouping.

Owns the `rebuild` write path. Domain modules own all byte/tag parsing:
PS-X payload and reviewed-range hashing (`domain.psx`), MIPS lui/%lo and
static-JAL decoding plus the trivial classifier (`domain.mips`), and lift
lifecycle derivation from progress tags (`domain.tags`).
"""

from __future__ import annotations

import os
import sqlite3
import tempfile
from pathlib import Path

from ..discovery import file_sha256
from ..domain import load_target_manifests, lift_lifecycle
from ..domain.claims import resolve_source_for_paths
from ..domain.layout import parse_splat_layout
from ..domain.mips import data_references, trivial_kind
from ..domain.psx import payload_for
from ..domain.sources import reviewed_function_name
from ..domain.symbols import load_target_symbols
from .index import SCHEMA_VERSION, index_path
from .index_groups import insert_duplicate_groups, insert_unconfirmed_candidates
from .index_snapshot import snapshot_for
from .schema import create_schema
from .project import prepare_target
from .type_index import (
    infer_type_candidates,
    insert_authored_types,
    insert_shared_scalar_types,
)
from .type_inputs import type_input_digest, type_input_rows
from .macro_index import insert_macro_registry, macro_input_digest, macro_input_rows


def _validate_candidate(path: Path, expected_targets: set[str]) -> None:
    """Reject an incomplete or corrupt candidate before atomic publication."""

    connection = sqlite3.connect(path)
    try:
        integrity = connection.execute("PRAGMA integrity_check").fetchone()
        if integrity != ("ok",):
            raise ValueError(f"reverse index integrity check failed: {integrity}")
        foreign_keys = connection.execute("PRAGMA foreign_key_check").fetchall()
        if foreign_keys:
            raise ValueError(f"reverse index foreign key check failed: {foreign_keys}")
        schema = connection.execute(
            "SELECT value FROM metadata WHERE key = 'schema'"
        ).fetchone()
        expected_tables = {
            "metadata",
            "targets",
            "symbols",
            "functions",
            "calls",
            "xrefs",
            "unresolved_calls",
            "data_references",
            "function_candidates",
            "duplicate_groups",
            "duplicate_members",
            "unconfirmed_candidates",
            "psyq_evidence",
            "type_declarations",
            "type_fields",
            "type_usages",
            "type_constraints",
            "type_conflicts",
            "type_candidates",
            "type_input_fingerprints",
            "macro_definitions",
            "macro_uses",
            "macro_templates",
            "macro_input_fingerprints",
        }
        tables = {
            row[0]
            for row in connection.execute(
                "SELECT name FROM sqlite_master WHERE type = 'table'"
            )
        }
        if schema != (SCHEMA_VERSION,) or not expected_tables <= tables:
            raise ValueError("reverse index schema validation failed")
        actual_targets = {
            row[0] for row in connection.execute("SELECT id FROM targets")
        }
        if actual_targets != expected_targets:
            raise ValueError(
                "reverse index target coverage failed: "
                f"missing={sorted(expected_targets - actual_targets)} "
                f"extra={sorted(actual_targets - expected_targets)}"
            )
    except sqlite3.Error as error:
        raise ValueError("reverse index schema validation failed") from error
    finally:
        connection.close()


def rebuild(root: Path) -> Path:
    """Rebuild the index atomically; retain the previous complete file on error."""

    manifests = load_target_manifests(root)
    records = []
    for target, manifest in sorted(manifests.items()):
        binary = root / manifest.binary
        if not binary.is_file():
            raise ValueError(f"missing target binary: {manifest.binary}")
        records.append((target, manifest, binary, *snapshot_for(root, target, binary)))
    output = index_path(root)
    output.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary = tempfile.mkstemp(
        dir=output.parent, prefix=".reverse.", suffix=".sqlite"
    )
    os.close(descriptor)
    temporary_path = Path(temporary)
    try:
        connection = sqlite3.connect(temporary_path)
        try:
            create_schema(connection)
            connection.execute(
                "INSERT INTO metadata VALUES (?, ?)", ("schema", SCHEMA_VERSION)
            )
            insert_shared_scalar_types(connection, root)
            for target, manifest, binary, path, snapshot in records:
                target_spec = prepare_target(root, target)
                _insert_target(
                    connection, root, target, manifest, binary, path, snapshot
                )
                _insert_function_candidates(
                    connection, root, target, manifest, target_spec, binary
                )
                _insert_symbols(connection, root, target)
                _insert_functions(
                    connection, root, target, manifest, target_spec, binary, snapshot
                )
                _insert_data_references(
                    connection, root, target, manifest, binary, snapshot
                )
                _insert_calls(connection, target, snapshot)
                insert_authored_types(connection, root, target, manifest)
                infer_type_candidates(connection, target)
            for target, manifest, *_unused in records:
                insert_macro_registry(connection, root, target, manifest)
            insert_duplicate_groups(connection)
            insert_unconfirmed_candidates(connection)
            connection.commit()
        finally:
            connection.close()
        _validate_candidate(temporary_path, set(manifests))
        temporary_path.replace(output)
    except BaseException:
        temporary_path.unlink(missing_ok=True)
        raise
    return output


def _insert_target(
    connection: sqlite3.Connection,
    root: Path,
    target: str,
    manifest,
    binary: Path,
    snapshot_path: Path,
    snapshot,
) -> None:
    inputs = type_input_rows(root, manifest)
    connection.execute(
        "INSERT INTO targets VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        (
            target,
            manifest.binary,
            file_sha256(binary),
            manifest.load_address,
            snapshot.engine["name"],
            snapshot.engine.get("version", ""),
            snapshot_path.relative_to(root).as_posix(),
            file_sha256(snapshot_path),
        ),
    )
    connection.execute(
        "INSERT INTO metadata VALUES (?, ?)",
        (f"type_inputs:{target}", type_input_digest(inputs)),
    )
    for source_path, digest, kind in inputs:
        connection.execute(
            "INSERT INTO type_input_fingerprints VALUES (?, ?, ?, ?)",
            (target, source_path, digest, kind),
        )
    macro_inputs = macro_input_rows(root, target, manifest)
    connection.execute(
        "INSERT INTO metadata VALUES (?, ?)",
        (f"macro_inputs:{target}", macro_input_digest(macro_inputs)),
    )


def _insert_function_candidates(
    connection: sqlite3.Connection,
    root: Path,
    target: str,
    manifest,
    target_spec,
    binary: Path,
) -> None:
    target_symbols = load_target_symbols(root, target)
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    payload = payload_for(
        binary.read_bytes(), manifest.load_address, binary_name=manifest.binary
    )
    for boundary in layout.boundaries:
        if not boundary.is_function:
            continue
        connection.execute(
            "INSERT OR IGNORE INTO function_candidates VALUES (?, ?, ?, ?, ?, ?, ?)",
            (
                target,
                boundary.virtual_start,
                boundary.virtual_end,
                boundary.function_name,
                "reviewed_range",
                "high",
                int(
                    manifest.load_address <= boundary.virtual_start
                    and boundary.virtual_end is not None
                    and boundary.virtual_end <= payload.payload_end
                ),
            ),
        )
    source_addresses = {
        address
        for address in (symbol.address for symbol in target_symbols)
        if target_spec.source_paths
        and resolve_source_for_paths(target_spec.source_paths, address) is not None
    }
    for symbol in target_symbols:
        if (
            not symbol.canonical_name.startswith("func_")
            and symbol.address not in source_addresses
        ):
            continue
        connection.execute(
            "INSERT OR IGNORE INTO function_candidates VALUES (?, ?, NULL, ?, ?, ?, ?)",
            (
                target,
                symbol.address,
                symbol.canonical_name,
                "mapped_entry",
                "low",
                int(manifest.load_address <= symbol.address < payload.payload_end),
            ),
        )


def _insert_symbols(connection: sqlite3.Connection, root: Path, target: str) -> None:
    for symbol in load_target_symbols(root, target):
        kind = "data" if symbol.canonical_name.startswith("D_") else "function"
        connection.execute(
            "INSERT INTO symbols VALUES (?, ?, ?, ?)",
            (target, symbol.address, symbol.canonical_name, kind),
        )


def _compiled_symbol(root: Path, target: str, address: int, layout) -> str | None:
    """Reviewed map/Splat identity: the target-owned compiled symbol at ``address``.

    ``None`` when the target-local map has not claimed the address, so a
    compiled symbol is never inferred from the analyzer name or Splat label.
    """
    try:
        return reviewed_function_name(root, target, address, layout=layout)
    except Exception:
        return None


def _insert_functions(
    connection: sqlite3.Connection,
    root: Path,
    target: str,
    manifest,
    target_spec,
    binary: Path,
    snapshot,
) -> None:
    binary_bytes = binary.read_bytes()
    payload = payload_for(
        binary_bytes, manifest.load_address, binary_name=manifest.binary
    )
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    reviewed_identity = layout.reviewed_range_identity(payload, binary=binary_bytes)
    claimed_paths = target_spec.source_paths
    claimed_by_address: dict[int, Path | None] = {}
    if claimed_paths:
        for address in sorted({function.address for function in snapshot.functions}):
            claimed_by_address[address] = resolve_source_for_paths(
                claimed_paths, address
            )
    data_addresses = [
        symbol.address
        for symbol in load_target_symbols(root, target)
        if symbol.canonical_name.startswith("D_")
    ]
    for function in snapshot.functions:
        identity = reviewed_identity.get(function.address)
        source = (
            claimed_by_address.get(function.address, function.source)
            if claimed_paths
            else function.source
        )
        lifecycle_text = None
        if source is not None:
            source_path = Path(source)
            lifecycle_text = source_path.read_text(encoding="utf-8", errors="replace")
        compiled_symbol = _compiled_symbol(root, target, function.address, layout)
        connection.execute(
            """INSERT INTO functions (
                id, target_id, address, size, name, compiled_symbol,
                analyzer_sha256, reviewed_sha256, reviewed_size, reviewed, lifted,
                source, lift_status, instruction_count, basic_blocks, cfg_edges,
                cyclomatic_complexity, loops, stack_frame, local_count,
                argument_count, trivial_kind, contains_data
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)""",
            (
                function.id,
                target,
                function.address,
                function.analyzer_size,
                function.analyzer_name,
                compiled_symbol,
                function.exact_sha256,
                identity[0] if identity else None,
                identity[1] if identity else None,
                int(identity is not None),
                int(function.is_lifted),
                source.as_posix() if isinstance(source, Path) else source,
                lift_lifecycle(lifecycle_text),
                (function.analyzer_size + 3) // 4,
                function.basic_blocks,
                function.edges,
                function.cyclomatic_complexity,
                function.loops,
                function.stack_frame,
                function.local_count,
                function.argument_count,
                trivial_kind(
                    binary_bytes[
                        payload.binary_offset
                        + function.address
                        - payload.load_address : payload.binary_offset
                        + function.address
                        - payload.load_address
                        + function.analyzer_size
                    ]
                ),
                int(
                    any(
                        function.address
                        <= address
                        < function.address + function.analyzer_size
                        for address in data_addresses
                    )
                ),
            ),
        )


def _insert_data_references(
    connection: sqlite3.Connection,
    root: Path,
    target: str,
    manifest,
    binary: Path,
    snapshot,
) -> None:
    binary_bytes = binary.read_bytes()
    payload = payload_for(
        binary_bytes, manifest.load_address, binary_name=manifest.binary
    )
    symbol_by_address = {
        symbol.address: symbol.canonical_name
        for symbol in load_target_symbols(root, target)
    }
    for function in snapshot.functions:
        function_bytes = binary_bytes[
            payload.binary_offset
            + function.address
            - payload.load_address : payload.binary_offset
            + function.address
            - payload.load_address
            + function.analyzer_size
        ]
        for source_offset, address, access_kind, opcode in data_references(
            function_bytes
        ):
            connection.execute(
                "INSERT OR IGNORE INTO data_references VALUES (?, ?, ?, ?, ?, ?, ?)",
                (
                    target,
                    function.id,
                    function.address + source_offset,
                    address,
                    symbol_by_address.get(address),
                    access_kind,
                    opcode,
                ),
            )


def _insert_calls(connection: sqlite3.Connection, target: str, snapshot) -> None:
    for call in snapshot.calls:
        connection.execute(
            "INSERT INTO calls VALUES (?, ?, ?)",
            (call.caller, call.callee, call.callsite),
        )
        connection.execute(
            "INSERT OR IGNORE INTO xrefs VALUES (?, ?, ?, ?)",
            (
                target,
                call.callsite,
                int(call.callee.rsplit("@", 1)[1], 16),
                "call",
            ),
        )
    for unresolved in snapshot.unresolved_calls:
        connection.execute(
            "INSERT INTO unresolved_calls VALUES (?, ?, ?, ?)",
            (
                unresolved.caller,
                unresolved.target_address,
                unresolved.callsite,
                unresolved.kind,
            ),
        )
        connection.execute(
            "INSERT OR IGNORE INTO xrefs VALUES (?, ?, ?, ?)",
            (
                target,
                unresolved.callsite,
                unresolved.target_address,
                unresolved.kind,
            ),
        )


__all__ = ["rebuild"]
