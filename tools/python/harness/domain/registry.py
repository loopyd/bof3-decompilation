"""Resolved target registry.

Every target identity, path, and mapping fact lives here.  All other
modules receive a ``ResolvedTarget`` instead of independently resolving
paths from a ``TargetManifest``.
"""

from __future__ import annotations

import hashlib
from dataclasses import dataclass
from pathlib import Path

from .ids import TargetId, normalize_target_id
from .manifests import TargetManifest, load_target_manifests


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
    return ResolvedTarget(
        id=target_id,
        manifest_path=manifest_path,
        disc_id=manifest.disc_id,
        kind=manifest.kind,
        source_dir=root / manifest.source_dir,
        binary_path=binary,
        splat_path=root / manifest.splat,
        reviewed_replay_path=root
        / "config"
        / "targets"
        / manifest.id.value
        / "reviewed.rz",
        load_address=manifest.load_address,
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
    "ResolvedTarget",
    "lookup_target_manifest",
    "resolve_all_targets",
    "resolve_target",
]
