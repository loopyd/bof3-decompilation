from __future__ import annotations

import unittest

from scripts.rebof3.inventory import ghidra_symbols as MODULE


class GhidraSymbolsTests(unittest.TestCase):
    def test_parse_args_supports_explicit_outputs(self) -> None:
        args = MODULE.parse_args(
            [
                "tmp/raw.json",
                "--index-out",
                "tmp/index.json",
                "--symbols-out",
                "tmp/functions.json",
                "--symbols-tsv-out",
                "tmp/functions.tsv",
                "--md-out",
                "tmp/index.md",
            ]
        )

        self.assertEqual(args.input, MODULE.Path("tmp/raw.json"))
        self.assertEqual(args.index_out, MODULE.Path("tmp/index.json"))
        self.assertEqual(args.symbols_out, MODULE.Path("tmp/functions.json"))
        self.assertEqual(args.symbols_tsv_out, MODULE.Path("tmp/functions.tsv"))
        self.assertEqual(args.md_out, MODULE.Path("tmp/index.md"))

    def test_parse_args_defaults_to_no_reports(self) -> None:
        args = MODULE.parse_args(["tmp/raw.json"])

        self.assertIsNone(args.index_out)
        self.assertIsNone(args.symbols_out)
        self.assertIsNone(args.symbols_tsv_out)
        self.assertIsNone(args.md_out)

    def test_infer_source_hint_for_boot_programs(self) -> None:
        self.assertEqual(
            MODULE.infer_source_hint("/boot/SLUS_004.22", "/boot", "SLUS_004.22"),
            "build/extracted/SLUS_004.22",
        )
        self.assertEqual(
            MODULE.infer_source_hint("/boot/LOGO/LOGO.EXE", "/boot/LOGO", "LOGO.EXE"),
            "build/extracted/LOGO/LOGO.EXE",
        )

    def test_infer_source_hint_for_overlay_like_programs(self) -> None:
        result = MODULE.infer_source_hint(
            "/bins/BIN/ETC/GAME/GAME_e01_801d0c00.bin",
            "/bins/BIN/ETC/GAME",
            "GAME_e01_801d0c00.bin",
        )

        self.assertEqual(result, "build/extracted/BIN/ETC/GAME.EMI#1")

    def test_infer_source_hint_for_raw_bin_program_names(self) -> None:
        result = MODULE.infer_source_hint(
            "/bins/BIN/ETC/GAME/1.bin",
            "/bins/BIN/ETC/GAME",
            "1.bin",
        )

        self.assertEqual(result, "build/extracted/BIN/ETC/GAME.EMI#1")

    def test_transform_export_builds_program_and_function_indexes(self) -> None:
        payload = {
            "project_name": "bof3_main",
            "programs": [
                {
                    "program_name": "SLUS_004.22",
                    "program_path": "/boot/SLUS_004.22",
                    "folder": "/boot",
                    "functions": [
                        {
                            "entry": "80162d00",
                            "name": "emi_ready",
                            "signature": "bool emi_ready(void)",
                            "body_min": "80162d00",
                            "body_max": "80162d1f",
                            "comment": "loader ready helper",
                            "repeatable_comment": None,
                            "namespace": "Global",
                            "name_source": "USER_DEFINED",
                            "is_thunk": False,
                        }
                    ],
                }
            ],
        }

        index_payload, rows, tsv_text, programs = MODULE.transform_export(payload)

        self.assertEqual(index_payload["program_count"], 1)
        self.assertEqual(index_payload["function_count"], 1)
        self.assertEqual(
            index_payload["programs"][0]["source_hint"], "build/extracted/SLUS_004.22"
        )
        self.assertEqual(programs[0]["functions"][0]["name"], "emi_ready")
        self.assertEqual(rows[0]["entry_hex"], "0x80162d00")
        self.assertIn("SLUS_004.22", tsv_text)

    def test_transform_export_accepts_flat_function_rows(self) -> None:
        payload = {
            "project_name": "bof3_main",
            "selected_programs": ["/boot/SLUS_004.22"],
            "rows": [
                {
                    "kind": "function",
                    "program_path": "/boot/SLUS_004.22",
                    "address": "80162d00",
                    "name": "emi_ready",
                    "type_spec": "bool emi_ready(void)",
                    "body_min": "80162d00",
                    "body_max": "80162d1f",
                    "comment": "loader ready helper",
                    "repeatable_comment": None,
                    "namespace": "Global",
                    "name_source": "USER_DEFINED",
                    "is_thunk": False,
                }
            ],
        }

        index_payload, rows, tsv_text, programs = MODULE.transform_export(payload)

        self.assertEqual(index_payload["program_count"], 1)
        self.assertEqual(index_payload["function_count"], 1)
        self.assertEqual(index_payload["selected_programs"], ["/boot/SLUS_004.22"])
        self.assertEqual(
            programs[0]["functions"][0]["signature"], "bool emi_ready(void)"
        )
        self.assertEqual(rows[0]["entry_hex"], "0x80162d00")
        self.assertIn("emi_ready", tsv_text)

    def test_transform_export_canonicalizes_shadow_boot_program_rows(self) -> None:
        payload = {
            "project_name": "bof3_main",
            "rows": [
                {
                    "kind": "function",
                    "program_path": "/SLUS_004.22.17",
                    "address": "80162d00",
                    "name": "emi_ready",
                    "type_spec": "bool emi_ready(void)",
                    "body_min": "80162d00",
                    "body_max": "80162d1f",
                    "namespace": "Global",
                    "name_source": "USER_DEFINED",
                    "is_thunk": False,
                },
                {
                    "kind": "function",
                    "program_path": "/boot/SLUS_004.22",
                    "address": "80162d00",
                    "name": "emi_ready",
                    "type_spec": "bool emi_ready(void)",
                    "body_min": "80162d00",
                    "body_max": "80162d1f",
                    "namespace": "Global",
                    "name_source": "USER_DEFINED",
                    "is_thunk": False,
                },
            ],
        }

        index_payload, rows, _tsv_text, programs = MODULE.transform_export(payload)

        self.assertEqual(index_payload["program_count"], 1)
        self.assertEqual(index_payload["function_count"], 1)
        self.assertEqual(programs[0]["program_path"], "/boot/SLUS_004.22")
        self.assertEqual(rows[0]["program_path"], "/boot/SLUS_004.22")

    def test_disambiguates_slug_collisions(self) -> None:
        slugs = MODULE.disambiguate_program_slugs(
            [
                {"program_path": "/boot/A-B", "program_name": "A-B"},
                {"program_path": "/boot/A_B", "program_name": "A_B"},
            ]
        )

        self.assertEqual(slugs["/boot/A-B"], "boot_a_b")
        self.assertEqual(slugs["/boot/A_B"], "boot_a_b_2")


if __name__ == "__main__":
    unittest.main()
