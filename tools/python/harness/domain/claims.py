"""Target-qualified source claims (manifest-claim-aware registry).

``domain.sources`` owns the strict metadata scan core and legacy
``source_dir`` inventory entry points.  This module adds the manifest-claim
layer: an explicit target ``sources``/``support_sources``/``headers`` claim
may point outside ``source_dir`` (semantic ``src/bof3/<class>/`` folders),
and every resolver here is target-qualified — ``(target, @source address)``
plus the exact manifest-claimed path — never path ancestry.  Unmigrated
targets (no claims) fall back to the legacy ``source_dir`` inventory.
"""

from __future__ import annotations

from pathlib import Path
from typing import Iterable, Mapping

from .manifests import TargetManifest
from .sources import (
    LiftMetadataError,
    _scan_lift_sources,
    lift_metadata,
    source_expected_key,
)


def manifest_source_paths(root: Path, manifest: TargetManifest) -> list[Path]:
    """Build inputs for one target: explicit claims when migrated, else the
    legacy ``source_dir`` inventory (all ``.c``/``.s``/``.S`` files)."""

    if manifest.has_explicit_sources:
        paths = {root / claimed for claimed in manifest.sources}
        paths.update(root / claimed for claimed in manifest.support_sources)
        return sorted(paths)
    source_dir = root / manifest.source_dir
    return sorted(
        path
        for path in source_dir.rglob("*")
        if path.is_file() and path.suffix in {".c", ".s", ".S"}
    )


def manifest_header_paths(root: Path, manifest: TargetManifest) -> list[Path]:
    """Target-private headers: explicit claims when migrated, else the legacy
    recursive ``source_dir`` ``*.h`` inventory."""

    if manifest.headers:
        return sorted(root / claimed for claimed in manifest.headers)
    source_dir = root / manifest.source_dir
    return sorted(path for path in source_dir.rglob("*.h") if path.is_file())


def collect_manifest_source_addresses(
    root: Path,
    manifest: TargetManifest,
    *,
    expected_lifts: Mapping[str, int] | None = None,
) -> list[tuple[Path, int]]:
    """Target-qualified lift scan: explicit claims when migrated, else the
    legacy ``source_dir`` inventory.  Same metadata rules as
    ``domain.sources.collect_source_addresses``; collisions name the owning
    target."""

    source_paths = [
        path for path in manifest_source_paths(root, manifest) if path.suffix == ".c"
    ]
    return _scan_lift_sources(
        source_paths, root / manifest.source_dir, expected_lifts, manifest.id.value
    )


def resolve_manifest_source_for_address(
    root: Path,
    manifest: TargetManifest,
    address: int,
    *,
    expected_lifts: Mapping[str, int] | None = None,
) -> Path | None:
    """Return the target's claimed source claiming ``address``, or None."""

    for candidate, candidate_address in collect_manifest_source_addresses(
        root, manifest, expected_lifts=expected_lifts
    ):
        if candidate_address == address:
            return candidate
    return None


def manifest_binding_sources(root: Path, manifest: TargetManifest) -> list[Path]:
    """Hand-maintained WEAK_SYMBOL_AT binding .c files for one target.

    Migrated targets: every claimed support ``.c`` except the generated PsyQ
    binding source, preferring the top-level ``symbols`` stem when present.
    Legacy targets: ``source_dir/symbols.c`` when it exists.
    """

    if not manifest.has_explicit_sources:
        top = root / manifest.source_dir / "symbols.c"
        return [top] if top.is_file() else []
    psyq = Path(manifest.psyq_source) if manifest.psyq_source else None
    candidates = [
        root / claimed
        for claimed in manifest.support_sources
        if Path(claimed).suffix == ".c" and Path(claimed) != psyq
    ]
    top = sorted(path for path in candidates if path.stem == "symbols")
    return top or sorted(candidates)


def resolve_source_for_paths(
    source_paths: Iterable[Path], address: int
) -> Path | None:
    """Return the claimed source carrying ``address`` in its ``@source`` tag."""

    for source_path in sorted(path for path in source_paths if path.suffix == ".c"):
        try:
            candidate, _behavior = lift_metadata(source_path)
        except (OSError, UnicodeError, LiftMetadataError):
            continue
        if candidate == address:
            return source_path
    return None


__all__ = [
    "collect_manifest_source_addresses",
    "manifest_binding_sources",
    "manifest_header_paths",
    "manifest_source_paths",
    "resolve_manifest_source_for_address",
    "resolve_source_for_paths",
    "source_expected_key",
]
