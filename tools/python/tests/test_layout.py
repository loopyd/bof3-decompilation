"""Tests for the Splat layout parser."""

from __future__ import annotations

from pathlib import Path

from harness.layout import LayoutBoundary, parse_splat_layout


def test_parse_splat_layout_extracts_reviewed_c_subsegments(tmp_path: Path) -> None:
    splat = tmp_path / "target.yaml"
    splat.write_text(
        "\n".join(
            [
                "name: test_target",
                "options:",
                "  platform: psx",
                "  compiler: psyq",
                "  basename: test_target",
                "  base_path: ../..",
                "  target_path: out/test.bin",
                "  asm_path: out/splat/test/asm",
                "  src_path: src/test",
                "  ld_script_path: out/splat/test/linker.ld",
                "  symbol_addrs_path:",
                "    - config/symbols/psyq.txt",
                "segments:",
                "  - [0x0, bin, header]",
                "  - name: main",
                "    type: code",
                "    start: 0x4",
                "    vram: 0x80196118",
                "    subsegments:",
                "      - [0x4, c, func_8019611c]",
                "      - [0x44, asm, func_8019615c]",
                "      - [0xa44, asm, func_80196b5c]",
                "  - [0xd1c, bin, data]",
                "  - [0xe70]",
            ]
        ),
        encoding="utf-8",
    )

    layout = parse_splat_layout(splat, 0x80196118)

    assert layout.has_reviewed_functions
    assert layout.load_address == 0x80196118
    assert 0x8019611c in layout.reviewed_function_addresses
    assert 0x8019615c in layout.reviewed_function_addresses
    assert 0x80196b5c in layout.reviewed_function_addresses


def test_parse_splat_layout_computes_virtual_address_from_vram(tmp_path: Path) -> None:
    splat = tmp_path / "target.yaml"
    splat.write_text(
        "\n".join(
            [
                "segments:",
                "  - [0x0, bin, header]",
                "  - name: main",
                "    type: code",
                "    start: 0x18",
                "    vram: 0x801f2be8",
                "    subsegments:",
                "      - [0x18, c, func_801f2c00]",
                "  - [0x100]",
            ]
        ),
        encoding="utf-8",
    )

    layout = parse_splat_layout(splat, 0x801f2be8)

    func = layout.boundary_starting_at(0x801f2c00)
    assert func is not None
    assert func.kind == "c"
    assert func.name == "func_801f2c00"


def test_parse_splat_layout_bootstrap_yields_no_functions(tmp_path: Path) -> None:
    splat = tmp_path / "target.yaml"
    splat.write_text(
        "segments:\n  - [0x0, bin]\n  - [0x100]\n",
        encoding="utf-8",
    )

    layout = parse_splat_layout(splat, 0x801D0C00)

    assert not layout.has_reviewed_functions
    assert layout.reviewed_function_addresses == ()


def test_parse_splat_layout_boundaries_have_file_end(tmp_path: Path) -> None:
    splat = tmp_path / "target.yaml"
    splat.write_text(
        "\n".join(
            [
                "segments:",
                "  - [0x0, bin, header]",
                "  - name: main",
                "    type: code",
                "    start: 0x4",
                "    vram: 0x80196118",
                "    subsegments:",
                "      - [0x4, c, func_8019611c]",
                "      - [0x44, asm, func_8019615c]",
                "  - [0xa0]",
            ]
        ),
        encoding="utf-8",
    )

    layout = parse_splat_layout(splat, 0x80196118)

    func = layout.boundary_starting_at(0x8019611c)
    assert func is not None
    assert func.file_end == 0x44
    assert func.file_size == 0x40

    func2 = layout.boundary_starting_at(0x8019615c)
    assert func2 is not None
    assert func2.file_end == 0xA0
    assert func2.file_size == 0x5C


def test_parse_splat_layout_containing(tmp_path: Path) -> None:
    splat = tmp_path / "target.yaml"
    splat.write_text(
        "\n".join(
            [
                "segments:",
                "  - [0x0, bin, header]",
                "  - name: main",
                "    type: code",
                "    start: 0x4",
                "    vram: 0x80196118",
                "    subsegments:",
                "      - [0x4, c, func_8019611c]",
                "      - [0x44, asm, func_8019615c]",
                "  - [0xa0]",
            ]
        ),
        encoding="utf-8",
    )

    layout = parse_splat_layout(splat, 0x80196118)

    # Inside the first function.
    b = layout.boundary_containing(0x80196130)
    assert b is not None
    assert b.name == "func_8019611c"

    # Inside the second function.
    b = layout.boundary_containing(0x80196160)
    assert b is not None
    assert b.name == "func_8019615c"


def test_parse_splat_layout_function_address_from_name(tmp_path: Path) -> None:
    b = LayoutBoundary(
        file_start=0x100,
        file_end=0x200,
        virtual_start=0x8019611c,
        virtual_end=0x8019621c,
        kind="c",
        name="func_8019611c",
    )
    assert b.function_address == 0x8019611C
    assert b.is_function
    assert b.file_size == 0x100


def test_parse_splat_layout_hash_changes_with_content(tmp_path: Path) -> None:
    splat = tmp_path / "target.yaml"
    splat.write_text("segments:\n  - [0x0, bin]\n  - [0x10]\n", encoding="utf-8")
    layout1 = parse_splat_layout(splat, 0x80000000)

    splat.write_text("segments:\n  - [0x0, bin]\n  - [0x20]\n", encoding="utf-8")
    layout2 = parse_splat_layout(splat, 0x80000000)

    assert layout1.sha256 != layout2.sha256


def test_parse_splat_layout_real_area008(tmp_path: Path) -> None:
    """Verify the formula against a real Splat config."""

    splat = tmp_path / "target.yaml"
    splat.write_text(
        "\n".join(
            [
                "segments:",
                "  - [0x0, bin, header]",
                "  - [0x14, bin, entry_header]",
                "  - name: main",
                "    type: code",
                "    start: 0x18",
                "    vram: 0x801f2be8",
                "    subsegments:",
                "      - [0x18, c, func_801f2c00]",
                "      - [0x5c, c, func_801f2c44]",
                "      - [0x124, asm, func_801f2d0c]",
                "  - [0x1a74, bin, data]",
                "  - [0x27f8]",
            ]
        ),
        encoding="utf-8",
    )

    layout = parse_splat_layout(splat, 0x801f2be8)

    assert 0x801f2c00 in layout.reviewed_function_addresses
    assert 0x801f2c44 in layout.reviewed_function_addresses
    assert 0x801f2d0c in layout.reviewed_function_addresses

    func = layout.boundary_starting_at(0x801f2c00)
    assert func is not None
    assert func.name == "func_801f2c00"
    assert func.file_end == 0x5c
    assert func.file_size == 0x44
