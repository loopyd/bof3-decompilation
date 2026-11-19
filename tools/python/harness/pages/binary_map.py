"""Partition original target bytes without promoting unproven classifications."""

from __future__ import annotations

from collections import defaultdict
from dataclasses import asdict, dataclass
import re

CATEGORIES = [
    {"id": key, "label": label, "color": color}
    for key, label, color in (
        ("header", "Header", "#f3ce69"),
        ("audio", "Audio", "#d58df0"),
        ("text", "Text", "#6dcbea"),
        ("texture", "Texture", "#eb91ac"),
        ("code-unlifted", "Code (unlifted)", "#7186bf"),
        ("code-lifted", "Code (lifted)", "#78cf9b"),
        ("table", "Table", "#efa467"),
        ("unknown", "Unknown", "#465260"),
        ("empty", "Empty (padding)", "#202b35"),
    )
]


@dataclass(frozen=True)
class IndexedFunction:
    address: int
    status: str
    source: str | None = None


@dataclass(frozen=True)
class Span:
    start: int
    end: int
    category: str
    detail: str
    priority: int = 30
    preview: str | None = None
    function: IndexedFunction | None = None


def partition(data: bytes, spans: list[Span]) -> list[dict]:
    """Resolve overlaps and cover every byte with a half-open physical span."""
    size = len(data)
    if not size:
        raise ValueError("cannot map an empty target")
    spans = [
        *spans,
        *(
            Span(m.start(), m.end(), "empty", "Observed zero fill; padding unproven", 5)
            for m in re.finditer(b"\x00{16,}", data)
        ),
    ]
    events = defaultdict(list)
    events[0] = []
    events[size] = []
    categories = {row["id"] for row in CATEGORIES}
    for index, span in enumerate(spans):
        if not 0 <= span.start < span.end <= size or span.category not in categories:
            raise ValueError(f"invalid physical byte span: {span}")
        events[span.start].append((index, True))
        events[span.end].append((index, False))
    active = set()
    result = []
    points = sorted(events)
    for start, end in zip(points, points[1:]):
        for index, entering in events[start]:
            if entering:
                active.add(index)
            else:
                active.remove(index)
        category, detail = "unknown", "No verified classification"
        preview = None
        functions = []
        if active:
            priority = max(spans[index].priority for index in active)
            winners = [
                spans[index] for index in active if spans[index].priority == priority
            ]
            kinds = {span.category for span in winners}
            if len(kinds) == 1:
                category = winners[0].category
                detail = "; ".join(sorted({span.detail for span in winners}))
                functions = [
                    asdict(function)
                    for function in sorted(
                        {span.function for span in winners if span.function},
                        key=lambda function: function.address,
                    )
                ]
                if category == "text":
                    previews = sorted(
                        {span.preview for span in winners if span.preview}
                    )
                    preview = "\n".join(previews) or None
                    if preview and len(preview) > 160:
                        preview = preview[:157] + "..."
            else:
                detail = "Conflicting evidence: " + ", ".join(sorted(kinds))
        if (
            result
            and result[-1]["category"] == category
            and result[-1]["detail"] == detail
            and result[-1].get("preview") == preview
            and result[-1].get("functions", []) == functions
        ):
            result[-1]["end"] = end
        else:
            result.append(
                {"start": start, "end": end, "category": category, "detail": detail}
            )
            if preview is not None:
                result[-1]["preview"] = preview
            if functions:
                result[-1]["functions"] = functions
    for span in result:
        if span["category"] == "header":
            span["preview"] = data[
                span["start"] : min(span["end"], span["start"] + 32)
            ].hex(" ")
            if span["end"] - span["start"] > 32:
                span["preview"] += " ..."
    return result
