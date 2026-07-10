"""M2C context builder — queries analysis DB and emits a C header for a function."""

from __future__ import annotations

import argparse
import json
import sqlite3
import sys
from pathlib import Path

from ..commands._common import run_main

DEFAULT_DB = Path("out/analysis.sqlite3")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Generate M2C context header for a function."
    )
    parser.add_argument(
        "--addr",
        required=True,
        metavar="HEX",
        help="Function address (e.g. 0x80123456)",
    )
    parser.add_argument(
        "--out",
        type=Path,
        metavar="FILE",
        help="Output file path (default: stdout)",
    )
    parser.add_argument(
        "--db",
        type=Path,
        default=DEFAULT_DB,
        metavar="FILE",
        help="Path to analysis.sqlite3",
    )
    parser.set_defaults(handler=main)
    return parser


def _normalize_addr(addr: str) -> str:
    """Ensure address has 0x prefix and lowercase hex digits."""
    if not addr.lower().startswith("0x"):
        return f"0x{addr.lower()}"
    return addr.lower()


def _func_display_name(row: sqlite3.Row) -> str:
    """Return a display name for a function row."""
    name = row["name"]
    if name:
        return name
    addr = row["address"]
    return f"func_{addr[2:]}"


def _parse_signature(sig: str, addr: str) -> tuple[str, str, str, str] | None:
    """Parse a Ghidra function signature into (return_type, name, addr, params).

    Returns None if unparseable.
    """
    if not sig:
        return None
    paren = sig.find("(")
    if paren == -1:
        return None
    prefix = sig[:paren].strip()
    params = sig[paren:].strip()
    if not prefix:
        return None
    words = prefix.split()
    if not words:
        return None
    name = words[-1]
    ret = " ".join(words[:-1])
    return (ret, name, addr, params)


def _build_context(conn: sqlite3.Connection, addr: str) -> str:
    """Build the m2c context header content from the database."""
    addr = _normalize_addr(addr)

    cur = conn.execute(
        """SELECT address, name, signature, body_min, body_max, program_path
           FROM functions WHERE address = ?""",
        (addr,),
    )
    func = cur.fetchone()
    if func is None:
        print(f"Function not found at {addr}", file=sys.stderr)
        sys.exit(1)

    func_name = _func_display_name(func)
    func_program = func["program_path"]
    func_signature = func["signature"] or ""

    body_min_str = func["body_min"]
    body_max_str = func["body_max"]
    try:
        body_min = int(body_min_str, 16) if body_min_str else None
        body_max = int(body_max_str, 16) if body_max_str else None
    except (ValueError, TypeError):
        body_min = None
        body_max = None

    lines: list[str] = []

    # 1. Include
    lines.append('#include "bof3/bof3.h"')
    lines.append("")

    # 2. Comment block with function metadata
    lines.append("/*")
    lines.append(f" * Function:  {func_name}")
    lines.append(f" * Address:   {addr}")
    if body_min_str and body_max_str:
        lines.append(f" * Body:      {body_min_str} — {body_max_str}")
    lines.append(f" * Program:   {func_program}")
    if func_signature:
        lines.append(f" * Signature: {func_signature}")
    lines.append(" */")
    lines.append("")

    # 3. Called by (callers) — incoming call edges
    callers = conn.execute(
        """SELECT f.name, f.address, f.program_path
           FROM call_edges ce
           JOIN functions f ON f.address = ce.from_func
           WHERE ce.to_func = ?
           ORDER BY f.name""",
        (addr,),
    ).fetchall()

    lines.append(f"/* Called by ({len(callers)}): */")
    for c in callers:
        cname = _func_display_name(c)
        tag = " [same]" if c["program_path"] == func_program else " [ext]"
        lines.append(f"/*   {cname} ({c['address']}){tag} */")
    lines.append("")

    # 4. Calls (callees) — outgoing call edges
    callees = conn.execute(
        """SELECT f.name, f.address, f.program_path, f.signature
           FROM call_edges ce
           JOIN functions f ON f.address = ce.to_func
           WHERE ce.from_func = ?
           ORDER BY f.name""",
        (addr,),
    ).fetchall()

    lines.append(f"/* Calls ({len(callees)}): */")
    for c in callees:
        cname = _func_display_name(c)
        external = c["program_path"] != func_program
        tag = " [external]" if external else ""
        lines.append(f"/*   {cname} ({c['address']}){tag} */")
    lines.append("")

    # 5. Function declarations for callees
    if callees:
        lines.append("/* Callee declarations */")
        for c in callees:
            cname = c["name"] or f"func_{c['address'][2:]}"
            caddr = c["address"]
            csig = c["signature"] or ""
            external = c["program_path"] != func_program
            parsed = _parse_signature(csig, caddr)

            if parsed and external:
                ret, _, _, params = parsed
                lines.append(f"DEFINE_FUNC_AT({ret}, {cname}, {caddr}, {params});")
            elif parsed:
                ret, _, _, params = parsed
                lines.append(f"extern {ret} {cname}{params};")
            elif external:
                lines.append(
                    f"/* DEFINE_FUNC_AT(???, {cname}, {caddr}, ???); — signature unknown */"
                )
            else:
                lines.append(f"/* extern {cname}(???); — signature unknown */")
            lines.append("")

    lines.append("")

    # 6. Constants in the function's address range
    if body_min is not None and body_max is not None:
        all_consts = conn.execute(
            """SELECT address, name, data_type, program_path
               FROM constants
               WHERE program_path = ?
               ORDER BY address""",
            (func_program,),
        ).fetchall()

        nearby: list[sqlite3.Row] = []
        for c in all_consts:
            try:
                ci = int(c["address"], 16)
            except (ValueError, TypeError):
                continue
            if body_min <= ci < body_max:
                nearby.append(c)

        if nearby:
            lines.append("/* Constants used in function body: */")
            for c in nearby:
                cname = c["name"] or f"const_{c['address']}"
                dtype = c["data_type"] or "?"
                lines.append(f"/*   {c['address']}  {cname}  ({dtype}) */")
            lines.append("")

    # 7. Duplicate groups containing this function
    dup_rows = conn.execute(
        """SELECT sha256, program_count, entries_json
           FROM duplicates
           WHERE program_count > 1""",
    ).fetchall()

    matching_dups: list[sqlite3.Row] = []
    for d in dup_rows:
        try:
            entries = json.loads(d["entries_json"])
        except (json.JSONDecodeError, TypeError):
            continue
        for entry in entries:
            if isinstance(entry, dict) and entry.get("address") == addr:
                matching_dups.append(d)
                break

    if matching_dups:
        lines.append("/* Duplicate groups: */")
        for d in matching_dups:
            try:
                entries = json.loads(d["entries_json"])
            except (json.JSONDecodeError, TypeError):
                entries = []
            others: list[str] = []
            for e in entries:
                if not isinstance(e, dict):
                    continue
                eaddr = e.get("address", "")
                if eaddr == addr:
                    continue
                ename = e.get("name", "") or f"func_{eaddr[2:]}" if eaddr else "?"
                eprog = e.get("program_path", "?")
                others.append(f"{ename} ({eprog})")
            if others:
                lines.append(
                    f"/*   {d['program_count']} programs: " + ", ".join(others) + " */"
                )
        lines.append("")

    return "\n".join(lines)


def main(args: argparse.Namespace) -> int:
    db_path: Path = args.db
    addr: str = args.addr
    out_path: Path | None = args.out

    if not db_path.is_file():
        print(f"analysis DB not found: {db_path}", file=sys.stderr)
        print("Run bin/analysis-build first.", file=sys.stderr)
        return 1

    conn = sqlite3.connect(str(db_path))
    conn.row_factory = sqlite3.Row
    try:
        content = _build_context(conn, addr)
    finally:
        conn.close()

    content += "\n"

    if out_path:
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(content, encoding="utf-8")
        print(f"Wrote {out_path}", file=sys.stderr)
    else:
        sys.stdout.write(content)

    return 0


if __name__ == "__main__":
    run_main(build_parser)
