from __future__ import annotations

import unittest

from scripts.rebof3.re.services import resolver


class MetadataTypesTests(unittest.TestCase):
    def test_normalize_type_spec_rewrites_aliases_and_calling_convention(self) -> None:
        result = resolver.normalize_type_spec(
            "void __stdcall game_request_scenario(uchar scenario_index)",
            kind="function",
        )

        self.assertEqual(result.status, "normalized")
        self.assertEqual(
            result.normalized,
            "void game_request_scenario(unsigned char scenario_index)",
        )

    def test_normalize_type_spec_rewrites_prefix_array(self) -> None:
        result = resolver.normalize_type_spec(
            "[14] PTR_BattleRoundAdvanceCaseHandlers",
            kind="data",
        )

        self.assertEqual(result.normalized, "PTR_BattleRoundAdvanceCaseHandlers[14]")

    def test_normalize_type_spec_marks_pseudo_types(self) -> None:
        result = resolver.normalize_type_spec("undefined label", kind="label")

        self.assertTrue(result.is_pseudo_type)
        self.assertEqual(result.status, "pseudo_type")

    def test_contains_unsupported_signature_shape_detects_function_pointer(
        self,
    ) -> None:
        self.assertTrue(
            resolver.contains_unsupported_signature_shape(
                "int callback_wrapper(void (*handler)(int))"
            )
        )

    def test_referenced_type_names_ignores_data_symbol_name(self) -> None:
        self.assertEqual(
            resolver.referenced_type_names(
                "void * switchdataD_801d0c2c[14]",
                kind="data",
            ),
            (),
        )

    def test_referenced_type_names_ignores_function_and_param_names(self) -> None:
        self.assertEqual(
            resolver.referenced_type_names(
                "int battle_dispatch_step(BattleState * state, int phase)",
                kind="function",
            ),
            ("BattleState",),
        )

    def test_referenced_type_names_ignores_undefined_scalar_family(self) -> None:
        self.assertEqual(
            resolver.referenced_type_names("undefined *", kind="data"),
            (),
        )
        self.assertEqual(
            resolver.referenced_type_names(
                "undefined4 game_loop(void)", kind="function"
            ),
            (),
        )

    def test_rewrite_function_pointer_signature_extracts_typedef(self) -> None:
        rewritten, typedefs = resolver.rewrite_function_pointer_signature(
            "int battle_dispatch_step(void (*handler)(int))"
        )

        self.assertEqual(rewritten, "int battle_dispatch_step(HandlerCallback handler)")
        self.assertEqual(
            typedefs,
            (
                {
                    "name": "HandlerCallback",
                    "target_type": "void (*)(int)",
                    "parameter_name": "handler",
                    "original": "void (*handler)(int)",
                },
            ),
        )
