"""Project decomp dashboard — reads analysis DB and harness DB to show progress."""

from __future__ import annotations

import argparse
import json
import sqlite3
import sys
from pathlib import Path

from ..commands._common import run_main

DEFAULT_DB = Path("output/analysis.sqlite3")
DEFAULT_HARNESS_DB = Path("output/harness/harness.sqlite3")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Show project decompilation status."
    )
    parser.add_argument(
        "--module", metavar="NAME",
        help="Filter to a specific program/module name",
    )
    parser.add_argument(
        "--json", dest="json_output", action="store_true",
        help="Machine-readable JSON output",
    )
    parser.add_argument(
        "--db", type=Path, default=DEFAULT_DB, metavar="FILE",
        help="Path to analysis.sqlite3",
    )
    parser.set_defaults(handler=main)
    return parser


def _normalize_addr(addr: str) -> str:
    if not addr.lower().startswith("0x"):
        return f"0x{addr.lower()}"
    return addr.lower()


def _is_psyq(func: sqlite3.Row) -> bool:
    """Check if a function is a PsyQ library import/thunk."""
    if func["is_thunk"]:
        return True
    ns = (func["namespace"] or "").lower()
    src = (func["name_source"] or "").lower()
    prog = (func["program_path"] or "").lower()
    if "psyq" in ns or "psyq" in src or "psyq" in prog:
        return True
    return False


def _load_lifted_addrs(harness_db: Path) -> set[str]:
    """Return the set of entry_hex addresses marked done in the harness DB."""
    if not harness_db.is_file():
        return set()
    conn = sqlite3.connect(str(harness_db))
    conn.row_factory = sqlite3.Row
    try:
        rows = conn.execute(
            "SELECT entry_hex FROM targets WHERE status = 'done'"
        ).fetchall()
        return {
            _normalize_addr(r["entry_hex"])
            for r in rows
            if r["entry_hex"]
        }
    except sqlite3.OperationalError:
        return set()
    finally:
        conn.close()


def _fmt_pct(numerator: int, denominator: int) -> str:
    if denominator == 0:
        return "0.0%"
    return f"{(numerator / denominator) * 100:.1f}%"


def _text_report(
    conn: sqlite3.Connection,
    lifted_addrs: set[str],
    module_filter: str | None,
) -> str:
    lines: list[str] = []
    lines.append("=== BOF3 Decomp Status ===")
    lines.append("")

    # --- Counts ---
    n_programs = conn.execute("SELECT COUNT(*) FROM programs").fetchone()[0]
    n_total_funcs = conn.execute("SELECT COUNT(*) FROM functions").fetchone()[0]

    all_funcs = conn.execute(
        """SELECT address, name, program_path, is_thunk, name_source, namespace
           FROM functions"""
    ).fetchall()

    psyq_funcs: list[sqlite3.Row] = []
    game_funcs: list[sqlite3.Row] = []
    for f in all_funcs:
        if _is_psyq(f):
            psyq_funcs.append(f)
        else:
            game_funcs.append(f)

    n_psyq = len(psyq_funcs)
    n_game = len(game_funcs)
    n_lifted = sum(1 for f in game_funcs if f["address"] in lifted_addrs)

    # Duplicate groups
    dup_rows = conn.execute(
        """SELECT sha256, program_count, entries_json
           FROM duplicates
           WHERE program_count > 1"""
    ).fetchall()
    n_dup_groups = len(dup_rows)
    n_dup_funcs = 0
    for d in dup_rows:
        try:
            entries = json.loads(d["entries_json"])
        except (json.JSONDecodeError, TypeError):
            entries = []
        n_dup_funcs += len(entries)

    n_remaining = n_game - n_lifted

    lines.append(f"Programs analyzed:  {n_programs}")
    lines.append(f"Total functions:    {n_total_funcs:,}")
    lines.append(f"  PsyQ / imported:  {n_psyq:,} (not target)")
    lines.append(f"  Game functions:   {n_game:,}")
    lifted_pct = _fmt_pct(n_lifted, n_game)
    lines.append(f"  Lifted / source:  {n_lifted:,} ({lifted_pct})")
    lines.append(f"  Duplicate groups: {n_dup_groups:,} ({n_dup_funcs:,} functions)")
    lines.append(f"  Remaining:        {n_remaining:,}")
    lines.append("")

    # --- Per-module ---
    if module_filter:
        filter_match = module_filter.lower()
        program_names = [
            row["name"]
            for row in conn.execute("SELECT name, path FROM programs").fetchall()
            if filter_match in (row["name"] or "").lower()
               or filter_match in (row["path"] or "").lower()
        ]
    else:
        program_names = [
            row["name"]
            for row in conn.execute("SELECT name FROM programs ORDER BY name").fetchall()
        ]

    # Sort: modules with functions first, then by name
    prog_counts: dict[str, tuple[int, int]] = {}
    for pname in program_names:
        prow = conn.execute(
            "SELECT path FROM programs WHERE name = ?", (pname,)
        ).fetchone()
        if not prow:
            continue
        ppath = prow["path"]
        total = conn.execute(
            "SELECT COUNT(*) FROM functions WHERE program_path = ?",
            (ppath,),
        ).fetchone()[0]
        func_addrs = conn.execute(
            "SELECT address FROM functions WHERE program_path = ?",
            (ppath,),
        ).fetchall()
        lifted_count = 0
        for fa in func_addrs:
            if fa["address"] in lifted_addrs:
                lifted_count += 1
        prog_counts[pname] = (lifted_count, total)

    if prog_counts:
        lines.append("Per-module:")
        for pname in sorted(
            prog_counts,
            key=lambda n: (-prog_counts[n][1], n),
        ):
            lifted_c, total_c = prog_counts[pname]
            pct = _fmt_pct(lifted_c, total_c)
            lines.append(f"  {pname:<28} {lifted_c:,}/{total_c:,} ({pct})")
        lines.append("")

    # --- Top leaf functions ready to decomp ---
    if not module_filter:
        game_addrs = {f["address"] for f in game_funcs}
        lifted_set = lifted_addrs

        # Get callee counts for each function
        callee_counts: dict[str, int] = {}
        edge_rows = conn.execute(
            """SELECT from_func, to_func
               FROM call_edges
               WHERE from_func IN (SELECT address FROM functions)"""
        ).fetchall()
        for e in edge_rows:
            if e["from_func"] in game_addrs:
                callee_counts[e["from_func"]] = (
                    callee_counts.get(e["from_func"], 0) + 1
                )

        # Get caller counts
        caller_counts: dict[str, int] = {}
        for e in edge_rows:
            if e["to_func"] in game_addrs:
                caller_counts[e["to_func"]] = (
                    caller_counts.get(e["to_func"], 0) + 1
                )

        # Find leaf functions (0 callees) that are not yet lifted
        leaf_candidates: list[tuple[sqlite3.Row, int, int]] = []
        for f in game_funcs:
            if f["address"] in lifted_set:
                continue
            n_callers = caller_counts.get(f["address"], 0)
            n_callees = callee_counts.get(f["address"], 0)
            # Prefer functions with fewer callers and 0 callees
            leaf_candidates.append((f, n_callers, n_callees))

        # Find unlifted callees count for each candidate
        def _unlifted_callees(func_addr: str) -> int:
            count = 0
            for e in edge_rows:
                if e["from_func"] == func_addr:
                    if e["to_func"] in game_addrs and e["to_func"] not in lifted_set:
                        count += 1
            return count

        scored: list[tuple[int, int, str, str, str]] = []
        for f, n_callers, n_callees in leaf_candidates:
            ucallees = _unlifted_callees(f["address"])
            scored.append((ucallees, n_callers, f["name"] or "", f["address"], f["program_path"]))

        scored.sort(key=lambda x: (x[0], x[1]))

        top_n = 20
        if scored:
            lines.append("Top leaf functions ready to decomp:")
            for i, (uc, nc, fname, faddr, fprog) in enumerate(scored[:top_n]):
                display = fname or f"func_{faddr[2:]}"
                prog_name = Path(fprog).name if fprog else fprog
                lines.append(
                    f"  {display:<24} ({faddr}) in {prog_name}"
                    f" — {nc} callers, {uc} unlifted callees"
                )

    return "\n".join(lines)


def _json_report(
    conn: sqlite3.Connection,
    lifted_addrs: set[str],
    module_filter: str | None,
) -> str:
    n_programs = conn.execute("SELECT COUNT(*) FROM programs").fetchone()[0]
    n_total_funcs = conn.execute("SELECT COUNT(*) FROM functions").fetchone()[0]

    all_funcs = conn.execute(
        """SELECT address, name, program_path, is_thunk, name_source, namespace
           FROM functions"""
    ).fetchall()

    psyq_funcs: list[sqlite3.Row] = []
    game_funcs: list[sqlite3.Row] = []
    for f in all_funcs:
        if _is_psyq(f):
            psyq_funcs.append(f)
        else:
            game_funcs.append(f)

    n_psyq = len(psyq_funcs)
    n_game = len(game_funcs)
    n_lifted = sum(1 for f in game_funcs if f["address"] in lifted_addrs)

    dup_rows = conn.execute(
        """SELECT sha256, program_count, entries_json
           FROM duplicates
           WHERE program_count > 1"""
    ).fetchall()
    n_dup_groups = len(dup_rows)
    n_dup_funcs = 0
    for d in dup_rows:
        try:
            entries = json.loads(d["entries_json"])
        except (json.JSONDecodeError, TypeError):
            entries = []
        n_dup_funcs += len(entries)

    payload: dict = {
        "programs": n_programs,
        "total_functions": n_total_funcs,
        "psyq_imported": n_psyq,
        "game_functions": n_game,
        "lifted": n_lifted,
        "lifted_pct": round((n_lifted / n_game * 100) if n_game else 0, 1),
        "duplicate_groups": n_dup_groups,
        "duplicate_functions": n_dup_funcs,
        "remaining": n_game - n_lifted,
    }

    # Per-module
    prog_names = [
        row["name"]
        for row in conn.execute("SELECT name FROM programs ORDER BY name").fetchall()
    ]
    per_module: dict[str, dict] = {}
    for pname in prog_names:
        if module_filter and module_filter.lower() not in (pname or "").lower():
            continue
        prow = conn.execute(
            "SELECT path FROM programs WHERE name = ?", (pname,)
        ).fetchone()
        if not prow:
            continue
        ppath = prow["path"]
        total = conn.execute(
            "SELECT COUNT(*) FROM functions WHERE program_path = ?", (ppath,)
        ).fetchone()[0]
        func_addrs = conn.execute(
            "SELECT address FROM functions WHERE program_path = ?", (ppath,)
        ).fetchall()
        lifted_count = sum(1 for fa in func_addrs if fa["address"] in lifted_addrs)
        per_module[pname] = {
            "total": total,
            "lifted": lifted_count,
            "pct": round((lifted_count / total * 100) if total else 0, 1),
        }
    payload["modules"] = per_module

    return json.dumps(payload, indent=2)


def main(args: argparse.Namespace) -> int:
    db_path: Path = args.db
    module_filter: str | None = args.module
    json_output: bool = args.json_output

    if not db_path.is_file():
        print(f"analysis DB not found: {db_path}", file=sys.stderr)
        print("Run bin/analysis-build first.", file=sys.stderr)
        return 1

    harness_db = DEFAULT_HARNESS_DB
    lifted_addrs = _load_lifted_addrs(harness_db)

    conn = sqlite3.connect(str(db_path))
    conn.row_factory = sqlite3.Row
    try:
        if json_output:
            output = _json_report(conn, lifted_addrs, module_filter)
        else:
            output = _text_report(conn, lifted_addrs, module_filter)
    finally:
        conn.close()

    print(output)
    return 0


if __name__ == "__main__":
    run_main(build_parser)
