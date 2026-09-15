"""Atomically build target records and groups using domain-owned byte/tag parsing."""

from __future__ import annotations

import os
import sqlite3
import tempfile
from pathlib import Path

from harness.domain.functions import select_lift_metadata

from harness.macros.index import (
    insert_macro_registry,
    macro_input_digest,
)
from harness.types.inference import infer_type_candidates
from harness.types.index import insert_authored_types
from harness.types.index import insert_shared_scalar_types
from harness.types.inputs import type_input_digest
from harness.types.usages import insert_source_usages

from ..domain.claims import index_source_paths
from ..domain.repository_layout import load_repository_layout
from ..domain.mips import data_references, trivial_kind
from ..domain.psx import payload_for
from ..domain.sources import reviewed_function_name
from ..domain.symbols import load_map, load_target_symbols, map_path
from ..domain.tags import lift_lifecycle
from .symbols import insert_symbols
from .index import SCHEMA_VERSION, index_path
from .index_groups import insert_duplicate_groups, insert_unconfirmed_candidates
from .index_snapshot import snapshot_for
from .project import prepare_target
from .schema import create_schema


def _validate_candidate(
    path: Path, expected_targets: set[str], *, repository=None
) -> None:
    """Reject an incomplete or corrupt candidate before atomic publication."""

    from .index_validation import validate_status_index, validate_structure

    connection = sqlite3.connect(path)
    try:
        integrity = connection.execute("PRAGMA integrity_check").fetchone()
        if integrity != ("ok",):
            raise ValueError(f"reverse index integrity check failed: {integrity}")
        foreign_keys = connection.execute("PRAGMA foreign_key_check").fetchall()
        if foreign_keys:
            raise ValueError(f"reverse index foreign key check failed: {foreign_keys}")
        if repository is None:
            validate_structure(connection)
        else:
            validate_status_index(
                connection, repository.files.root, repository=repository
            )
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

    root = root.resolve()
    repository = load_repository_layout(root)
    manifests = repository.manifests
    records = []
    for target, manifest in sorted(manifests.items()):
        binary = root / manifest.binary
        records.append(
            (
                target,
                manifest,
                binary,
                *snapshot_for(
                    root,
                    target,
                    binary,
                    manifest=manifest,
                    layout=repository.splats[target],
                    files=repository.files,
                ),
            )
        )
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
            insert_shared_scalar_types(connection, root, files=repository.files)
            for target, manifest, binary, path, snapshot in records:
                target_spec = prepare_target(
                    root,
                    target,
                    manifest=manifest,
                    layout=repository.splats[target],
                    files=repository.files,
                )
                symbols = load_target_symbols(
                    root,
                    target,
                    psyq_space=manifest.psyq_space,
                    read_file=repository.files.read_optional,
                )
                local_symbols = load_map(
                    map_path(root, target), read_file=repository.files.read
                )
                source_addresses = index_source_paths(
                    target_spec.source_paths, read_source=repository.files.text
                )
                _insert_target(
                    connection,
                    root,
                    target,
                    manifest,
                    binary,
                    path,
                    snapshot,
                    repository,
                )
                _insert_function_candidates(
                    connection,
                    root,
                    target,
                    manifest,
                    target_spec,
                    binary,
                    repository,
                    symbols,
                    source_addresses,
                )
                insert_authored_types(
                    connection, root, target, manifest, files=repository.files
                )
                insert_symbols(connection, root, target, manifest, symbols=symbols)
                _insert_functions(
                    connection,
                    root,
                    target,
                    manifest,
                    target_spec,
                    binary,
                    snapshot,
                    repository,
                    symbols,
                    local_symbols,
                    source_addresses,
                )
                insert_source_usages(
                    connection, root, target, manifest, files=repository.files
                )
                _insert_data_references(
                    connection,
                    root,
                    target,
                    manifest,
                    binary,
                    snapshot,
                    repository,
                    symbols,
                )
                _insert_calls(connection, target, snapshot)
                infer_type_candidates(connection, target)
            for target, manifest, *_unused in records:
                insert_macro_registry(
                    connection,
                    root,
                    target,
                    manifest,
                    files=repository.files,
                    inputs=repository.macro_sources[target],
                )
            insert_duplicate_groups(connection)
            insert_unconfirmed_candidates(connection)
            connection.commit()
        finally:
            connection.close()
        _validate_candidate(temporary_path, set(manifests), repository=repository)
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
    repository,
) -> None:
    inputs = list(repository.type_input_rows[target])
    connection.execute(
        "INSERT INTO targets VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        (
            target,
            manifest.binary,
            repository.files.digest(binary),
            manifest.load_address,
            snapshot.engine["name"],
            snapshot.engine.get("version", ""),
            snapshot_path.relative_to(root).as_posix(),
            repository.files.digest(snapshot_path),
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
    connection.executemany(
        "INSERT INTO selected_map_fingerprints VALUES (?, ?, ?)",
        (
            (target, path, digest)
            for path, digest in repository.selected_map_rows[target]
        ),
    )
    connection.executemany(
        "INSERT INTO source_fingerprints VALUES (?, ?, ?)",
        (
            (target, path, digest)
            for path, digest in repository.source_input_rows[target]
        ),
    )
    macro_inputs = list(repository.macro_input_rows[target])
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
    repository,
    target_symbols,
    source_addresses,
) -> None:
    layout = repository.splats[target]
    payload = payload_for(
        repository.files.read(binary),
        manifest.load_address,
        binary_name=manifest.binary,
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


def _compiled_symbol(
    root: Path, target: str, address: int, layout, manifest=None, symbols=None
) -> str | None:
    """Return reviewed map/Splat identity or None; never infer an analyzer name."""
    try:
        return reviewed_function_name(
            root, target, address, layout=layout, manifest=manifest, symbols=symbols
        )
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
    repository,
    symbols,
    local_symbols,
    claimed_by_address,
) -> None:
    binary_bytes = repository.files.read(binary)
    payload = payload_for(
        binary_bytes, manifest.load_address, binary_name=manifest.binary
    )
    layout = repository.splats[target]
    reviewed_identity = layout.reviewed_range_identity(payload, binary=binary_bytes)
    claimed_paths = target_spec.source_paths
    data_addresses = [
        symbol.address for symbol in symbols if symbol.canonical_name.startswith("D_")
    ]
    for function in snapshot.functions:
        identity = reviewed_identity.get(function.address)
        source = (
            claimed_by_address.get(function.address)
            if claimed_paths
            else function.source
        )
        lifecycle_text = None
        if source is not None:
            source_path = Path(source)
            lifecycle_text = select_lift_metadata(
                repository.files.text(source_path, errors="replace"),
                function.address,
            )
        compiled_symbol = _compiled_symbol(
            root, target, function.address, layout, manifest, local_symbols
        )
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
                int(source is not None),
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
    repository,
    symbols,
) -> None:
    binary_bytes = repository.files.read(binary)
    payload = payload_for(
        binary_bytes, manifest.load_address, binary_name=manifest.binary
    )
    symbol_by_address = {symbol.address: symbol.canonical_name for symbol in symbols}
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
