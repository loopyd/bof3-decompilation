"""Structural parser for reviewed Splat layouts."""

from __future__ import annotations

import hashlib
import re
from dataclasses import dataclass
from pathlib import Path

import yaml


_RAW_FUNCTION = re.compile(r"func_[0-9A-F]{8}\Z")
_MIXED_CASE_RAW_FUNCTION = re.compile(r"func_[0-9A-Fa-f]{8}\Z")


@dataclass(frozen=True)
class LayoutBoundary:
    file_start: int
    file_end: int | None
    virtual_start: int
    virtual_end: int | None
    kind: str
    name: str | None

    @property
    def is_function(self) -> bool:
        return self.kind in {"c", "asm"}

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
        return None if self.file_end is None else self.file_end - self.file_start

    @property
    def virtual_size(self) -> int | None:
        return (
            None if self.virtual_end is None else self.virtual_end - self.virtual_start
        )


@dataclass(frozen=True)
class ReviewedLayout:
    boundaries: tuple[LayoutBoundary, ...]
    load_address: int
    sha256: str

    @property
    def has_reviewed_functions(self) -> bool:
        return any(boundary.is_function for boundary in self.boundaries)

    @property
    def reviewed_function_addresses(self) -> tuple[int, ...]:
        return tuple(
            boundary.virtual_start
            for boundary in self.boundaries
            if boundary.is_function
        )

    def boundary_containing(self, address: int) -> LayoutBoundary | None:
        return next(
            (
                boundary
                for boundary in self.boundaries
                if boundary.virtual_start <= address
                and (boundary.virtual_end is None or address < boundary.virtual_end)
            ),
            None,
        )

    def boundary_starting_at(self, address: int) -> LayoutBoundary | None:
        return next(
            (
                boundary
                for boundary in self.boundaries
                if boundary.virtual_start == address
            ),
            None,
        )


def _integer(value: object, *, field: str) -> int:
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        try:
            return int(value, 0)
        except ValueError as exc:
            raise ValueError(f"invalid Splat {field}: {value!r}") from exc
    raise ValueError(f"invalid Splat {field}: {value!r}")


def _row(value: object) -> tuple[int, str, str | None] | None:
    if not isinstance(value, list) or len(value) < 2 or not isinstance(value[1], str):
        return None
    name = str(value[2]) if len(value) >= 3 else None
    if (
        name
        and _MIXED_CASE_RAW_FUNCTION.fullmatch(name)
        and not _RAW_FUNCTION.fullmatch(name)
    ):
        raise ValueError(f"non-canonical Splat function name: {name}")
    return (
        _integer(value[0], field="offset"),
        value[1],
        name,
    )


def parse_splat_layout(splat_path: Path, load_address: int) -> ReviewedLayout:
    text = splat_path.read_text(encoding="utf-8")
    document = yaml.safe_load(text)
    if not isinstance(document, dict) or not isinstance(document.get("segments"), list):
        raise ValueError(f"invalid Splat segments in {splat_path}")

    starts: list[tuple[int, int, str, str | None]] = []
    eof: int | None = None
    for segment in document["segments"]:
        if isinstance(segment, list):
            if len(segment) == 1:
                eof = _integer(segment[0], field="EOF")
                continue
            parsed = _row(segment)
            if parsed is not None:
                offset, kind, name = parsed
                starts.append((offset, load_address + offset, kind, name))
            continue
        if not isinstance(segment, dict):
            raise ValueError(f"unsupported Splat segment in {splat_path}: {segment!r}")
        segment_start = _integer(segment.get("start", 0), field="segment start")
        segment_vram = _integer(
            segment.get("vram", load_address + segment_start), field="segment vram"
        )
        subsegments = segment.get("subsegments", [])
        if not isinstance(subsegments, list):
            raise ValueError(f"invalid Splat subsegments in {splat_path}")
        for subsegment in subsegments:
            parsed = _row(subsegment)
            if parsed is None:
                raise ValueError(
                    f"unsupported Splat subsegment in {splat_path}: {subsegment!r}"
                )
            offset, kind, name = parsed
            starts.append((offset, segment_vram + offset - segment_start, kind, name))

    starts.sort(key=lambda item: item[0])
    boundaries: list[LayoutBoundary] = []
    for index, (file_start, virtual_start, kind, name) in enumerate(starts):
        file_end = starts[index + 1][0] if index + 1 < len(starts) else eof
        virtual_end = (
            virtual_start + file_end - file_start if file_end is not None else None
        )
        boundaries.append(
            LayoutBoundary(file_start, file_end, virtual_start, virtual_end, kind, name)
        )
    return ReviewedLayout(
        tuple(boundaries), load_address, hashlib.sha256(text.encode()).hexdigest()
    )


__all__ = ["LayoutBoundary", "ReviewedLayout", "parse_splat_layout"]
