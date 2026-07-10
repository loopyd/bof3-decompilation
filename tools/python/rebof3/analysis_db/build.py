"""Build analysis.sqlite3 from a Ghidra analysis.json export."""

from __future__ import annotations

import argparse
import json
import sqlite3
from pathlib import Path
from typing import Any

from ..commands._common import run_main
from .schema import init_db


DEFAULT_INPUT = Path("output") / "inventory" / "analysis.json"
DEFAULT_DB = Path("output") / "analysis.sqlite3"


def _parse_hex(addr: str) -> int | None:
    try:
        return int(addr, 16)
    except (ValueError, TypeError):
        return None


def _parse_int(v: Any) -> int | None:
    try:
        return int(v)
    except (ValueError, TypeError):
        return None


def build_db(
    *,
    input_path: Path,
    db_path: Path,
) -> dict[str, int]:
    """Consume analysis.json and populate analysis.sqlite3.

    Returns row counts per table.
    """
    with input_path.open(encoding="utf-8") as fh:
        payload = json.load(fh)

    if not isinstance(payload, dict):
        raise ValueError("analysis.json must be a JSON object")

    db_path.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(str(db_path))
    conn.execute("PRAGMA journal_mode=WAL")
    conn.execute("PRAGMA foreign_keys=ON")
    conn.row_factory = sqlite3.Row

    try:
        init_db(conn)

        # Clear existing data
        for table in (
            "duplicates",
            "constants",
            "call_edges",
            "xrefs",
            "symbols",
            "functions",
            "programs",
        ):
            conn.execute(f"DELETE FROM {table}")

        counts: dict[str, int] = {}

        # --- programs ---
        funcs = payload.get("functions", [])
        program_set: set[str] = set()
        for f in funcs:
            if isinstance(f, dict):
                pp = f.get("program_path", "")
                if pp:
                    program_set.add(pp)
        for sym in payload.get("symbols", []):
            if isinstance(sym, dict):
                pp = sym.get("program_path", "")
                if pp:
                    program_set.add(pp)
        for pp in sorted(program_set):
            conn.execute(
                "INSERT OR IGNORE INTO programs(path, name) VALUES(?, ?)",
                (pp, Path(pp).name),
            )
        counts["programs"] = len(program_set)

        # --- functions ---
        count_f = 0
        for f in funcs:
            if not isinstance(f, dict):
                continue
            conn.execute(
                """INSERT INTO functions(
                    address, name, signature, body_min, body_max,
                    program_path, is_thunk, name_source, namespace
                ) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)""",
                (
                    str(f.get("address") or ""),
                    f.get("name"),
                    f.get("signature"),
                    f.get("body_min"),
                    f.get("body_max"),
                    str(f.get("program_path") or ""),
                    1 if f.get("is_thunk") else 0,
                    f.get("name_source"),
                    f.get("namespace"),
                ),
            )
            count_f += 1
        counts["functions"] = count_f

        # --- symbols ---
        count_s = 0
        for s in payload.get("symbols", []):
            if not isinstance(s, dict):
                continue
            conn.execute(
                """INSERT INTO symbols(address, name, kind, program_path, name_source)
                   VALUES(?, ?, ?, ?, ?)""",
                (
                    str(s.get("address") or ""),
                    s.get("name"),
                    s.get("kind"),
                    str(s.get("program_path") or ""),
                    s.get("name_source"),
                ),
            )
            count_s += 1
        counts["symbols"] = count_s

        # --- xrefs ---
        count_x = 0
        for x in payload.get("xrefs", []):
            if not isinstance(x, dict):
                continue
            conn.execute(
                """INSERT INTO xrefs(from_address, to_address, reference_type, program_path)
                   VALUES(?, ?, ?, ?)""",
                (
                    str(x.get("from_address") or ""),
                    str(x.get("to_address") or ""),
                    x.get("reference_type"),
                    str(x.get("program_path") or ""),
                ),
            )
            count_x += 1
        counts["xrefs"] = count_x

        # --- call_edges ---
        count_e = 0
        for ce in payload.get("call_edges", []):
            if not isinstance(ce, dict):
                continue
            conn.execute(
                """INSERT INTO call_edges(from_func, to_func, from_program, to_external)
                   VALUES(?, ?, ?, ?)""",
                (
                    str(ce.get("from_func") or ""),
                    str(ce.get("to_func") or ""),
                    str(ce.get("from_program") or ""),
                    1 if ce.get("to_external") else 0,
                ),
            )
            count_e += 1
        counts["call_edges"] = count_e

        # --- constants ---
        count_c = 0
        for c in payload.get("constants", []):
            if not isinstance(c, dict):
                continue
            conn.execute(
                """INSERT INTO constants(address, name, data_type, program_path, xref_count)
                   VALUES(?, ?, ?, ?, ?)""",
                (
                    str(c.get("address") or ""),
                    c.get("name"),
                    c.get("data_type"),
                    str(c.get("program_path") or ""),
                    int(c.get("xref_count", 0)),
                ),
            )
            count_c += 1
        counts["constants"] = count_c

        # --- duplicates ---
        count_d = 0
        for d in payload.get("duplicates", []):
            if not isinstance(d, dict):
                continue
            entries = d.get("entries", [])
            if not isinstance(entries, list):
                entries = []
            conn.execute(
                """INSERT OR REPLACE INTO duplicates(sha256, program_count, entries_json)
                   VALUES(?, ?, ?)""",
                (
                    str(d.get("sha256") or ""),
                    int(d.get("program_count", 0)),
                    json.dumps(entries, sort_keys=True),
                ),
            )
            count_d += 1
        counts["duplicates"] = count_d

        conn.commit()
    finally:
        conn.close()

    return counts


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Build analysis.sqlite3 from a Ghidra analysis.json export."
    )
    parser.add_argument(
        "--input",
        type=Path,
        default=DEFAULT_INPUT,
        help="Path to analysis.json (default: out/inventory/analysis.json)",
    )
    parser.add_argument(
        "--db",
        type=Path,
        default=DEFAULT_DB,
        help="Path to output SQLite database (default: out/analysis.sqlite3)",
    )
    parser.set_defaults(handler=main)
    return parser


def main(args: argparse.Namespace) -> int:
    input_path: Path = args.input
    db_path: Path = args.db

    if not input_path.is_file():
        print(f"analysis.json not found: {input_path}", flush=True)
        print("Run bin/ghidra-export-analysis first.", flush=True)
        return 1

    counts = build_db(input_path=input_path, db_path=db_path)
    print(f"analysis.sqlite3 built: {db_path}", flush=True)
    for table, count in sorted(counts.items()):
        print(f"  {table}: {count}", flush=True)
    return 0


build_main = main

if __name__ == "__main__":
    run_main(build_parser)
