from __future__ import annotations

import json
from pathlib import Path
import subprocess
from types import SimpleNamespace

import pytest

from harness.commands._asm_diff_output import format_asm_diff_llm
from harness.commands.harness import run_diff
from harness.match.asm_diff import (
    build_result_payload,
    build_target_for_source,
    default_binary_for_source,
    extract_original_bytes,
    infer_original_size,
    infer_size_from_sibling_sources,
    matching_instruction_count,
    object_path_for_source,
    overlay_load_address_for_source,
    parse_source_address,
    run_build_object,
)
from harness.match._asm_disasm import extract_instructions
from harness.match._asm_link import link_object_at_address
from harness.io import repo_layout
from harness.symbols import load_weak_symbol_bindings


def asm_diff_output_payload(diff_path: Path, *, exact: bool = False) -> dict:
    return {
        "exact_match": exact,
        "function": "func_80100000",
        "address": "0x80100000",
        "original_size": 16,
        "current_size": 16,
        "size_delta": 0,
        "instruction_count": {
            "original": 4,
            "current": 4,
            "matching": 3 if not exact else 4,
            "match_percent": 75.0 if not exact else 100.0,
        },
        "first_mismatch": (
            None
            if exact
            else {
                "original_index": 1,
                "current_index": 1,
                "original_offset": 4,
                "current_offset": 4,
            }
        ),
        "outputs": {"diff": str(diff_path)},
    }


def test_llm_diff_exact_is_summary_and_artifact_path(tmp_path: Path) -> None:
    diff_path = tmp_path / "diff.patch"
    diff_path.write_text("\n", encoding="utf-8")

    output = format_asm_diff_llm(
        asm_diff_output_payload(diff_path, exact=True), root=tmp_path
    )

    assert output.startswith("MATCH func_80100000@0x80100000")
    assert output.endswith("full-diff=diff.patch")
    assert "@@" not in output


def test_llm_diff_prints_one_bounded_hunk_and_omission_count(
    tmp_path: Path,
) -> None:
    diff_path = tmp_path / "diff.patch"
    diff_path.write_text(
        "--- original\n"
        "+++ current\n"
        "@@ -1,4 +1,4 @@\n"
        " same-1\n"
        "-old-1\n"
        "+new-1\n"
        " same-2\n"
        "@@ -20,2 +20,2 @@\n"
        "-old-2\n"
        "+new-2\n",
        encoding="utf-8",
    )

    output = format_asm_diff_llm(
        asm_diff_output_payload(diff_path), root=tmp_path, max_hunk_lines=3
    )

    assert "--- original\n+++ current\n@@ -1,4 +1,4 @@\n same-1\n-old-1" in output
    assert "+new-1" not in output
    assert "@@ -20,2 +20,2 @@" not in output
    assert "... omitted 1 hunk(s), 5 line(s)" in output
    assert output.endswith("full-diff=diff.patch")


def test_llm_diff_without_hunks_remains_bounded(tmp_path: Path) -> None:
    diff_path = tmp_path / "diff.patch"
    diff_path.write_text("unexpected diff output\n" * 100, encoding="utf-8")

    output = format_asm_diff_llm(asm_diff_output_payload(diff_path), root=tmp_path)

    assert "unexpected diff output" not in output
    assert output.endswith("full-diff=diff.patch")


@pytest.mark.parametrize("conflict", ["json", "show_diff"])
def test_llm_diff_rejects_conflicting_output_modes(conflict: str) -> None:
    args = SimpleNamespace(llm=True, json=False, show_diff=False)
    setattr(args, conflict, True)

    with pytest.raises(ValueError, match="--llm cannot be combined"):
        run_diff(args)


def write_psx_exe(path: Path, *, load_address: int, payload: bytes) -> None:
    header = bytearray(0x800)
    header[:8] = b"PS-X EXE"
    header[0x18:0x1C] = load_address.to_bytes(4, byteorder="little")
    header[0x1C:0x20] = len(payload).to_bytes(4, byteorder="little")
    path.write_bytes(bytes(header) + payload)


def test_weak_symbol_bindings_are_loaded_for_matching(tmp_path: Path) -> None:
    symbols = tmp_path / "symbols.c"
    symbols.write_text(
        "WEAK_SYMBOL_AT(GAME_TABLE, 0x801ca70c);\n",
        encoding="utf-8",
    )

    assert load_weak_symbol_bindings(symbols) == {"GAME_TABLE": 0x801CA70C}


def test_linker_places_reviewed_object_section(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    commands: list[list[str]] = []

    def fake_run(command: list[str], **_kwargs: object) -> subprocess.CompletedProcess:
        commands.append(command)
        return subprocess.CompletedProcess(command, 0, "", "")

    monkeypatch.setattr("harness.match._asm_link.subprocess.run", fake_run)
    layout = SimpleNamespace(
        root=tmp_path,
        psn00b_toolchain_root=tmp_path / "toolchain",
    )

    link_object_at_address(
        object_path=tmp_path / "function.o",
        address=0x80197378,
        undefined_symbols=[],
        layout=layout,
        section_addresses={".rodata": 0x80195830},
    )

    assert "--section-start=.rodata=0x80195830" in commands[0]


def test_weak_symbol_bindings_load_shallow_target_symbol_units(tmp_path: Path) -> None:
    units_dir = tmp_path / "symbols"
    units_dir.mkdir()
    symbols = tmp_path / "symbols.c"
    symbols.write_text("/* Canonical binding entry point. */\n", encoding="utf-8")
    (units_dir / "functions.c").write_text(
        "WEAK_SYMBOL_AT(func_80100000, 0x80100000);\n", encoding="utf-8"
    )
    (units_dir / "variables.c").write_text(
        "WEAK_SYMBOL_AT(DAT_80110000, 0x80110000);\n", encoding="utf-8"
    )

    assert load_weak_symbol_bindings(symbols) == {
        "func_80100000": 0x80100000,
        "DAT_80110000": 0x80110000,
    }


def test_weak_symbol_units_reject_conflicting_bindings(
    tmp_path: Path,
) -> None:
    units_dir = tmp_path / "symbols"
    units_dir.mkdir()
    (units_dir / "functions.c").write_text(
        "WEAK_SYMBOL_AT(func_80100000, 0x80100004);\n", encoding="utf-8"
    )
    symbols = tmp_path / "symbols.c"
    symbols.write_text(
        "WEAK_SYMBOL_AT(func_80100000, 0x80100000);\n",
        encoding="utf-8",
    )

    with pytest.raises(ValueError, match="conflicting weak bindings"):
        load_weak_symbol_bindings(symbols)


def test_reviewed_target_bindings_cover_previous_link_errors() -> None:
    root = Path(__file__).resolve().parents[3]
    expected = {
        "src/emi/etc/game/00/symbols.c": {
            "D_801CCE84": 0x801CCE84,
            "D_801CCF7C": 0x801CCF7C,
        },
        "src/emi/world00/area008/13/symbols.c": {
            "GetClut": 0x8017A6F0,
            "SetSemiTrans": 0x8017A904,
            "SetPolyFT4": 0x8017A9B8,
            "GetGraphType": 0x8017B2B4,
            "SetDrawMode": 0x8017C2D8,
        },
        "src/exe/slus_004_22/symbols.c": {
            "sprintf": 0x8017E3F4,
            "SetSprt8": 0x8017A9F4,
            "SetSemiTrans": 0x8017A904,
            "game_front_local_mode_callback_loop": 0x8014ED6C,
        },
    }

    for relative_path, bindings in expected.items():
        actual = load_weak_symbol_bindings(root / relative_path)
        for name, address in bindings.items():
            assert actual[name] == address


def test_source_address_and_size_are_inferred_from_source_files(tmp_path: Path) -> None:
    source_dir = tmp_path / "bof3" / "src" / "exe" / "slus_004_22"
    source_dir.mkdir(parents=True)
    current = source_dir / "func_80162178.c"
    current.write_text(
        "/* @source 0x80162178 FUN_80162178 */\nvoid func_80162178(void) {}\n",
        encoding="utf-8",
    )
    next_source = source_dir / "func_801621e8.c"
    next_source.write_text("void func_801621e8(void) {}\n", encoding="utf-8")

    assert parse_source_address(current) == 0x80162178
    assert infer_size_from_sibling_sources(current, 0x80162178) == 0x70


def test_implausible_sibling_gap_falls_back_to_binary_return(
    tmp_path: Path,
) -> None:
    source_dir = tmp_path / "bof3" / "src" / "exe" / "slus_004_22"
    source_dir.mkdir(parents=True)
    current = source_dir / "func_801971e8.c"
    current.write_text("void func_801971e8(void) {}\n", encoding="utf-8")
    next_source = source_dir / "func_801a1ae4.c"
    next_source.write_text("void func_801a1ae4(void) {}\n", encoding="utf-8")
    binary = tmp_path / "0.bin"
    payload = bytearray(0x2000)
    return_offset = 0x80197370 - 0x80195800
    payload[return_offset : return_offset + 4] = (0x03E00008).to_bytes(
        4, byteorder="little"
    )
    binary.write_bytes(payload)

    assert infer_size_from_sibling_sources(current, 0x801971E8) is None
    assert (
        infer_original_size(
            current,
            address=0x801971E8,
            binary_path=binary,
            load_address=0x80195800,
        )
        == 0x190
    )


def test_binary_return_beats_sparse_sibling_boundary(tmp_path: Path) -> None:
    source_dir = tmp_path / "bof3" / "src" / "emi" / "etc" / "commu00" / "00"
    source_dir.mkdir(parents=True)
    current = source_dir / "func_801f18f8.c"
    current.write_text("void func_801f18f8(void) {}\n", encoding="utf-8")
    next_source = source_dir / "func_801f1bc8.c"
    next_source.write_text("void func_801f1bc8(void) {}\n", encoding="utf-8")
    binary = tmp_path / "0.bin"
    payload = bytearray(0x400)
    payload[0x200:0x204] = (0x03E00008).to_bytes(4, byteorder="little")
    binary.write_bytes(payload)

    assert (
        infer_original_size(
            current,
            address=0x801F18F8,
            binary_path=binary,
            load_address=0x801F18F8,
        )
        == 0x208
    )


def test_extract_original_bytes_reads_psx_exe_load_address(tmp_path: Path) -> None:
    binary = tmp_path / "SLUS_004.22"
    payload = bytes(range(0x40))
    write_psx_exe(binary, load_address=0x80010000, payload=payload)

    assert (
        extract_original_bytes(
            binary,
            address=0x80010008,
            size=4,
            load_address=None,
        )
        == payload[8:12]
    )


def test_object_path_matches_make_object_layout(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    source = layout.root / "src" / "exe" / "slus_004_22" / "func_80162178.c"
    source.parent.mkdir(parents=True)
    source.write_text("void func_80162178(void) {}\n", encoding="utf-8")

    assert object_path_for_source(layout, source) == (
        layout.build_dir / "src" / "exe" / "slus_004_22" / "func_80162178.o"
    )
    assert object_path_for_source(
        layout, Path("src/exe/slus_004_22/func_80162178.c")
    ) == object_path_for_source(layout, source)
    assert (
        build_target_for_source(layout, source)
        == "build/src/exe/slus_004_22/func_80162178.o"
    )


def test_object_path_rejects_source_outside_repository(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path / "repo")
    outside_source = tmp_path / "outside.c"

    with pytest.raises(ValueError, match="outside repository"):
        object_path_for_source(layout, outside_source)


def test_run_build_object_invokes_make_for_canonical_object(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    layout = repo_layout(tmp_path)
    source = layout.root / "src" / "exe" / "slus_004_22" / "func_80162178.c"
    source.parent.mkdir(parents=True)
    source.write_text("void func_80162178(void) {}\n", encoding="utf-8")
    (layout.root / "Makefile").write_text("all:\n", encoding="utf-8")
    object_path = object_path_for_source(layout, source)
    calls: list[tuple[list[str], Path | None]] = []

    def fake_run(command: list[str], **kwargs: object) -> subprocess.CompletedProcess:
        calls.append((command, kwargs.get("cwd")))
        object_path.parent.mkdir(parents=True, exist_ok=True)
        object_path.write_bytes(b"object")
        return subprocess.CompletedProcess(command, 0, "built\n", "warning\n")

    monkeypatch.setattr("harness.match.asm_diff.run_command", fake_run)
    build_log = layout.out_dir / "matching" / "build.log"

    run_build_object(layout, source, build_log)

    assert calls == [
        (
            [
                "make",
                "--no-print-directory",
                "build/src/exe/slus_004_22/func_80162178.o",
            ],
            layout.root,
        )
    ]
    assert build_log.read_text(encoding="utf-8") == "built\n\nwarning\n"


def test_overlay_manifest_uses_catalog_payload_base(
    tmp_path: Path, monkeypatch
) -> None:
    layout = repo_layout(tmp_path)
    source = layout.root / "src" / "emi" / "etc" / "game" / "00" / "func_80196ffc.c"
    source.parent.mkdir(parents=True)
    source.write_text("void func_80196ffc(void) {}\n", encoding="utf-8")
    catalog = layout.root / "out" / "catalog" / "emi.json"
    catalog.parent.mkdir(parents=True)
    catalog.write_text(
        json.dumps(
            {
                "entries": [
                    {
                        "id": "ETC/GAME#0",
                        "archive_id": "ETC/GAME",
                        "slot": 0,
                        "load_address": 0x80195800,
                    }
                ]
            }
        ),
        encoding="utf-8",
    )
    manifest = SimpleNamespace(
        source_dir="src/emi/etc/game/00",
        binary="out/binaries/emi/etc/game/00.bin",
        disc_id="ETC/GAME#0",
        load_address=0x8019611C,
    )
    monkeypatch.setattr(
        "harness.targets.load_target_manifests",
        lambda _root: {"emi/etc/game/00": manifest},
    )

    assert (
        default_binary_for_source(layout, source)
        == layout.root / "out/binaries/emi/etc/game/00.bin"
    )
    assert overlay_load_address_for_source(layout, source) == 0x80195800


def test_extract_instructions_strips_addresses_and_bytes() -> None:
    disassembly = """
801f3c2c <func_801f3c2c>:
801f3c2c:\t27bdffe0 \taddiu\tsp,sp,-32
801f3c30:\t24040078 \tli\ta0,120
801f3c4c:\t0c07cf62 \tjal\t801f3d88 <func_801f3d88>
"""

    assert extract_instructions(disassembly) == [
        "addiu sp,sp,-32",
        "li a0,120",
        "jal 0x801f3d88",
    ]


def test_extract_instructions_normalizes_hex() -> None:
    disassembly = "801f3ce0:\ta0205ad5 \tsb\tzero,0x5ad5(at)\n"
    assert extract_instructions(disassembly) == ["sb zero,0x5ad5(at)"]


def test_extract_instructions_normalizes_branch_target_after_registers() -> None:
    disassembly = "801d3858:\t14620008\tbne\tv1,v0,801d3884\n"
    assert extract_instructions(disassembly) == ["bne v1,v0,0x801d3884"]


def test_extract_instructions_skips_relocation_lines() -> None:
    disassembly = """
801f3c4c:\t0c000000 \tjal\t0 <func_801f3d88>
\t\t\t801f3c4c: R_MIPS_26\tfunc_801f3d88
"""
    assert extract_instructions(disassembly) == ["jal 0"]


def test_result_payload_reports_instruction_match_percent(tmp_path: Path) -> None:
    payload = build_result_payload(
        source_path=tmp_path / "func_80000000.c",
        function_name="func_80000000",
        address=0x80000000,
        original_size=12,
        current_size=12,
        binary_path=tmp_path / "0.bin",
        object_path=tmp_path / "func_80000000.c.obj",
        output_dir=tmp_path,
        original_lines=["addiu sp,sp,-16", "jr ra", "nop"],
        current_lines=["addiu sp,sp,-16", "move v0,zero", "nop"],
    )

    assert (
        matching_instruction_count(
            ["addiu sp,sp,-16", "jr ra", "nop"],
            ["addiu sp,sp,-16", "move v0,zero", "nop"],
        )
        == 2
    )
    assert payload["instruction_count"]["matching"] == 2
    assert payload["instruction_count"]["match_percent"] == 66.67
    assert payload["first_mismatch"] == {
        "original_index": 1,
        "current_index": 1,
        "original_offset": 4,
        "current_offset": 4,
        "original": "jr ra",
        "current": "move v0,zero",
    }
    assert payload["outputs"] == {
        "directory": str(tmp_path),
        "summary": str(tmp_path / "summary.json"),
        "diff": str(tmp_path / "diff.patch"),
        "original": str(tmp_path / "original.s"),
        "current": str(tmp_path / "current.s"),
        "compiler": str(tmp_path / "compiler.s"),
        "original_bytes": str(tmp_path / "original.bin"),
        "build_log": str(tmp_path / "build.log"),
    }


def test_result_payload_uses_byte_match_as_authority(tmp_path: Path) -> None:
    payload = build_result_payload(
        source_path=tmp_path / "func_80000000.c",
        function_name="func_80000000",
        address=0x80000000,
        original_size=4,
        current_size=4,
        byte_match=True,
        binary_path=tmp_path / "0.bin",
        object_path=tmp_path / "func_80000000.c.obj",
        output_dir=tmp_path,
        original_lines=["beq v0,v1,0x80000010"],
        current_lines=["beq v0,v1,80000010"],
    )

    assert payload["exact_match"] is True
    assert payload["byte_match"] is True
    assert payload["instruction_count"]["match_percent"] == 100.0
    assert payload["first_mismatch"] is not None
