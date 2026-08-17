"""Resolved target registry.

Every target identity, path, and mapping fact lives here.  All other
modules receive a ``ResolvedTarget`` instead of independently resolving
paths from a ``TargetManifest``.
"""

from __future__ import annotations

import hashlib
from dataclasses import dataclass
from pathlib import Path

from .layout import parse_splat_layout
from .claims import (
    manifest_header_paths,
    manifest_source_paths,
    resolve_manifest_source_for_address,
)
from .ids import FunctionId, TargetId, normalize_target_id, parse_function_id
from .manifests import TargetManifest, load_target_manifests
from .sources import CompiledSymbolError, reviewed_function_name


@dataclass(frozen=True)
class ResolvedTarget:
    """A fully resolved target with absolute repository-relative paths."""

    id: TargetId
    manifest_path: Path
    disc_id: str
    kind: str
    source_dir: Path
    binary_path: Path
    splat_path: Path
    reviewed_replay_path: Path
    load_address: int
    # Explicit build inputs and private headers (claim-aware; empty when the
    # target is unmigrated and uses the legacy source_dir inventory).
    source_paths: tuple[Path, ...] = ()
    header_paths: tuple[Path, ...] = ()

    @property
    def binary_end(self) -> int:
        return self.load_address + self.binary_size

    @property
    def binary_size(self) -> int:
        return self.binary_path.stat().st_size

    def function_id(self, address: int) -> str:
        return f"{self.id.value}@{address:08x}"

    def input_hash(self) -> str:
        """Return a composite hash of tracked input files."""

        pieces = [
            self.manifest_path.read_bytes() if self.manifest_path.is_file() else b"",
            self.binary_path.read_bytes() if self.binary_path.is_file() else b"",
            self.splat_path.read_bytes() if self.splat_path.is_file() else b"",
        ]
        return hashlib.sha256(
            b"\x00".join(hashlib.sha256(piece).digest() for piece in pieces)
        ).hexdigest()[:16]


@dataclass(frozen=True)
class ResolvedFunction:
    """One target-qualified function and every owned fact about it.

    Composes the existing authorities: manifest identity (target), metadata
    claim (source path), and map/Splat-agreed compiled symbol.  ``source`` is
    None when no authored lift claims the address; ``compiled_symbol`` is None
    when the target-local map/Splat has not yet agreed on a function symbol
    (e.g. a raw Splat boundary not yet owned by the map).
    """

    id: FunctionId
    target: ResolvedTarget
    manifest: TargetManifest
    source: Path | None
    compiled_symbol: str | None


def resolve_function(root: Path, value: str | FunctionId) -> ResolvedFunction:
    """Resolve one ``TARGET@0xADDRESS`` selector (or parsed ID) to a function.

    Shares :func:`resolve_target`'s canonical manifest path/identity
    validation: a missing canonical manifest raises ``FileNotFoundError`` and
    a misplaced manifest whose ``id`` disagrees with its canonical path
    raises ``RuntimeError``.  Unlike :func:`resolve_target`, the target
    binary is intentionally not required.  The compiled symbol is computed
    only when the target-local map/Splat agree on one; the source claim is
    optional.
    """

    function = parse_function_id(value) if isinstance(value, str) else value
    manifest = lookup_target_manifest(root, function.target.value)
    if manifest is None:
        raise ValueError(f"unknown target: {function.target.value!r}")
    target_id = normalize_target_id(function.target.value)
    manifest_path = root / "config" / "targets" / target_id.value / "target.toml"
    if not manifest_path.is_file():
        raise FileNotFoundError(f"target manifest missing: {manifest_path}")
    _validate_manifest_identity(root, target_id, manifest, manifest_path)
    target = _resolved_target(root, target_id, manifest)
    source = resolve_manifest_source_for_address(root, manifest, function.address)
    try:
        compiled_symbol = reviewed_function_name(
            root,
            target.id.value,
            function.address,
            layout=parse_splat_layout(root / manifest.splat, manifest.load_address),
        )
    except (CompiledSymbolError, OSError):
        # Missing layout or a not-yet-owned map entry means no agreed
        # compiled symbol; callers that require one report the absence.
        compiled_symbol = None
    return ResolvedFunction(
        id=function,
        target=target,
        manifest=manifest,
        source=source,
        compiled_symbol=compiled_symbol,
    )


def resolve_target(root: Path, value: str) -> ResolvedTarget:
    """Resolve a shipped or canonical ID to a ``ResolvedTarget``.

    Raises ``ValueError`` if the ID is invalid, ``FileNotFoundError`` if
    the manifest or binary is missing, and ``RuntimeError`` if the manifest
    is structurally inconsistent with the canonical identity.
    """

    target_id = normalize_target_id(value)
    manifests = load_target_manifests(root)
    manifest = manifests.get(target_id.value)
    if manifest is None:
        raise ValueError(f"unknown target: {value!r} (canonical: {target_id.value!r})")
    manifest_path = root / "config" / "targets" / target_id.value / "target.toml"
    if not manifest_path.is_file():
        raise FileNotFoundError(f"target manifest missing: {manifest_path}")
    _validate_manifest_identity(root, target_id, manifest, manifest_path)
    binary = root / manifest.binary
    if not binary.is_file():
        raise FileNotFoundError(f"target binary missing: {manifest.binary}")
    return _resolved_target(root, target_id, manifest)


def _resolved_target(
    root: Path, target_id: TargetId, manifest: TargetManifest
) -> ResolvedTarget:
    """Build the resolved target record without requiring the binary to exist."""

    return ResolvedTarget(
        id=target_id,
        manifest_path=root / "config" / "targets" / target_id.value / "target.toml",
        disc_id=manifest.disc_id,
        kind=manifest.kind,
        source_dir=root / manifest.source_dir,
        binary_path=root / manifest.binary,
        splat_path=root / manifest.splat,
        reviewed_replay_path=root
        / "config"
        / "targets"
        / manifest.id.value
        / "reviewed.rz",
        load_address=manifest.load_address,
        source_paths=tuple(manifest_source_paths(root, manifest)),
        header_paths=tuple(manifest_header_paths(root, manifest)),
    )


def lookup_target_manifest(root: Path, value: str) -> TargetManifest | None:
    """Return the ``TargetManifest`` for a shipped or canonical selector.

    Unlike :func:`resolve_target`, this never constructs resolved paths and
    never requires a target binary to exist.  Raises ``ValueError`` if the
    selector itself is malformed; returns ``None`` for a valid but unknown
    target.
    """

    target_id = normalize_target_id(value)
    return load_target_manifests(root).get(target_id.value)


def _validate_manifest_identity(
    root: Path, target_id: TargetId, manifest: TargetManifest, manifest_path: Path
) -> None:
    """Catch manifest/identity inconsistencies early."""

    expected_path = root / "config" / "targets" / target_id.value / "target.toml"
    if manifest_path != expected_path:
        raise RuntimeError(
            f"manifest path {manifest_path.relative_to(root)} does not match "
            f"target ID {target_id.value!r}"
        )
    if target_id.kind == "executable" and manifest.kind != "executable":
        raise RuntimeError(
            f"ID {target_id.value!r} has executable kind but manifest is {manifest.kind!r}"
        )
    if target_id.kind == "emi" and manifest.kind != "emi":
        raise RuntimeError(
            f"ID {target_id.value!r} has emi kind but manifest is {manifest.kind!r}"
        )


def resolve_all_targets(root: Path) -> dict[str, ResolvedTarget]:
    """Return every promoted target resolved from manifests."""

    manifests = load_target_manifests(root)
    result: dict[str, ResolvedTarget] = {}
    for target_id_str, manifest in sorted(manifests.items()):
        try:
            resolved = resolve_target(root, target_id_str)
        except FileNotFoundError:
            continue
        result[target_id_str] = resolved
    return result


__all__ = [
    "ResolvedFunction",
    "ResolvedTarget",
    "lookup_target_manifest",
    "resolve_all_targets",
    "resolve_function",
    "resolve_target",
]
