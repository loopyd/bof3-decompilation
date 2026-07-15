"""Tests for the source declaration inventory."""

from __future__ import annotations

from pathlib import Path

from harness.source_inventory import build_source_inventory


def test_inventory_classifies_function_bindings_without_source_as_known(
    tmp_path: Path,
) -> None:
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "symbols.c").write_text(
        "WEAK_SYMBOL_AT(func_801d0c00, 0x801d0c00)\n"
        "WEAK_SYMBOL_AT(func_801d0c80, 0x801d0c80)\n",
        encoding="utf-8",
    )

    inv = build_source_inventory(source_dir, "emi/etc/game/00")

    assert len(inv.functions) == 2
    assert inv.functions[0].address == 0x801D0C00
    assert not inv.functions[0].is_lifted
    assert inv.functions[0].is_reviewed


def test_inventory_marks_source_as_lifted(tmp_path: Path) -> None:
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "symbols.c").write_text(
        "WEAK_SYMBOL_AT(func_801d0c00, 0x801d0c00)\n",
        encoding="utf-8",
    )
    (source_dir / "func_801d0c00.c").write_text(
        "void func_801d0c00(void) {}\n",
        encoding="utf-8",
    )

    inv = build_source_inventory(source_dir, "emi/etc/game/00")

    assert len(inv.functions) == 1
    assert inv.functions[0].is_lifted


def test_inventory_classifies_data_bindings(tmp_path: Path) -> None:
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "symbols.c").write_text(
        "WEAK_SYMBOL_AT(D_80143b40, 0x80143b40)\n"
        "WEAK_SYMBOL_AT(func_801d0c00, 0x801d0c00)\n",
        encoding="utf-8",
    )

    inv = build_source_inventory(source_dir, "emi/etc/game/00")

    assert len(inv.data) == 1
    assert inv.data[0].address == 0x80143B40
    assert inv.data[0].name == "D_80143b40"


def test_inventory_uses_declarations_for_semantic_function_aliases(
    tmp_path: Path,
) -> None:
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "internal.h").write_text(
        "void game_random_u16(void);\n"
        "extern u16 D_80143b40;\n",
        encoding="utf-8",
    )
    (source_dir / "symbols.c").write_text(
        "WEAK_SYMBOL_AT(game_random_u16, 0x8017e3d4)\n"
        "WEAK_SYMBOL_AT(D_80143b40, 0x80143b40)\n",
        encoding="utf-8",
    )

    inv = build_source_inventory(source_dir, "emi/etc/game/00")

    assert len(inv.functions) == 1
    assert inv.functions[0].address == 0x8017E3D4
    assert inv.functions[0].name == "game_random_u16"
    assert inv.functions[0].semantic_name is None  # name IS the semantic name

    assert len(inv.data) == 1
    assert inv.data[0].address == 0x80143B40


def test_inventory_psyq_bindings_separated(tmp_path: Path) -> None:
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "symbols" / "psyq.c").parent.mkdir(parents=True)
    (source_dir / "symbols" / "psyq.c").write_text(
        "/* LIBGPU */\n"
        "WEAK_SYMBOL_AT(SetPolyG3, 0x800a0000)\n"
        "/* LIBC */\n"
        "WEAK_SYMBOL_AT(memcpy, 0x800b0000)\n",
        encoding="utf-8",
    )

    inv = build_source_inventory(source_dir, "emi/etc/game/00")

    assert len(inv.psyq) == 2
    # LIBC sorts before LIBGPU alphabetically.
    assert inv.psyq[0].name == "memcpy"
    assert inv.psyq[0].library == "LIBC"
    assert inv.psyq[1].name == "SetPolyG3"
    assert inv.psyq[1].library == "LIBGPU"


def test_inventory_semantic_name_preserved_with_func_binding(tmp_path: Path) -> None:
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "internal.h").write_text(
        "void game_random_u16(void);\n",
        encoding="utf-8",
    )
    (source_dir / "symbols.c").write_text(
        "WEAK_SYMBOL_AT(func_8017e3d4, 0x8017e3d4)\n"
        "WEAK_SYMBOL_AT(game_random_u16, 0x8017e3d4)\n",
        encoding="utf-8",
    )

    inv = build_source_inventory(source_dir, "emi/etc/game/00")

    assert len(inv.functions) == 1
    func = inv.functions[0]
    assert func.address == 0x8017E3D4
    assert func.name == "func_8017e3d4"
    assert func.semantic_name == "game_random_u16"


def test_inventory_hashes_change_with_content(tmp_path: Path) -> None:
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "symbols.c").write_text(
        "WEAK_SYMBOL_AT(func_801d0c00, 0x801d0c00)\n",
        encoding="utf-8",
    )
    inv1 = build_source_inventory(source_dir, "emi/etc/game/00")

    (source_dir / "symbols.c").write_text(
        "WEAK_SYMBOL_AT(func_801d0c00, 0x801d0c00)\n"
        "WEAK_SYMBOL_AT(func_801d0c80, 0x801d0c80)\n",
        encoding="utf-8",
    )
    inv2 = build_source_inventory(source_dir, "emi/etc/game/00")

    assert inv1.input_hash != inv2.input_hash


def test_inventory_data_from_declarations(tmp_path: Path) -> None:
    source_dir = tmp_path / "src"
    source_dir.mkdir()
    (source_dir / "internal.h").write_text(
        "extern vu16 GAME_FRONT_STATE;\n",
        encoding="utf-8",
    )
    (source_dir / "symbols.c").write_text(
        "WEAK_SYMBOL_AT(GAME_FRONT_STATE, 0x80143c10)\n",
        encoding="utf-8",
    )

    inv = build_source_inventory(source_dir, "emi/etc/game/00")

    assert len(inv.data) == 1
    assert inv.data[0].name == "GAME_FRONT_STATE"
    assert inv.data[0].address == 0x80143C10
