"""Atomic cross-target reverse-engineering index.

The index is derived entirely from target-qualified Rizin snapshots and maps.
It is a query cache, never an authority for binary layout or symbol names.
"""

from __future__ import annotations

import os
import sqlite3
import struct
import tempfile
from collections import defaultdict
from pathlib import Path
from typing import Iterable

from ..discovery import file_sha256
from ..domain import load_target_manifests
from ..domain.symbols import load_target_symbols
from .schema import create_schema
from .snapshot import read_snapshot, snapshot_path, validate_snapshot_identity


SCHEMA_VERSION = "bof3.reverse-index/v4"


def index_path(root: Path) -> Path:
    return root / "out" / "index" / "reverse.sqlite"


def _trivial_kind(data: bytes) -> str | None:
    if data == b"\x08\x00\xe0\x03\x00\x00\x00\x00":
        return "return_void"
    return None


_LUI = 0x0F
# opcode -> immediate signedness for %lo uses of a lui-materialized register
_LO_OPS = {
    0x09: "s",  # addiu
    0x0D: "z",  # ori
    0x20: "s",
    0x21: "s",
    0x23: "s",
    0x24: "s",
    0x25: "s",  # lb/lh/lw/lbu/lhu
    0x28: "s",
    0x29: "s",
    0x2B: "s",  # sb/sh/sw
}
# SPECIAL functs that write rd
_SPECIAL_WRITES_RD = {
    0x00,
    0x02,
    0x03,
    0x04,
    0x08,
    0x09,
    0x0F,
    0x10,
    0x11,
    0x12,
    0x13,
    0x18,
    0x19,
    0x1A,
    0x1B,
    0x20,
    0x21,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x2A,
    0x2B,
}
_LUI_WINDOW = 12


def _data_references(data: bytes) -> list[int]:
    """Addresses materialized by lui/%lo pairs inside one function's bytes."""

    references: set[int] = set()
    lui: dict[int, tuple[int, int]] = {}
    for index in range(len(data) // 4):
        (word,) = struct.unpack_from("<I", data, index * 4)
        op = word >> 26
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        rd = (word >> 11) & 31
        imm = word & 0xFFFF
        if op == _LUI:
            lui[rt] = (imm << 16, index)
            continue
        if op in _LO_OPS and rs in lui and index - lui[rs][1] <= _LUI_WINDOW:
            hi, _ = lui[rs]
            simm = (
                imm if _LO_OPS[op] == "z" else (imm - 0x10000 if imm & 0x8000 else imm)
            )
            references.add((hi + simm) & 0xFFFFFFFF)
        if op == 0 and (word & 0x3F) in _SPECIAL_WRITES_RD and rd in lui:
            del lui[rd]
        if op in _LO_OPS and rt in lui and op not in (0x28, 0x29, 0x2B):
            del lui[rt]
    return sorted(references)


def _snapshot_for(root: Path, target: str, binary: Path):
    path = snapshot_path(root, target)
    if not path.is_file():
        raise ValueError(f"missing Rizin snapshot: {path.relative_to(root)}")
    snapshot = read_snapshot(path)
    errors = validate_snapshot_identity(snapshot)
    if errors:
        raise ValueError(
            f"invalid Rizin snapshot {path.relative_to(root)}: {'; '.join(errors)}"
        )
    if snapshot.target != target:
        raise ValueError(f"stale Rizin snapshot target: {path.relative_to(root)}")
    if snapshot.engine.get("name") != "rizin":
        raise ValueError(
            f"snapshot was not produced by Rizin: {path.relative_to(root)}"
        )
    if snapshot.inputs.get("binary_sha256") != file_sha256(binary):
        raise ValueError(f"stale Rizin snapshot bytes: {path.relative_to(root)}")
    from .project import prepare_target

    if (
        snapshot.inputs.get("replay_sha256")
        != prepare_target(root, target).replay_sha256
    ):
        raise ValueError(f"stale Rizin snapshot recipe: {path.relative_to(root)}")
    return path, snapshot


def rebuild(root: Path) -> Path:
    """Rebuild the index atomically; retain the previous complete file on error."""

    manifests = load_target_manifests(root)
    records = []
    for target, manifest in sorted(manifests.items()):
        binary = root / manifest.binary
        if not binary.is_file():
            raise ValueError(f"missing target binary: {manifest.binary}")
        records.append((target, manifest, binary, *_snapshot_for(root, target, binary)))
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
            for target, manifest, binary, path, snapshot in records:
                binary_bytes = binary.read_bytes()
                connection.execute(
                    "INSERT INTO targets VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                    (
                        target,
                        manifest.binary,
                        file_sha256(binary),
                        manifest.load_address,
                        snapshot.engine["name"],
                        snapshot.engine.get("version", ""),
                        path.relative_to(root).as_posix(),
                        file_sha256(path),
                    ),
                )
                target_symbols = load_target_symbols(root, target)
                data_addresses = [
                    s.address
                    for s in target_symbols
                    if s.canonical_name.startswith("D_")
                ]
                for symbol in target_symbols:
                    kind = (
                        "data" if symbol.canonical_name.startswith("D_") else "function"
                    )
                    connection.execute(
                        "INSERT INTO symbols VALUES (?, ?, ?, ?)",
                        (target, symbol.address, symbol.canonical_name, kind),
                    )
                for function in snapshot.functions:
                    connection.execute(
                        """INSERT INTO functions (
                            id, target_id, address, size, name, exact_sha256,
                            reviewed, lifted, source, instruction_count,
                            basic_blocks, cfg_edges, cyclomatic_complexity,
                            loops, stack_frame, local_count, argument_count,
                            trivial_kind, contains_data
                        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)""",
                        (
                            function.id,
                            target,
                            function.address,
                            function.analyzer_size,
                            function.analyzer_name,
                            function.exact_sha256,
                            int(function.is_reviewed),
                            int(function.is_lifted),
                            function.source,
                            (function.analyzer_size + 3) // 4,
                            function.basic_blocks,
                            function.edges,
                            function.cyclomatic_complexity,
                            function.loops,
                            function.stack_frame,
                            function.local_count,
                            function.argument_count,
                            _trivial_kind(
                                binary_bytes[
                                    function.address
                                    - manifest.load_address : function.address
                                    - manifest.load_address
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
                for function in snapshot.functions:
                    function_bytes = binary_bytes[
                        function.address - manifest.load_address : function.address
                        - manifest.load_address
                        + function.analyzer_size
                    ]
                    symbol_by_address = {
                        s.address: s.canonical_name for s in target_symbols
                    }
                    for address in _data_references(function_bytes):
                        connection.execute(
                            "INSERT OR IGNORE INTO data_references VALUES (?, ?, ?, ?)",
                            (
                                target,
                                function.id,
                                address,
                                symbol_by_address.get(address),
                            ),
                        )
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
            rows = connection.execute(
                "SELECT exact_sha256, size, id FROM functions ORDER BY exact_sha256, id"
            ).fetchall()
            groups: dict[tuple[str, int], list[str]] = defaultdict(list)
            for digest, size, function_id in rows:
                groups[(digest, size)].append(function_id)
            for (digest, size), members in groups.items():
                if len(members) < 2:
                    continue
                connection.execute(
                    "INSERT INTO duplicate_groups VALUES (?, ?, ?)",
                    (digest, size, len(members)),
                )
                connection.executemany(
                    "INSERT INTO duplicate_members VALUES (?, ?)",
                    ((digest, member) for member in members),
                )
            connection.commit()
        finally:
            connection.close()
        temporary_path.replace(output)
    except BaseException:
        temporary_path.unlink(missing_ok=True)
        raise
    return output


def connect(root: Path) -> sqlite3.Connection:
    path = index_path(root)
    if not path.is_file():
        raise FileNotFoundError(
            f"reverse index not found: {path.relative_to(root)}; run just index"
        )
    connection = sqlite3.connect(path)
    connection.row_factory = sqlite3.Row
    try:
        row = connection.execute(
            "SELECT value FROM metadata WHERE key = 'schema'"
        ).fetchone()
    except sqlite3.Error as exc:
        connection.close()
        raise ValueError("invalid reverse index; run just index") from exc
    if row is None or row[0] != SCHEMA_VERSION:
        connection.close()
        found = row[0] if row is not None else "missing"
        raise ValueError(
            f"reverse index schema mismatch: expected {SCHEMA_VERSION}, got {found}; run just index"
        )
    try:
        indexed_targets = connection.execute(
            "SELECT id, binary, binary_sha256, snapshot, snapshot_sha256 FROM targets"
        )
        for (
            target,
            binary_name,
            binary_digest,
            snapshot_name,
            snapshot_digest,
        ) in indexed_targets:
            binary = root / binary_name
            snapshot = root / snapshot_name
            if not binary.is_file() or file_sha256(binary) != binary_digest:
                raise ValueError(
                    f"stale reverse index binary for {target}; run just index"
                )
            if not snapshot.is_file() or file_sha256(snapshot) != snapshot_digest:
                raise ValueError(
                    f"stale reverse index snapshot for {target}; run just index"
                )
            _snapshot_for(root, target, binary)
    except BaseException:
        connection.close()
        raise
    return connection


def rows(
    connection: sqlite3.Connection, query: str, params: Iterable[object] = ()
) -> list[dict[str, object]]:
    return [dict(row) for row in connection.execute(query, tuple(params))]


__all__ = ["connect", "index_path", "rebuild", "rows"]
