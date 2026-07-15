"""Reviewed Splat layout parser.

A constrained line parser for the subset of Splat YAML used by the
harness.  It exposes reviewed subsegment starts, virtual addresses,
and kinds without pulling in a YAML dependency.

Unsupported or malformed subsegment syntax fails explicitly rather
than silently disappearing.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path



_SUBSEGMENT_RE = re.compile(
    r"^\s*-\s*\[\s*(?P<offset>0x[0-9a-fA-F]+|\d+)"
    r"\s*,\s*(?P<kind>[a-z]+)"
    r"(?:\s*,\s*(?P<name>\S+))?"
    r"\s*\]"
)

_CODE_SEGMENT_RE = re.compile(r"^\s*type:\s*code\s*$")
_NAME_RE = re.compile(r"^\s*name:\s*(\S+)")
_VRAM_RE = re.compile(r"^\s*vram:\s*(0x[0-9a-fA-F]+|\d+)")
_START_RE = re.compile(r"^\s*start:\s*(0x[0-9a-fA-F]+|\d+)")
_BIN_EOF_RE = re.compile(r"^\s*-\s*\[\s*(0x[0-9a-fA-F]+|\d+)\s*\]")


@dataclass(frozen=True)
class LayoutBoundary:
    """One reviewed subsegment from a Splat layout."""

    file_start: int
    file_end: int | None
    virtual_start: int
    virtual_end: int | None
    kind: str
    name: str | None

    @property
    def is_function(self) -> bool:
        return self.kind in ("c", "asm")

    @property
    def function_name(self) -> str | None:
        return self.name

    @property
    def function_address(self) -> int | None:
        if self.name and self.name.startswith("func_"):
            try:
                return int(self.name[5:], 16)
            except ValueError:
                return None
        return None

    @property
    def file_size(self) -> int | None:
        if self.file_end is not None:
            return self.file_end - self.file_start
        return None

    @property
    def virtual_size(self) -> int | None:
        if self.virtual_end is not None:
            return self.virtual_end - self.virtual_start
        return None


@dataclass(frozen=True)
class ReviewedLayout:
    """A fully parsed Splat layout with reviewed boundaries."""

    boundaries: tuple[LayoutBoundary, ...]
    load_address: int
    sha256: str

    @property
    def has_reviewed_functions(self) -> bool:
        return any(b.is_function for b in self.boundaries)

    @property
    def reviewed_function_addresses(self) -> tuple[int, ...]:
        return tuple(
            b.function_address
            for b in self.boundaries
            if b.is_function and b.function_address is not None
        )

    def boundary_containing(self, address: int) -> LayoutBoundary | None:
        """Return the first reviewed boundary containing ``address``.

        The last boundary may be open-ended (``virtual_end is None``)
        if no EOF sentinel was present; such boundaries match any
        address at or after their start.
        """

        for b in self.boundaries:
            if b.virtual_end is None:
                if b.virtual_start <= address:
                    return b
            elif b.virtual_start <= address < b.virtual_end:
                return b
        return None

    def boundary_starting_at(self, address: int) -> LayoutBoundary | None:
        """Return the boundary whose virtual start matches ``address``."""

        for b in self.boundaries:
            if b.virtual_start == address:
                return b
        return None


def parse_splat_layout(splat_path: Path, load_address: int) -> ReviewedLayout:
    """Parse a Splat YAML and return its reviewed boundaries.

    Raises ``ValueError`` on unsupported or malformed subsegment syntax.
    """

    import hashlib

    text = splat_path.read_text(encoding="utf-8")
    sha256 = hashlib.sha256(text.encode("utf-8")).hexdigest()
    boundaries = list(_parse_boundaries(text, load_address))
    return ReviewedLayout(
        boundaries=tuple(boundaries),
        load_address=load_address,
        sha256=sha256,
    )


def _parse_boundaries(text: str, load_address: int) -> list[LayoutBoundary]:
    """Extract reviewed boundaries from Splat YAML text.

    The formula for computing virtual addresses is:
        virtual = segment_vram + (file_offset - segment_start)
    where ``segment_vram`` is the runtime address at the segment's
    ``start`` file offset.  When no ``start`` is specified, the
    segment start defaults to 0.
    """

    lines = text.splitlines()
    boundaries: list[LayoutBoundary] = []
    segment_vram: int | None = None
    segment_start: int = 0
    in_segments = False
    in_code_segment = False

    for line in lines:
        if line.startswith("segments:"):
            in_segments = True
            continue
        if not in_segments:
            continue

        # Code segment header: extract vram and start offset.
        if _CODE_SEGMENT_RE.match(line):
            in_code_segment = True
            segment_start = 0
            continue

        if in_code_segment:
            m = _VRAM_RE.match(line)
            if m:
                segment_vram = _parse_int(m.group(1))
                continue
            m = _START_RE.match(line)
            if m:
                segment_start = _parse_int(m.group(1))
                continue

        # Inline [offset, kind, name] subsegment.
        m = _SUBSEGMENT_RE.match(line)
        if m:
            file_start = _parse_int(m.group("offset"))
            kind = m.group("kind")
            name = m.group("name")

            if in_code_segment and segment_vram is not None:
                virtual_start = segment_vram + (file_start - segment_start)
            else:
                virtual_start = file_start

            boundaries.append(
                LayoutBoundary(
                    file_start=file_start,
                    file_end=None,
                    virtual_start=virtual_start,
                    virtual_end=None,
                    kind=kind,
                    name=name,
                )
            )
            # The preceding boundary ends here.
            if len(boundaries) >= 2:
                prev = boundaries[-2]
                boundaries[-2] = LayoutBoundary(
                    file_start=prev.file_start,
                    file_end=file_start,
                    virtual_start=prev.virtual_start,
                    virtual_end=virtual_start,
                    kind=prev.kind,
                    name=prev.name,
                )
            continue

        # EOF sentinel [offset] terminates the layout.
        m = _BIN_EOF_RE.match(line)
        if m:
            file_end = _parse_int(m.group(1))
            virtual_end = (
                segment_vram + (file_end - segment_start)
                if in_code_segment and segment_vram is not None
                else file_end
            )
            # Close all boundaries that are still open.
            for i, b in enumerate(boundaries):
                if b.file_end is None:
                    boundaries[i] = LayoutBoundary(
                        file_start=b.file_start,
                        file_end=file_end,
                        virtual_start=b.virtual_start,
                        virtual_end=virtual_end,
                        kind=b.kind,
                        name=b.name,
                    )
            # End of segments section.
            in_segments = False
            in_code_segment = False
            continue

    return boundaries


def _parse_int(value: str) -> int:
    return int(value, 0)


__all__ = [
    "LayoutBoundary",
    "ReviewedLayout",
    "parse_splat_layout",
]
