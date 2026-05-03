from __future__ import annotations

from pathlib import Path

from rebof3.match.asm_diff import (
    compiler_asm_path_for_object,
    extract_original_bytes,
    infer_size_from_sibling_sources,
    normalize_disassembly,
    object_path_for_source,
    parse_source_address,
)
from rebof3.paths import repo_layout


def write_psx_exe(path: Path, *, load_address: int, payload: bytes) -> None:
    header = bytearray(0x800)
    header[:8] = b"PS-X EXE"
    header[0x18:0x1C] = load_address.to_bytes(4, byteorder="little")
    header[0x1C:0x20] = len(payload).to_bytes(4, byteorder="little")
    path.write_bytes(bytes(header) + payload)


def test_source_address_and_size_are_inferred_from_source_files(tmp_path: Path) -> None:
    source_dir = tmp_path / "bof3" / "src" / "core" / "emi"
    source_dir.mkdir(parents=True)
    current = source_dir / "func_80162178.c"
    current.write_text(
        "/* @source: 0x80162178 FUN_80162178 */\nvoid func_80162178(void) {}\n",
        encoding="utf-8",
    )
    next_source = source_dir / "func_801621e8.c"
    next_source.write_text("void func_801621e8(void) {}\n", encoding="utf-8")

    assert parse_source_address(current) == 0x80162178
    assert infer_size_from_sibling_sources(current, 0x80162178) == 0x70


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


def test_object_path_matches_cmake_object_layout(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    source = layout.bof3_dir / "src" / "core" / "emi" / "func_80162178.c"
    source.parent.mkdir(parents=True)
    source.write_text("void func_80162178(void) {}\n", encoding="utf-8")

    assert object_path_for_source(layout, source) == (
        layout.build_dir
        / "default"
        / "bof3"
        / "CMakeFiles"
        / "bof3.dir"
        / "src"
        / "core"
        / "emi"
        / "func_80162178.c.obj"
    )


def test_compiler_asm_path_matches_maspsx_wrapper_output(tmp_path: Path) -> None:
    object_path = tmp_path / "func_8009c868.c.obj"

    assert compiler_asm_path_for_object(object_path) == (
        tmp_path / "func_8009c868.c.obj.s"
    )


def test_normalize_disassembly_keeps_only_instruction_text() -> None:
    disassembly = """
00000000 <func_80162178>:
   0:\t27bdffe8 \taddiu\tsp,sp,-24
   4:\t3c040000 \tlui\ta0,0x0
\t\t\t4: R_MIPS_HI16\tDAT_80146808
   8:\t0c000000 \tjal\t0 <func_80162178>
\t\t\t8: R_MIPS_26\tCdIntToPos
"""

    assert normalize_disassembly(disassembly) == [
        "addiu sp,sp,-24",
        "lui a0,0x0 # %hi(DAT_80146808)",
        "jal CdIntToPos",
    ]
