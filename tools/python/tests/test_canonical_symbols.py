from __future__ import annotations

from pathlib import Path
import subprocess
from types import SimpleNamespace

import pytest

from harness.canonical import Symbol, format_map, parse_map, weak_bindings_c
from harness.commands.symbols import main as symbols_main
from harness.match._asm_link import (
    _target_map_bindings,
    link_object_at_address,
    resolve_symbol_address,
)


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


def _write_check_target(root: Path, target: str, *, kind: str = "executable") -> None:
    manifest = root / "config" / "targets" / target / "target.toml"
    manifest.parent.mkdir(parents=True)
    lines = [
        'schema = "harness.target/v2"',
        f'id = "{target}"',
        f'kind = "{kind}"',
        f'source_dir = "src/{target}"',
        f'binary = "out/binaries/{target}.bin"',
        f'splat = "config/targets/{target}/splat.yaml"',
        "load_address = 0x801CE000",
    ]
    manifest.write_text("\n".join(lines) + "\n", encoding="utf-8")


def test_symbols_check_target_scope(tmp_path: Path, capsys) -> None:
    """`symbols check [TARGET]` selects one target; no-operand checks all."""
    # --- shared and SDK maps (empty but exist) ---
    (tmp_path / "config" / "targets" / "shared").mkdir(parents=True)
    (tmp_path / "config" / "targets" / "shared" / "symbols.txt").write_text(
        "", encoding="utf-8"
    )
    (tmp_path / "config" / "sdk").mkdir(parents=True)
    (tmp_path / "config" / "sdk" / "psyq-slus.txt").write_text("", encoding="utf-8")

    # --- Target 1: exe/keep (clean) ---
    _write_check_target(tmp_path, "exe/keep")
    map1 = tmp_path / "config" / "targets" / "exe" / "keep" / "symbols.txt"
    map1.write_text("func_80100000 = 0x80100000;\n", encoding="utf-8")
    src1 = tmp_path / "src" / "exe" / "keep"
    src1.mkdir(parents=True)
    (src1 / "func_80100000.c").write_text("// stub\n", encoding="utf-8")

    # --- Target 2: emi/battle/keep/15 (source/map drift) ---
    _write_check_target(tmp_path, "emi/battle/keep/15", kind="emi")
    map2 = (
        tmp_path
        / "config"
        / "targets"
        / "emi"
        / "battle"
        / "keep"
        / "15"
        / "symbols.txt"
    )
    map2.write_text("func_80200004 = 0x80200004;\n", encoding="utf-8")
    src2 = tmp_path / "src" / "emi" / "battle" / "keep" / "15"
    src2.mkdir(parents=True)
    # func_80200000.c exists but map only has func_80200004 → drift
    (src2 / "func_80200000.c").write_text("// stub\n", encoding="utf-8")

    # --- 1: selected clean target ---
    code1 = symbols_main(["--root", str(tmp_path), "check", "exe/keep"])
    captured1 = capsys.readouterr()
    assert code1 == 0, f"expected 0 for clean target, got {code1}: {captured1.err}"
    assert "func_80200000" not in captured1.out + captured1.err, (
        "selected check must not mention unrelated target"
    )

    # --- 2: shipped spelling resolves to EMI target ---
    code2 = symbols_main(["--root", str(tmp_path), "check", "battle/keep.emi#15"])
    captured2 = capsys.readouterr()
    assert code2 == 2, f"expected 2 for drifted EMI, got {code2}: {captured2.err}"
    assert "source/map drift" in captured2.err, (
        f"expected source/map drift in output: {captured2.err}"
    )

    # --- 3: unknown target retains the standard error/exit contract ---
    code3 = symbols_main(["--root", str(tmp_path), "check", "exe/no_such_target"])
    captured3 = capsys.readouterr()
    assert code3 == 2
    assert "unknown target: exe/no_such_target" in captured3.err

    # --- 4: no operand checks all targets ---
    code3 = symbols_main(["--root", str(tmp_path), "check"])
    captured3 = capsys.readouterr()
    assert code3 == 2, f"expected 2 for all-target check, got {code3}: {captured3.err}"
    assert "exe/keep" not in captured3.err, "exe/keep should have no errors"
    assert "emi/battle/keep/15" in captured3.err, (
        "full check should name the failing target"
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


def test_link_uses_supplied_bindings_without_map_reload(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(
        "harness.match._asm_link._target_map_bindings",
        lambda *_: (_ for _ in ()).throw(AssertionError("fallback map loaded")),
    )
    commands: list[list[str]] = []
    monkeypatch.setattr(
        subprocess,
        "run",
        lambda command, **_: (
            commands.append(command) or subprocess.CompletedProcess(command, 0, "", "")
        ),
    )
    object_path = tmp_path / "test.o"
    object_path.touch()
    link_object_at_address(
        object_path=object_path,
        address=0x801CE000,
        undefined_symbols=["PadRead"],
        symbols_c_path=tmp_path / "symbols.c",
        canonical_bindings={"PadRead": 0x801CE760},
        layout=SimpleNamespace(psn00b_toolchain_root=tmp_path, root=tmp_path),
    )
    assert f"--defsym=PadRead={0x801CE760}" in commands[0]


def test_parse_source_tag_skips_data_declaration_tags() -> None:
    from harness.domain.tags import parse_source_tag

    text = (
        "/* region */\n/* @source 0x801455C8 @kind unknown */\n"
        "extern volatile u8 activeRecordBytes[];\n\n"
        "/* @source 0x801F02E4\n * @behavior counts bytes\n */\n"
        "u8 countActiveRecords(void) { return 0; }\n"
    )
    assert parse_source_tag(text) == 0x801F02E4
    assert parse_source_tag("extern u8 x; /* @source 0x80100000 */\n") is None


def test_parse_declaration_source_tag_forms() -> None:
    from harness.domain.tags import parse_declaration_source_tag

    trailing = "extern u8 foo; /* @source 0x80100000 @kind bss */\n"
    leading = "/* @source 0x80100001 @kind table */\nextern const u8 bar[];\n"
    split = "/* @source 0x80149328 */\n/* @kind: bss — counter 1; stepped */\nextern volatile u16 counter1;\n"
    funcdecl = "/* @source 0x8014ED6C @kind unknown */\nvoid loopBody(void);\n"
    assert parse_declaration_source_tag(trailing, "foo") == 0x80100000
    assert parse_declaration_source_tag(leading, "bar") == 0x80100001
    assert parse_declaration_source_tag(split, "counter1") == 0x80149328
    assert parse_declaration_source_tag(funcdecl, "loopBody") == 0x8014ED6C
    assert parse_declaration_source_tag("/* counter1 table */\nextern u16 counter1;\n", "counter1") is None


def test_prefixed_raw_names_rejected_by_check() -> None:
    from harness.domain.tags import PREFIXED_RAW_NAME_RE, RAW_SYMBOL_NAME_RE

    assert PREFIXED_RAW_NAME_RE.search("SCENA16_D_80145EC4")
    assert not RAW_SYMBOL_NAME_RE.fullmatch("SCENA16_D_80145EC4")
    assert RAW_SYMBOL_NAME_RE.fullmatch("D_80145EC4")
    assert not PREFIXED_RAW_NAME_RE.search("D_80146864_BYTE")
