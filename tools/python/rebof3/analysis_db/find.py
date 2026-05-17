"""CLI query tool for the BOF3 analysis database.

Usage:
    bin/db-find func --name func_801d
    bin/db-find func --addr 0x80123456
    bin/db-find xrefs --to 0x80123456
    ...
"""

from __future__ import annotations

import argparse
import json
import sqlite3
from pathlib import Path

from ..commands._common import run_main

DEFAULT_DB = Path("output") / "analysis.sqlite3"


def _connect(db_path: Path) -> sqlite3.Connection:
    conn = sqlite3.connect(str(db_path))
    conn.row_factory = sqlite3.Row
    return conn


def _format_table(rows: list[sqlite3.Row]) -> str:
    if not rows:
        return ""
    cols = rows[0].keys()
    widths = [len(col) for col in cols]
    str_rows: list[list[str]] = []
    for row in rows:
        cells = [str(row[col]) if row[col] is not None else "" for col in cols]
        str_rows.append(cells)
        for i, cell in enumerate(cells):
            widths[i] = max(widths[i], len(cell))
    lines: list[str] = []
    header = "  ".join(col.ljust(widths[i]) for i, col in enumerate(cols))
    lines.append(header)
    lines.append("  ".join("-" * widths[i] for i in range(len(cols))))
    for cells in str_rows:
        lines.append("  ".join(cell.ljust(widths[i]) for i, cell in enumerate(cells)))
    return "\n".join(lines)


def _output(rows: list[sqlite3.Row], json_output: bool) -> None:
    if json_output:
        result = [dict(row) for row in rows]
        print(json.dumps(result, indent=2, default=str))
    else:
        table = _format_table(rows)
        if table:
            print(table)
        print(f"\n{len(rows)} row(s)")


# ---------------------------------------------------------------------------
# func subcommand
# ---------------------------------------------------------------------------

def run_func(args: argparse.Namespace) -> int:
    conn = _connect(args.db)
    try:
        where: list[str] = []
        params: list[str] = []
        if args.name:
            where.append("name LIKE ?")
            params.append(args.name)
        if args.addr:
            where.append("address = ?")
            params.append(args.addr)
        if args.program:
            where.append("program_path LIKE ?")
            params.append(args.program)
        if args.module:
            where.append("program_path LIKE ?")
            params.append(f"%{args.module}%")

        query = "SELECT * FROM functions"
        if where:
            query += " WHERE " + " AND ".join(where)
        query += " ORDER BY program_path, address"
        rows = conn.execute(query, params).fetchall()
        _output(rows, args.json_output)
        return 0
    finally:
        conn.close()


def configure_func_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--name", help="LIKE pattern for function name")
    parser.add_argument("--addr", help="exact address match")
    parser.add_argument("--program", help="LIKE pattern for program_path")
    parser.add_argument("--module", help="LIKE shorthand for program_path (e.g. GAME#0)")
    parser.set_defaults(handler=run_func)


# ---------------------------------------------------------------------------
# xrefs subcommand
# ---------------------------------------------------------------------------

def run_xrefs(args: argparse.Namespace) -> int:
    conn = _connect(args.db)
    try:
        where: list[str] = []
        params: list[str] = []
        if args.to:
            where.append("to_address = ?")
            params.append(args.to)
        if args.from_:
            where.append("from_address = ?")
            params.append(args.from_)

        query = "SELECT * FROM xrefs"
        if where:
            query += " WHERE " + " AND ".join(where)
        query += " ORDER BY from_address, to_address"
        rows = conn.execute(query, params).fetchall()
        _output(rows, args.json_output)
        return 0
    finally:
        conn.close()


def configure_xrefs_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--to", help="xrefs pointing TO this address")
    parser.add_argument("--from", dest="from_", help="xrefs FROM this address")
    parser.set_defaults(handler=run_xrefs)


# ---------------------------------------------------------------------------
# calls subcommand
# ---------------------------------------------------------------------------

def run_calls(args: argparse.Namespace) -> int:
    conn = _connect(args.db)
    try:
        if args.leaves and args.program:
            query = """
                SELECT f.* FROM functions f
                WHERE f.program_path LIKE ?
                  AND f.address NOT IN (
                    SELECT from_func FROM call_edges
                  )
                ORDER BY f.address
            """
            rows = conn.execute(query, [args.program]).fetchall()
        elif args.entry and args.program:
            query = """
                SELECT f.* FROM functions f
                WHERE f.program_path LIKE ?
                  AND f.address NOT IN (
                    SELECT to_func FROM call_edges
                  )
                ORDER BY f.address
            """
            rows = conn.execute(query, [args.program]).fetchall()
        elif args.from_:
            query = """
                SELECT ce.*, f.name AS to_name
                FROM call_edges ce
                LEFT JOIN functions f ON f.address = ce.to_func
                WHERE ce.from_func = ?
                ORDER BY ce.to_func
            """
            rows = conn.execute(query, [args.from_]).fetchall()
        elif args.to:
            query = """
                SELECT ce.*, f.name AS from_name
                FROM call_edges ce
                LEFT JOIN functions f ON f.address = ce.from_func
                WHERE ce.to_func = ?
                ORDER BY ce.from_func
            """
            rows = conn.execute(query, [args.to]).fetchall()
        else:
            query = "SELECT * FROM call_edges ORDER BY from_func, to_func"
            rows = conn.execute(query).fetchall()

        _output(rows, args.json_output)
        return 0
    finally:
        conn.close()


def configure_calls_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--from", dest="from_", help="functions called BY this address")
    parser.add_argument("--to", help="functions that CALL this address")
    parser.add_argument("--leaves", action="store_true", help="leaf functions (call nothing)")
    parser.add_argument("--entry", action="store_true", help="entry points (nothing calls them)")
    parser.add_argument("--program", help="program_path filter for --leaves/--entry")
    parser.set_defaults(handler=run_calls)


# ---------------------------------------------------------------------------
# dups subcommand
# ---------------------------------------------------------------------------

def run_dups(args: argparse.Namespace) -> int:
    conn = _connect(args.db)
    try:
        if args.min_files is not None:
            query = """
                SELECT sha256, program_count, entries_json
                FROM duplicates
                WHERE program_count >= ?
                ORDER BY program_count DESC
            """
            rows = conn.execute(query, [args.min_files]).fetchall()
        elif args.name:
            query = """
                SELECT * FROM duplicates
                WHERE entries_json LIKE ?
                ORDER BY program_count DESC
            """
            rows = conn.execute(query, [f"%{args.name}%"]).fetchall()
        elif args.addr:
            query = """
                SELECT * FROM duplicates
                WHERE entries_json LIKE ?
                ORDER BY program_count DESC
            """
            rows = conn.execute(query, [f"%{args.addr}%"]).fetchall()
        else:
            query = "SELECT * FROM duplicates ORDER BY program_count DESC"
            rows = conn.execute(query).fetchall()

        _output(rows, args.json_output)
        return 0
    finally:
        conn.close()


def configure_dups_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--min-files", type=int, help="duplicate groups spanning >=N programs")
    parser.add_argument("--name", help="duplicates matching function name pattern")
    parser.add_argument("--addr", help="duplicate groups containing this function address")
    parser.set_defaults(handler=run_dups)


# ---------------------------------------------------------------------------
# const subcommand
# ---------------------------------------------------------------------------

def run_const(args: argparse.Namespace) -> int:
    conn = _connect(args.db)
    try:
        where: list[str] = []
        params: list = []
        if args.program:
            where.append("program_path LIKE ?")
            params.append(args.program)
        if args.addr:
            where.append("address = ?")
            params.append(args.addr)
        if args.min_xrefs is not None:
            where.append("xref_count >= ?")
            params.append(args.min_xrefs)

        query = "SELECT * FROM constants"
        if where:
            query += " WHERE " + " AND ".join(where)
        query += " ORDER BY program_path, address"
        rows = conn.execute(query, params).fetchall()
        _output(rows, args.json_output)
        return 0
    finally:
        conn.close()


def configure_const_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--program", help="constants in a program (LIKE pattern)")
    parser.add_argument("--addr", help="constant at specific address")
    parser.add_argument("--min-xrefs", type=int, help="widely-used globals (>=N xrefs)")
    parser.set_defaults(handler=run_const)


# ---------------------------------------------------------------------------
# sym subcommand
# ---------------------------------------------------------------------------

def run_sym(args: argparse.Namespace) -> int:
    conn = _connect(args.db)
    try:
        where: list[str] = []
        params: list[str] = []
        if args.name:
            where.append("name LIKE ?")
            params.append(args.name)
        if args.kind:
            where.append("kind = ?")
            params.append(args.kind)
        if args.addr:
            where.append("address = ?")
            params.append(args.addr)

        query = "SELECT * FROM symbols"
        if where:
            query += " WHERE " + " AND ".join(where)
        query += " ORDER BY program_path, address"
        rows = conn.execute(query, params).fetchall()
        _output(rows, args.json_output)
        return 0
    finally:
        conn.close()


def configure_sym_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--name", help="symbols matching name pattern (LIKE)")
    parser.add_argument("--kind", help="symbols of a specific kind")
    parser.add_argument("--addr", help="symbol at specific address")
    parser.set_defaults(handler=run_sym)


# ---------------------------------------------------------------------------
# root parser
# ---------------------------------------------------------------------------

def configure_root_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--db", type=Path, default=DEFAULT_DB,
        help="Path to analysis SQLite database (default: output/analysis.sqlite3)",
    )
    parser.add_argument(
        "--json", dest="json_output", action=argparse.BooleanOptionalAction,
        default=True, help="Output as JSON (default); --no-json for table",
    )
    subparsers = parser.add_subparsers(required=True)

    func = subparsers.add_parser("func", help="search functions")
    configure_func_parser(func)

    xrefs = subparsers.add_parser("xrefs", help="search cross-references")
    configure_xrefs_parser(xrefs)

    calls = subparsers.add_parser("calls", help="search call edges")
    configure_calls_parser(calls)

    dups = subparsers.add_parser("dups", help="search duplicates")
    configure_dups_parser(dups)

    const = subparsers.add_parser("const", help="search constants")
    configure_const_parser(const)

    sym = subparsers.add_parser("sym", help="search symbols")
    configure_sym_parser(sym)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="db-find")
    configure_root_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
