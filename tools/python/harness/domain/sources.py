"""Metadata-backed source registry (strict).

``domain.tags`` remains the parser authority for @source/@behavior tags.
This module centralizes lift-source identity:

- a lift source is identified ONLY by its function-level @source tag;
  filenames are never parsed for addresses and never confer candidacy;
- lift candidacy comes only from function-level metadata or reviewed Splat
  expected-lift evidence; helper/support translation units are ignored;
- a lift requires BOTH @source and @behavior; a candidate missing either is
  a deterministic :class:`LiftMetadataError` naming the file and the gap;
- duplicate target-local address claims raise
  :class:`SourceAddressCollision`;
- compiled symbol identity is target-owned and requires map/Splat agreement;
  ``func_<ADDR>`` is never synthesized from a missing map entry.
"""

from __future__ import annotations

from pathlib import Path
from typing import Iterable, Mapping

from .layout import ReviewedSplatLayout, parse_splat_layout
from .symbols import load_map, map_path
from .manifests import load_target_manifests
from .tags import parse_behavior_tag, parse_source_tag


class LiftMetadataError(ValueError):
    """A lift candidate has missing or inconsistent @source/@behavior."""

    def __init__(self, source_path: Path, reason: str, detail: str = "") -> None:
        self.source_path = Path(source_path)
        self.reason = reason  # missing_source | missing_behavior | address_mismatch
        message = f"{self.source_path}: {reason}"
        if detail:
            message += f": {detail}"
        super().__init__(message)


class SourceAddressCollision(ValueError):
    """Two lift sources claim the same original address."""


class CompiledSymbolError(ValueError):
    """No target-owned compiled symbol satisfies map/Splat agreement."""

    def __init__(
        self,
        source_path: Path | None,
        address: int,
        detail: str,
    ) -> None:
        self.source_path = source_path
        self.address = address
        message = (
            f"cannot resolve compiled symbol for 0x{address:08X}"
            + (f" in {source_path}" if source_path is not None else "")
            + f": {detail}"
        )
        super().__init__(message)


def source_address(source_path: Path) -> int:
    """Return the lift's function-level @source address (never the filename)."""

    address = parse_source_tag(source_path.read_text(encoding="utf-8"))
    if address is None:
        raise LiftMetadataError(
            source_path,
            "missing_source",
            "expected a function-level '@source 0xXXXXXXXX' tag",
        )
    return address


def lift_metadata(source_path: Path) -> tuple[int, str]:
    """Return (address, behavior) for a lift source, raising on any gap."""

    text = source_path.read_text(encoding="utf-8")
    address = parse_source_tag(text)
    if address is None:
        raise LiftMetadataError(
            source_path,
            "missing_source",
            "expected a function-level '@source 0xXXXXXXXX' tag",
        )
    behavior = parse_behavior_tag(text)
    if behavior is None:
        raise LiftMetadataError(
            source_path,
            "missing_behavior",
            "expected an '@behavior' tag",
        )
    return address, behavior


def _repo_root(source_dir: Path) -> Path | None:
    """Return the repository root when ``source_dir`` lies under a ``src`` tree."""

    try:
        index = source_dir.parts.index("src")
    except ValueError:
        return None
    return Path(*source_dir.parts[:index])


def source_expected_key(source_dir: Path, source_path: Path) -> str | None:
    """Return the ``expected_lifts`` key for ``source_path``.

    In-root sources use their ``source_dir``-relative stem (legacy and
    in-root ``@source`` convention).  Out-of-root claimed sources use the
    repository-relative stem, which is exactly what
    :func:`expected_lift_sources` emits for out-of-root ``@source`` paths.
    Returns None when neither convention applies.
    """

    try:
        return source_path.relative_to(source_dir).with_suffix("").as_posix()
    except ValueError:
        root = _repo_root(source_dir)
        if root is None:
            return None
        try:
            return source_path.relative_to(root).with_suffix("").as_posix()
        except ValueError:
            return None


def _scan_lift_sources(
    source_paths: Iterable[Path],
    source_dir: Path,
    expected_lifts: Mapping[str, int] | None,
    owner: str | None = None,
) -> list[tuple[Path, int]]:
    """Shared strict lift scan over an explicit candidate set.

    ``expected_lifts`` maps file stems to reviewed Splat ``c``-boundary
    addresses (see :func:`expected_lift_sources`).  A file is a lift
    candidate only when it carries function-level ``@source`` metadata or
    Splat expects it; helper/support translation units are ignored.  Two
    sources claiming the same address raise :class:`SourceAddressCollision`;
    a Splat-expected candidate missing metadata raises
    :class:`LiftMetadataError`.  Rows sort by ``(address, filename)``.
    ``owner`` names the target for out-of-root collision messages.
    """

    rows: list[tuple[Path, int]] = []
    claimed: dict[int, Path] = {}
    for source_path in sorted(source_paths):
        text = source_path.read_text(encoding="utf-8")
        address = parse_source_tag(text)
        expected = None
        if expected_lifts is not None:
            key = source_expected_key(source_dir, source_path)
            if key is not None:
                expected = expected_lifts.get(key)
        if address is None:
            if expected is None:
                continue  # support/helper translation unit, not a lift
            raise LiftMetadataError(
                source_path,
                "missing_source",
                f"expected lift from reviewed Splat boundary 0x{expected:08X}",
            )
        if expected is not None and address != expected:
            raise LiftMetadataError(
                source_path,
                "address_mismatch",
                f"Splat boundary claims 0x{expected:08X}",
            )
        behavior = parse_behavior_tag(text)
        if behavior is None:
            raise LiftMetadataError(
                source_path,
                "missing_behavior",
                "expected an '@behavior' tag",
            )
        previous = claimed.get(address)
        if previous is not None:
            location = owner or str(source_dir)
            raise SourceAddressCollision(
                f"source address collision 0x{address:08X}: "
                f"{previous.name} and {source_path.name} in {location}"
            )
        claimed[address] = source_path
        rows.append((source_path, address))
    return sorted(rows, key=lambda row: (row[1], row[0].name))


def collect_source_addresses(
    source_dir: Path,
    *,
    expected_lifts: Mapping[str, int] | None = None,
) -> list[tuple[Path, int]]:
    """Scan one target source directory deterministically.

    Legacy entry point used by unmigrated targets and folder-local callers;
    target-qualified consumers should prefer
    :func:`collect_manifest_source_addresses`.
    """

    return _scan_lift_sources(
        sorted(source_dir.rglob("*.c")), source_dir, expected_lifts
    )


def expected_lift_sources(
    layout: ReviewedSplatLayout, source_dir: Path
) -> dict[str, int]:
    """Map every reviewed Splat ``c`` boundary to a target-relative source stem.

    Relocated sources use their reviewed ``@source`` path.  In-root paths stay
    ``source_dir``-relative; out-of-root claimed paths become repository-
    relative.  Legacy boundaries without a source path remain top-level names.
    """

    result: dict[str, int] = {}
    source_parts = source_dir.parts
    for boundary in layout.boundaries:
        if boundary.kind != "c":
            continue
        stem: str | None = None
        if boundary.source:
            path = Path(boundary.source).with_suffix("")
            parts = path.parts
            try:
                source_index = source_parts.index("src")
            except ValueError:
                source_suffix = source_parts
            else:
                source_suffix = source_parts[source_index:]
            for index in range(len(parts)):
                if parts[index : index + len(source_suffix)] == source_suffix:
                    path = Path(*parts[index + len(source_suffix) :])
                    break
            stem = path.as_posix()
        elif boundary.name:
            stem = boundary.name
        if stem is None:
            continue
        result[stem] = boundary.virtual_start
    return result


def resolve_source_for_address(
    source_dir: Path,
    address: int,
    *,
    expected_lifts: Mapping[str, int] | None = None,
) -> Path | None:
    """Return the source claiming ``address``, or None when absent.

    Legacy folder-scoped resolver; target-qualified consumers should prefer
    ``domain.claims.resolve_manifest_source_for_address``.
    Strict: a Splat-expected candidate missing metadata raises
    :class:`LiftMetadataError` instead of being silently skipped.
    """

    for candidate, candidate_address in collect_source_addresses(
        source_dir, expected_lifts=expected_lifts
    ):
        if candidate_address == address:
            return candidate
    return None


def owning_manifest(root: Path, source_path: Path):
    """Return the manifest that explicitly claims ``source_path``, or None.

    Ownership is target-qualified and path-independent: only explicit
    ``sources``/``support_sources``/``headers`` claims confer it, so a lift
    moved into a semantic ``src/bof3/<class>/`` folder keeps its owner.
    ``source_dir`` directory ancestry never confers ownership.
    """

    try:
        source_rel = (
            source_path.expanduser().resolve().relative_to(root.expanduser().resolve())
        )
    except ValueError:
        return None
    claimed_owners = [
        manifest
        for manifest in load_target_manifests(root).values()
        if any(Path(claimed) == source_rel for claimed in manifest.sources)
        or any(Path(claimed) == source_rel for claimed in manifest.support_sources)
        or any(Path(claimed) == source_rel for claimed in manifest.headers)
    ]
    if not claimed_owners:
        return None
    return max(
        claimed_owners,
        key=lambda manifest: len(
            manifest.sources + manifest.support_sources + manifest.headers
        ),
    )


def reviewed_function_name(
    root: Path,
    target: str,
    address: int,
    *,
    layout: ReviewedSplatLayout | None = None,
) -> str:
    """Return the target-owned compiled symbol name at ``address``.

    Requires, deterministically:

    - a reviewed Splat function boundary at ``address``, or a containing reviewed
      ``bin`` segment for a metadata/map-owned lift not yet split in Splat;
    - a target-local map entry (never shared/SDK) at that address;
    - a function entry, not a ``D_*`` data symbol;
    - boundary/map agreement (equal names, or Splat ``@source`` metadata).

    Raises :class:`CompiledSymbolError` on any failure.  Never synthesizes
    ``func_<ADDR>``.
    """

    manifests = load_target_manifests(root)
    manifest = manifests.get(target)
    if manifest is None:
        raise CompiledSymbolError(None, address, f"unknown target: {target}")
    if layout is None:
        layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    boundary = layout.find_boundary_at(address)
    entry = next(
        (
            symbol
            for symbol in load_map(map_path(root, target))
            if symbol.address == address
        ),
        None,
    )
    if entry is None:
        raise CompiledSymbolError(
            None, address, "no target-local map entry at this address"
        )
    if entry.is_raw and entry.name.startswith("D_"):
        raise CompiledSymbolError(
            None, address, "target-local map entry is a data symbol, not a function"
        )
    if boundary is None or not boundary.is_function:
        containing = layout.find_containing_boundary(address)
        if containing is None or containing.kind != "bin":
            raise CompiledSymbolError(
                None,
                address,
                "no reviewed Splat function boundary or containing bin segment",
            )
        return entry.name
    boundary_name = boundary.name
    if boundary_name == entry.name:
        return entry.name
    if boundary.source is not None:
        # Reviewed @source metadata names the lift; the display name may
        # legitimately differ from the compiled symbol.
        return entry.name
    raise CompiledSymbolError(
        None,
        address,
        f"Splat boundary {boundary_name!r} disagrees with map entry {entry.name!r}",
    )


def compiled_symbol_name(
    root: Path,
    source_path: Path,
    address: int,
    *,
    layout: ReviewedSplatLayout | None = None,
) -> str:
    """Return the object symbol compiled from ``source_path`` at ``address``.

    Target-owned and map/Splat-agreed via :func:`reviewed_function_name`.
    Raises :class:`CompiledSymbolError` when the address is not a reviewed
    target-local function symbol; never confuses shared/SDK/data symbols and
    never fabricates ``func_<ADDR>``.
    """

    manifest = owning_manifest(root, source_path)
    if manifest is None:
        raise CompiledSymbolError(
            source_path,
            address,
            "source is not claimed by or inside a known target source directory",
        )
    return reviewed_function_name(root, manifest.id.value, address, layout=layout)


__all__ = [
    "CompiledSymbolError",
    "LiftMetadataError",
    "SourceAddressCollision",
    "collect_source_addresses",
    "compiled_symbol_name",
    "expected_lift_sources",
    "lift_metadata",
    "owning_manifest",
    "resolve_source_for_address",
    "reviewed_function_name",
    "source_address",
    "source_expected_key",
]
