"""Domain-level reviewed function identity and collision rules.

One identity record per reviewed Splat function boundary: which target owns
it, at which address, under which compiled name, in which source parent,
over which reviewed bytes, and which Splat source claim names it.  The
collision matrix owns the cross-target name rules (all name comparisons
are case-insensitive):

- same target, same name, different address: reject;
- same name, identical reviewed bytes, independent ownership: allow;
- same name, different reviewed bytes, same source parent: reject
  (a distinct semantic name is required);
- same name, different reviewed bytes, different source parents: allow.

Composed-map and Splat/source bijection rules share the same finding type.
"""

from __future__ import annotations

import dataclasses
from dataclasses import dataclass
from pathlib import Path
from typing import TYPE_CHECKING

from .layout import SplatBoundary, ReviewedSplatLayout, parse_splat_layout
from .psx import payload_for, reviewed_range_digest
from .symbols import (
    MapSymbol,
    load_map,
    map_path,
)

if TYPE_CHECKING:
    from .manifests import TargetManifest


@dataclass(frozen=True)
class ReviewedFunctionIdentity:
    """Every reviewed fact about one reviewed function boundary."""

    target: str
    address: int
    end: int | None
    kind: str
    boundary_name: str | None
    compiled_name: str | None
    source_parent: str | None  # directory owning the Splat source claim
    claim: str | None  # Splat @source path (file, not directory)
    reviewed_sha256: str | None
    reviewed_size: int | None

    @property
    def address_hex(self) -> str:
        return f"0x{self.address:08X}"

    @property
    def boundary(self) -> tuple[int, int | None]:
        return (self.address, self.end)

    @property
    def folded_name(self) -> str:
        return (self.compiled_name or self.boundary_name or "").casefold()

    @property
    def reviewed_bytes(self) -> tuple[str, int] | None:
        """Reviewed (sha256, size) or None when the range is not hashable."""
        if self.reviewed_sha256 is None:
            return None
        return (self.reviewed_sha256, self.reviewed_size or 0)


def reviewed_function_identities(
    root: Path, target: str, manifest: "TargetManifest"
) -> list[ReviewedFunctionIdentity]:
    """Build one identity record per named reviewed function boundary.

    The target binary must exist; a target whose reviewed ranges escape
    the payload simply carries None reviewed bytes.
    """
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    binary = (root / manifest.binary).read_bytes()
    payload = payload_for(binary, manifest.load_address, binary_name=manifest.binary)
    by_address: dict[int, str] = {}
    for symbol in load_map(map_path(root, target)):
        by_address.setdefault(symbol.address, symbol.canonical_name)
    identities: list[ReviewedFunctionIdentity] = []
    for boundary in layout.boundaries:
        if not boundary.is_function or boundary.name is None:
            continue
        digest = reviewed_range_digest(
            payload, boundary.virtual_start, boundary.virtual_end, binary=binary
        )
        claim = boundary.source
        identities.append(
            ReviewedFunctionIdentity(
                target=target,
                address=boundary.virtual_start,
                end=boundary.virtual_end,
                kind=boundary.kind,
                boundary_name=boundary.name,
                compiled_name=by_address.get(boundary.virtual_start),
                source_parent=Path(claim).parent.as_posix() if claim else None,
                claim=claim,
                reviewed_sha256=digest[0] if digest else None,
                reviewed_size=digest[1] if digest else None,
            )
        )
    return identities


@dataclass(frozen=True)
class IdentityCollisionFinding:
    """One collision-matrix verdict (mechanical, not a semantic claim)."""

    rule: str
    verdict: str  # "reject" | "allow"
    left_target: str
    left_address: int
    right_target: str
    right_address: int
    detail: str

    def __str__(self) -> str:
        return (
            f"{self.rule} ({self.verdict}): "
            f"{self.left_target}@0x{self.left_address:08X} vs "
            f"{self.right_target}@0x{self.right_address:08X}: {self.detail}"
        )


def _reject(
    rule: str,
    left_target: str,
    left_address: int,
    right_target: str,
    right_address: int,
    detail: str,
) -> IdentityCollisionFinding:
    return IdentityCollisionFinding(
        rule, "reject", left_target, left_address, right_target, right_address, detail
    )


def _allow(
    rule: str,
    left_target: str,
    left_address: int,
    right_target: str,
    right_address: int,
    detail: str,
) -> IdentityCollisionFinding:
    return IdentityCollisionFinding(
        rule, "allow", left_target, left_address, right_target, right_address, detail
    )


def _pair_finding(
    left: ReviewedFunctionIdentity, right: ReviewedFunctionIdentity
) -> IdentityCollisionFinding | None:
    """One same-name cross-boundary verdict, or None when the pair is unrelated."""
    if left.target == right.target:
        return _reject(
            "same_target_name",
            left.target,
            left.address,
            right.target,
            right.address,
            f"same name {left.compiled_name} at two addresses "
            f"0x{left.address:08X} and 0x{right.address:08X}",
        )
    left_bytes = left.reviewed_bytes
    right_bytes = right.reviewed_bytes
    if left_bytes is not None and left_bytes == right_bytes:
        return _allow(
            "identical_bytes",
            left.target,
            left.address,
            right.target,
            right.address,
            "identical reviewed bytes; independent ownership required "
            "(each target owns its Splat boundary and map)",
        )
    if (left.source_parent or "") == (right.source_parent or ""):
        return _reject(
            "same_parent_different_bytes",
            left.target,
            left.address,
            right.target,
            right.address,
            "same source parent with different (or unprovable) reviewed "
            "bytes; a distinct semantic name is required",
        )
    if left_bytes is None or right_bytes is None:
        return _reject(
            "same_parent_different_bytes",
            left.target,
            left.address,
            right.target,
            right.address,
            "reviewed bytes cannot be hashed from the image; ownership "
            "independence is unproven, a distinct semantic name is required",
        )
    return _allow(
        "different_parent",
        left.target,
        left.address,
        right.target,
        right.address,
        "different source parents; a distinct compiled name is allowed",
    )


def collision_findings(
    identities: list[ReviewedFunctionIdentity],
) -> list[IdentityCollisionFinding]:
    """Apply the collision matrix to every same-name pair.

    Only function identities participate; data symbols (``D_*``) are
    address-named and outside the function-name rules.  Name comparison
    is case-insensitive; groups are compared within their folded name so
    unrelated symbols never touch.
    """
    groups: dict[str, list[ReviewedFunctionIdentity]] = {}
    for identity in identities:
        if (
            identity.compiled_name is None
            or identity.compiled_name.startswith("D_")
            or identity.compiled_name.startswith("func_")
        ):
            continue
        groups.setdefault(identity.folded_name, []).append(identity)
    findings: list[IdentityCollisionFinding] = []
    for members in sorted(groups.values(), key=lambda group: group[0].folded_name):
        members.sort(key=lambda item: (item.target, item.address))
        for index, left in enumerate(members):
            for right in members[index + 1 :]:
                finding = _pair_finding(left, right)
                if finding is not None:
                    findings.append(finding)
    return findings


def composed_map_findings(
    maps: dict[str, list[MapSymbol]],
    *,
    precedence: tuple[str, ...] = ("shared", "sdk", "local"),
) -> list[IdentityCollisionFinding]:
    """Validate shared/SDK/local maps after per-address precedence.

    ``maps`` maps a layer name (ordered by ``precedence``, lowest first) to
    a parsed ``MapSymbol`` list; a lower-precedence layer always owns any
    address it binds, so same-address override is legal.  The composed map
    must stay unique: the same or case-folded name bound at two different
    addresses is rejected.
    """
    winners: dict[int, tuple[str, MapSymbol]] = {}
    for layer in precedence:
        for symbol in maps.get(layer, ()):
            winners[symbol.address] = (layer, symbol)
    folded: dict[str, tuple[MapSymbol, int]] = {}
    findings: list[IdentityCollisionFinding] = []
    for address in sorted(winners):
        layer, symbol = winners[address]
        key = symbol.canonical_name.casefold()
        previous = folded.get(key)
        if previous is not None and previous[1] != address:
            previous_symbol, previous_address = previous
            previous_layer = winners[previous_address][0]
            findings.append(
                _reject(
                    "composed_name_address_mismatch",
                    previous_layer,
                    previous_address,
                    layer,
                    address,
                    f"{symbol.canonical_name} composed at two addresses: "
                    f"0x{previous_address:08X} (via {previous_layer}) and "
                    f"0x{address:08X} (via {layer})",
                )
            )
        else:
            folded[key] = (symbol, address)
    return findings


def splat_source_findings(
    target: str, layout: ReviewedSplatLayout
) -> list[IdentityCollisionFinding]:
    """Splat/source bijection for one target's reviewed layout.

    Rejects: duplicate starts, overlapping finite ranges, duplicate
    function names (c/asm boundaries), and two C boundaries claiming one
    source.  A data label reusing a function name is a data label, not a
    function-identity collision.  The boundary/source *address*
    disagreement stays with ``domain.sources`` (``expected_lift_sources``
    plus the lift scan own it), so it is not duplicated here.
    """
    findings: list[IdentityCollisionFinding] = []
    starts: dict[int, int] = {}
    for boundary in layout.boundaries:
        starts[boundary.virtual_start] = starts.get(boundary.virtual_start, 0) + 1
    for boundary in layout.boundaries:
        if starts[boundary.virtual_start] > 1:
            findings.append(
                _splat_reject(
                    target,
                    boundary.virtual_start,
                    f"duplicate Splat start 0x{boundary.virtual_start:08X}",
                )
            )
    for index, boundary in enumerate(layout.boundaries):
        if boundary.virtual_end is None:
            continue
        for other in layout.boundaries[index + 1 :]:
            if other.virtual_start < boundary.virtual_end:
                findings.append(
                    _splat_reject(
                        target,
                        boundary.virtual_start,
                        "overlapping finite ranges "
                        f"0x{boundary.virtual_start:08X}..0x{boundary.virtual_end:08X} "
                        f"and 0x{other.virtual_start:08X}..",
                    )
                )
    named: dict[str, list[SplatBoundary]] = {}
    for boundary in layout.boundaries:
        if boundary.is_function and boundary.name is not None:
            named.setdefault(boundary.name.casefold(), []).append(boundary)
    for key in sorted(named):
        members = named[key]
        if len(members) > 1:
            labeled = ", ".join(
                f"{b.name!r} at 0x{b.virtual_start:08X}" for b in members
            )
            findings.append(
                _splat_reject(
                    target,
                    members[0].virtual_start,
                    f"duplicate target-local Splat name (case-insensitive) "
                    f"{key!r}: {labeled}",
                )
            )
    claimed: dict[str, list[SplatBoundary]] = {}
    for boundary in layout.boundaries:
        if boundary.kind == "c" and boundary.source:
            claimed.setdefault(boundary.source, []).append(boundary)
    for source in sorted(claimed):
        if len(claimed[source]) > 1:
            addresses = ", ".join(f"0x{b.virtual_start:08X}" for b in claimed[source])
            findings.append(
                _splat_reject(
                    target,
                    claimed[source][0].virtual_start,
                    f"two C boundaries claim source {source!r} at {addresses}",
                )
            )
    return findings


def _splat_reject(target: str, address: int, detail: str) -> IdentityCollisionFinding:
    return IdentityCollisionFinding(
        "splat_source", "reject", target, address, target, address, detail
    )


def propose_collision(
    identities: list[ReviewedFunctionIdentity],
    target: str,
    address: int,
    new_name: str,
) -> IdentityCollisionFinding | None:
    """Simulate one naming transaction against the cross-target matrix.

    The proposed compiled name replaces the transaction's old spelling at
    every reviewed boundary that shares the proposed target's compiled
    name (the rename's scope is the whole repository), then the collision
    matrix re-runs: the proposal is rejected only when a rejecting finding
    names the proposed boundary (``target@address``).  ``None`` is a
    clean simulation.
    """
    simulated = [
        dataclasses.replace(identity, compiled_name=new_name)
        if identity.target == target and identity.address == address
        else identity
        for identity in identities
    ]
    rejects = [
        finding
        for finding in collision_findings(simulated)
        if finding.verdict == "reject"
        and (
            finding.left_target == target
            and finding.left_address == address
            or finding.right_target == target
            and finding.right_address == address
        )
    ]
    return rejects[0] if rejects else None


__all__ = [
    "IdentityCollisionFinding",
    "ReviewedFunctionIdentity",
    "collision_findings",
    "composed_map_findings",
    "reviewed_function_identities",
    "splat_source_findings",
    "propose_collision",
]
