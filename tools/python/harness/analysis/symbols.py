"""Classify mapped symbols using target-owned declarations and function evidence."""

from __future__ import annotations

import sqlite3
from pathlib import Path

from ..domain.symbols import load_target_symbols


def insert_symbols(
    connection: sqlite3.Connection, root: Path, target: str, manifest, *, symbols=None
) -> None:
    globals_ = {
        row[0]
        for row in connection.execute(
            "SELECT subject FROM type_usages WHERE target_id = ? "
            "AND use_kind = 'global' AND provenance = 'header_claim'",
            (target,),
        )
    }
    mapped = (
        load_target_symbols(root, target, psyq_space=manifest.psyq_space)
        if symbols is None
        else symbols
    )
    for symbol in mapped:
        declared_global = symbol.canonical_name in globals_
        if (
            declared_global
            and connection.execute(
                "SELECT 1 FROM function_candidates WHERE target_id = ? AND address = ? "
                "UNION ALL SELECT 1 FROM type_usages WHERE target_id = ? "
                "AND subject = ? AND use_kind = 'prototype' LIMIT 1",
                (target, symbol.address, target, symbol.canonical_name),
            ).fetchone()
        ):
            raise ValueError(
                f"conflicting function/global symbol: {target}:{symbol.canonical_name}"
            )
        kind = (
            "data"
            if declared_global or symbol.canonical_name.startswith("D_")
            else "function"
        )
        connection.execute(
            "INSERT INTO symbols VALUES (?, ?, ?, ?)",
            (target, symbol.address, symbol.canonical_name, kind),
        )
