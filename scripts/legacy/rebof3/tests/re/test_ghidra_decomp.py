from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.re import ghidra_helpers as MODULE
from scripts.rebof3.re.services.asm_normalize import AddressSymbolResolver


class GhidraDecompTests(unittest.TestCase):
    def test_parse_source_spec_supports_archive_entry_suffix(self) -> None:
        path, entry = MODULE.parse_source_spec("build/extracted/BIN/ETC/GAME.EMI#1")

        self.assertEqual(path, Path("build/extracted/BIN/ETC/GAME.EMI"))
        self.assertEqual(entry, 1)

    def test_infer_bin_metadata_from_adjacent_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            payload = root / "1.bin"
            payload.write_bytes(b"\x00" * 4)
            manifest = root / "emi.json"
            manifest.write_text(
                json.dumps(
                    {
                        "entries": [
                            {
                                "name": "1.bin",
                                "index": 1,
                                "type": 0,
                                "ram_ptr": 0x801D0C00,
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            result = MODULE.infer_bin_metadata(payload)

            self.assertIsNotNone(result)
            self.assertEqual(result["entry_index"], 1)
            self.assertEqual(result["entry_type"], 0)
            self.assertEqual(result["load_address"], 0x801D0C00)

    def test_default_program_name_for_emi_entry_includes_load_address(self) -> None:
        result = MODULE.default_program_name(
            "build/extracted/BIN/ETC/GAME.EMI#1", 0x801D0C00
        )

        self.assertEqual(result, "GAME_e01_801d0c00.bin")

    def test_default_artifacts_dir_keeps_entry_scoped_layout(self) -> None:
        result = MODULE.default_artifacts_dir(
            MODULE.ROOT / "tmp" / "ghidra_decomp",
            MODULE.ROOT / "build" / "extracted" / "BIN" / "ETC" / "GAME.EMI",
            0x801D0C04,
            1,
        )

        self.assertEqual(
            result,
            MODULE.ROOT
            / "tmp"
            / "ghidra_decomp"
            / "build"
            / "extracted"
            / "BIN"
            / "ETC"
            / "GAME.EMI"
            / "entry_1"
            / "0x801d0c04",
        )

    def test_extract_decompiled_c_returns_first_export_body(self) -> None:
        exported = [
            {
                "name": "FUN_80162d00",
                "c": "bool FUN_80162d00(void) { return DAT_80146494 == '\\\\x03'; }",
            }
        ]

        result = MODULE.extract_decompiled_c(exported)

        self.assertIn("FUN_80162d00", result)

    def test_bundle_function_metadata_omits_embedded_c_body(self) -> None:
        result = MODULE.bundle_function_metadata(
            {
                "name": "FUN_80162d00",
                "entry": "80162d00",
                "c": "bool FUN_80162d00(void) { return DAT_80146494 == '\\\\x03'; }",
            }
        )

        assert result is not None
        self.assertEqual(result["name"], "FUN_80162d00")
        self.assertEqual(result["entry"], "80162d00")
        self.assertNotIn("c", result)

    def test_bundle_artifact_paths_use_func_prefix(self) -> None:
        artifacts = MODULE.bundle_artifact_paths(Path("/tmp/out"))

        self.assertEqual(artifacts["json"], Path("/tmp/out/func.json"))
        self.assertEqual(artifacts["ghidra_c"], Path("/tmp/out/func.ghidra.c"))
        self.assertEqual(artifacts["ghidra_asm"], Path("/tmp/out/func.ghidra.s"))
        self.assertEqual(artifacts["spim_asm"], Path("/tmp/out/func.spim.s"))
        self.assertEqual(artifacts["asm"], Path("/tmp/out/func.s"))
        self.assertEqual(
            artifacts["m2c_context_source"], Path("/tmp/out/func.m2c.ctx.c")
        )
        self.assertEqual(artifacts["m2c_context"], Path("/tmp/out/func.m2c.ctx.i"))
        self.assertEqual(artifacts["m2c_asm"], Path("/tmp/out/func.m2c.s"))
        self.assertEqual(artifacts["m2c_c"], Path("/tmp/out/func.m2c.c"))

    def test_rewrite_asm_for_m2c_labels_local_branch_targets(self) -> None:
        asm_text = """.text\n\n.globl FUN_801ef27c\nFUN_801ef27c:\n/* 801ef2c4 */ andi v0, v1, 0xff\n/* 801ef2c8 */ addu v0, v0, a3\n/* 801ef2cc */ sb zero, 0x44(v0)\n/* 801ef2d0 */ addiu v1, v1, 0x1\n/* 801ef2d4 */ andi v0, v1, 0xff\n/* 801ef2d8 */ bne v0, zero, 0x801ef2c4\n/* 801ef2dc */ andi v0, v1, 0xff\n"""

        result = MODULE.rewrite_asm_for_m2c(asm_text)

        self.assertIn(".L801ef2c4:\n/* 801ef2c4 */ andi v0, v1, 0xff", result)
        self.assertIn("bne v0, zero, .L801ef2c4", result)

    def test_rewrite_asm_for_m2c_keeps_external_calls_numeric(self) -> None:
        asm_text = """.text\n\n.globl FUN_801eeec8\nFUN_801eeec8:\n/* 801eeed8 */ jal 0x8017ed6c\n/* 801eeedc */ nop\n"""

        result = MODULE.rewrite_asm_for_m2c(asm_text)

        self.assertIn("jal 0x8017ed6c", result)
        self.assertNotIn(".L8017ed6c", result)

    def test_rewrite_asm_for_m2c_symbolizes_calls_and_hi_lo_pairs(self) -> None:
        asm_text = (
            ".text\n\n"
            ".globl FUN_80162d00\n"
            "FUN_80162d00:\n"
            "/* 80162d00 */ lui a0, 0x8014\n"
            "/* 80162d04 */ lw v0, 0x6494(a0)\n"
            "/* 80162d08 */ jal 0x80123456\n"
            "/* 80162d0c */ nop\n"
        )

        resolver = AddressSymbolResolver(
            function_symbols={0x80123456: "func_80123456"},
            data_symbols={0x80146494: "DAT_80146494"},
        )

        result = MODULE.rewrite_asm_for_m2c(asm_text, resolver=resolver)

        self.assertIn("lui a0, %hi(DAT_80146494)", result)
        self.assertIn("lw v0, %lo(DAT_80146494)(a0)", result)
        self.assertIn("jal func_80123456", result)

    def test_rewrite_asm_for_m2c_preserves_exported_labels_and_symbolic_calls(
        self,
    ) -> None:
        asm_text = (
            ".text\n\n"
            ".globl FUN_80161fdc\n"
            "FUN_80161fdc:\n"
            ".L80162034:\n"
            "/* 80162034 */ lui at, 0x8014\n"
            "/* 80162038 */ addu at, at, a1\n"
            "/* 8016203c */ sb a2, 0x64a0(at)\n"
            "/* 80162040 */ addiu a1, a1, 0x1\n"
            "/* 80162044 */ sltiu v0, a1, 0x18\n"
            "/* 80162048 */ bne v0, zero, .L80162034\n"
            "/* 8016204c */ li v0, 0x2\n"
            "/* 8016206c */ jal CdSync\n"
            "/* 80162070 */ move a1, s0\n"
        )

        result = MODULE.rewrite_asm_for_m2c(asm_text)

        self.assertIn(".L80162034:", result)
        self.assertIn("bne v0, zero, .L80162034", result)
        self.assertIn("jal CdSync", result)

    def test_rewrite_asm_for_m2c_rewrites_hi_lo_on_current_export_shape(self) -> None:
        asm_text = (
            ".text\n\n"
            ".globl FUN_80161fdc\n"
            "FUN_80161fdc:\n"
            "/* 80162050 */ lui v1, 0x8014\n"
            "/* 80162054 */ lbu v1, 0x6840(v1)\n"
            "/* 80162064 */ lui s0, 0x8014\n"
            "/* 80162068 */ addiu s0, s0, 0x6498\n"
        )

        resolver = AddressSymbolResolver(
            function_symbols={},
            data_symbols={
                0x80146840: "DAT_80146840",
                0x80146498: "DAT_80146498",
            },
        )

        result = MODULE.rewrite_asm_for_m2c(asm_text, resolver=resolver)

        self.assertIn("lui v1, %hi(DAT_80146840)", result)
        self.assertIn("lbu v1, %lo(DAT_80146840)(v1)", result)
        self.assertIn("lui s0, %hi(DAT_80146498)", result)
        self.assertIn("addiu s0, s0, %lo(DAT_80146498)", result)

    def test_source_program_path_maps_boot_and_emi_inputs(self) -> None:
        self.assertEqual(
            MODULE.source_program_path("build/extracted/SLUS_004.22"),
            "/boot/SLUS_004.22",
        )
        self.assertEqual(
            MODULE.source_program_path("build/extracted/BIN/ETC/GAME.EMI#1"),
            "/bins/BIN/ETC/GAME/1.bin",
        )


if __name__ == "__main__":
    unittest.main()
