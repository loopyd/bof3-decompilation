"""Target-local symbol map commands."""

from __future__ import annotations

import argparse
from pathlib import Path
import re

from ..canonical import (
    format_map,
    load_map,
    load_target_symbols,
    map_path,
    weak_bindings_c,
    write_map,
)
from ..domain import (
    FUNCTION_ID_FORMAT,
    FUNCTION_ID_HELP,
    load_target_manifests,
)
from ._common import add_example_argument, add_root_argument, run_main

from ._symbols_psyq import (
    _root,
    _targets,
    run_dedupe,
    run_import_psyq,
    run_psyq_bindings,
    run_psyq_report,
)

_WEAK_BINDING = re.compile(
    r"WEAK_SYMBOL_AT\(\s*(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*,\s*"
    r"(?P<address>0x[0-9A-Fa-f]+)\s*\)"
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
    root = _root(args)
    manifests = load_target_manifests(root)
    errors: list[str] = []
    for target in _targets(root, args.target, manifests=manifests):
        manifest = manifests[target]
        path = map_path(root, target)
        if not path.is_file():
            errors.append(f"missing target map: {path.relative_to(root)}")
            continue
        try:
            symbols = load_map(path)
            if path.read_text(encoding="utf-8") != format_map(symbols):
                errors.append(f"unnormalized map: {path.relative_to(root)}")
        except ValueError as exc:
            errors.append(str(exc))
            continue
        addresses = {symbol.address for symbol in symbols}
        # Bindings may reference shared engine globals and PSX SDK symbols, not
        # just the target-local map; validate them against the composed set.
        by_address = {
            symbol.address: symbol for symbol in load_target_symbols(root, target)
        }
        source_dir = root / manifest.source_dir
        for source in source_dir.glob("func_*.c"):
            encoded = source.stem.removeprefix("func_")
            if len(encoded) != 8 or encoded != encoded.upper():
                errors.append(f"invalid lifted filename: {source.relative_to(root)}")
                continue
            try:
                address = int(encoded, 16)
            except ValueError:
                errors.append(f"invalid lifted filename: {source.relative_to(root)}")
                continue
            if address not in addresses:
                errors.append(
                    f"source/map drift: {source.relative_to(root)} has no map address"
                )
        bindings_dir = source_dir / "symbols"
        if bindings_dir.is_dir():
            for binding in sorted(bindings_dir.rglob("*.c")):
                for match in _WEAK_BINDING.finditer(
                    binding.read_text(encoding="utf-8")
                ):
                    address = int(match.group("address"), 0)
                    expected = by_address.get(address)
                    if expected is None or expected.canonical_name != match.group(
                        "name"
                    ):
                        errors.append(
                            f"binding/map drift: {binding.relative_to(root)} "
                            f"has {match.group('name')} at 0x{address:08X}"
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

def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="symbols")
    add_root_argument(parser)
    add_example_argument(parser, "bin/symbols normalize exe/logo --write")
    sub = parser.add_subparsers(dest="command", required=True)
    normalize = sub.add_parser("normalize", help="format target map files")
    normalize.add_argument("target", nargs="?")
    normalize.add_argument("--write", action="store_true")
    normalize.set_defaults(handler=run_normalize)
    check = sub.add_parser("check", help="validate target map(s)")
    check.add_argument("target", nargs="?")
    check.set_defaults(handler=run_check)
    bindings = sub.add_parser("bindings", help="generate target weak bindings")
    bindings.add_argument("target", nargs="?")
    bindings.add_argument("--write", action="store_true")
    bindings.set_defaults(handler=run_bindings)
    psyq_bindings = sub.add_parser(
        "psyq-bindings",
        help="generate src/<target>/symbols/psyq.c from the SDK map",
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
    dedupe = sub.add_parser(
        "dedupe", help="extract symbols duplicated across N+ targets into shared base"
    )
    dedupe.add_argument("--threshold", type=int, default=5)
    dedupe.add_argument("--write", action="store_true")
    dedupe.set_defaults(handler=run_dedupe)
    return parser

def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)

if __name__ == "__main__":
    raise SystemExit(main())
