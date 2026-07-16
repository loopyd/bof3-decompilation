"""Query the generated cross-target reverse index."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

from ..io import repo_layout
from ..reverse_index import connect, rows
from ._common import run_main


def _root(args: argparse.Namespace) -> Path:
    return args.root.resolve()


def _print(payload: list[dict[str, object]], as_json: bool) -> None:
    if as_json:
        print(json.dumps(payload, indent=2, sort_keys=True))
        return
    for row in payload:
        print("\t".join(str(value) for value in row.values()))


def run_query(args: argparse.Namespace) -> int:
    connection = connect(_root(args))
    try:
        limit = args.limit
        if args.command == "symbols":
            pattern = f"%{args.pattern}%" if args.pattern else "%"
            payload = rows(connection, "SELECT target_id, printf('0x%08X', address) AS address, name, kind FROM symbols WHERE name LIKE ? ORDER BY target_id, address, name LIMIT ?", (pattern, limit))
        elif args.command == "xrefs":
            payload = rows(connection, "SELECT target_id, printf('0x%08X', source) AS source, printf('0x%08X', destination) AS destination, kind FROM xrefs WHERE destination = ? ORDER BY target_id, source LIMIT ?", (int(args.address, 0), limit))
        elif args.command == "duplicates":
            payload = rows(connection, "SELECT d.hash, d.size, d.members, group_concat(m.function_id, ',') AS functions FROM duplicate_groups d JOIN duplicate_members m ON m.hash = d.hash GROUP BY d.hash ORDER BY d.members DESC, d.size DESC LIMIT ?", (limit,))
        elif args.command == "hotspots":
            payload = rows(connection, "SELECT f.id, f.size, COUNT(c.caller) AS callers FROM functions f LEFT JOIN calls c ON c.callee = f.id GROUP BY f.id ORDER BY callers DESC, f.size DESC LIMIT ?", (limit,))
        elif args.command == "leafs":
            payload = rows(connection, "SELECT f.id, f.size FROM functions f LEFT JOIN calls c ON c.caller = f.id GROUP BY f.id HAVING COUNT(c.callee) = 0 ORDER BY f.size DESC, f.id LIMIT ?", (limit,))
        elif args.command == "calls":
            payload = rows(connection, "SELECT caller, callee, printf('0x%08X', callsite) AS callsite FROM calls WHERE caller = ? OR callee = ? ORDER BY caller, callsite LIMIT ?", (args.function, args.function, limit))
        elif args.command == "variables":
            pattern = f"%{args.pattern}%" if args.pattern else "%"
            payload = rows(connection, "SELECT target_id, printf('0x%08X', address) AS address, name FROM symbols WHERE kind = 'data' AND name LIKE ? ORDER BY target_id, address LIMIT ?", (pattern, limit))
        else:  # status
            payload = rows(connection, "SELECT t.id, t.engine, COUNT(DISTINCT f.id) AS functions, COUNT(DISTINCT s.address) AS symbols FROM targets t LEFT JOIN functions f ON f.target_id = t.id LEFT JOIN symbols s ON s.target_id = t.id GROUP BY t.id ORDER BY t.id")
        _print(payload, args.json)
    finally:
        connection.close()
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="rev-query")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--limit", type=int, default=20)
    sub = parser.add_subparsers(dest="command", required=True)
    symbols = sub.add_parser("symbols", help="find canonical target-local symbols")
    symbols.add_argument("pattern", nargs="?")
    xrefs = sub.add_parser("xrefs", help="find indexed references to an address")
    xrefs.add_argument("address")
    calls = sub.add_parser("calls", help="show calls to or from TARGET@ADDRESS")
    calls.add_argument("function")
    variables = sub.add_parser("variables", help="list mapped data symbols")
    variables.add_argument("pattern", nargs="?")
    for name, help_text in (("duplicates", "show exact duplicate functions"), ("hotspots", "show most-called functions"), ("leafs", "show functions with no outgoing calls"), ("status", "show index coverage")):
        sub.add_parser(name, help=help_text)
    for command in sub.choices.values():
        command.set_defaults(handler=run_query)
    return parser


def main(argv: list[str] | None = None) -> int:
    arguments = sys.argv[1:] if argv is None else argv
    if arguments == ["--example"]:
        print("bin/rev-query symbols func_")
        return 0
    return run_main(build_parser, arguments)


if __name__ == "__main__":
    raise SystemExit(main())
