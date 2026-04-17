from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import source_map as MODULE


class SourceMapTests(unittest.TestCase):
    def test_extract_tagged_functions_from_text_skips_non_functions(self) -> None:
        text = """/* @source: 0x80182444 DAT_80182444 */\nstatic const int g_table[] = { 1, 2, 3 };\n\n/* @source: 0x80162d00 FUN_80162d00 */\nbool emi_ready(void)\n{\n    return 1;\n}\n"""

        mappings = MODULE.extract_tagged_functions_from_text(
            text, file_path="bof3/src/core/emi/func_80162d00.c"
        )

        self.assertEqual(len(mappings), 1)
        self.assertEqual(mappings[0]["entry_hex"], "0x80162d00")
        self.assertEqual(mappings[0]["source_function"], "emi_ready")

    def test_extract_tagged_functions_from_text_skips_data_labeled_provenance(
        self,
    ) -> None:
        text = """/* @source: 0x80182444 DAT_80182444 */\nconst int *slot_table_find(void)\n{\n    return 0;\n}\n"""

        mappings = MODULE.extract_tagged_functions_from_text(
            text, file_path="bof3/src/core/disc/slot_table_find.c"
        )

        self.assertEqual(mappings, [])

    def test_extract_tagged_functions_from_text_accepts_address_named_function(
        self,
    ) -> None:
        text = """s32 func_80162d00(void)\n{\n    return 1;\n}\n"""

        mappings = MODULE.extract_tagged_functions_from_text(
            text, file_path="bof3/src/core/emi/func_80162d00.c"
        )

        self.assertEqual(len(mappings), 1)
        self.assertEqual(mappings[0]["entry_hex"], "0x80162d00")
        self.assertEqual(mappings[0]["source_function"], "func_80162d00")

    def test_extract_tagged_functions_from_text_dedupes_provenance_comment_and_address_name(
        self,
    ) -> None:
        text = """/* @source: 0x80162d00 FUN_80162d00 */\ns32 func_80162d00(void)\n{\n    return 1;\n}\n"""

        mappings = MODULE.extract_tagged_functions_from_text(
            text, file_path="bof3/src/core/emi/func_80162d00.c"
        )

        self.assertEqual(len(mappings), 1)
        self.assertEqual(mappings[0]["entry_hex"], "0x80162d00")
        self.assertEqual(mappings[0]["source_function"], "func_80162d00")

    def test_extract_tagged_functions_from_text_skips_in_body_address_named_calls(
        self,
    ) -> None:
        text = """void func_801d0c90(void)\n{\n    while (!func_80162d00()) {\n        func_8014b87c(1u);\n    }\n}\n\nbool func_80162d00(void)\n{\n    return 1;\n}\n"""

        mappings = MODULE.extract_tagged_functions_from_text(
            text, file_path="bof3/src/modules/game/front.c"
        )

        self.assertEqual(
            [mapping["source_function"] for mapping in mappings],
            ["func_801d0c90", "func_80162d00"],
        )

    def test_predict_object_candidates_returns_obj_and_o(self) -> None:
        candidates = MODULE.predict_object_candidates(
            "bof3/src/core/emi/func_80162d00.c"
        )

        self.assertGreaterEqual(len(candidates), 2)
        self.assertTrue(
            any(
                candidate.endswith("func_80162d00.c.obj")
                for candidate in candidates
            )
        )
        self.assertTrue(
            any(candidate.endswith("func_80162d00.c.o") for candidate in candidates)
        )

    def test_predict_object_candidates_discovers_profiled_build_tree(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            object_path = (
                root
                / "build"
                / "bof3-psyq40"
                / "bof3"
                / "CMakeFiles"
                / "bof3.dir"
                / "src"
                / "func_80162d00.c.obj"
            )
            object_path.parent.mkdir(parents=True, exist_ok=True)
            object_path.write_bytes(b"obj")

            candidates = MODULE.predict_object_candidates(
                "bof3/src/core/emi/func_80162d00.c", build_root=root / "build"
            )

            self.assertEqual(candidates[0], str(object_path.resolve()))
            self.assertTrue(any("bof3-psyq40" in candidate for candidate in candidates))

    def test_predict_object_candidates_accepts_legacy_psxbof3_object_dir(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            object_path = (
                root
                / "build"
                / "bof3-psyq40"
                / "bof3"
                / "CMakeFiles"
                / "psxbof3.dir"
                / "src"
                / "func_80162d00.c.obj"
            )
            object_path.parent.mkdir(parents=True, exist_ok=True)
            object_path.write_bytes(b"obj")

            candidates = MODULE.predict_object_candidates(
                "bof3/src/core/emi/func_80162d00.c", build_root=root / "build"
            )

            self.assertEqual(candidates[0], str(object_path.resolve()))

    def test_find_source_mapping_uses_temp_source_tree(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source_file = root / "loader.c"
            source_file.parent.mkdir(parents=True, exist_ok=True)
            source_file.write_text(
                "bool func_80162d00(void)\n"
                "{\n"
                "    return 1;\n"
                "}\n",
                encoding="utf-8",
            )

            mapping = MODULE.find_source_mapping("0x80162d00", root)

            assert mapping is not None
            self.assertEqual(mapping["source_function"], "func_80162d00")
            self.assertIn("object_candidates", mapping)

    def test_find_source_mapping_prefers_program_named_source_on_duplicates(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            slot_table = root / "bof3" / "src" / "core" / "disc" / "slot_table_logo_str.c"
            logo_dir = root / "bof3" / "src" / "modules" / "logo"
            slot_table.parent.mkdir(parents=True, exist_ok=True)
            logo_dir.mkdir(parents=True, exist_ok=True)
            logo_file = logo_dir / "func_801cedfc.c"
            slot_table.write_text(
                "/* @source: 0x801cedfc FUN_801cedfc */\nconst int *slot_table_logo_str(void)\n{\n    return 0;\n}\n",
                encoding="utf-8",
            )
            logo_file.write_text(
                "void func_801cedfc(void)\n{\n}\n",
                encoding="utf-8",
            )

            mapping = MODULE.find_source_mapping(
                "0x801cedfc",
                root,
                program_path="/boot/LOGO/LOGO.EXE",
                program_name="LOGO.EXE",
                source_hint="build/extracted/LOGO/LOGO.EXE",
            )

            assert mapping is not None
            self.assertEqual(mapping["source_file"], MODULE.relative_to_root(logo_file))
            self.assertEqual(mapping["source_function"], "func_801cedfc")


if __name__ == "__main__":
    unittest.main()
