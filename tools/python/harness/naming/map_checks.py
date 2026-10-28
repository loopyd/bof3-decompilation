"""Mechanical symbol-map, source-claim, binding and naming-debt checks."""

from __future__ import annotations

from pathlib import Path

from harness.domain.claims import (
    collect_manifest_source_addresses,
    manifest_header_paths,
    manifest_source_paths,
)
from harness.domain.identity import (
    collision_findings,
    composed_map_findings,
    reviewed_function_identities,
    splat_source_findings,
)
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.sources import (
    LiftMetadataError,
    SourceAddressCollision,
    expected_lift_sources,
)
from harness.domain.symbols import (
    format_map,
    load_map,
    load_target_symbols,
    map_path,
    parse_weak_symbol_bindings,
    sdk_map_path,
    shared_map_path,
)
from harness.domain.tags import (
    PREFIXED_RAW_NAME_RE,
    RAW_SYMBOL_NAME_RE,
    parse_declaration_source_tag,
)
from .debt import collect_naming_debt, load_naming_baseline, naming_debt_regressions


def symbol_findings(
    root: Path, selected_target: str | None = None, *, reviewed: bool = False
) -> list[str]:
    manifests = load_target_manifests(root)
    errors: list[str] = []
    for target in sorted(manifests) if selected_target is None else [selected_target]:
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
            symbol.address: symbol
            for symbol in load_target_symbols(
                root, target, psyq_space=manifest.psyq_space
            )
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
        lift_addresses = {address for _source, address in lift_rows}
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
            if symbol.address in lift_addresses:
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
    if selected_target is None or reviewed:
        selected_manifests = (
            manifests
            if selected_target is None
            else {selected_target: manifests[selected_target]}
        )
        debt = collect_naming_debt(root, selected_manifests)
        if selected_target is not None:
            from dataclasses import replace

            claimed = set(manifests[selected_target].sources)
            debt = replace(
                debt,
                raw_function_files=frozenset(debt.raw_function_files & claimed),
                invalid_semantic_files=frozenset(debt.invalid_semantic_files & claimed),
            )
        errors.extend(naming_debt_regressions(debt, load_naming_baseline(root)))
        identities = [
            identity
            for target, manifest in selected_manifests.items()
            for identity in reviewed_function_identities(root, target, manifest)
        ]
        if selected_target is not None:
            names = {
                identity.folded_name
                for identity in identities
                if identity.compiled_name
                and not identity.compiled_name.startswith(("func_", "D_"))
            }
            for target, manifest in manifests.items():
                if target == selected_target:
                    continue
                if any(
                    symbol.canonical_name.casefold() in names
                    for symbol in load_map(map_path(root, target))
                ):
                    identities.extend(
                        identity
                        for identity in reviewed_function_identities(
                            root, target, manifest
                        )
                        if identity.folded_name in names
                    )
        errors.extend(
            str(finding)
            for finding in collision_findings(identities)
            if finding.verdict == "reject"
            and (
                selected_target is None
                or selected_target in {finding.left_target, finding.right_target}
            )
        )
        for target, manifest in selected_manifests.items():
            layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
            errors.extend(
                str(finding)
                for finding in splat_source_findings(root, target, manifest, layout)
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
    return errors
