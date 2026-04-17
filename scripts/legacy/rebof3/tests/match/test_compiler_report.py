from __future__ import annotations

import argparse
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.match import compiler_report as MODULE


class CompilerReportTests(unittest.TestCase):
    def test_requested_compiler_ids_defaults_to_canonical(self) -> None:
        result = MODULE.requested_compiler_ids(None, None)

        self.assertEqual(result, ("gcc-2.7.2-psx",))

    def test_requested_compiler_ids_expands_tested_matrix(self) -> None:
        result = MODULE.requested_compiler_ids(None, ["tested-matrix"])

        self.assertEqual(result[0], "gcc-2.7.2-psx")
        self.assertIn("gcc-2.7.0-mipsel", result)
        self.assertIn("gcc-2.95.2-psx", result)

    def test_resolve_compilers_uses_optional_root(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            canonical = root / "canonical" / "gcc"
            optional = root / "old_gcc" / "gcc-2.7.0-mipsel" / "gcc"
            canonical.parent.mkdir(parents=True, exist_ok=True)
            optional.parent.mkdir(parents=True, exist_ok=True)
            canonical.write_text("", encoding="utf-8")
            optional.write_text("", encoding="utf-8")

            with patch.object(MODULE, "GCC272_PSX_GCC", canonical):
                result = MODULE.resolve_compilers(
                    ("gcc-2.7.2-psx", "gcc-2.7.0-mipsel"),
                    old_gcc_root=root / "old_gcc",
                )

        self.assertEqual(
            tuple(spec.compiler_id for spec in result),
            ("gcc-2.7.2-psx", "gcc-2.7.0-mipsel"),
        )

    def test_render_function_rows_tsv_includes_match_percent(self) -> None:
        text = MODULE.render_function_rows_tsv(
            [
                {
                    "compiler_id": "gcc-2.7.2-psx",
                    "source_file": "bof3/src/modules/battle/03/func_801df8ac.c",
                    "source_function": "func_801df8ac",
                    "status": "ok",
                    "entry_hex": "0x801df8ac",
                    "program_path": "/bins/BIN/BATTLE/BATTLE/3.bin",
                    "objdiff_backend_report": "tmp/compiler_reports/foo/backend.json",
                    "match_metrics": {
                        "objdiff_match_percent": 68.0,
                        "objdiff_mismatch_count": 16,
                    },
                }
            ]
        )

        self.assertIn("objdiff_match_percent", text.splitlines()[0])
        self.assertIn("68.0", text)

    def test_render_function_rows_tsv_preserves_zero_values(self) -> None:
        text = MODULE.render_function_rows_tsv(
            [
                {
                    "compiler_id": "gcc-2.7.2-psx",
                    "source_file": "bof3/src/modules/example.c",
                    "source_function": "func_80000000",
                    "status": "ok",
                    "entry_hex": "0x80000000",
                    "program_path": "/bins/example.bin",
                    "objdiff_backend_report": "tmp/compiler_reports/foo/backend.json",
                    "match_metrics": {
                        "objdiff_match_percent": 0.0,
                        "objdiff_mismatch_count": 0,
                    },
                }
            ]
        )

        self.assertIn("\tok\t0.0\t0\t", text)

    def test_sort_function_rows_orders_by_match_descending(self) -> None:
        rows = MODULE.sort_function_rows(
            [
                {
                    "compiler_id": "gcc-2.7.2-psx",
                    "source_file": "bof3/src/modules/example/func_80000020.c",
                    "source_function": "func_80000020",
                    "entry_hex": "0x80000020",
                    "match_metrics": {
                        "objdiff_match_percent": 50.0,
                        "objdiff_mismatch_count": 8,
                    },
                },
                {
                    "compiler_id": "gcc-2.7.2-psx",
                    "source_file": "bof3/src/modules/example/func_80000010.c",
                    "source_function": "func_80000010",
                    "entry_hex": "0x80000010",
                    "match_metrics": {
                        "objdiff_match_percent": 75.0,
                        "objdiff_mismatch_count": 4,
                    },
                },
                {
                    "compiler_id": "gcc-2.7.2-psx",
                    "source_file": "bof3/src/modules/example/func_80000030.c",
                    "source_function": "func_80000030",
                    "entry_hex": "0x80000030",
                    "status": "compile_failed",
                    "match_metrics": {},
                },
            ]
        )

        self.assertEqual(
            [row["source_function"] for row in rows],
            ["func_80000010", "func_80000020", "func_80000030"],
        )

    def test_sort_summary_rows_orders_by_best_match_descending(self) -> None:
        rows = MODULE.sort_summary_rows(
            [
                {
                    "compiler_id": "gcc-2.7.2-psx",
                    "successful_functions": 2,
                    "highest_objdiff_match_percent": 65.0,
                    "average_objdiff_match_percent": 40.0,
                },
                {
                    "compiler_id": "gcc-2.95.2-psx",
                    "successful_functions": 2,
                    "highest_objdiff_match_percent": 90.0,
                    "average_objdiff_match_percent": 55.0,
                },
                {
                    "compiler_id": "gcc-2.7.0-mipsel",
                    "successful_functions": 0,
                    "highest_objdiff_match_percent": None,
                    "average_objdiff_match_percent": None,
                },
            ]
        )

        self.assertEqual(
            [row["compiler_id"] for row in rows],
            ["gcc-2.95.2-psx", "gcc-2.7.2-psx", "gcc-2.7.0-mipsel"],
        )

    def test_filter_targets_supports_exact_function_name(self) -> None:
        targets = [
            {
                "source_file": "bof3/src/modules/commu00/00/func_801f0ec8.c",
                "source_function": "func_801f0ec8",
            },
            {
                "source_file": "bof3/src/modules/commu00/00/func_801f1204.c",
                "source_function": "func_801f1204",
            },
        ]

        filtered = MODULE.filter_targets(
            targets,
            source_prefixes=(),
            source_files=(),
            source_functions=("func_801f1204",),
        )

        self.assertEqual(
            [item["source_function"] for item in filtered], ["func_801f1204"]
        )

    def test_resolve_source_filters_uses_default_prefix_only_without_explicit_filters(
        self,
    ) -> None:
        args = argparse.Namespace(
            source_prefix=None,
            source_file=["bof3/src/modules/commu00/00/func_801f1204.c"],
            source_function=None,
        )

        source_prefixes, source_files, source_functions = MODULE.resolve_source_filters(
            args
        )

        self.assertEqual(source_prefixes, ())
        self.assertEqual(source_files, ("bof3/src/modules/commu00/00/func_801f1204.c",))
        self.assertEqual(source_functions, ())

    def test_parse_args_accepts_output_mode_and_stdout_flags(self) -> None:
        args = MODULE.parse_args(
            [
                "--output-mode",
                "stdout",
                "--stdout-view",
                "functions",
                "--stdout-format",
                "brief",
            ]
        )

        self.assertEqual(args.output_mode, "stdout")
        self.assertEqual(args.stdout_view, "functions")
        self.assertEqual(args.stdout_format, "brief")

    def test_render_function_rows_brief_is_human_readable(self) -> None:
        text = MODULE.render_function_rows_brief(
            [
                {
                    "source_file": "bof3/src/modules/commu00/00/func_801f1204.c",
                    "source_function": "func_801f1204",
                    "status": "ok",
                    "entry_hex": "0x801f1204",
                    "match_metrics": {
                        "objdiff_match_percent": 99.0,
                        "objdiff_mismatch_count": 2,
                    },
                }
            ]
        )

        self.assertIn("func_801f1204: ok", text)
        self.assertIn("match 99.000", text)
        self.assertIn("mismatches 2", text)

    def test_render_stdout_payload_supports_brief_both_view(self) -> None:
        text = MODULE.render_stdout_payload(
            summary_rows=[
                {
                    "compiler_id": "gcc-2.7.2-psx",
                    "successful_functions": 1,
                    "failed_functions": 0,
                    "highest_objdiff_match_percent": 99.0,
                    "average_objdiff_match_percent": 99.0,
                    "median_objdiff_match_percent": 99.0,
                    "lowest_objdiff_match_percent": 99.0,
                }
            ],
            function_rows=[
                {
                    "source_file": "bof3/src/modules/commu00/00/func_801f1204.c",
                    "source_function": "func_801f1204",
                    "status": "ok",
                    "entry_hex": "0x801f1204",
                    "match_metrics": {
                        "objdiff_match_percent": 99.0,
                        "objdiff_mismatch_count": 2,
                    },
                }
            ],
            view="both",
            output_format="brief",
        )

        self.assertIn("Summary", text)
        self.assertIn("Functions", text)
        self.assertIn("gcc-2.7.2-psx: ok 1", text)
        self.assertIn("func_801f1204: ok", text)

    def test_render_stdout_payload_supports_function_table(self) -> None:
        text = MODULE.render_stdout_payload(
            summary_rows=[],
            function_rows=[
                {
                    "compiler_id": "gcc-2.7.2-psx",
                    "source_file": "bof3/src/modules/commu00/00/func_801f1204.c",
                    "source_function": "func_801f1204",
                    "status": "ok",
                    "match_metrics": {
                        "objdiff_match_percent": 99.0,
                        "objdiff_mismatch_count": 2,
                    },
                }
            ],
            view="functions",
            output_format="table",
        )

        self.assertIn("source_function", text)
        self.assertIn("func_801f1204", text)


if __name__ == "__main__":
    unittest.main()
