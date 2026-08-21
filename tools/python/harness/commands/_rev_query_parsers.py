"""rev-query argparse wiring; dispatch handlers resolve lazily from the owner."""

from __future__ import annotations

import argparse

from ..domain import FUNCTION_ID_FORMAT, FUNCTION_ID_HELP
from ..output import add_detail_argument
from ._common import add_example_argument, add_root_argument


def build_parser() -> argparse.ArgumentParser:
    from .rev_query import run_mission, run_query

    parser = argparse.ArgumentParser(prog="rev-query")
    add_root_argument(parser)
    add_example_argument(parser, "bin/rev-query symbols func_")
    parser.add_argument("--json", action="store_true")

    def nonnegative(value: str) -> int:
        parsed = int(value)
        if parsed < 0:
            raise argparse.ArgumentTypeError("must be nonnegative")
        return parsed

    parser.add_argument(
        "--limit", type=nonnegative, default=20, help="maximum rows; 0 means all"
    )
    sub = parser.add_subparsers(dest="command", required=True)
    describe = sub.add_parser(
        "describe",
        help="describe target-qualified payload, Splat, symbol, and references",
    )
    describe.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    symbols = sub.add_parser("symbols", help="find canonical target-local symbols")
    symbols.add_argument("pattern", nargs="?")
    xrefs = sub.add_parser(
        "xrefs", help="find target-local indexed references to an address"
    )
    xrefs.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    transaction_scope = sub.add_parser(
        "transaction-scope",
        help="derive the exact tracked files one spelling rename must touch",
    )
    transaction_scope.add_argument("target")
    transaction_scope.add_argument("symbol", help="current raw or semantic symbol name")
    inventory = sub.add_parser(
        "inventory", help="list target-local raw function and data naming debt"
    )
    inventory.add_argument("target")
    owners = sub.add_parser(
        "owners",
        help="find other indexed images with function bytes covering an address",
    )
    owners.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    calls = sub.add_parser("calls", help="show calls to or from a function selector")
    calls.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    variables = sub.add_parser("variables", help="list mapped data symbols")
    variables.add_argument("pattern", nargs="?")
    types = sub.add_parser("types", help="list target-owned type declarations")
    types.add_argument("pattern", nargs="?")
    types.add_argument("--target")
    types.add_argument("--untyped", action="store_true")
    add_detail_argument(types)
    type_uses = sub.add_parser("type-uses", help="list indexed type use sites")
    type_uses.add_argument("pattern", nargs="?")
    type_uses.add_argument("--target")
    add_detail_argument(type_uses)
    macros = sub.add_parser("macros", help="list indexed existing macros and templates")
    macros.add_argument("pattern", nargs="?")
    macros.add_argument("--target")
    macros.add_argument("--classification")
    macro_uses = sub.add_parser("macro-uses", help="list indexed macro expansion sites")
    macro_uses.add_argument("pattern", nargs="?")
    macro_uses.add_argument("--target")
    macro_opportunities = sub.add_parser(
        "macro-opportunities",
        help="rank blocked repetitive-source and exact-group leads",
    )
    macro_opportunities.add_argument("--target")
    macro_opportunities.add_argument(
        "--kind",
        choices=("constant", "expression_accessor", "statement_window", "exact_group"),
    )
    near_duplicates = sub.add_parser(
        "near-duplicates", help="rank blocked immediate-only function similarity leads"
    )
    near_duplicates.add_argument("--target")
    type_candidates = sub.add_parser(
        "type-candidates", help="list evidence-only representation candidates"
    )
    type_candidates.add_argument("--target")
    type_candidates.add_argument(
        "--status", choices=("blocked", "proposed", "accepted", "rejected")
    )
    type_candidates.add_argument("--kind")
    add_detail_argument(type_candidates)
    ranked = (
        ("metrics", "show raw and derived function metrics"),
        ("quick-wins", "rank low-effort, high-leverage candidates"),
        ("hotspots", "rank high-impact functions"),
        ("leafs", "show SCC-aware leaf candidates"),
        ("pareto", "show nondominated effort/value candidates"),
        ("duplicates", "show exact duplicate groups"),
        (
            "analyzer-candidates",
            "show unconfirmed analyzer-equality candidate groups",
        ),
    )
    for name, help_text in ranked:
        command = sub.add_parser(name, help=help_text)
        command.add_argument("--json", action="store_true", default=argparse.SUPPRESS)
        command.add_argument("--limit", type=nonnegative, default=argparse.SUPPRESS)
        add_detail_argument(command)
        command.add_argument("--target")
        if name not in {"duplicates", "analyzer-candidates"}:
            command.add_argument(
                "--exclusions",
                action="store_true",
                help="show candidate rows rejected by canonical-code checks",
            )
        command.add_argument("--unlifted", action="store_true")
        command.add_argument(
            "--include-trivial",
            action="store_true",
            help="include classified return-only stubs",
        )
        if name in {"metrics", "duplicates", "analyzer-candidates"}:
            command.add_argument("function", nargs="?")
    sub.add_parser("status", help="show index coverage")
    for command in sub.choices.values():
        command.set_defaults(handler=run_query)
    mission = sub.add_parser("mission", help="compose a single-function lifting brief")
    mission.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    mission.add_argument("--json", action="store_true", default=argparse.SUPPRESS)
    mission.set_defaults(handler=run_mission)
    return parser


__all__ = ["build_parser"]
