from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
import subprocess

from scripts.rebof3.match import object_slices as MODULE


class ObjectSlicesTests(unittest.TestCase):
    def test_parse_symbol_table_extracts_function_bounds(self) -> None:
        text = "00000094 g     F .text\t00000040 emi_ready\n"

        parsed = MODULE.parse_symbol_table(text, "emi_ready")

        self.assertEqual(parsed, (0x94, 0x40))

    def test_function_words_from_disassembly_extracts_words(self) -> None:
        text = (
            "00000094 <emi_ready>:\n"
            "  94: 3c028014 lui v0,0x8014\n"
            "  98: 34426494 ori v0,v0,0x6494\n"
        )

        words = MODULE.function_words_from_disassembly(text)

        self.assertEqual(words, [0x3C028014, 0x34426494])

    def test_function_disassembly_keeps_local_labels_within_symbol_size(self) -> None:
        original_run_command = MODULE.run_command
        try:
            MODULE.run_command = lambda _args: subprocess.CompletedProcess(  # type: ignore[assignment]
                _args,
                0,
                stdout=(
                    "Disassembly of section .text:\n\n"
                    "00000000 <func_801f6c90>:\n"
                    "   0: 27bdffe8 addiu sp,sp,-24\n"
                    "\n"
                    "00000004 <LM3>:\n"
                    "   4: 3c028014 lui v0,0x8014\n"
                    "\n"
                    "00000008 <next_symbol>:\n"
                    "   8: 03e00008 jr ra\n"
                ),
                stderr="",
            )

            text = MODULE.function_disassembly(
                Path("dummy.o"),
                "func_801f6c90",
                start_offset=0,
                size=8,
            )
        finally:
            MODULE.run_command = original_run_command  # type: ignore[assignment]

        self.assertIn("00000004 <LM3>:", text)
        self.assertNotIn("00000008 <next_symbol>:", text)

    def test_rename_top_level_symbol_updates_expected_asm(self) -> None:
        rewritten = MODULE.rename_top_level_symbol(
            ".text\n.globl FUN_80162d00\nFUN_80162d00:\n.word 0x3c028014\n",
            old_symbol="FUN_80162d00",
            new_symbol="emi_ready",
        )

        self.assertIn(".globl emi_ready", rewritten)
        self.assertIn("emi_ready:", rewritten)
        self.assertNotIn("FUN_80162d00:", rewritten)

    def test_write_current_slice_asm_renders_word_assembly(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            output_path = Path(tmp_dir) / "current.s"
            slice_data = MODULE.FunctionSlice(
                symbol_name="emi_ready",
                start_offset=0x94,
                size=0x40,
                asm_text=(
                    "00000094 <emi_ready>:\n"
                    "  94: 3c028014 lui v0,0x8014\n"
                    "  98: 34426494 ori v0,v0,0x6494\n"
                ),
            )

            MODULE.write_current_slice_asm(slice_data, output_path)

            text = output_path.read_text(encoding="utf-8")
            self.assertIn(".globl emi_ready", text)
            self.assertIn(".word 0x3c028014", text)

    def test_normalize_expected_asm_rewrites_register_names(self) -> None:
        normalized = MODULE.normalize_expected_asm(
            ".text\n/* 80162d00 */ lui v0, 0x8014\n/* 80162d10 */ jr ra\n"
        )

        self.assertIn("lui $v0, 0x8014", normalized)
        self.assertIn("jr $ra", normalized)

    def test_normalize_expected_asm_rewrites_break_immediates(self) -> None:
        normalized = MODULE.normalize_expected_asm(
            ".text\n/* 80100000 */ break 0x1c00\n/* 80100004 */ break 0x1800\n"
        )

        self.assertIn(".word 0x0007000d", normalized)
        self.assertIn(".word 0x0006000d", normalized)

    def test_extract_expected_body_lines_rewrites_local_branch_targets(self) -> None:
        body_lines = MODULE.extract_expected_body_lines(
            ".text\n.globl FUN_1\nFUN_1:\n/* 80100000 */ beq v0, zero, 0x8010000c\n/* 80100004 */ nop\n/* 80100008 */ jr ra\n/* 8010000c */ nop\n",
            "FUN_1",
        )

        self.assertIn("beq $v0, $zero, .L8010000c", body_lines)
        self.assertIn(".L8010000c:", body_lines)

    def test_extract_expected_body_lines_rewrites_known_metadata_addresses(
        self,
    ) -> None:
        body_lines = MODULE.extract_expected_body_lines(
            ".text\n.globl FUN_1\nFUN_1:\n/* 80100000 */ lui v0, 0x8014\n/* 80100004 */ lw v0, 0x3d40(v0)\n/* 80100008 */ jal 0x8017ee0c\n",
            "FUN_1",
            resolver=MODULE.AddressSymbolResolver(
                function_symbols={0x8017EE0C: "func_8017ee0c"},
                data_symbols={0x80143D40: "DAT_80143d40"},
            ),
        )

        self.assertIn("lui $v0, %hi(DAT_80143d40)", body_lines)
        self.assertIn("lw $v0, %lo(DAT_80143d40)($v0)", body_lines)
        self.assertIn("jal func_8017ee0c", body_lines)

    def test_patch_expected_asm_text_renames_and_symbolizes(self) -> None:
        patched = MODULE.patch_expected_asm_text(
            ".text\n.globl FUN_1\nFUN_1:\n/* 80100000 */ lui v0, 0x8014\n/* 80100004 */ lw v0, 0x3d40(v0)\n/* 80100008 */ jr ra\n",
            original_symbol_name="FUN_1",
            target_symbol_name="func_80100000",
            resolver=MODULE.AddressSymbolResolver(
                function_symbols={},
                data_symbols={0x80143D40: "DAT_80143d40"},
            ),
        )

        self.assertIn(".globl func_80100000", patched)
        self.assertIn("func_80100000:", patched)
        self.assertIn("%hi(DAT_80143d40)", patched)

    def test_patch_expected_asm_text_sanitizes_invalid_metadata_symbols(self) -> None:
        patched = MODULE.patch_expected_asm_text(
            ".text\n.globl FUN_1\nFUN_1:\n/* 80100000 */ lui a0, 0x8015\n/* 80100004 */ addiu a0, a0, -0x6800\n/* 80100008 */ jal 0x8017ee0c\n",
            original_symbol_name="FUN_1",
            target_symbol_name="func_80100000",
            resolver=MODULE.AddressSymbolResolver(
                function_symbols={0x8017EE0C: "func_8017ee0c"},
                data_symbols={0x80149800: r"s_\LOGO\LOGO.EXE;1_80149800"},
            ),
        )

        self.assertIn("%hi(s__LOGO_LOGO_EXE_1_80149800)", patched)
        self.assertIn("%lo(s__LOGO_LOGO_EXE_1_80149800)", patched)
        self.assertIn("jal func_8017ee0c", patched)

    def test_patch_expected_asm_text_strips_macro_inc_and_decomp_macros(self) -> None:
        patched = MODULE.patch_expected_asm_text(
            '.include "macro.inc"\n'
            ".set noat\n"
            ".set noreorder\n"
            ".set gp=64\n"
            ".section .text\n"
            ".align 4\n"
            "nonmatching FUN_1, 0x8\n"
            "glabel FUN_1\n"
            "/* 80100000 */ jr ra\n"
            "/* 80100004 */ addiu v0, zero, 1\n"
            "endlabel FUN_1\n",
            original_symbol_name="FUN_1",
            target_symbol_name="func_80100000",
        )

        self.assertNotIn("macro.inc", patched)
        self.assertNotIn("nonmatching", patched)
        self.assertNotIn("glabel", patched)
        self.assertNotIn("endlabel", patched)
        self.assertIn("jr $ra", patched)
        self.assertIn("addiu $v0, $zero, 1", patched)


if __name__ == "__main__":
    unittest.main()
