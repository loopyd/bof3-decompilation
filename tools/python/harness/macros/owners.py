"""Resolve target-owned macro application paths and function claims."""

import hashlib
from pathlib import Path
from typing import Any

from harness.common.files import read_file
from harness.domain import functions as source_functions
from harness.domain.claims import (
    collect_manifest_source_addresses,
    manifest_header_paths,
    manifest_source_paths,
)
from harness.domain.ids import normalize_target_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.registry import resolve_function
from harness.domain.sources import expected_lift_sources, source_expected_key
from harness.domain.tags import count_function_metadata


def resolve_target(value: object, manifests: dict[str, Any]) -> str:
    if not isinstance(value, str):
        raise ValueError("macro transaction target must be canonical")
    try:
        target = normalize_target_id(value).value
    except ValueError as error:
        raise ValueError(
            f"unknown or non-canonical macro transaction target: {value}"
        ) from error
    if target != value or target not in manifests:
        raise ValueError(f"unknown or non-canonical macro transaction target: {value}")
    return target


def resolve_functions(
    root: Path, connection: Any, values: object, targets: set[str]
) -> list[dict[str, str]]:
    if (
        not isinstance(values, list)
        or not values
        or any(not isinstance(item, str) for item in values)
    ):
        raise ValueError("affected_functions must be non-empty strings")
    result = []
    for selector in sorted(set(values)):
        if "@0x" not in selector:
            raise ValueError(f"invalid affected function selector: {selector}")
        target, raw = selector.rsplit("@0x", 1)
        if target not in targets:
            raise ValueError(f"affected function selector has wrong target: {selector}")
        try:
            address = int(raw, 16)
        except ValueError as error:
            raise ValueError(
                f"invalid affected function selector: {selector}"
            ) from error
        try:
            resolved = resolve_function(root, selector)
        except (OSError, RuntimeError, ValueError) as error:
            raise ValueError(
                f"affected function identity is not canonical: {selector}"
            ) from error
        row = connection.execute(
            "SELECT lift_status FROM functions WHERE target_id=? AND address=?",
            (target, address),
        ).fetchone()
        if (
            row is None
            or row[0] not in {"exact", "partial"}
            or resolved.source is None
            or resolved.compiled_symbol is None
        ):
            raise ValueError(
                f"affected function must be exact/partial and manifest-claimed: {selector}"
            )
        result.append(
            {
                "selector": selector,
                "target": target,
                "address": f"0x{address:08X}",
                "function": resolved.compiled_symbol,
                "status": row[0],
                "source": resolved.source.relative_to(root).as_posix(),
            }
        )
    _validate_function_groups(root, connection, result)
    return result


def _validate_function_groups(
    root: Path, connection: Any, functions: list[dict[str, str]]
) -> None:
    manifests = load_target_manifests(root)
    groups = {}
    for function in functions:
        groups.setdefault((function["target"], function["source"]), []).append(function)
    for (target, name), selected in groups.items():
        raw = read_file(root, name, max_bytes=4 * 1024 * 1024)
        text = raw.decode("utf-8")
        if count_function_metadata(text) <= 1:
            source_functions.validate_single_source_changes({name: text})
            continue
        records = source_functions.parse_function_records(text)
        manifest = manifests[target]
        owners = [
            entry.id.value
            for entry in manifests.values()
            if name in entry.sources + entry.support_sources
        ]
        if owners != [target] or name not in manifest.sources:
            raise ValueError(
                f"grouped macro source requires one explicit lift owner: {name}"
            )
        layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
        expected = expected_lift_sources(layout, root / manifest.source_dir)
        addresses = expected.get(
            source_expected_key(root / manifest.source_dir, root / name)
        )
        addresses = set(addresses) if isinstance(addresses, tuple) else {addresses}
        claimed = {
            address
            for path, address in collect_manifest_source_addresses(
                root, manifest, expected_lifts=expected
            )
            if path == root / name
        }
        identities = {
            (record.address, record.spelling, record.status) for record in records
        }
        supplied = {
            (int(function["address"], 16), function["function"], function["status"])
            for function in selected
        }
        indexed = connection.execute(
            "SELECT address, compiled_symbol, lift_status FROM functions "
            "WHERE target_id=? AND source IN (?, ?)",
            (target, name, str(root / name)),
        ).fetchall()
        fingerprints = connection.execute(
            "SELECT sha256 FROM macro_input_fingerprints WHERE target_id=? "
            "AND source_path=? AND input_kind='source_claim' AND owner_target=?",
            (target, name, target),
        ).fetchall()
        if (
            any(record.kind != "function" for record in records)
            or {record.address for record in records} != addresses
            or claimed != addresses
            or supplied != identities
            or len(selected) != len(records)
            or len(indexed) != len(records)
            or {tuple(row) for row in indexed} != identities
            or len(fingerprints) != 1
            or fingerprints[0][0] != hashlib.sha256(raw).hexdigest()
        ):
            raise ValueError(
                f"grouped macro source membership or index differs: {name}"
            )


def validate_source_changes(
    root: Path, manifest: dict[str, Any], changes: object
) -> None:
    if not isinstance(changes, dict):
        return
    affected_sources = {item["source"] for item in manifest["affected_functions"]}
    for name, content in changes.items():
        if (
            not isinstance(name, str)
            or not name.endswith(".c")
            or not isinstance(content, str)
        ):
            continue
        if name not in manifest["allowed_paths"]:
            source_functions.validate_single_source_changes({name: content})
            continue
        before = read_file(root, name, max_bytes=4 * 1024 * 1024).decode("utf-8")
        if count_function_metadata(before) <= 1:
            source_functions.validate_single_source_changes({name: content})
            continue
        if name not in affected_sources:
            raise ValueError(
                f"grouped macro edit requires every affected function: {name}"
            )
        old = source_functions.parse_function_records(before)
        new = source_functions.parse_function_records(content)

        def identity(record):
            return (
                record.address,
                record.metadata,
                record.spelling,
                record.kind,
                record.status,
            )

        def context(text, records):
            parts, position = [], 0
            for record in records:
                parts.append(text[position : record.implementation_start])
                position = record.implementation_end
            return "".join(parts) + text[position:]

        if tuple(map(identity, old)) != tuple(map(identity, new)) or context(
            before, old
        ) != context(content, new):
            raise ValueError(
                f"grouped macro edit must preserve records and source context: {name}"
            )


def target_owned_paths(root: Path, target: str, manifest: Any) -> set[str]:
    paths = manifest_source_paths(root, manifest) + manifest_header_paths(
        root, manifest
    )
    paths.extend(
        root / name
        for name in (
            f"config/targets/{target}/target.toml",
            manifest.splat,
            f"config/targets/{target}/symbols.txt",
            f"config/targets/{target}/reviewed.rz",
        )
    )
    return {
        path.resolve().relative_to(root.resolve()).as_posix()
        for path in paths
        if path.is_file()
    }
