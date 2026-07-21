from __future__ import annotations

from pathlib import Path
from types import SimpleNamespace

from harness.canonical import Symbol, format_map, parse_map, weak_bindings_c
from harness.match._asm_link import _target_map_bindings, resolve_symbol_address


def test_maps_normalize_raw_data_and_function_spelling() -> None:
    symbols = parse_map("func_80143B44 = 0x80143B44;\nDAT_80143b40 = 0x80143B40;\n")

    assert format_map(symbols) == (
        "D_80143B40 = 0x80143B40;\nfunc_80143B44 = 0x80143B44;\n"
    )


def test_maps_normalize_and_render_weak_bindings() -> None:
    rendered = weak_bindings_c(
        [Symbol(0x80100004, "D_80100004"), Symbol(0x80100000, "func_80100000")]
    )

    assert "WEAK_SYMBOL_AT(func_80100000, 0x80100000);" in rendered
    assert "WEAK_SYMBOL_AT(D_80100004, 0x80100004);" in rendered


def test_semantic_map_symbol_resolves_without_authored_binding(tmp_path: Path) -> None:
    assert (
        resolve_symbol_address(
            "PadRead",
            symbols_c_path=tmp_path / "symbols.c",
            canonical_bindings={"PadRead": 0x801CE760},
        )
        == 0x801CE760
    )


def test_source_target_uses_its_canonical_map_for_link_bindings(tmp_path: Path) -> None:
    source = tmp_path / "src" / "exe" / "logo"
    source.mkdir(parents=True)
    target_map = tmp_path / "config" / "targets" / "exe" / "logo" / "symbols.txt"
    target_map.parent.mkdir(parents=True)
    target_map.write_text("PadRead = 0x801CE760;\n", encoding="utf-8")

    assert _target_map_bindings(
        SimpleNamespace(root=tmp_path), source / "symbols.c"
    ) == {"PadRead": 0x801CE760}
