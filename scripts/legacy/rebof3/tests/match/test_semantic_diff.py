from __future__ import annotations

import unittest

from scripts.rebof3.match import semantic_diff as MODULE


def _symbol(
    name: str, match_percent: float, instructions: list[dict[str, object]]
) -> dict[str, object]:
    return {
        "name": name,
        "kind": "SYMBOL_FUNCTION",
        "match_percent": match_percent,
        "instructions": instructions,
    }


def _instruction(text: str, diff_kind: str | None = None) -> dict[str, object]:
    entry: dict[str, object] = {"instruction": {"formatted": text}}
    if diff_kind is not None:
        entry["diff_kind"] = diff_kind
    return entry


class SemanticDiffTests(unittest.TestCase):
    def test_classify_objdiff_payload_reports_asm_view_only_noise(self) -> None:
        summary = MODULE.classify_objdiff_payload(
            {
                "left": {
                    "symbols": [
                        _symbol(
                            "func_1",
                            100.0,
                            [_instruction("jr ra"), _instruction("nop ")],
                        )
                    ]
                },
                "right": {
                    "symbols": [
                        _symbol(
                            "func_1",
                            100.0,
                            [_instruction("jr ra"), _instruction("nop ")],
                        )
                    ]
                },
            },
            symbol_name="func_1",
            asm_score=300,
        )

        self.assertEqual(summary["semantic_status"], "asm_view_only_noise")
        self.assertTrue(summary["asm_view_only_noise"])

    def test_classify_objdiff_payload_reports_sugar_only(self) -> None:
        summary = MODULE.classify_objdiff_payload(
            {
                "left": {
                    "symbols": [
                        _symbol(
                            "func_1",
                            95.0,
                            [
                                _instruction("or a1, s0, zero", "DIFF_REPLACE"),
                                _instruction("or a0, zero, zero", "DIFF_REPLACE"),
                            ],
                        )
                    ]
                },
                "right": {
                    "symbols": [
                        _symbol(
                            "func_1",
                            95.0,
                            [
                                _instruction("addu a1, s0, zero", "DIFF_REPLACE"),
                                _instruction("addu a0, zero, zero", "DIFF_REPLACE"),
                            ],
                        )
                    ]
                },
            },
            symbol_name="func_1",
            asm_score=10,
        )

        self.assertEqual(summary["semantic_status"], "sugar_only")
        self.assertEqual(summary["category_counts"]["move_zero_sugar"], 2)
        self.assertEqual(summary["unclassified_mismatch_count"], 0)

    def test_classify_objdiff_payload_reports_relocation_only(self) -> None:
        summary = MODULE.classify_objdiff_payload(
            {
                "left": {
                    "symbols": [
                        _symbol(
                            "func_1",
                            99.0,
                            [
                                _instruction("lui at, 0x8014", "DIFF_ARG_MISMATCH"),
                                _instruction("sw a1, 0x3b44(at)", "DIFF_ARG_MISMATCH"),
                                _instruction("jal func_00175640", "DIFF_ARG_MISMATCH"),
                            ],
                        )
                    ]
                },
                "right": {
                    "symbols": [
                        _symbol(
                            "func_1",
                            99.0,
                            [
                                _instruction("lui at, 0x0", "DIFF_ARG_MISMATCH"),
                                _instruction("sw a1, 0x0(at)", "DIFF_ARG_MISMATCH"),
                                _instruction("jal func_00000000", "DIFF_ARG_MISMATCH"),
                            ],
                        )
                    ]
                },
            },
            symbol_name="func_1",
            asm_score=20,
        )

        self.assertEqual(summary["semantic_status"], "relocation_only")
        self.assertEqual(summary["category_counts"]["address_materialization"], 2)
        self.assertEqual(summary["category_counts"]["call_target_reloc"], 1)

    def test_classify_objdiff_payload_reports_structural_when_unclassified(
        self,
    ) -> None:
        summary = MODULE.classify_objdiff_payload(
            {
                "left": {
                    "symbols": [
                        _symbol(
                            "func_1",
                            60.0,
                            [
                                _instruction("sw v0, 0x8(v1)", "DIFF_REPLACE"),
                                _instruction("bnez v0, 0x20", "DIFF_REPLACE"),
                            ],
                        )
                    ]
                },
                "right": {
                    "symbols": [
                        _symbol(
                            "func_1",
                            60.0,
                            [
                                _instruction("nop ", "DIFF_REPLACE"),
                                _instruction("beqz v1, 0x30", "DIFF_REPLACE"),
                            ],
                        )
                    ]
                },
            },
            symbol_name="func_1",
            asm_score=400,
        )

        self.assertEqual(summary["semantic_status"], "structural")
        self.assertEqual(summary["unclassified_mismatch_count"], 2)


if __name__ == "__main__":
    unittest.main()
