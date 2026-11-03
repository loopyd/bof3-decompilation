"""Text renderers for one asm-diff payload, including aligned side-by-side pairs."""

from __future__ import annotations

import difflib
from dataclasses import dataclass
from pathlib import Path
from typing import Any

_SIDE_PLACEHOLDER = "--"


def _relative(path: str, root: Path) -> str:
    candidate = Path(path)
    try:
        return candidate.relative_to(root).as_posix()
    except ValueError:
        return candidate.as_posix()


def format_asm_diff_summary(payload: dict[str, Any], *, root: Path) -> str:
    instructions = payload["instruction_count"]
    denominator = max(instructions["original"], instructions["current"], 1)
    current_size = payload["current_size"]
    current_text = "?" if current_size is None else str(current_size)
    delta = payload["size_delta"]
    delta_text = "?" if delta is None else f"{delta:+d}"
    first = payload["first_mismatch"]
    first_text = "-"
    if first is not None:
        offset = first["original_offset"]
        if offset is None:
            offset = first["current_offset"]
        original_index = first["original_index"]
        current_index = first["current_index"]
        original_text = "-" if original_index is None else str(original_index)
        current_index_text = "-" if current_index is None else str(current_index)
        first_text = f"+0x{offset:04x}[o{original_text}/c{current_index_text}]"

    status = "MATCH" if payload["exact_match"] else "DIFF"
    line = (
        f"{status} {payload['function']}@{payload['address']} "
        f"insn={instructions['matching']}/{denominator}"
        f"({instructions['match_percent']:.2f}%) "
        f"bytes={payload['original_size']}->{current_text}({delta_text}) "
        f"first={first_text}"
    )
    if not payload["exact_match"]:
        line += f" diff={_relative(payload['outputs']['diff'], root)}"
    return line


def format_asm_diff_llm(
    payload: dict[str, Any],
    *,
    root: Path,
    max_hunk_lines: int = 24,
) -> str:
    """Render deterministic, bounded diff context while retaining the artifact path."""
    summary = format_asm_diff_summary(payload, root=root)
    diff_path = Path(payload["outputs"]["diff"])
    artifact = _relative(str(diff_path), root)
    if payload["exact_match"] or not diff_path.is_file():
        return f"{summary}\nfull-diff={artifact}"

    lines = diff_path.read_text(encoding="utf-8").splitlines()
    hunk_starts = [index for index, line in enumerate(lines) if line.startswith("@@")]
    if not hunk_starts:
        return f"{summary}\nfull-diff={artifact}"

    first_start = hunk_starts[0]
    first_end = hunk_starts[1] if len(hunk_starts) > 1 else len(lines)
    first_hunk = lines[first_start:first_end]
    shown_hunk = first_hunk[:max_hunk_lines]
    omitted_lines = len(first_hunk) - len(shown_hunk)
    omitted_lines += sum(
        hunk_starts[index + 1] - hunk_starts[index]
        for index in range(1, len(hunk_starts) - 1)
    )
    if len(hunk_starts) > 1:
        omitted_lines += len(lines) - hunk_starts[-1]

    output = [summary, *lines[:first_start], *shown_hunk]
    if omitted_lines or len(hunk_starts) > 1:
        output.append(
            f"... omitted {len(hunk_starts) - 1} hunk(s), {omitted_lines} line(s)"
        )
    output.append(f"full-diff={artifact}")
    return "\n".join(output)


@dataclass(frozen=True)
class _AlignedRow:
    """One aligned instruction pair, with any equal rows skipped before it."""

    index: int
    original: str | None
    current: str | None
    omitted: int = 0

    @property
    def differs(self) -> bool:
        return self.original != self.current


def _instruction_lines(path: Path) -> list[str]:
    """Read one ``.s`` artifact as instruction text, dropping labels and comments."""

    lines: list[str] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        text = raw.strip()
        if not text or text.startswith("#") or text.endswith(":"):
            continue
        lines.append(text)
    return lines


def _align_rows(original: list[str], current: list[str]) -> list[_AlignedRow]:
    """Pair both instruction lists in order, marking insertions and deletions."""

    matcher = difflib.SequenceMatcher(None, original, current, autojunk=False)
    rows: list[_AlignedRow] = []
    for tag, start, stop, current_start, current_stop in matcher.get_opcodes():
        original_side = original[start:stop]
        current_side = current[current_start:current_stop]
        if tag == "equal":
            for left, right in zip(original_side, current_side):
                rows.append(_AlignedRow(len(rows), left, right))
            continue
        for offset in range(max(len(original_side), len(current_side))):
            rows.append(
                _AlignedRow(
                    len(rows),
                    original_side[offset] if offset < len(original_side) else None,
                    current_side[offset] if offset < len(current_side) else None,
                )
            )
    return rows


def _window_rows(rows: list[_AlignedRow], context: int) -> list[_AlignedRow]:
    """Keep differing rows and ``context`` equal rows on each side of them."""

    keep = [row.differs for row in rows]
    for index, row in enumerate(rows):
        if not row.differs:
            continue
        start = max(0, index - context)
        stop = min(len(rows), index + context + 1)
        for neighbour in range(start, stop):
            keep[neighbour] = True

    windowed: list[_AlignedRow] = []
    omitted = 0
    for row, visible in zip(rows, keep):
        if visible:
            windowed.append(_AlignedRow(row.index, row.original, row.current, omitted))
            omitted = 0
        else:
            omitted += 1
    if omitted:
        windowed.append(_AlignedRow(len(rows), None, None, omitted))
    return windowed


def format_asm_side_by_side(
    payload: dict[str, Any],
    *,
    root: Path,
    context: int | None = None,
) -> str:
    """Render the original and current instructions as aligned, numbered rows.

    ``context`` limits the listing to differing rows plus that many equal rows on
    each side.  ``None`` keeps every row.  A differing row is marked ``<<<`` and a
    missing side prints ``--``.
    """

    outputs = payload["outputs"]
    original = _instruction_lines(Path(outputs["original"]))
    current = _instruction_lines(Path(outputs["current"]))
    rows = _align_rows(original, current)
    if context is not None:
        if context < 0:
            raise ValueError("context must not be negative")
        rows = _window_rows(rows, context)

    original_width = max(
        len("original"),
        max((len(row.original) for row in rows if row.original), default=0),
    )
    current_width = max(
        len("current"),
        max((len(row.current) for row in rows if row.current), default=0),
    )
    index_width = max(3, len(str(len(rows))))

    lines = [
        format_asm_diff_summary(payload, root=root),
        f"{'#':>{index_width}}  {'original':<{original_width}}  {'current':<{current_width}}",
    ]
    for row in rows:
        if row.omitted:
            lines.append(f"... {row.omitted} equal instruction(s) omitted")
        if row.original is None and row.current is None:
            continue
        original_text = row.original if row.original is not None else _SIDE_PLACEHOLDER
        current_text = row.current if row.current is not None else _SIDE_PLACEHOLDER
        marker = "  <<<" if row.differs else ""
        lines.append(
            f"{row.index:>{index_width}}  "
            f"{original_text:<{original_width}}  "
            f"{current_text:<{current_width}}{marker}"
        )
    return "\n".join(lines)
