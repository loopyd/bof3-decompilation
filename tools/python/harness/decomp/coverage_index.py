"""Read-only reverse-index coverage, unavailable on unsafe or stale generations."""

from __future__ import annotations

from contextlib import closing
import os
import sqlite3

from ..analysis.index_validation import validate_status_index
from .coverage_inputs import (
    CoverageInputs,
    checked_path,
    read_manifests,
    read_layout_document,
)


def read_index(inputs: CoverageInputs, targets: list[str]) -> dict:
    relative = "out/index/reverse.sqlite"
    try:
        data = inputs.read(relative)
        for suffix in ("-journal", "-wal", "-shm"):
            sidecar = relative + suffix
            if os.path.lexists(inputs.root / sidecar):
                checked_path(inputs.root, sidecar)
                raise ValueError(f"index sidecar requires separate recovery: {sidecar}")
            inputs.missing.add(sidecar)
        # A WAL-mode header may create sidecars even for mode=ro. Never ignore WAL.
        if len(data) < 100 or data[:16] != b"SQLite format 3\0":
            raise ValueError("invalid SQLite header")
        if data[18:20] != b"\x01\x01":
            raise ValueError(
                "index requires WAL access; read-only coverage unavailable"
            )
        # The existing freshness validator traverses these inputs. Precheck every
        # lexical component before its resolving readers, then rehash afterward.
        for directory in ("config", "src", "include", "out/reverse/snapshots"):
            for path in inputs.walk(directory):
                inputs.read(path)
        for manifest in read_manifests(inputs).values():
            read_layout_document(inputs, manifest.splat)
        uri = checked_path(inputs.root, relative).as_uri() + "?mode=ro"
        with closing(sqlite3.connect(uri, uri=True)) as connection:
            connection.row_factory = sqlite3.Row
            connection.execute("PRAGMA query_only=ON")
            # Snapshot locators are untrusted database input, not filesystem authority.
            for row in connection.execute("SELECT binary, snapshot FROM targets"):
                for path in row:
                    inputs.read(path)
            validate_status_index(connection, inputs.root)
            functions = []
            candidates = []
            for target in targets:
                functions.extend(
                    dict(row)
                    for row in connection.execute(
                        "SELECT target_id, address, size, analyzer_sha256, reviewed_sha256, "
                        "reviewed_size, reviewed, lifted, source, contains_data FROM functions "
                        "WHERE target_id=? ORDER BY address, id",
                        (target,),
                    )
                )
                candidates.extend(
                    dict(row)
                    for row in connection.execute(
                        "SELECT * FROM function_candidates WHERE target_id=? "
                        "ORDER BY address, provenance",
                        (target,),
                    )
                )
        inputs.verify()
        return {
            "available": True,
            "freshness_scope": "all configured targets",
            "functions": functions,
            "candidates": candidates,
            "reason": None,
        }
    except (OSError, ValueError, sqlite3.DatabaseError) as exc:
        return {
            "available": False,
            "freshness_scope": "all configured targets",
            "functions": None,
            "candidates": None,
            "reason": str(exc),
        }


def reconcile_index(target: dict, index: dict) -> list[str]:
    if not index["available"]:
        return []
    errors = []
    reviewed = {
        row["virtual_start"]: row
        for row in target.get("ranges", [])
        if row["kind"] in {"c", "asm"}
    }
    authored = {row["address"] for row in target.get("authored", [])}
    functions = [
        row for row in index["functions"] if row["target_id"] == target["target"]
    ]
    seen = set()
    for row in functions:
        address = row["address"]
        identity = f"{target['target']}@0x{address:08X}"
        if address in seen:
            errors.append(f"duplicate indexed function: {identity}")
        seen.add(address)
        boundary = reviewed.get(address)
        if bool(row["reviewed"]) != (boundary is not None):
            errors.append(f"indexed reviewed flag mismatch: {identity}")
        if bool(row["lifted"]) != (address in authored):
            errors.append(f"indexed authored flag mismatch: {identity}")
        if (
            boundary
            and boundary.get("sha256")
            and (
                row["reviewed_sha256"] != boundary["sha256"]
                or row["reviewed_size"] != boundary["file_end"] - boundary["file_start"]
            )
        ):
            errors.append(f"indexed reviewed range identity mismatch: {identity}")
        if row["contains_data"]:
            errors.append(f"indexed function contains data: {identity}")
    errors.extend(
        f"reviewed start missing from index: {target['target']}@0x{a:08X}"
        for a in sorted(reviewed.keys() - seen)
    )
    return errors
