from __future__ import annotations

import unittest

from scripts.rebof3 import program_identity as MODULE


class ProgramIdentityTests(unittest.TestCase):
    def test_infer_source_hint_for_boot_and_logo_programs(self) -> None:
        self.assertEqual(
            MODULE.infer_source_hint("/boot/SLUS_004.22", "/boot", "SLUS_004.22"),
            "build/extracted/SLUS_004.22",
        )
        self.assertEqual(
            MODULE.infer_source_hint("/boot/LOGO/LOGO.EXE", "/boot/LOGO", "LOGO.EXE"),
            "build/extracted/LOGO/LOGO.EXE",
        )

    def test_infer_source_hint_for_overlay_style_and_raw_bin_names(self) -> None:
        self.assertEqual(
            MODULE.infer_source_hint(
                "/bins/BIN/ETC/GAME/GAME_e01_801d0c00.bin",
                "/bins/BIN/ETC/GAME",
                "GAME_e01_801d0c00.bin",
            ),
            "build/extracted/BIN/ETC/GAME.EMI#1",
        )
        self.assertEqual(
            MODULE.infer_source_hint(
                "/bins/BIN/ETC/GAME/1.bin",
                "/bins/BIN/ETC/GAME",
                "1.bin",
            ),
            "build/extracted/BIN/ETC/GAME.EMI#1",
        )

    def test_parse_bin_program_path_normalizes_world_slot(self) -> None:
        result = MODULE.parse_bin_program_path("/bins/BIN/WORLD02/AREA078/4.bin")

        self.assertEqual(result.family, "world02")
        self.assertEqual(result.archive, "area078")
        self.assertEqual(result.slot_token, "4")
        self.assertEqual(result.normalized_slot, "04")
        self.assertEqual(result.slot_index, 4)

    def test_parse_bin_program_path_strips_duplicate_suffix_from_raw_bin_alias(self) -> None:
        result = MODULE.parse_bin_program_path("/bins/BIN/BOSS/BOSS037/3.bin.0")

        self.assertEqual(result.family, "boss")
        self.assertEqual(result.archive, "boss037")
        self.assertEqual(result.slot_token, "3")
        self.assertEqual(result.normalized_slot, "03")
        self.assertEqual(result.slot_index, 3)


if __name__ == "__main__":
    unittest.main()
