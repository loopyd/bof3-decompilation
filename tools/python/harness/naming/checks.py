"""Derive exact function checks from frozen naming transaction scopes."""

from __future__ import annotations

from pathlib import Path

from harness.domain.claims import collect_manifest_source_addresses
from harness.domain.functions import FunctionRecord, parse_function_records
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.registry import resolve_function
from harness.domain.sources import expected_lift_sources, source_expected_key


def caller_records(
    root: Path, target: str, relative: str
) -> tuple[FunctionRecord, ...]:
    """Require complete, exact target-owned records for an affected caller."""
    source = root / relative
    try:
        records = parse_function_records(source.read_text(encoding="utf-8"))
    except ValueError as error:
        raise ValueError(f"invalid caller metadata: {relative}: {error}") from error
    if not records or any(record.status != "exact" for record in records):
        raise ValueError(
            f"function identity requires exact caller metadata: {relative}"
        )
    manifests = load_target_manifests(root)
    manifest = manifests[target]
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    expected = expected_lift_sources(layout, root / manifest.source_dir)
    key = source_expected_key(root / manifest.source_dir, source)
    addresses = expected.get(key)
    if len(records) > 1 or isinstance(addresses, tuple):
        owners = [
            entry.id.value
            for entry in manifests.values()
            if relative in entry.sources + entry.support_sources
        ]
        if owners != [target] or relative not in manifests[target].sources:
            raise ValueError(
                f"grouped caller requires one explicit lift owner: {relative}"
            )
        addresses = set(addresses) if isinstance(addresses, tuple) else {addresses}
        if {record.address for record in records} != addresses:
            raise ValueError(
                f"grouped caller differs from reviewed boundaries: {relative}"
            )
        claimed = {
            address
            for path, address in collect_manifest_source_addresses(
                root, manifest, expected_lifts=expected
            )
            if path == source
        }
        if claimed != addresses:
            raise ValueError(f"grouped caller claims are incomplete: {relative}")
    for record in records:
        selector = f"{target}@0x{record.address:08X}"
        resolved = resolve_function(root, selector)
        if resolved.source != source or resolved.compiled_symbol != record.spelling:
            raise ValueError(f"function caller ownership is ambiguous: {relative}")
    return records


def collect_function_checks(root: Path, row) -> list[dict]:
    facts = row["pre_apply"]["facts"]
    start, end = (int(value, 0) for value in facts["unchanged_range"].split(".."))
    checks = [
        {
            "selector": row["identity"]["selector"],
            "address": start,
            "size": end - start,
            "name": row["new_name"],
            "source": facts["destination"],
        }
    ]
    scope = facts["scope"]
    manifest = load_target_manifests(root)[scope["target"]]
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    selectors = {checks[0]["selector"]}
    for relative in sorted(set(scope["source_locations"])):
        if relative == scope["definition"] or not relative.endswith(".c"):
            continue
        for record in caller_records(root, scope["target"], relative):
            selector = f"{scope['target']}@0x{record.address:08X}"
            if selector in selectors:
                raise ValueError(f"function caller ownership is ambiguous: {relative}")
            boundary = layout.find_boundary_at(record.address)
            if boundary is None or not boundary.is_function:
                raise ValueError(f"function caller has no reviewed function boundary: {selector}")
            if (
                record.address % 4
                or boundary.virtual_size is None
                or boundary.virtual_size <= 0
                or boundary.virtual_size % 4
            ):
                raise ValueError(
                    f"function caller requires a finite aligned reviewed range: {selector}"
                )
            selectors.add(selector)
            checks.append(
                {
                    "selector": selector,
                    "address": record.address,
                    "size": boundary.virtual_size,
                    "name": record.spelling,
                    "source": relative,
                }
            )
    return checks
