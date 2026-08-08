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
from typing import Mapping

from ..canonical import load_map, map_path
from ..layout import ReviewedLayout, parse_splat_layout
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


def collect_source_addresses(
    source_dir: Path,
    *,
    expected_lifts: Mapping[str, int] | None = None,
) -> list[tuple[Path, int]]:
    """Scan one target source directory deterministically.

    ``expected_lifts`` maps file stems to reviewed Splat ``c``-boundary
    addresses (see :func:`expected_lift_sources`).  A file is a lift
    candidate only when it carries function-level ``@source`` metadata or
    Splat expects it; helper/support translation units are ignored.  Two
    sources claiming the same address raise :class:`SourceAddressCollision`;
    a Splat-expected candidate missing metadata raises
    :class:`LiftMetadataError`.  Rows sort by ``(address, filename)``.
    """

    rows: list[tuple[Path, int]] = []
    claimed: dict[int, Path] = {}
    for source_path in sorted(source_dir.glob("*.c")):
        text = source_path.read_text(encoding="utf-8")
        address = parse_source_tag(text)
        expected = (
            None if expected_lifts is None else expected_lifts.get(source_path.stem)
        )
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
            raise SourceAddressCollision(
                f"source address collision 0x{address:08X}: "
                f"{previous.name} and {source_path.name} in {source_dir}"
            )
        claimed[address] = source_path
        rows.append((source_path, address))
    return sorted(rows, key=lambda row: (row[1], row[0].name))


def expected_lift_sources(
    layout: ReviewedLayout, source_dir: Path
) -> dict[str, int]:
    """Map every reviewed Splat ``c`` boundary to its expected source stem.

    The boundary's ``@source`` metadata names the exact file; otherwise the
    boundary name is the source stem (``func_<ADDR>`` or a reviewed semantic
    name).
    """

    result: dict[str, int] = {}
    for boundary in layout.boundaries:
        if boundary.kind != "c":
            continue
        stem: str | None = None
        if boundary.source:
            stem = Path(boundary.source).stem
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

    Strict: a Splat-expected candidate missing metadata raises
    :class:`LiftMetadataError` instead of being silently skipped.
    """

    for candidate, candidate_address in collect_source_addresses(
        source_dir, expected_lifts=expected_lifts
    ):
        if candidate_address == address:
            return candidate
    return None


def _owning_manifest(root: Path, source_path: Path):
    source_rel = source_path.expanduser().resolve().relative_to(root)
    source_dir = str(Path(source_rel).parent)
    for manifest in load_target_manifests(root).values():
        if manifest.source_dir == source_dir:
            return manifest
    return None


def reviewed_function_name(
    root: Path,
    target: str,
    address: int,
    *,
    layout: ReviewedLayout | None = None,
) -> str:
    """Return the target-owned compiled symbol name at ``address``.

    Requires, deterministically:

    - a reviewed Splat function boundary starting exactly at ``address``;
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
    boundary = layout.boundary_starting_at(address)
    if boundary is None or not boundary.is_function:
        raise CompiledSymbolError(
            None, address, "no reviewed Splat function boundary at this address"
        )
    entry = next(
        (symbol for symbol in load_map(map_path(root, target)) if symbol.address == address),
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
    layout: ReviewedLayout | None = None,
) -> str:
    """Return the object symbol compiled from ``source_path`` at ``address``.

    Target-owned and map/Splat-agreed via :func:`reviewed_function_name`.
    Raises :class:`CompiledSymbolError` when the address is not a reviewed
    target-local function symbol; never confuses shared/SDK/data symbols and
    never fabricates ``func_<ADDR>``.
    """

    manifest = _owning_manifest(root, source_path)
    if manifest is None:
        raise CompiledSymbolError(
            source_path,
            address,
            "source is not inside a known target source directory",
        )
    return reviewed_function_name(
        root, manifest.id.value, address, layout=layout
    )


__all__ = [
    "CompiledSymbolError",
    "LiftMetadataError",
    "SourceAddressCollision",
    "collect_source_addresses",
    "compiled_symbol_name",
    "expected_lift_sources",
    "lift_metadata",
    "resolve_source_for_address",
    "reviewed_function_name",
    "source_address",
]
