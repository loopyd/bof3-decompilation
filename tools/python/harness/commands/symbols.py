"""Target-local symbol map commands."""

from __future__ import annotations

import argparse
from pathlib import Path

from ..domain.tags import (
    PREFIXED_RAW_NAME_RE,
    RAW_SYMBOL_NAME_RE,
    parse_declaration_source_tag,
)
from ..domain.symbols import (
    format_map,
    load_map,
    load_target_symbols,
    map_path,
    sdk_map_path,
    shared_map_path,
    weak_bindings_c,
    parse_weak_symbol_bindings,
    write_map,
)
from ..domain.claims import (
    collect_manifest_source_addresses,
    manifest_header_paths,
    manifest_source_paths,
    resolve_manifest_source_for_address,
)
from ..domain.naming_debt import (
    collect_naming_debt,
    load_naming_baseline,
    naming_debt_regressions,
)
from ..domain.sources import (
    LiftMetadataError,
    SourceAddressCollision,
    expected_lift_sources,
)
from ..domain import (
    FUNCTION_ID_FORMAT,
    FUNCTION_ID_HELP,
    load_target_manifests,
)
from ..domain.identity import (
    collision_findings,
    composed_map_findings,
    reviewed_function_identities,
    splat_source_findings,
)
from ..domain.layout import parse_splat_layout
from ._common import add_example_argument, add_root_argument, run_main

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
        try:
            layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
            expected = expected_lift_sources(layout, source_dir)
        except (OSError, ValueError):
            expected = {}
        try:
            lift_rows = collect_manifest_source_addresses(
                root, manifest, expected_lifts=expected
            )
        except LiftMetadataError as exc:
            errors.append(str(exc))
            lift_rows = []
        except SourceAddressCollision as exc:
            errors.append(str(exc))
            lift_rows = []
        for source, address in lift_rows:
            if address not in addresses:
                errors.append(
                    f"source/map drift: {source.relative_to(root)} "
                    f"(0x{address:08X}) has no map address"
                )
        # Generated bindings (strict: name must equal the composed map) and
        # hand-maintained top-level bindings (lenient: only addresses no map
        # owns; a different name at a mapped address is a deliberate typed
        # alias).  Migrated targets name their support files explicitly: the
        # generated PsyQ source is the strict set and every other claimed
        # support ``.c`` is hand-maintained.  Legacy targets (and migrated
        # targets keeping the legacy support layout unclaimed) fall back to
        # ``source_dir/symbols/*.c`` plus ``source_dir/symbols.c``.
        if manifest.has_explicit_sources and any(
            Path(claimed).suffix == ".c" for claimed in manifest.support_sources
        ):
            psyq = Path(manifest.psyq_source) if manifest.psyq_source else None
            claimed_c = [
                root / claimed
                for claimed in manifest.support_sources
                if Path(claimed).suffix == ".c"
            ]
            strict_files = (
                [root / psyq] if psyq is not None and (root / psyq) in claimed_c else []
            )
            lenient_files = [path for path in claimed_c if path not in strict_files]
        else:
            bindings_dir = source_dir / "symbols"
            strict_files = (
                sorted(bindings_dir.rglob("*.c")) if bindings_dir.is_dir() else []
            )
            lenient_files = []
            top_level = source_dir / "symbols.c"
            if top_level.is_file():
                lenient_files.append(top_level)
        for binding in strict_files:
            for name, address in parse_weak_symbol_bindings(
                binding.read_text(encoding="utf-8")
            ).items():
                expected = by_address.get(address)
                if expected is None or expected.canonical_name != name:
                    errors.append(
                        f"binding/map drift: {binding.relative_to(root)} "
                        f"has {name} at 0x{address:08X}"
                    )
        top_text = ""
        for top_level in lenient_files:
            text = top_level.read_text(encoding="utf-8")
            top_text += text + "\n"
            for name, address in parse_weak_symbol_bindings(text).items():
                if address not in by_address:
                    errors.append(
                        f"binding/map drift: {top_level.relative_to(root)} "
                        f"has {name} at 0x{address:08X}"
                    )
        # Naming rule: raw hex names must be the whole name; conflicts resolve
        # by a different name or a suffix, never an overlay-name prefix.
        for symbol in symbols:
            name = symbol.canonical_name
            if not RAW_SYMBOL_NAME_RE.fullmatch(name) and (
                PREFIXED_RAW_NAME_RE.search(name)
            ):
                errors.append(f"prefixed raw name: {path.relative_to(root)} has {name}")
        # Tracking rule: every non-address-named game symbol (SDK exempt)
        # needs a definition carrying its origin address: a lift file with a
        # matching @source tag, a tagged header declaration, or a matching
        # WEAK_SYMBOL_AT binding.
        sdk_names = {
            s.canonical_name for s in load_map(sdk_map_path(root, manifest.psyq_space))
        }
        header = source_dir / "internal.h"
        header_text = header.read_text(encoding="utf-8") if header.is_file() else ""
        owned = symbols
        for symbol in owned:
            name = symbol.canonical_name
            if RAW_SYMBOL_NAME_RE.fullmatch(name) or name in sdk_names:
                continue
            lift = resolve_manifest_source_for_address(root, manifest, symbol.address)
            if lift is not None:
                continue
            declared = parse_declaration_source_tag(header_text, name)
            if declared != symbol.address:
                sources = manifest_header_paths(root, manifest) + sorted(
                    path
                    for path in manifest_source_paths(root, manifest)
                    if path.suffix == ".c"
                )
                for source in sources:
                    if source == header:
                        continue
                    declared = parse_declaration_source_tag(
                        source.read_text(encoding="utf-8"), name
                    )
                    if declared == symbol.address:
                        break
            if declared == symbol.address:
                continue
            if parse_weak_symbol_bindings(top_text).get(name) == symbol.address:
                continue
            errors.append(
                f"untracked symbol: {path.relative_to(root)} has {name} = "
                f"0x{symbol.address:08X} with no @source-tagged definition"
            )
    if args.target is None:
        debt = collect_naming_debt(root, manifests)
        errors.extend(naming_debt_regressions(debt, load_naming_baseline(root)))
        identities = [
            identity
            for target, manifest in manifests.items()
            for identity in reviewed_function_identities(root, target, manifest)
        ]
        errors.extend(
            str(finding)
            for finding in collision_findings(identities)
            if finding.verdict == "reject"
        )
        for target, manifest in manifests.items():
            layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
            errors.extend(
                str(finding)
                for finding in splat_source_findings(target, layout)
                if finding.verdict == "reject"
            )
            layers = {
                "shared": load_map(shared_map_path(root)),
                "sdk": load_map(sdk_map_path(root, manifest.psyq_space)),
                "local": load_map(map_path(root, target)),
            }
            errors.extend(
                str(finding)
                for finding in composed_map_findings(layers)
                if finding.verdict == "reject"
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
