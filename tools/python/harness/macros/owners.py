"""Resolve target-owned macro application paths and function claims."""

from pathlib import Path
from typing import Any

from harness.domain import functions as source_functions
from harness.domain.claims import manifest_header_paths, manifest_source_paths
from harness.domain.ids import normalize_target_id
from harness.domain.registry import resolve_function


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
        source_functions.require_single_source(resolved.source)
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
    return result


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
