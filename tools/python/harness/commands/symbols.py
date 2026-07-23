"""Target-local symbol map commands."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys

from ..canonical import (
    Symbol,
    format_map,
    load_map,
    load_target_symbols,
    map_path,
    sdk_map_path,
    shared_map_path,
    weak_bindings_c,
    write_map,
)
from ..domain import load_target_manifests, normalize_target_id, parse_function_id
from ..io import repo_layout
from ._common import run_main


_WEAK_BINDING = re.compile(
    r"WEAK_SYMBOL_AT\(\s*(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*,\s*"
    r"(?P<address>0x[0-9A-Fa-f]+)\s*\)"
)


def _root(args: argparse.Namespace) -> Path:
    return args.root.resolve()


def _targets(root: Path, target: str | None, *, manifests: dict | None = None) -> list[str]:
    pool = manifests if manifests is not None else load_target_manifests(root)
    if target is None:
        return sorted(pool)
    normalized = normalize_target_id(target).value
    if normalized not in pool:
        raise ValueError(f"unknown target: {target}")
    return [normalized]


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


def _psyq_rows(path: Path) -> list[dict[str, object]]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict) or payload.get("schema") != "bof3.psyq-find/v1":
        raise ValueError(f"not a psyq-find proposal: {path}")
    rows = payload.get("matches")
    if not isinstance(rows, list):
        raise ValueError(f"invalid psyq-find matches: {path}")
    valid: list[dict[str, object]] = []
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError(f"invalid psyq-find match: {path}")
        if not all(
            isinstance(row.get(key), str) for key in ("target", "address", "name")
        ):
            raise ValueError(f"invalid psyq-find match: {path}")
        if row.get("confidence") == "exact" and row.get("external") is True:
            valid.append(row)
    return valid


def _selector(row: dict[str, object]) -> str:
    return f"{row['target']}@0x{int(str(row['address']), 16):08X}"


def _import_rows(args: argparse.Namespace) -> list[dict[str, object]]:
    rows = _psyq_rows(args.proposal)
    selectors = {
        f"{function.target.value}@0x{function.address:08X}"
        for function in (parse_function_id(value) for value in args.selectors)
    }
    if args.all_qualified and selectors:
        raise ValueError("--all-qualified cannot be combined with TARGET@ADDRESS")
    if not args.all_qualified and not selectors:
        raise ValueError("select at least one TARGET@ADDRESS or pass --all-qualified")
    selected = (
        rows
        if args.all_qualified
        else [row for row in rows if _selector(row) in selectors]
    )
    found = {_selector(row) for row in selected}
    missing = sorted(selectors - found)
    if missing:
        raise ValueError(f"proposal has no exact PsyQ match for: {missing[0]}")
    return selected


def run_import_psyq(args: argparse.Namespace) -> int:
    """Apply reviewed exact PsyQ provenance to the shared SDK map.

    PsyQ symbols are owned by the SDK space (slus/logo), not by one target, so
    candidates are grouped by the proposing target's space and written to
    config/sdk/psyq-<space>.txt where every target in that space picks them up.
    """
    root = _root(args)
    manifests = load_target_manifests(root)
    selected = _import_rows(args)
    by_space: dict[str, list[dict[str, object]]] = {}
    for row in selected:
        target = normalize_target_id(str(row["target"])).value
        if target not in manifests:
            raise ValueError(f"proposal names an unknown target: {target}")
        by_space.setdefault(manifests[target].psyq_space, []).append(row)
    changed = False
    for space, rows in sorted(by_space.items()):
        path = sdk_map_path(root, space)
        existing = load_map(path)
        by_address = {symbol.address: symbol for symbol in existing}
        by_name = {symbol.canonical_name: symbol for symbol in existing}
        replacement: dict[int, Symbol] = {}
        for row in sorted(
            rows, key=lambda item: (str(item["address"]), str(item["name"]))
        ):
            address = int(str(row["address"]), 16)
            candidate = Symbol(address, str(row["name"]))
            current = by_address.get(address)
            same_name = by_name.get(candidate.canonical_name)
            if same_name is not None and same_name.address != address:
                raise ValueError(
                    f"{space}: PsyQ name already belongs to "
                    f"0x{same_name.address:08X}: {candidate.canonical_name}"
                )
            if (
                current is not None
                and current.canonical_name == candidate.canonical_name
            ):
                continue
            if current is not None and not current.is_raw:
                raise ValueError(
                    f"{space}: address 0x{address:08X} already has semantic name "
                    f"{current.canonical_name}"
                )
            replacement[address] = candidate
            by_address[address] = candidate
            by_name[candidate.canonical_name] = candidate
        if not replacement:
            print(f"unchanged {path.relative_to(root)}")
            continue
        changed = True
        updated = [replacement.get(symbol.address, symbol) for symbol in existing]
        for address, symbol in replacement.items():
            if not any(old.address == address for old in existing):
                updated.append(symbol)
        format_map(updated)
        if args.write:
            write_map(path, updated)
            print(f"wrote {path.relative_to(root)}")
        else:
            print(f"would write {path.relative_to(root)}")
    return 1 if changed and not args.write else 0


def run_psyq_bindings(args: argparse.Namespace) -> int:
    """Generate src/<target>/symbols/psyq.c weak bindings from the SDK map.

    The build compiles these (CMake globs src/*.c); they provide link-time
    addresses for the PSX SDK functions each target calls. Only psyq.c is
    written, so authored semantic binding files are never touched.
    """
    root = _root(args)
    manifests = load_target_manifests(root)
    for target in _targets(root, args.target):
        space = manifests[target].psyq_space
        content = weak_bindings_c(load_map(sdk_map_path(root, space)))
        output = root / manifests[target].source_dir / "symbols" / "psyq.c"
        if args.write:
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_text(content, encoding="utf-8")
            print(output.relative_to(root))
        else:
            print(content, end="")
    return 0


def run_psyq_report(args: argparse.Namespace) -> int:
    """Report which SDK symbols each target's game code actually references.

    Regenerated on demand; this preserves the per-target "what it calls"
    evidence that generating the full SDK binding set otherwise discards.
    Scans lifted func_*.c and internal.h, not the generated bindings.
    """
    root = _root(args)
    manifests = load_target_manifests(root)
    for target in _targets(root, args.target):
        space = manifests[target].psyq_space
        sdk = load_map(sdk_map_path(root, space))
        source_dir = root / manifests[target].source_dir
        haystacks = sorted(source_dir.glob("func_*.c"))
        internal = source_dir / "internal.h"
        if internal.is_file():
            haystacks.append(internal)
        text = "".join(path.read_text(encoding="utf-8") for path in haystacks)
        referenced = sorted(
            (
                symbol
                for symbol in sdk
                if re.search(rf"\b{re.escape(symbol.canonical_name)}\b", text)
            ),
            key=lambda symbol: symbol.address,
        )
        print(
            f"{target} ({space}): {len(referenced)}/{len(sdk)} SDK symbols referenced"
        )
        for symbol in referenced:
            print(f"  {symbol.canonical_name} = 0x{symbol.address:08X}")
    return 0


def run_dedupe(args: argparse.Namespace) -> int:
    """Extract symbols duplicated across N+ targets into the shared base."""
    root = _root(args)
    threshold = args.threshold
    manifests = load_target_manifests(root)
    targets = sorted(manifests.keys())

    addr_count: dict[int, int] = {}
    addr_symbol: dict[int, Symbol] = {}
    for target in targets:
        seen: set[int] = set()
        for symbol in load_map(map_path(root, target)):
            if symbol.address not in seen:
                addr_count[symbol.address] = addr_count.get(symbol.address, 0) + 1
                addr_symbol[symbol.address] = symbol
                seen.add(symbol.address)

    shared_existing = load_map(shared_map_path(root))
    shared_addrs = {s.address for s in shared_existing}
    new_shared = [
        addr_symbol[addr]
        for addr, count in sorted(addr_count.items())
        if count >= threshold and addr not in shared_addrs
    ]
    if not new_shared:
        print(f"no symbols duplicated in {threshold}+ targets")
        return 0

    merged = sorted(shared_existing + new_shared)
    print(
        f"extracting {len(new_shared)} symbols into shared base ({len(merged)} total)"
    )
    if args.write:
        write_map(shared_map_path(root), merged)
        for target in targets:
            local = load_map(map_path(root, target))
            trimmed = [
                s for s in local if s.address not in {x.address for x in new_shared}
            ]
            if len(trimmed) < len(local):
                write_map(map_path(root, target), trimmed)
        print(f"wrote {shared_map_path(root).relative_to(root)}")
    else:
        for s in new_shared[:10]:
            print(
                f"  {s.canonical_name} = 0x{s.address:08X}; ({addr_count[s.address]}×)"
            )
        if len(new_shared) > 10:
            print(f"  ... and {len(new_shared) - 10} more")
        print("pass --write to apply")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="symbols")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
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
    import_psyq.add_argument("selectors", nargs="*", metavar="TARGET@ADDRESS")
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
    arguments = sys.argv[1:] if argv is None else argv
    if arguments == ["--example"]:
        print("bin/symbols normalize exe/logo --write")
        return 0
    return run_main(build_parser, arguments)


if __name__ == "__main__":
    raise SystemExit(main())
