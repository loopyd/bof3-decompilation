"""Target-qualified source claims (manifest-claim-aware registry).

``domain.sources`` owns the strict metadata scan core.  This module adds the
manifest-claim layer: an explicit target
``sources``/``support_sources``/``headers`` claim may point outside
``source_dir`` (semantic ``src/bof3/<class>/`` folders), and every resolver
here is target-qualified — ``(target, @source address)`` plus the exact
manifest-claimed path — never path ancestry.  Every target must declare
claims; the legacy ``source_dir`` inventory is no longer consulted.
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


def _require_claims(manifest: TargetManifest, kind: str) -> None:
    if not manifest.has_explicit_sources:
        raise ValueError(
            f"{manifest.id.value}: target has no explicit source claims; "
            "declare manifest sources/support_sources (legacy source_dir "
            f"inventory is no longer consulted for {kind})"
        )


def manifest_source_paths(root: Path, manifest: TargetManifest) -> list[Path]:
    """Build inputs for one target from its explicit source claims."""

    _require_claims(manifest, "sources")
    paths = {root / claimed for claimed in manifest.sources}
    paths.update(root / claimed for claimed in manifest.support_sources)
    return sorted(paths)


def manifest_header_paths(root: Path, manifest: TargetManifest) -> list[Path]:
    """Target-private headers from explicit claims; empty when none claimed.

    Legacy implicit scanning of ``source_dir`` for ``*.h`` is removed: a
    target without header claims has no private headers.
    """

    return sorted(root / claimed for claimed in manifest.headers)


def collect_manifest_source_addresses(
    root: Path,
    manifest: TargetManifest,
    *,
    expected_lifts: Mapping[str, int] | None = None,
) -> list[tuple[Path, int]]:
    """Target-qualified lift scan from explicit claims.  Same metadata rules
    as ``domain.sources.collect_source_addresses``; collisions name the
    owning target."""

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

    Every claimed support ``.c`` except the generated PsyQ binding source,
    preferring the top-level ``symbols`` stem when present.
    """

    _require_claims(manifest, "binding sources")
    psyq = Path(manifest.psyq_source) if manifest.psyq_source else None
    candidates = [
        root / claimed
        for claimed in manifest.support_sources
        if Path(claimed).suffix == ".c" and Path(claimed) != psyq
    ]
    top = sorted(path for path in candidates if path.stem == "symbols")
    return top or sorted(candidates)


def resolve_source_for_paths(source_paths: Iterable[Path], address: int) -> Path | None:
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
