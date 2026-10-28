"""Target-local symbol map commands."""

from __future__ import annotations

import argparse
from pathlib import Path

from harness.common.cli import add_example_argument, add_root_argument, run_main

from ..domain.ids import FUNCTION_ID_FORMAT, FUNCTION_ID_HELP
from ..domain.manifests import load_target_manifests
from harness.naming.debt import (
    collect_naming_debt,
    load_naming_baseline,
    merge_naming_baseline,
)
from ..domain.symbols import (
    format_map,
    load_map,
    load_target_symbols,
    map_path,
    weak_bindings_c,
    write_map,
)
from .symbols_psyq import (
    _root,
    _targets,
    run_import_psyq,
    run_psyq_bindings,
    run_psyq_report,
)


def run_normalize(args: argparse.Namespace) -> int:
    root = _root(args)
    changed = False
    for target in _targets(root, args.target):
        path = map_path(root, target)
        symbols = load_map(path)
        text = format_map(symbols)
        if path.is_file() and path.read_text(encoding="utf-8") == text:
            print(f"unchanged {path.relative_to(root)}")
            continue
        changed = True
        if args.write:
            write_map(path, symbols)
            print(f"wrote {path.relative_to(root)}")
        else:
            print(f"would write {path.relative_to(root)}")
    return 1 if changed and not args.write else 0


def run_check(args: argparse.Namespace) -> int:
    from harness.naming.map_checks import symbol_findings

    root = _root(args)
    targets = _targets(root, args.target)
    errors = (
        symbol_findings(root)
        if args.target is None
        else [error for target in targets for error in symbol_findings(root, target)]
    )
    if errors:
        raise ValueError("; ".join(errors))
    print("symbol maps: OK")
    return 0


def run_bindings(args: argparse.Namespace) -> int:
    root = _root(args)
    for target in _targets(root, args.target):
        output = root / "out" / "bindings" / target / "symbols.c"
        content = weak_bindings_c(load_target_symbols(root, target))
        if args.write:
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_text(content, encoding="utf-8")
            print(output.relative_to(root))
        else:
            print(content, end="")
    return 0


def run_baseline(args: argparse.Namespace) -> int:
    root = _root(args)
    manifests = load_target_manifests(root)
    debt = collect_naming_debt(root, manifests)
    baseline = load_naming_baseline(root)
    pending = {
        category: sorted(set(rows) - baseline.get(category, set()))
        for category, rows in debt.to_rows().items()
    }
    pending = {category: rows for category, rows in pending.items() if rows}
    if not pending:
        print("naming baseline: up to date")
        return 0
    if not args.write:
        for category, rows in sorted(pending.items()):
            for row in rows:
                print(f"would record ({category}): {row}")
        return 1
    added = merge_naming_baseline(root, debt)
    for category, rows in sorted(added.items()):
        for row in rows:
            print(f"recorded ({category}): {row}")
    print("wrote config/symbol-naming-baseline.json")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness source symbols")
    add_root_argument(parser)
    add_example_argument(
        parser, "bin/harness source symbols normalize exe/logo --write"
    )
    sub = parser.add_subparsers(dest="command", required=True)
    normalize = sub.add_parser("normalize", help="format target map files")
    normalize.add_argument("target", nargs="?")
    normalize.add_argument("--write", action="store_true")
    normalize.set_defaults(handler=run_normalize)
    check = sub.add_parser("check", help="validate target map(s)")
    check.add_argument("target", nargs="?")
    check.set_defaults(handler=run_check)
    baseline = sub.add_parser(
        "baseline",
        help="record current raw-spelling naming debt in the baseline",
    )
    baseline.add_argument("--write", action="store_true")
    baseline.set_defaults(handler=run_baseline)
    bindings = sub.add_parser("bindings", help="generate target weak bindings")
    bindings.add_argument("target", nargs="?")
    bindings.add_argument("--write", action="store_true")
    bindings.set_defaults(handler=run_bindings)
    psyq_bindings = sub.add_parser(
        "psyq-bindings",
        help="generate each target's manifest-owned psyq_source bindings from the SDK map",
    )
    psyq_bindings.add_argument("target", nargs="?")
    psyq_bindings.add_argument("--write", action="store_true")
    psyq_bindings.set_defaults(handler=run_psyq_bindings)
    psyq_report = sub.add_parser(
        "psyq-report",
        help="report which SDK symbols each target's game code references",
    )
    psyq_report.add_argument("target", nargs="?")
    psyq_report.set_defaults(handler=run_psyq_report)
    import_psyq = sub.add_parser(
        "import-psyq", help="apply reviewed exact PsyQ provenance to target maps"
    )
    import_psyq.add_argument("proposal", type=Path)
    import_psyq.add_argument(
        "selectors", nargs="*", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP
    )
    import_psyq.add_argument("--all-qualified", action="store_true")
    import_psyq.add_argument("--write", action="store_true")
    import_psyq.set_defaults(handler=run_import_psyq)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
