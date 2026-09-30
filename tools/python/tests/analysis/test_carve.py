"""Unit tests for isolated ``bin -> asm`` carve regrouping."""

from __future__ import annotations

import pytest

from harness.analysis.probe import _segment_start, carve_rows, carve_segments


def test_carve_rows_splits_bin_row_into_head_asm_tail() -> None:
    rows = [[100, "bin", "blob"], [400, "asm", "func_A"]]
    assert carve_rows(150, 200, "func_X")(rows) is True
    assert rows == [
        [100, "bin", "blob_head"],
        [150, "asm", "func_X"],
        [200, "bin", "blob_tail"],
        [400, "asm", "func_A"],
    ]


def test_carve_rows_omits_tail_when_range_reaches_the_next_row() -> None:
    rows = [[100, "bin", "blob"], [200, "asm", "func_A"]]
    assert carve_rows(150, 200, "func_X")(rows) is True
    assert rows == [
        [100, "bin", "blob_head"],
        [150, "asm", "func_X"],
        [200, "asm", "func_A"],
    ]


def test_carve_rows_declines_range_outside_a_bin_row() -> None:
    rows = [[0, "bin", "blob"], [100, "asm", "func_A"]]
    assert carve_rows(100, 150, "func_X")(rows) is False


def test_carve_rows_declines_range_spanning_two_rows() -> None:
    rows = [[0, "bin", "blob"], [100, "bin", "other"], [200, "asm", "func_A"]]
    assert carve_rows(50, 150, "func_X")(rows) is False


@pytest.mark.parametrize(
    ("start", "end", "name"),
    [(10, 10, "func_X"), (20, 10, "func_X"), (0, 10, "1bad"), (0, 10, "has space")],
)
def test_carve_rows_rejects_invalid_input(start: int, end: int, name: str) -> None:
    with pytest.raises(ValueError):
        carve_rows(start, end, name)


def test_carve_segments_regroups_a_bare_bin_segment() -> None:
    segments = [
        [0, "bin", "header"],
        {
            "name": "main",
            "type": "code",
            "start": 11640,
            "vram": 0x801D3978,
            "subsegments": [[11640, "asm", "func_801D3978"]],
        },
    ]
    assert carve_segments(11304, 11640, "func_801D3828", 0x801D3828)(segments) is True
    assert segments[0] == [0, "bin", "header_head"]
    assert segments[1] == {
        "name": "func_801D3828",
        "type": "code",
        "start": 11304,
        "vram": 0x801D3828,
        "subsegments": [[11304, "asm", "func_801D3828"]],
    }
    assert segments[2]["start"] == 11640  # the original code segment survives


def test_carve_segments_keeps_a_tail_segment_when_the_range_ends_early() -> None:
    segments = [[0, "bin", "header"], {"name": "main", "start": 200, "vram": 200}]
    assert carve_segments(100, 150, "func_X", 100)(segments) is True
    assert [_segment_start(segment) for segment in segments] == [0, 100, 150, 200]


def test_carve_segments_requires_vram() -> None:
    with pytest.raises(ValueError):
        carve_segments(0, 8, "func_X", None)


def test_carve_segments_declines_when_no_bare_bin_segment_matches() -> None:
    segments = [
        {"name": "main", "type": "code", "start": 0, "vram": 0, "subsegments": []}
    ]
    assert carve_segments(0, 8, "func_X", 0)(segments) is False


def test_segment_start_handles_both_segment_forms() -> None:
    assert _segment_start([16, "bin", "header"]) == 16
    assert _segment_start({"name": "main", "start": 32}) == 32
    assert _segment_start({"name": "main"}) is None
    assert _segment_start("nope") is None
