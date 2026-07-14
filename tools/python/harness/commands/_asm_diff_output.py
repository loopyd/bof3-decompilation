from __future__ import annotations

from pathlib import Path
from typing import Any


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
