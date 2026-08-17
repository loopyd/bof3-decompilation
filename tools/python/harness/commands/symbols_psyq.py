"""PsyQ-backed symbol command handlers (import, bindings, report, dedupe).

Algorithms live in ``psyq.bindings`` and ``domain.symbols``; this module
only adapts parsed arguments, prints, and maps exit codes.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from ..domain import (
    load_target_manifests,
    normalize_target_id,
    parse_function_id,
)
from ..domain.symbols import dedupe_shared_symbols
from ..psyq.bindings import (
    apply_psyq_provenance,
    parse_psyq_find,
    sdk_references,
    sdk_weak_bindings,
    select_import_rows,
)
from ._common import resolved_root


_root = resolved_root


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


def run_import_psyq(args: argparse.Namespace) -> int:
    """Apply reviewed exact PsyQ provenance to the shared SDK map."""
    root = _root(args)
    manifests = load_target_manifests(root)
    rows = parse_psyq_find(args.proposal)
    selectors = {
        f"{function.target.value}@0x{function.address:08X}"
        for function in (parse_function_id(value) for value in args.selectors)
    }
    selected = select_import_rows(rows, selectors, args.all_qualified)
    changed, messages = apply_psyq_provenance(
        root, manifests, selected, write=args.write
    )
    for message in messages:
        print(message)
    return 1 if changed and not args.write else 0


def run_psyq_bindings(args: argparse.Namespace) -> int:
    """Generate each target's claimed PsyQ weak-binding source from the SDK map."""
    root = _root(args)
    manifests = load_target_manifests(root)
    for target in _targets(root, args.target):
        manifest = manifests[target]
        content = sdk_weak_bindings(root, manifest)
        output = root / manifest.psyq_source
        if args.write:
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_text(content, encoding="utf-8")
            print(output.relative_to(root))
        else:
            print(content, end="")
    return 0


def run_psyq_report(args: argparse.Namespace) -> int:
    """Report which SDK symbols each target's game code actually references."""
    root = _root(args)
    manifests = load_target_manifests(root)
    for target in _targets(root, args.target):
        space, referenced, total = sdk_references(root, manifests, target)
        print(f"{target} ({space}): {len(referenced)}/{total} SDK symbols referenced")
        for symbol in referenced:
            print(f"  {symbol.canonical_name} = 0x{symbol.address:08X}")
    return 0


def run_dedupe(args: argparse.Namespace) -> int:
    """Extract symbols duplicated across N+ targets into the shared base."""
    root = _root(args)
    manifests = load_target_manifests(root)
    _changed, messages = dedupe_shared_symbols(
        root, manifests, args.threshold, write=args.write
    )
    for message in messages:
        print(message)
    return 0
