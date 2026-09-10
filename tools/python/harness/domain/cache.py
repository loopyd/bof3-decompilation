"""Validation-safe process-local target-manifest cache."""

from __future__ import annotations

import hashlib
from dataclasses import dataclass, replace
from pathlib import Path
from types import MappingProxyType
from typing import Any, Mapping

TargetManifest = Any

ManifestFingerprint = tuple[tuple[str, str], ...]
ValidationFingerprint = tuple[tuple[str, int, str, str], ...]


@dataclass(frozen=True)
class ClaimFiles:
    """One canonical metadata pass over all manifest-claimed files."""

    fingerprint: ValidationFingerprint
    canonical: Mapping[str, Path]


_CACHE: dict[
    tuple[Path, ManifestFingerprint],
    tuple[ClaimFiles, dict[str, TargetManifest]],
] = {}
_CLAIMS: dict[Path, ClaimFiles] = {}


def read_manifest_inputs(
    directory: Path,
) -> tuple[ManifestFingerprint, dict[Path, bytes]]:
    """Read each manifest once and return its content-keyed generation."""
    contents = {path: path.read_bytes() for path in sorted(directory.rglob("*.toml"))}
    fingerprint = tuple(
        (path.as_posix(), hashlib.sha256(content).hexdigest())
        for path, content in contents.items()
    )
    return fingerprint, contents


def clone_manifest(manifest: TargetManifest) -> TargetManifest:
    return replace(
        manifest,
        libraries=manifest.libraries.copy(),
        library_confidence=manifest.library_confidence.copy(),
        library_evidence=manifest.library_evidence.copy(),
        section_placements=manifest.section_placements.copy(),
    )


def collect_claim_files(root: Path, manifests: dict[str, TargetManifest]) -> ClaimFiles:
    """Hash every claim with pass-local, rechecked parent resolution."""
    paths = {manifest.binary for manifest in manifests.values() if manifest.binary}
    for manifest in manifests.values():
        paths.update(manifest.sources)
        paths.update(manifest.support_sources)
        paths.update(manifest.headers)
    rows = []
    canonical = {}
    parents = {}
    for relative in sorted(paths):
        try:
            path = root / relative
            parent = parents.get(path.parent)
            if parent is None:
                parent = path.parent.resolve(strict=True)
                parents[path.parent] = parent
            resolved = parent / path.name
            if resolved.is_symlink():
                resolved = resolved.resolve(strict=True)
            content = resolved.read_bytes()
            rows.append(
                (
                    relative,
                    len(content),
                    hashlib.sha256(content).hexdigest(),
                    resolved.as_posix(),
                )
            )
            canonical[relative] = resolved
        except OSError:
            rows.append((relative, -1, "", ""))
    for parent, expected in parents.items():
        message = f"manifest claim directory changed during validation: {parent}"
        try:
            current = parent.resolve(strict=True)
        except OSError as error:
            raise ValueError(message) from error
        if current != expected:
            raise ValueError(message)
    return ClaimFiles(tuple(rows), MappingProxyType(canonical))


def get_validated_claim_files(root: Path) -> ClaimFiles | None:
    """Return claim metadata established by the latest manifest load."""
    return _CLAIMS.get(root)


def get_manifests(
    root: Path, fingerprint: ManifestFingerprint
) -> dict[str, TargetManifest] | None:
    cached = _CACHE.get((root, fingerprint))
    if cached is None:
        return None
    expected, values = cached
    manifests = {key: clone_manifest(value) for key, value in values.items()}
    current = collect_claim_files(root, manifests)
    if current.fingerprint != expected.fingerprint:
        return None
    _CLAIMS[root] = current
    return manifests


def store_manifests(
    root: Path,
    fingerprint: ManifestFingerprint,
    manifests: dict[str, TargetManifest],
    *,
    claims: ClaimFiles | None = None,
) -> None:
    claims = claims or collect_claim_files(root, manifests)
    _CACHE.clear()
    _CLAIMS.clear()
    _CLAIMS[root] = claims
    _CACHE[(root, fingerprint)] = (
        claims,
        {key: clone_manifest(value) for key, value in manifests.items()},
    )
