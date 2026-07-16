from __future__ import annotations

from harness.canonical import Symbol, format_map, parse_map, weak_bindings_c


def test_maps_normalize_raw_data_and_function_spelling() -> None:
    symbols = parse_map(
        "func_80143b44 = 0x80143B44;\nDAT_80143b40 = 0x80143B40;\n"
    )

    assert format_map(symbols) == (
        "D_80143B40 = 0x80143B40;\nfunc_80143B44 = 0x80143B44;\n"
    )


def test_maps_normalize_and_render_weak_bindings() -> None:
    rendered = weak_bindings_c(
        [Symbol(0x80100004, "D_80100004"), Symbol(0x80100000, "func_80100000")]
    )

    assert "WEAK_SYMBOL_AT(func_80100000, 0x80100000);" in rendered
    assert "WEAK_SYMBOL_AT(D_80100004, 0x80100004);" in rendered
