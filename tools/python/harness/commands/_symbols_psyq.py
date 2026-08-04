"""PsyQ-backed symbol commands: import, bindings, report, and dedupe."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re

from ..canonical import (
    Symbol,
    format_map,
    load_map,
    map_path,
    sdk_map_path,
    shared_map_path,
    weak_bindings_c,
    write_map,
)
from ..domain import (
    FUNCTION_ID_FORMAT,
    load_target_manifests,
    normalize_target_id,
    parse_function_id,
)


def _root(args: argparse.Namespace) -> Path:
    return args.root.resolve()


def _targets(
    root: Path, target: str | None, *, manifests: dict | None = None
) -> list[str]:
    pool = manifests if manifests is not None else load_target_manifests(root)
    if target is None:
        return sorted(pool)
    normalized = normalize_target_id(target).value
    if normalized not in pool:
        raise ValueError(f"unknown target: {target}")
    return [normalized]


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
        raise ValueError(
            f"--all-qualified cannot be combined with {FUNCTION_ID_FORMAT}"
        )
    if not args.all_qualified and not selectors:
        raise ValueError(
            f"select at least one {FUNCTION_ID_FORMAT} or pass --all-qualified"
        )
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
